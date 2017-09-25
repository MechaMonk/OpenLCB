#include <inttypes.h>
#include "interrupthandler.h"
#include "FreeRTOS.h"
#include "task.hpp"
#include "queue.h"
#include "tcpserver.h"
#include "mcp_can.hpp"
#include "canmessage.h"

#define os_zalloc(s)  calloc(1, (s))

extern MCP_CAN mcpcan;
extern struct queueHandles qh;

// Interrupt handler for gpio
// check the interrupt is from the MCP2515, if so, queue the can message for processing

extern "C" void gpio_intr_handler(uint8_t gpio_num) {
    CAN_MESSAGE canMsg;

    if (gpio_num == CAN_INT) {
        if (mcpcan.readMsgBuf(&canMsg.id, &canMsg.ext, &canMsg.len, (INT8U*)&canMsg.dataBytes) == CAN_OK) {
            if(xQueueSendToBackFromISR(qh.xQueueCanToWiFi, &canMsg, NULL)!= pdPASS ) {
                printf("Queue xQueueWiFiToCan full, discarding message\n");
            }
        }
    }
}

