#include "Modem.h"
#include <Wire.h>
#include <Arduino.h>
#include <string>

#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define I2C_SDA              21
#define I2C_SCL              22

#define endline "\r\n\0"





Modem::Modem(HardwareSerial& modemSerialConn, HardwareSerial& deviceSerialConn, int bRate)
{
    //set the serial streams to the user specified streams    
    _modemPort = &modemSerialConn;
    _devicePort = &deviceSerialConn;
    baudRate = bRate;
}

void Modem::Init()
{
    // I2C for SIM800 (to keep it running when powered from battery)//not sure if nessesary
    TwoWire I2CPower = TwoWire(0);

    //_devicePort->begin(baudRate);

    // Set modem reset, enable, power pins
    pinMode(MODEM_PWKEY, OUTPUT);
    pinMode(MODEM_RST, OUTPUT);
    pinMode(MODEM_POWER_ON, OUTPUT);

    _devicePort->print("Initializing modem...");

    //can you initialize the modem, and then turn it on and still get it to work?

    //digitalWrite(MODEM_PWKEY, HIGH);
    digitalWrite(MODEM_PWKEY, LOW);
    //delay(1200);
    //digitalWrite(MODEM_PWKEY, HIGH);

    digitalWrite(MODEM_RST, HIGH);
    digitalWrite(MODEM_POWER_ON, HIGH);
    

    // Set GSM module baud rate and UART pins
    _modemPort->begin(baudRate, SERIAL_8N1, MODEM_RX, MODEM_TX);   
    while (!_modemPort)
    {

    }

    //delay(6000);

    initializedComms = true;
    _devicePort->print("done\n");
    
}

void Modem::DeInit()
{
    _devicePort->print("De-initializing modem...");
    //as per the manual
    digitalWrite(MODEM_PWKEY, HIGH);
    digitalWrite(MODEM_PWKEY, LOW);
    delay(1500);
    digitalWrite(MODEM_PWKEY, HIGH);
    //turn off things that were on
    digitalWrite(MODEM_RST, LOW);
    digitalWrite(MODEM_POWER_ON, LOW);

    //disconnect from the serial connection
    _modemPort->end();


    initializedComms = false;
    delay(500);
    _devicePort->print("done\n");
}

void Modem::Connect()
{
    //SendCmdAndWait("AT+CSTT=\"internet\"");
    SendCmdAndWait("AT+CSTT?");
}

bool Modem::isInitialized()
{
    return initializedComms;
}



void Modem::SendAT(const char* cmd)
{
    _modemPort->write(cmd);
    _modemPort->write(endline);
    _modemPort->flush();
    delay(1000);
}

void Modem::removeCmd(char inputBuffer[], const char* cmd, char returnBuffer[], int returnBufferSize)
{
    int returnIndex = 0;
    for (int i = (strlen(cmd) + 3); i < strlen(inputBuffer); i++)
    {
        returnBuffer[returnIndex] = inputBuffer[i];
        returnIndex++;
        if (returnIndex == (returnBufferSize - 1) || i == (strlen(inputBuffer) - 1))
        {
            //SerialMon.println("Adding Null Character");
            returnBuffer[returnIndex] = '\0';
            break;
        }
    }
}

void Modem::SendCmdAndWait(const char* cmd)
{
    //send cmd
    SendAT(cmd);
    //now wait 
    unsigned int bufferLen = 100;
    char responseBuffer[bufferLen];
    int responseIndex = 0;

    for (int i = 0; i < bufferLen; i++)
    {
        if (_modemPort->available())
        {
            char c = _modemPort->read();
            if (c == '\n')
            {
                responseBuffer[i] = ' ';
            }
            else if (c == '\r')
            {
                responseBuffer[i] = ' ';
            }
            else
            {
                responseBuffer[i] = c;
            }
        }
        else
        {
            responseBuffer[i] = '\0';
            responseBuffer[i - 1] = '\0';
            responseBuffer[i - 2] = '\0';
            //this is done because I know the last two characters are newline and carraige return  
            break;
        }
    }

    int responseOnlyBufferSize = 30;
    char responseOnly[responseOnlyBufferSize];

    removeCmd(responseBuffer, cmd, responseOnly, responseOnlyBufferSize);
    //SerialMon.println("Response:");
    _devicePort->write(responseOnly);
    _devicePort->write(endline);
}

void Modem::GetCGREG(CGREG* returnState)
{


}




