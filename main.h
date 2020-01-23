#include "mbed.h"
#include "SerialHandler.h"
#include "PacketHandler.h"
#include "SIM800.h"
#include "GSM_MQTT.h"
#include "ResetReason.h"
#include <string>

using namespace std;

std::string reset_reason_to_string(const reset_reason_t reason){
    switch (reason) {
        case RESET_REASON_POWER_ON:
            return "Power On";
        case RESET_REASON_PIN_RESET:
            return "Hardware Pin";
        case RESET_REASON_SOFTWARE:
            return "Software Reset";
        case RESET_REASON_WATCHDOG:
            return "Watchdog";
        default:
            return "Other Reason";
    }
}

char* user = (char*)"QZTd9afhwna6iVFxAFHR";
char* pass = NULL;
char* clID = NULL;
char* host = (char*)"things.saymantechcloud.ir";
char* port = (char*)"1883";

Watchdog &watchdog = Watchdog::get_instance();

EventQueue ev_queue(100 * EVENTS_EVENT_SIZE);

RawSerial serial(PA_9, PA_10, 9600);
RawSerial pc(PA_2, PA_3, 9600);
SerialHandler ser(0);
GSM_MQTT MQTT(user, pass, clID, host, port, 10);
PacketHandler tcp(0);
SIM800 sim800(PC_9, PB_7);
DigitalOut led(PC_8, 0);
AnalogIn sensor(PC_2);

float sum = 0;
float avg = 0;
float pressure;
