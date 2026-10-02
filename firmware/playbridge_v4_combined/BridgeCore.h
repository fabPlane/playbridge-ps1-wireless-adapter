#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <algorithm>

// Platform-independent transport used unchanged by firmware and native tests.
// One task owns this object and its IO. No locks/network calls in pad engine.
namespace GameBridge {
struct IO {
  virtual ~IO() = default;
  virtual int tcpAvailable() = 0;
  virtual bool connected() = 0;
  virtual int tcpRead(uint8_t *, size_t) = 0;
  // positive=progress, 0=retry (including EINTR), negative=fatal
  virtual int tcpSend(const uint8_t *, size_t) = 0;
  virtual void closeClient() = 0;
  virtual int uartAvailable() = 0;
  virtual int uartRead(uint8_t *, size_t) = 0;
  virtual size_t uartWritable() = 0;
  virtual int uartWrite(const uint8_t *, size_t) = 0;
  virtual bool txIdle() = 0; // zero-tick query, never an unbounded flush
  virtual bool setBaud(uint32_t) = 0;
  // Delete/reinstall UART driver: discard RX/TX software rings and reset FIFOs.
  // Cannot retract bytes that have already physically left TX.
  virtual bool resetUart(uint32_t) = 0;
  virtual void ack(const uint8_t *, uint32_t ip, uint16_t port) = 0;
};
struct Status {
  bool active = false, fault = false;
  uint32_t baud = 115200, sessions = 0, aborts = 0;
  uint32_t tcpReceived = 0, uartWritten = 0, uartReceived = 0, tcpWritten = 0;
  uint32_t discarded = 0, socketErrors = 0, uartErrors = 0, shortWrites = 0;
  uint32_t wouldBlock = 0, rejected = 0, malformed = 0, duplicates = 0;
  uint32_t controlAcks = 0, drainTimeouts = 0, resetFailures = 0;
  uint32_t toTcpPeak = 0, toUartPeak = 0;
};
template<size_t N> struct Queue {
  uint8_t bytes[N]{};
  size_t start = 0, end = 0;
  size_t size() const { return end - start; }
  void clear() { start = end = 0; }
  void compact() {
    if (!size()) clear();
    else if (end == N && start) { memmove(bytes, bytes+start, size()); end-=start; start=0; }
  }
};
inline uint32_t le32(const uint8_t *p) {
  return uint32_t(p[0]) | (uint32_t(p[1])<<8) | (uint32_t(p[2])<<16) | (uint32_t(p[3])<<24);
}
inline uint32_t crc32(const uint8_t *p, size_t n) {
  uint32_t c=0xFFFFFFFFu;
  while(n--) { c^=*p++; for(unsigned i=0;i<8;++i)c=(c>>1)^(0xEDB88320u & (0u-(c&1u))); }
  return ~c;
}
inline bool supported(uint32_t b) {
  return b==115200||b==230400||b==518400||b==691200||b==1036800||b==2073600;
}
class Core {
 public:
  IO &io;
  Status stats;
  Queue<16384> toTcp;
  Queue<1024> toUart;
  explicit Core(IO &adapter):io(adapter){}
  bool controlPending() const { return changing; }
  bool open(uint32_t ip, uint32_t now) {
    if(stats.active||stats.fault||changing){++stats.rejected;return false;}
    // Quarantine all previous RX/TX before giving a new client ownership.
    if(!reset()){return false;}
    owner=ip;stats.active=true;++stats.sessions;
    txProgress=rxProgress=now;
    return true;
  }
  void abort() {
    ++stats.aborts;
    stats.discarded+=toTcp.size()+toUart.size();
    toTcp.clear();toUart.clear();io.closeClient();stats.active=false;changing=false;
    reset();
  }
  void uartError() { ++stats.uartErrors; abort(); }
  void control(const uint8_t *p,size_t length,uint32_t ip,uint16_t port,uint32_t now) {
    if(length!=20||memcmp(p,"PSB1",4)||le32(p+16)!=crc32(p,16)||
       !supported(le32(p+8))||le32(p+12)>1){++stats.malformed;return;}
    if(stats.fault||(stats.active&&ip!=owner))return;
    for(const auto &c:cache) {
      if(c.valid&&uint32_t(now-c.time)<10000&&c.ip==ip&&c.port==port&&!memcmp(c.packet,p,20)){
        ++stats.duplicates;
        // Old controls must never roll back a later baud, nor repeat a reset.
        if(!changing && stats.baud==le32(p+8)){io.ack(p,ip,port);++stats.controlAcks;}
        return;
      }
    }
    if(changing)return; // Retry is serviced after the bounded handoff.
    if(le32(p+12)==1)abort();
    if(stats.fault)return;
    // Both forwarding directions and new input must be quiescent.
    if(toUart.size()||toTcp.size()||io.tcpAvailable()>0||io.uartAvailable()>0)return;
    memcpy(request,p,20);requestIp=ip;requestPort=port;changeStart=now;changing=true;
    finishControl(now);
  }
  void poll(uint32_t now) {
    if(stats.fault)return;
    if(changing){
      // New data cancels the handoff, rather than changing baud mid-stream.
      if(io.tcpAvailable()>0||io.uartAvailable()>0)changing=false;
      else {finishControl(now);if(changing)return;}
    }
    if(!stats.active){
      uint8_t trash[512];
      const int n=io.uartRead(trash,sizeof(trash));
      if(n>0)stats.discarded+=n;
      return;
    }
    if(!io.connected()&&io.tcpAvailable()==0&&!toUart.size()){
      // TCP disconnect aborts session, including queued hardware TX.
      abort();return;
    }
    toUart.compact();
    if(!toUart.size()){
      const size_t n=std::min<size_t>(std::max(0,io.tcpAvailable()),sizeof(toUart.bytes));
      if(n){
        const int got=io.tcpRead(toUart.bytes,n);
        if(got<0){++stats.socketErrors;abort();return;}
        if(got>0){toUart.end=got;stats.tcpReceived+=got;txProgress=now;}
      }
    }
    stats.toUartPeak=std::max<uint32_t>(stats.toUartPeak,toUart.size());
    if(toUart.size()){
      const size_t n=std::min(toUart.size(),io.uartWritable());
      if(n){
        const int wrote=io.uartWrite(toUart.bytes+toUart.start,n);
        if(wrote<0||size_t(wrote)>n){uartError();return;}
        if(size_t(wrote)<n)++stats.shortWrites;
        toUart.start+=wrote;stats.uartWritten+=wrote;
        if(wrote)txProgress=now;
      }
    }
    toTcp.compact();
    size_t room=std::min<size_t>(1024,sizeof(toTcp.bytes)-toTcp.end);
    if(room){
      const bool empty=!toTcp.size();
      const int got=io.uartRead(toTcp.bytes+toTcp.end,room);
      if(got<0){uartError();return;}
      if(got>0){toTcp.end+=got;stats.uartReceived+=got;if(empty)rxProgress=now;}
    }
    stats.toTcpPeak=std::max<uint32_t>(stats.toTcpPeak,toTcp.size());
    if(toTcp.size()){
      const size_t n=std::min<size_t>(1460,toTcp.size());
      const int sent=io.tcpSend(toTcp.bytes+toTcp.start,n);
      if(sent<0||size_t(sent)>n){++stats.socketErrors;abort();return;}
      if(!sent)++stats.wouldBlock;
      else{toTcp.start+=sent;stats.tcpWritten+=sent;rxProgress=now;}
    }
    // Each direction has its own progress timer; opposite traffic can't hide a stall.
    if((toUart.size()&&uint32_t(now-txProgress)>=10000)||
       (toTcp.size()&&uint32_t(now-rxProgress)>=10000)){++stats.socketErrors;abort();}
  }
 private:
  struct Cached {bool valid=false;uint32_t ip=0,time=0;uint16_t port=0;uint8_t packet[20]{};};
  Cached cache[8]{};
  unsigned cacheNext=0;
  uint32_t owner=0,txProgress=0,rxProgress=0;
  bool changing=false;
  uint8_t request[20]{};
  uint32_t requestIp=0,changeStart=0;
  uint16_t requestPort=0;
  bool reset(){
    if(io.resetUart(stats.baud))return true;
    ++stats.resetFailures;stats.fault=true;stats.active=false;return false;
  }
  void finishControl(uint32_t now){
    if(!io.txIdle()){
      if(uint32_t(now-changeStart)>=150){++stats.drainTimeouts;changing=false;}
      return;
    }
    const uint32_t baud=le32(request+8);
    if(baud!=stats.baud&&!io.setBaud(baud)){
      stats.fault=true;stats.active=false;changing=false;io.closeClient();return;
    }
    stats.baud=baud;changing=false;
    auto &c=cache[cacheNext++%8];c.valid=true;c.ip=requestIp;c.port=requestPort;c.time=now;
    memcpy(c.packet,request,20);
    // Cache the side effect BEFORE UDP send: a lost ACK must not repeat abort.
    io.ack(request,requestIp,requestPort);++stats.controlAcks;
  }
};
}
