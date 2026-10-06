#include "Ps1GameBridge.h"
#include "BoardProfile.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <driver/uart.h>
#include <lwip/sockets.h>
#include <errno.h>

namespace Ps1GameBridge {
namespace {
constexpr uart_port_t PORT=UART_NUM_2;
class Adapter : public GameBridge::IO {
 public:
  WiFiServer server{3333};
  WiFiClient client;
  WiFiUDP control;
  QueueHandle_t events=nullptr;
  bool installed=false;
  size_t emptyTxCapacity=0;
  int tcpAvailable() override{return client.available();}
  bool connected() override{return client.connected();}
  int tcpRead(uint8_t *p,size_t n) override{return client.read(p,n);}
  int tcpSend(const uint8_t *p,size_t n) override{
    const int sent=lwip_send(client.fd(),p,n,MSG_DONTWAIT);
    if(sent<0&&(errno==EAGAIN||errno==EWOULDBLOCK||errno==EINTR))return 0;
    return sent==0?-1:sent;
  }
  void closeClient() override{client.stop();}
  int uartAvailable() override{
    size_t n=0;return installed&&uart_get_buffered_data_len(PORT,&n)==ESP_OK?int(n):0;
  }
  int uartRead(uint8_t *p,size_t n) override{return installed?uart_read_bytes(PORT,p,n,0):-1;}
  size_t uartWritable() override{
    size_t n=0;
    if(!installed||uart_get_tx_buffer_free_size(PORT,&n)!=ESP_OK)return 0;
    // Leave ring-buffer item overhead/headroom; only this task can enqueue.
    return n>128?std::min<size_t>(n-128,1024):0;
  }
  int uartWrite(const uint8_t *p,size_t n) override{return uart_write_bytes(PORT,p,n);}
  bool txIdle() override{
    size_t free=0;
    // TX FSM alone can be idle between buffered chunks. Require the driver
    // ring to be fully empty as well, then check the physical transmitter.
    return installed&&uart_get_tx_buffer_free_size(PORT,&free)==ESP_OK&&
           free==emptyTxCapacity&&uart_wait_tx_done(PORT,0)==ESP_OK;
  }
  bool setBaud(uint32_t baud) override{
    if(uart_set_baudrate(PORT,baud)!=ESP_OK)return false;
    uint32_t actual=0;
    return uart_get_baudrate(PORT,&actual)==ESP_OK &&
           actual>baud*99/100 && actual<baud*101/100;
  }
  bool resetUart(uint32_t baud) override{
    // Driver deletion resets FIFO/interrupts and frees both RX/TX rings.
    // A byte already on the wire cannot be retracted; the PS1 launcher must
    // resynchronize after abort. No buffered old bytes enter a new session.
    if(installed){
      if(uart_driver_delete(PORT)!=ESP_OK)return false;
      installed=false;events=nullptr;
    }
    uart_config_t cfg{};
    cfg.baud_rate=baud;cfg.data_bits=UART_DATA_8_BITS;cfg.parity=UART_PARITY_DISABLE;
    cfg.stop_bits=UART_STOP_BITS_1;cfg.flow_ctrl=UART_HW_FLOWCTRL_DISABLE;
    cfg.source_clk=UART_SCLK_APB;
    if(uart_param_config(PORT,&cfg)!=ESP_OK||
       uart_set_pin(PORT,UART_TX,UART_RX,UART_PIN_NO_CHANGE,UART_PIN_NO_CHANGE)!=ESP_OK)return false;
    // IDF allocates the ISR on the calling core (owner task is core0).
    if(uart_driver_install(PORT,16384,4096,20,&events,0)!=ESP_OK)return false;
    installed=true;
    if(uart_set_rx_full_threshold(PORT,64)!=ESP_OK||uart_set_rx_timeout(PORT,2)!=ESP_OK||
       uart_get_tx_buffer_free_size(PORT,&emptyTxCapacity)!=ESP_OK||!emptyTxCapacity){
      uart_driver_delete(PORT);installed=false;events=nullptr;return false;
    }
    return true;
  }
  void ack(const uint8_t *p,uint32_t ip,uint16_t port) override{
    if(control.beginPacket(IPAddress(ip),port)){control.write(p,20);control.endPacket();}
  }
};
Adapter io;
GameBridge::Core bridge(io);
Snapshot live,published;
portMUX_TYPE statusMux=portMUX_INITIALIZER_UNLOCKED;
TaskHandle_t owner=nullptr;
uint32_t lastPoll=0,lastPublish=0;
void publish(){
  live.transport=bridge.stats;
  live.heapFree=ESP.getFreeHeap();
  live.stackFreeBytes=uxTaskGetStackHighWaterMark(nullptr);
  portENTER_CRITICAL(&statusMux);published=live;portEXIT_CRITICAL(&statusMux);
}
}
bool begin(){
  if(owner||xPortGetCoreID()!=0)return false;
  owner=xTaskGetCurrentTaskHandle();
  live.ownerCore=xPortGetCoreID();
  if(!io.resetUart(115200)||!io.control.begin(3334)){
    bridge.stats.fault=true;publish();return false;
  }
  io.server.begin();io.server.setNoDelay(true);
  // NetworkServer exposes bool conversion for successful listening socket.
  live.initialized=bool(io.server);
  if(!live.initialized)bridge.stats.fault=true;
  publish();return live.initialized;
}
void poll(){
  if(xTaskGetCurrentTaskHandle()!=owner||!live.initialized)return;
  const uint32_t start=micros(),now=millis();
  if(lastPoll)live.maxServiceGapUs=std::max(live.maxServiceGapUs,uint32_t(start-lastPoll));
  lastPoll=start;
  // UART events first: don't acknowledge a control change on a corrupted link.
  uart_event_t e;
  for(unsigned i=0;i<20&&io.events&&xQueueReceive(io.events,&e,0)==pdTRUE;++i){
    if(e.type==UART_FIFO_OVF||e.type==UART_BUFFER_FULL||e.type==UART_PARITY_ERR||
       e.type==UART_FRAME_ERR||e.type==UART_BREAK){bridge.uartError();break;}
  }
  const int packetSize=io.control.parsePacket();
  if(packetSize>0){
    uint8_t p[20]{};
    const auto ip=uint32_t(io.control.remoteIP());const uint16_t port=io.control.remotePort();
    if(packetSize==20&&io.control.read(p,20)==20)bridge.control(p,20,ip,port,now);
    else{io.control.flush();++bridge.stats.malformed;}
  }
  bridge.poll(now);
  // One accept attempt per iteration; reject extras without replacing owner.
  WiFiClient candidate=io.server.accept();
  if(candidate.fd()>=0){
    if(bridge.open(uint32_t(candidate.remoteIP()),now)){io.client=candidate;io.client.setNoDelay(true);}
    else candidate.stop();
  }
  live.maxIterationUs=std::max(live.maxIterationUs,uint32_t(micros()-start));
  if(uint32_t(now-lastPublish)>=100){publish();lastPublish=now;}
}
Snapshot snapshot(){
  portENTER_CRITICAL(&statusMux);const Snapshot s=published;portEXIT_CRITICAL(&statusMux);return s;
}
}
