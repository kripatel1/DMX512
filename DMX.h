//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------


#ifndef DMX_H_
#define DMX_H_


//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void tx_on();
void loadDataPackets();
void primePumpUart1();
void startTX();
void transmitPackets();
void stopTX();
void updateMaxDevices(uint16_t max);

uint32_t getOperationMode();
void setOperationMode();
void setControllerMode();
void setDeviceMode();
void setAddress(uint32_t value);
uint32_t getAddress();
void startPOLL();
void devicesFound();
void setAddressValue(uint16_t addr, uint16_t value);
uint16_t getAddressValue(uint16_t addr);
void clearAllValues();

#endif

