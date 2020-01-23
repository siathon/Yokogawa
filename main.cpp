#include "main.h"

void connect(){
    serial.attach(callback(&ser, &SerialHandler::rx));
    while (true) {
        Watchdog::get_instance().kick();
        if (sim800.checkSim() != 0) {
            sim800.start();
            continue;
        }
        Watchdog::get_instance().kick();
        if (sim800.setGPRSSettings() != 0) {
            continue;
        }
        MQTT.beginTCPConnection();
        serial.attach(callback(&tcp, &PacketHandler::rx));
        Watchdog::get_instance().kick();
        MQTT.connect(1, 0, 0, 0, (char*)"", (char*)"");
        Watchdog::get_instance().kick();
        break;
    }
}

// float readSensor(){
//     sum = 0;
//     for (size_t i = 0; i < 10; i++) {
//         sum += sensor.read();
//         wait_us(10000);
//     }
//     avg = sum / 10.0;
//     float mv = avg * 3300.0;
//     float psi = ((mv - 40.0) * 4000.0) / 160.0;
//     if (psi <= 0.0) {
//         psi = 0;
//     }
//     pc.printf("sensor = %.1f psi\n", psi);
//     return psi;
// }

float readSensor(){
    sum = 0;
    for (size_t i = 0; i < 10; i++) {
        sum += sensor.read();
        wait_us(10000);
    }
    avg = sum / 10.0;
    return avg;
}

void onPacket(char* packet) {
    MQTT.parsePacket(packet);
}

void onMessage(int channel, string msgID, string value){
    pc.printf("%d - %s - %s\n", channel, msgID.c_str(), value.c_str());
}

void check(){
    Watchdog::get_instance().kick();
    if (MQTT.publishFailCount > 2 || MQTT.pingFailCount > 2) {
        connect();
    }
}

void sub(){
    sprintf(MQTT.Topic, "v1/devices/me/telemetry/response/+");
    pc.printf("sunscribe to topic: %s\n",  MQTT.Topic);
    MQTT.subscribe(0, MQTT._generateMessageID(), MQTT.Topic, 0);
}

void pub(){
    pressure = readSensor();
    sprintf(MQTT.Topic, "v1/devices/me/telemetry");
    sprintf(MQTT.Message, "{\"water_level\":%f}", pressure);
    // time_t seconds = time(NULL);
    // char secSt[3], minSt[3], hrSt[3];
    // strftime(secSt, 32, "%S", localtime(&seconds));
    // strftime(minSt, 32, "%M", localtime(&seconds));
    // strftime(hrSt , 32, "%H", localtime(&seconds));
    pc.printf("publish to topic: %s message %s\n", MQTT.Topic, MQTT.Message);
    MQTT.publish(0, 1, 0, MQTT._generateMessageID(), MQTT.Topic, MQTT.Message);
}

int main() {
    const reset_reason_t reason = ResetReason::get();
    pc.printf("Last system reset reason: %s\r\n", reset_reason_to_string(reason).c_str());
    watchdog.start();
    connect();
    Watchdog::get_instance().kick();
    ev_queue.call_in(5000, sub);
    ev_queue.call_every(60000, pub);
    ev_queue.call_every(10, callback(&tcp, &PacketHandler::checkForPacket));
    ev_queue.call_every(1000, callback(&MQTT, &GSM_MQTT::keepAlive));
    ev_queue.call_every(20000, check);
    ev_queue.dispatch_forever();
}
