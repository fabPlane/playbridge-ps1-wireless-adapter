#include <WiFi.h>
#include <WebServer.h>
#include "ps1_polled_engine.h"
#include "pad_state.h"
#include "controller_page.h"
#include "Ps1GameBridge.h"

// Verified bench mapping, NOT an assertion that production V4 uses CMD34.
// Keep DATA C50 pull-up=1k, E51 base resistor=1k, C51 pull-down=100k.
// Bench readback: E50--1k--F50/G50--GPIO19.
// The separate core0 serial task owns J2 RX GPIO16 / TX GPIO17.
static WebServer server(80);
static portMUX_TYPE stateMux = portMUX_INITIALIZER_UNLOCKED;
static PadState pad;
struct Diagnostics {
  uint32_t frames, addresses, commands, replies, aborts, mismatches;
  uint8_t rx[5], data[5], count;
};
static Diagnostics published = {};
static bool serviceReady = false;
static bool bridgeTaskReady = false;
static constexpr const char *FIRMWARE_VERSION = "v4-combined-polled-0.1.0";

static void publishDiagnostics() {
  // Called only by the bus task after pollBus has released its critical section.
  portENTER_CRITICAL(&stateMux);
  published.frames=transactions; published.addresses=addresses;
  published.commands=commands; published.replies=fullReplies;
  published.aborts=aborted; published.mismatches=dataByteMismatches;
  published.count=lastBytes;
  for(unsigned i=0;i<5;++i){published.rx[i]=lastRx[i];published.data[i]=lastData[i];}
  portEXIT_CRITICAL(&stateMux);
}

static void handleState() {
  const String value=server.arg("mask");
  if(value.length()==0 || value.length()>5){server.send(400,"text/plain","Invalid mask");return;}
  uint32_t number=0;
  for(unsigned i=0;i<value.length();++i){
    if(value[i]<'0'||value[i]>'9'){server.send(400,"text/plain","Invalid mask");return;}
    number=number*10+(value[i]-'0');
  }
  if(number>65535){server.send(400,"text/plain","Invalid mask");return;}
  const uint32_t now=millis();
  portENTER_CRITICAL(&stateMux);pad.set(number,now);portEXIT_CRITICAL(&stateMux);
  server.send(200,"text/plain","Received");
}

static void handleStatus() {
  Diagnostics s;
  portENTER_CRITICAL(&stateMux);s=published;portEXIT_CRITICAL(&stateMux);
  char json[440];
  snprintf(json,sizeof(json),
    "{\"firmware\":\"v4-combined-polled-0.1.0\",\"transactions\":%lu,\"addresses\":%lu,\"commands\":%lu,\"fullReplies\":%lu,\"aborted\":%lu,\"dataMismatches\":%lu,\"bytes\":%u,\"rx\":\"%02X %02X %02X %02X %02X\",\"data\":\"%02X %02X %02X %02X %02X\"}",
    (unsigned long)s.frames,(unsigned long)s.addresses,(unsigned long)s.commands,
    (unsigned long)s.replies,(unsigned long)s.aborts,(unsigned long)s.mismatches,s.count,
    s.rx[0],s.rx[1],s.rx[2],s.rx[3],s.rx[4],s.data[0],s.data[1],s.data[2],s.data[3],s.data[4]);
  server.sendHeader("Cache-Control","no-store");server.send(200,"application/json",json);
}

