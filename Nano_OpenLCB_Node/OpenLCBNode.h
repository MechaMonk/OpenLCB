#ifndef OpenLCBNodeIncluded
#define OpenLCBNodeIncluded

#if (ARDUINO >= 100)
    #include "Arduino.h"
#else
    #include "WProgram.h"    
#endif

#include "OpenLCBMessage.h"
#include "OpenLCBAliasRegistry.h"
#include "OpenLCBCANInterface.h"

class OpenLCBNode {
    uint64_t nodeId;        // 48-bit id
    uint16_t alias;     // 16-bit alias
    char strNodeId[18]; // = "00.00.00.00.00.00";
    bool permitted;     // false => inhibited
    OpenLCBMessage msgIn, msgOut;
    OpenLCBAliasRegistry registry;
    uint32_t waitStart;
    OpenLCBCANInterface* canInt;
    
    const char* Manufacturer  = "Solwiz.local";
    const char* ModelName     = "Open LCB node";
    const char* HardwareVersion  = "0.1";
    const char* SoftwareVersion  = "0.1";


  public:
    OpenLCBNode();
    uint64_t getNodeId();
    uint16_t getAlias();
    void setCanInt(OpenLCBCANInterface* canInt);
    char* ToString();
    void loop();
    void setNodeId(const char* Id);

  private:
    void genAlias();
    void processIncoming();
    bool registerMe();
 //   bool sendAMD();
    bool sendOIR(uint16_t errorCode, uint16_t senderAlias, MTI mti);    
    
};


#endif