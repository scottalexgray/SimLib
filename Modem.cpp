#include "Modem.h"
#include <Wire.h>
#include <Arduino.h>
//#include <SoftwareSerial.h>

// TTGO T-Call pins
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define I2C_SDA              21
#define I2C_SCL              22

//Serial AT commands defining
//#define SerialAT Serial1


Modem::Modem(HardwareSerial& modemPort,Stream& devicePort, char board[], char APN[], char PIN[])
{
    //SoftwareSerial Serial1(26, 27); // RX, TX
    _modemPort = &modemPort;
    _devicePort = &devicePort;

	TwoWire I2CPower = TwoWire(0);

    // Set modem reset, enable, power pins (Specifically for TTGO ESP32 SIM800L)
    pinMode(MODEM_PWKEY, OUTPUT);
    pinMode(MODEM_RST, OUTPUT);
    pinMode(MODEM_POWER_ON, OUTPUT);
    digitalWrite(MODEM_PWKEY, LOW);
    digitalWrite(MODEM_RST, HIGH);
    digitalWrite(MODEM_POWER_ON, HIGH);

    // Set GSM module baud rate and UART pins
    _modemPort->begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
    //_serialPort->write("AT\r\n");
    _devicePort->write("HEEEEELLLLLOOOOOO");
    
    delay(3000);

}

int Modem::CIPSTATUS()
{
    _devicePort->write("HEEEEELLLLLOOOOOO");
    int ret = sendCmdAndWaitForResp("AT\r\n", "OK", DEFAULT_TIMEOUT);
    return ret;
}

void Modem::sendCmd(const char* cmd)
{
    _modemPort->write(cmd);
}

int Modem::sendATTest(void)
{
    int ret = sendCmdAndWaitForResp("AT\r\n", "OK", DEFAULT_TIMEOUT);
    return ret;
}

int Modem::waitForResp(const char* resp, unsigned int timeout)
{
    int len = strlen(resp);
    int sum = 0;
    unsigned long timerStart, timerEnd;
    timerStart = millis();

    while (1) {
        if (_modemPort->available()) {
            char c = _modemPort->read();
            sum = (c == resp[sum]) ? sum + 1 : 0;
            if (sum == len)break;
        }
        timerEnd = millis();
        if (timerEnd - timerStart > 1000 * timeout) {
            return -1;
        }
    }

    while (_modemPort->available()) {
        _modemPort->read();
    }

    return 0;
}



int Modem::sendCmdAndWaitForResp(const char* cmd, const char* resp, unsigned timeout)
{
    sendCmd(cmd);
    return waitForResp(resp, timeout);
}


