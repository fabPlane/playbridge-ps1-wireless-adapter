#include "../BridgeCore.h"
#include <cassert>
#include <deque>
#include <vector>
#include <array>
#include <cstdio>
using namespace GameBridge;
struct Fake:IO {
  std::deque<uint8_t> tcpIn,uartIn;
  std::vector<uint8_t> wire,network,hardwareTx;
  std::vector<std::array<uint8_t,20>> acks;
  bool online=true,idle=true,resetOK=true,baudOK=true,sendFatal=false,loseAck=false;
  size_t writeLimit=1024,sendLimit=1460,writable=4096;
  uint32_t resets=0,baud=115200;
  static int read(std::deque<uint8_t>&q,uint8_t*p,size_t n){
    n=std::min(n,q.size());for(size_t i=0;i<n;++i){p[i]=q.front();q.pop_front();}return n;
  }
  int tcpAvailable()override{return tcpIn.size();}
  bool connected()override{return online;}
  int tcpRead(uint8_t*p,size_t n)override{return read(tcpIn,p,n);}
  int tcpSend(const uint8_t*p,size_t n)override{
    if(sendFatal)return -1;n=std::min(n,sendLimit);network.insert(network.end(),p,p+n);return n;
  }
  void closeClient()override{online=false;tcpIn.clear();}
  int uartAvailable()override{return uartIn.size();}
  int uartRead(uint8_t*p,size_t n)override{return read(uartIn,p,n);}
  size_t uartWritable()override{return writable;}
  int uartWrite(const uint8_t*p,size_t n)override{
    n=std::min(n,writeLimit);wire.insert(wire.end(),p,p+n);hardwareTx.insert(hardwareTx.end(),p,p+n);return n;
  }
  bool txIdle()override{return idle;}
  bool setBaud(uint32_t b)override{if(baudOK)baud=b;return baudOK;}
  bool resetUart(uint32_t b)override{++resets;baud=b;uartIn.clear();hardwareTx.clear();return resetOK;}
  void ack(const uint8_t*p,uint32_t,uint16_t)override{
    if(loseAck)return;std::array<uint8_t,20>a;memcpy(a.data(),p,20);acks.push_back(a);
  }
};
static void put32(uint8_t*p,uint32_t v){for(unsigned i=0;i<4;++i)p[i]=v>>(8*i);}
static std::array<uint8_t,20> packet(uint32_t baud=2073600,uint32_t flags=0,uint32_t nonce=1){
  std::array<uint8_t,20>p{};memcpy(p.data(),"PSB1",4);put32(p.data()+4,nonce);
  put32(p.data()+8,baud);put32(p.data()+12,flags);put32(p.data()+16,crc32(p.data(),16));return p;
}
static void control(Core&c,const std::array<uint8_t,20>&p,uint32_t now=1,uint32_t ip=7){
  c.control(p.data(),p.size(),ip,99,now);
}
static void open(Core&c,Fake&f,uint32_t now=0){f.online=true;assert(c.open(7,now));}
int main(){
  assert(crc32(reinterpret_cast<const uint8_t*>("123456789"),9)==0xCBF43926);
  {Fake f;Core c(f);open(c,f);f.writeLimit=3;f.sendLimit=5;
   std::vector<uint8_t>expected;
   for(unsigned i=0;i<40000;++i){expected.push_back(i%256);f.tcpIn.push_back(i%256);f.uartIn.push_back(i%256);}
   for(uint32_t t=0;t<20000&&(c.stats.uartWritten<expected.size()||c.stats.tcpWritten<expected.size());++t)c.poll(t);
   assert(f.wire==expected&&f.network==expected&&c.stats.shortWrites>0&&c.stats.aborts==0);
  }
  {Fake f;Core c(f);open(c,f);f.sendLimit=0;
   for(unsigned i=0;i<18000;++i)f.uartIn.push_back(i%256);
   for(unsigned t=0;t<30;++t)c.poll(t);
   assert(c.toTcp.size()==16384&&f.uartIn.size()==1616);
   f.sendLimit=1460;for(unsigned t=30;t<60;++t)c.poll(t);
   assert(f.network.size()==18000);for(unsigned i=0;i<18000;++i)assert(f.network[i]==i%256);
  }
  {Fake f;Core c(f);open(c,f);f.sendLimit=0;f.uartIn.push_back(42);c.poll(0);
   for(unsigned t=1;t<=10000;t+=100){f.tcpIn.push_back(1);c.poll(t);}
   c.poll(10000);assert(c.stats.aborts==1&&!c.stats.active&&f.hardwareTx.empty());
  }
  {Fake f;Core c(f);open(c,f);f.writable=0;f.tcpIn.push_back(42);c.poll(0);c.poll(10000);
   assert(c.stats.aborts==1&&c.toUart.size()==0);
  }
  {Fake f;Core c(f);open(c,f);assert(!c.open(8,0));assert(c.stats.rejected==1);
   f.tcpIn={1,2,3};c.poll(0);assert(!f.hardwareTx.empty());
   f.uartIn.push_back(88);c.abort();assert(f.hardwareTx.empty()&&f.uartIn.empty());
   open(c,f);c.poll(1);assert(f.network.empty());
  }
  {Fake f;Core c(f);open(c,f);f.tcpIn={1,2};f.writeLimit=1;f.online=false;c.poll(0);c.poll(1);
   assert(c.stats.uartWritten==2);c.poll(2);assert(!c.stats.active&&f.hardwareTx.empty());
  }
  {Fake f;Core c(f);open(c,f);f.sendFatal=true;f.uartIn={1};c.poll(0);assert(c.stats.socketErrors==1&&!c.stats.active);}
  {Fake f;Core c(f);open(c,f);c.uartError();assert(c.stats.uartErrors==1&&!c.stats.active);}
  {Fake f;Core c(f);f.resetOK=false;assert(!c.open(7,0)&&c.stats.fault);}
  {Fake f;Core c(f);auto p=packet();
   c.control(p.data(),19,7,99,0);p[0]='X';control(c,p);p=packet(9600);control(c,p);
   p=packet(115200,2);control(c,p);p=packet();p[19]^=1;control(c,p);
   assert(c.stats.malformed==5&&f.acks.empty()&&c.stats.baud==115200);
  }
  for(uint32_t b:{115200u,230400u,518400u,691200u,1036800u,2073600u}){
   Fake f;Core c(f);auto p=packet(b);control(c,p);assert(c.stats.baud==b&&f.acks.size()==1&&f.acks[0]==p);
  }
  {Fake f;Core c(f);open(c,f);control(c,packet(),0,8);assert(f.acks.empty());
   f.idle=false;control(c,packet(),1);assert(c.controlPending());c.poll(150);assert(f.acks.empty());
   c.poll(151);assert(!c.controlPending()&&c.stats.drainTimeouts==1&&c.stats.baud==115200);
   f.idle=true;control(c,packet(),152);assert(c.stats.baud==2073600);
  }
  {Fake f;Core c(f);open(c,f);f.idle=false;control(c,packet(),UINT32_MAX-50);
   c.poll(99);assert(c.stats.drainTimeouts==1);
  }
  {Fake f;Core c(f);open(c,f);f.idle=false;control(c,packet(),1);f.tcpIn={55};c.poll(2);
   assert(!c.controlPending()&&c.stats.baud==115200&&f.acks.empty()&&f.wire[0]==55);
  }
  {Fake f;Core c(f);open(c,f);f.sendLimit=0;f.uartIn={1};c.poll(0);control(c,packet());
   assert(f.acks.empty()&&!c.controlPending()); // Don't ignore UART->TCP pending data.
  }
  {Fake f;Core c(f);open(c,f);auto p=packet(115200,1);f.loseAck=true;control(c,p);
   assert(c.stats.aborts==1);open(c,f,2);f.loseAck=false;control(c,p,3);
   assert(c.stats.active&&c.stats.aborts==1&&f.acks.size()==1&&c.stats.duplicates==1);
   auto next=packet(230400,0,2);control(c,next,4);control(c,p,5);
   assert(c.stats.active&&c.stats.baud==230400&&f.acks.size()==2); // No stale success echo.
  }
  {Fake f;Core c(f);f.baudOK=false;control(c,packet());assert(c.stats.fault&&f.acks.empty());}
  std::puts("PASS: actual transport core: partial writes, binary data, backpressure, independent stalls, ownership, cleanup, UART/socket faults, PSB1 validation/CRC, every baud, bounded drain, rollover, duplicate/lost-ACK reset, stale control.");
}