static void handleBridgeStatus() {
  const auto s=Ps1GameBridge::snapshot();
  const auto &t=s.transport;
  char json[1600];
  snprintf(json,sizeof(json),
    "{\"firmware\":\"%s\",\"profile\":\"%s\",\"dataReadbackEnabled\":%s,"
    "\"taskCreated\":%s,\"initialized\":%s,\"fault\":%s,\"active\":%s,"
    "\"baud\":%lu,\"sessions\":%lu,\"aborts\":%lu,\"tcpReceived\":%lu,\"uartWritten\":%lu,"
    "\"uartReceived\":%lu,\"tcpWritten\":%lu,\"uartErrors\":%lu,\"socketErrors\":%lu,"
    "\"discardedSoftwareBytes\":%lu,\"shortWrites\":%lu,\"wouldBlock\":%lu,"
    "\"rejectedClients\":%lu,\"malformedControls\":%lu,\"duplicateControls\":%lu,"
    "\"controlAckAttempts\":%lu,\"drainTimeouts\":%lu,\"uartResetFailures\":%lu,"
    "\"toTcpPeak\":%lu,\"toUartPeak\":%lu,\"ownerCore\":%d,\"maxServiceGapUs\":%lu,"
    "\"maxIterationUs\":%lu,\"heapFree\":%lu,\"stackFreeBytes\":%lu}",
    FIRMWARE_VERSION,PROFILE_NAME,HAS_DATA_READBACK?"true":"false",
    bridgeTaskReady?"true":"false",s.initialized?"true":"false",t.fault?"true":"false",t.active?"true":"false",
    (unsigned long)t.baud,(unsigned long)t.sessions,(unsigned long)t.aborts,
    (unsigned long)t.tcpReceived,(unsigned long)t.uartWritten,(unsigned long)t.uartReceived,(unsigned long)t.tcpWritten,
    (unsigned long)t.uartErrors,(unsigned long)t.socketErrors,(unsigned long)t.discarded,
    (unsigned long)t.shortWrites,(unsigned long)t.wouldBlock,(unsigned long)t.rejected,
    (unsigned long)t.malformed,(unsigned long)t.duplicates,(unsigned long)t.controlAcks,
    (unsigned long)t.drainTimeouts,(unsigned long)t.resetFailures,(unsigned long)t.toTcpPeak,
    (unsigned long)t.toUartPeak,s.ownerCore,(unsigned long)s.maxServiceGapUs,
    (unsigned long)s.maxIterationUs,(unsigned long)s.heapFree,(unsigned long)s.stackFreeBytes);
  server.sendHeader("Cache-Control","no-store");server.send(200,"application/json",json);
}

static void bridgeTask(void *) {
  if(!Ps1GameBridge::begin()){Serial.println("J2 init failed; controller remains available");vTaskDelete(nullptr);}
  while(true){Ps1GameBridge::poll();vTaskDelay(1);}
}

static void webTask(void *) {
  WiFi.persistent(false); // No credential/NVS writes while timing tasks run.
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("PlayBridge-Test","playbridge");
  WiFi.setAutoReconnect(true);
  WiFi.begin(); // Reuse saved credentials; no password in source or logs.
  WiFi.setSleep(false);
  // UART driver/ISR is installed by its owner on core0, never the pad core.
  bridgeTaskReady=xTaskCreatePinnedToCore(bridgeTask,"playbridge-j2",6144,nullptr,2,nullptr,0)==pdPASS;
  server.on("/",HTTP_GET,[]{server.sendHeader("Cache-Control","no-store");server.send_P(200,"text/html",CONTROLLER_PAGE);});
  server.on("/state",HTTP_POST,handleState);
  server.on("/status",HTTP_GET,handleStatus);
  server.on("/bridge/status",HTTP_GET,handleBridgeStatus);
  server.onNotFound([]{server.send(404,"text/plain","Not found");});
  server.begin();
  Serial.printf("Web polled controller: AP http://%s/ (PlayBridge-Test)\n",WiFi.softAPIP().toString().c_str());
  bool connected=false;
  while(true){
    server.handleClient();
    const bool current=WiFi.status()==WL_CONNECTED;
    if(current&&!connected)Serial.printf("Local UI: http://%s/\n",WiFi.localIP().toString().c_str());
    connected=current;
    vTaskDelay(1);
  }
}

void setup() {
  digitalWrite(DATA_DRIVE,LOW);digitalWrite(ACK_DRIVE,LOW);
  pinMode(DATA_DRIVE,OUTPUT);pinMode(ACK_DRIVE,OUTPUT);
  pinMode(CMD,INPUT);pinMode(CLK,INPUT);pinMode(ATT,INPUT);
  if(HAS_DATA_READBACK)pinMode(DATA_SENSE,INPUT);
  Serial.begin(115200);
  cyclesPerUs=getCpuFrequencyMhz();
  senseEnabled=HAS_DATA_READBACK;
  // Arduino loop runs on core1; network handling runs on core0. No per-edge ISR.
  if(xPortGetCoreID()!=1){Serial.println("ERROR: this build requires Arduino loop on core1");return;}
  serviceReady=xTaskCreatePinnedToCore(webTask,"playbridge-web",8192,nullptr,1,nullptr,0)==pdPASS;
  Serial.printf("%s profile=%s; UART2 RX16 TX17; neutral at boot, 800ms lease\n",FIRMWARE_VERSION,PROFILE_NAME);
  if(!serviceReady)Serial.println("ERROR: web task creation failed");
}

void loop() {
  if(!serviceReady){dataBit(true);ack(false);delay(10);return;}
  const uint32_t now=millis();
  portENTER_CRITICAL(&stateMux);const uint16_t buttons=pad.get(now);portEXIT_CRITICAL(&stateMux);
  pollBus(buttons);
  publishDiagnostics();
  delay(1); // Same idle yield as the console-verified diagnostic firmware.
}
