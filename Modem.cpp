#include "Modem.h"
#include <Wire.h>
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


Modem::Modem(HardwareSerial& port, char board[], char APN[], char PIN[])
{
    //SoftwareSerial Serial1(26, 27); // RX, TX
    _serialPort = &port;

	TwoWire I2CPower = TwoWire(0);

    // Set modem reset, enable, power pins (Specifically for TTGO ESP32 SIM800L)
    pinMode(MODEM_PWKEY, OUTPUT);
    pinMode(MODEM_RST, OUTPUT);
    pinMode(MODEM_POWER_ON, OUTPUT);
    digitalWrite(MODEM_PWKEY, LOW);
    digitalWrite(MODEM_RST, HIGH);
    digitalWrite(MODEM_POWER_ON, HIGH);

    // Set GSM module baud rate and UART pins
    _serialPort->begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
    _serialPort->write("AT\r\n");
    
    delay(3000);

}
