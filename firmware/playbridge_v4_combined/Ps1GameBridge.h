#pragma once
#include "BridgeCore.h"
namespace Ps1GameBridge {
struct Snapshot {
  GameBridge::Status transport;
  bool initialized=false;
  int ownerCore=-1;
  uint32_t maxServiceGapUs=0,maxIterationUs=0,heapFree=0,stackFreeBytes=0;
};
// All UART/socket operations must run in the dedicated core0 owner task.
bool begin();
void poll();
Snapshot snapshot(); // Safe for HTTP; never accesses live UART/socket objects.
}
