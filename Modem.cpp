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
    RDY = false;
    //set the serial streams to the user specified streams    
    _modemPort = &modemSerialConn;
    _devicePort = &deviceSerialConn;
    baudRate = bRate;
}

void Modem::ConnectGPRS()
{
    //SendCmdAndWait("AT+CSTT=\"internet\"");
    //SendCmdAndWait("AT+CSTT?");
    SetCFUN(1);



}


void Modem::SetCFUN(int val)
{
    SendAT("AT+CFUN=1");    
}

void Modem::GetCGREG(CGREG* returnState)
{


}




void Modem::Init()
{
    RDY = false;
    // I2C for SIM800 (to keep it running when powered from battery)//not sure if nessesary
    TwoWire I2CPower = TwoWire(0);

    //_devicePort->begin(baudRate);

    // Set modem reset, enable, power pins
    pinMode(MODEM_PWKEY, OUTPUT);
    pinMode(MODEM_RST, OUTPUT);
    pinMode(MODEM_POWER_ON, OUTPUT);

    _devicePort->print("Initializing modem...");

    // Set GSM module baud rate and UART pins
    _modemPort->begin(baudRate, SERIAL_8N1, MODEM_RX, MODEM_TX);
    while (!_modemPort)
    {
        //wait here until serial connection is ready
    }  
    //set the power on the pins
    digitalWrite(MODEM_PWKEY, LOW);
    digitalWrite(MODEM_RST, HIGH);
    digitalWrite(MODEM_POWER_ON, HIGH);
    

    //delay(6000);
    ModemRDYCheck();

    unsigned long timeoutTime = 8000;
    unsigned long startTime = millis();

    while (!RDY)
    {
        unsigned long currentTime = millis();
        if ((currentTime - startTime) > timeoutTime)
        {
            _devicePort->print("timeout_");
            return;
        }
    }            
    _devicePort->print("done\n");       
}

void Modem::DeInit()
{
    RDY = false;
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


    
    delay(500);
    _devicePort->print("done\n");
}

void Modem::ModemRDYCheck()
{
    if (RDY)
    {
        return;
    }

    int bufferLen = 4;
    char responseBuffer[bufferLen];

    WaitForResponse(bufferLen, responseBuffer, 6000); //will timeout after 6s

    //_devicePort->print("Zero if == RDY: ");
    int state = strcmp(responseBuffer, "RDY");

    if (state == 0)
    {
        RDY = true;
    }
    //_devicePort->println(state);
    //_devicePort->println(responseBuffer);

}



//When this gets run, it copies all the available chars, up to the length of bufferLen into repsonse
void Modem::WaitForResponse(int bufferLen, char response[], unsigned long timeoutTime)
{
    //TODO make timeout so that it doesn't hang here foreve causing device to Freeze forever


    //int bufferLen = 4;
    char initialResponseBuffer[bufferLen];

    bool done = false; //used to exit while loop, not for actual modem ready state
    
    //int actualResponseLength = 0;
    //int readIndex = 0;

    int responseIndex = 0;
    //_devicePort->print("Entering Wait Loop..\n");

    unsigned long timeStarted;

    unsigned long timeLastAvailable;

    timeStarted = millis();
    while (!done)
    {        
        if (_modemPort->available())
        {
            timeStarted = millis();
            char c = _modemPort->read();
            if (responseIndex < bufferLen - 1)
            {
                if (c == '\0' || c == '\n' || c == '\r')
                {
                    //do nothing with it
                }
                else
                {
                    //_devicePort->write(c);
                    initialResponseBuffer[responseIndex] = c;
                    //_devicePort->write(initialResponseBuffer[responseIndex]);
                    //_devicePort->println(responseIndex);
                    responseIndex += 1;
                }
            }
            else
            {
                initialResponseBuffer[responseIndex] = '\0';
                //_devicePort->print("Exiting Wait Loop..\n");
                done = true;
            }
        }
        else
        {
            unsigned long timeCurrent = millis();
            if ((timeCurrent - timeStarted) > timeoutTime)
            {
                done = true;
                //also make last character null
                initialResponseBuffer[bufferLen-1] = '\0';

                _devicePort->println("Timout!!!");
            }
        }
    }
    strcpy(response, initialResponseBuffer);
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






