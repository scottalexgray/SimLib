/*
    Modem.cpp - Library for using a SIM800L modem
    Created by Scott Alexander Gray - 2020
*/
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



void Modem::SetSimSettings(const char* apn, const char* pin)
{
    APN = apn;
    PIN = pin;
}

void Modem::ConnectGPRSBlocking()
{
    //SendCmdAndWait("AT+CSTT=\"internet\"");
    //SendCmdAndWait("AT+CSTT?");

    // Disabled to test SendCmdAndWait
    SetCFUN(1); //set the functionality of the modem
    SetCSTT(APN); //set the sim access point name
    SetCPIN(PIN); //set the sim pin

    _devicePort->print("\nWaiting For CGREG... ");
    
    while (cgreg != Roaming && cgreg != HomeRegistered)
    {
        RefreshCGREG();
    }
    _devicePort->print("Done");


    
    SetCGATT(1); //attach to gsm network    
    SetCGACT(1, 1);    
    SetCIICR();
    SetCIFSR();
    

}


void Modem::SetCFUN(int val)
{
    int bufferSize = 32; //length of buffer
    char* full_cmd = new char[bufferSize]; //create buffer for command to be sent

    const char* at_cmd = "AT+CFUN="; //AT command

    char valChar[5]; //create buffer for char version of val
    itoa(val, valChar, 10); //convert int to char array
    
    strcpy(full_cmd, at_cmd); //copy the base command to the buffer
    strcat(full_cmd, valChar); //append the valChar to the cmd buffer
    
    //_devicePort->print("Command to be sent:\n");
    //_devicePort->print(full_cmd);

    int responseLen = 3; //two characters and one null character
    char responseBuffer[responseLen];

    SendCmdAndWait(full_cmd, responseBuffer, responseLen, false, false);  //send command to the modem 
    _devicePort->print("\nCFUN Response: ");
    _devicePort->print(responseBuffer);
    
}


void Modem::SetCSTT(const char* apn)
{
    int bufferSize = 32; //length of buffer
    char* full_cmd = new char[bufferSize]; //create buffer for command to be sent

    const char* at_cmd = "AT+CSTT="; //AT command

    strcpy(full_cmd, at_cmd); //copy the base command to the buffer    
    strcat(full_cmd, apn); //append the valChar to the cmd buffer    

    int responseLen = 3; //two characters and one null character
    char responseBuffer[responseLen];

    SendCmdAndWait(full_cmd, responseBuffer, responseLen, false, false);  //send command to the modem 
    _devicePort->print("\nCSTT Response: ");
    _devicePort->print(responseBuffer);
}

void Modem::SetCPIN(const char* pin)
{
    if (*pin == '\0')
    {
        //_devicePort->print("Found pin char to contain null character");        
        return; //This skips sending a command if the pin is empty
    }
    //_devicePort->print("Didn't find null char");

    int bufferSize = 32; //length of buffer
    char* full_cmd = new char[bufferSize]; //create buffer for command to be sent

    const char* at_cmd = "AT+CPIN="; //AT command

    strcpy(full_cmd, at_cmd); //copy the base command to the buffer
    strcat(full_cmd, pin); //append the valChar to the cmd buffer

    int responseLen = 3; //two characters and one null character
    char responseBuffer[responseLen];

    SendCmdAndWait(full_cmd, responseBuffer, responseLen, false, false);  //send command to the modem 
    _devicePort->print("\nSetCPIN Response: ");
    _devicePort->print(responseBuffer);
}

void Modem::GetCPIN()
{
    int bufferSize = 32; //length of buffer
    char* full_cmd = new char[bufferSize]; //create buffer for command to be sent

    const char* at_cmd = "AT+CPIN=?"; //AT command

    strcpy(full_cmd, at_cmd); //copy the base command to the buffer    

    int responseLen = 16; 
    char responseBuffer[responseLen];

    SendCmdAndWait(full_cmd, responseBuffer, responseLen, false, false);  //send command to the modem 
    _devicePort->print("\nGetCPIN Response: ");
    _devicePort->print(responseBuffer);
}

void Modem::RefreshCGREG()
{
    int bufferSize = 32; //length of buffer
    char* full_cmd = new char[bufferSize]; //create buffer for command to be sent

    const char* at_cmd = "AT+CGREG?"; //AT command

    strcpy(full_cmd, at_cmd); //copy the base command to the buffer    

    int responseLen = 16; //two characters and one null character
    char wholeResponseBuffer[responseLen];

    SendCmdAndWait(full_cmd, wholeResponseBuffer, responseLen, false, false);  //send command to the modem 

    int stateBufferLen = 16; //state only length
    int startIndex = 0; //index to start from
    int endIndex = 11; //index to end on
    char stateBuffer[stateBufferLen]; //buffer for state only char array

    trimChar(wholeResponseBuffer, stateBuffer, stateBufferLen, startIndex, endIndex); //trim the char to convert it to more appropriate terms for checking

    //comparing the state buffer to known responses to update cgreg enum
    if (strcmp(stateBuffer, "+CGREG: 0,0") == 0)
    {
        cgreg = NotRegistered;
    }
    else if (strcmp(stateBuffer, "+CGREG: 0,1") == 0)
    {
        cgreg = HomeRegistered;
    }
    else if (strcmp(stateBuffer, "+CGREG: 0,2") == 0)
    {
        cgreg = Searching;
    }
    else if (strcmp(stateBuffer, "+CGREG: 0,3") == 0)
    {
        cgreg = RegistrationDenied;
    }
    else if (strcmp(stateBuffer, "+CGREG: 0,4") == 0)
    {
        cgreg = Unknown;
    }
    else if (strcmp(stateBuffer, "+CGREG: 0,5") == 0) //rare
    {
        cgreg = Roaming;
    }
}



CGREG Modem::GetCGREG()
{
    RefreshCGREG();
    return cgreg;
}

void Modem::SetCGATT(int val) //val can only be 1 or 0, 1 attaches, 0 detaches
{
    int bufferSize = 32; //length of buffer
    char* full_cmd = new char[bufferSize]; //create buffer for command to be sent

    const char* at_cmd = "AT+CGATT="; //AT command

    char valChar[5]; //create buffer for char version of val
    itoa(val, valChar, 10); //convert int to char array

    strcpy(full_cmd, at_cmd); //copy the base command to the buffer
    strcat(full_cmd, valChar); //append the valChar to the cmd buffer

    int responseLen = 3; //two characters and one null character
    char responseBuffer[responseLen];

    SendCmdAndWait(full_cmd, responseBuffer, responseLen, false, false);  //send command to the modem 
    _devicePort->print("\nSetCGATT Response: ");
    _devicePort->print(responseBuffer);
}

void Modem::SetCGDCONT(int PDPNum, const char* ipMode, const char* apn)
{
    int bufferSize = 32; //length of buffer
    char* full_cmd = new char[bufferSize]; //create buffer for command to be sent

    const char* at_cmd = "AT+CGDCONT="; //AT command

    char pdpNum[5]; //create buffer for char version of PDPNum
    itoa(PDPNum, pdpNum, 10); //convert int to char array

    strcpy(full_cmd, at_cmd); //copy the base command to the buffer
    strcat(full_cmd, pdpNum); //append the valChar to the cmd buffer
    strcat(full_cmd, ",");
    strcat(full_cmd, ipMode);
    strcat(full_cmd, ",");
    strcat(full_cmd, apn);
    
    SendAT(full_cmd);  //send command to the modem 
}

void Modem::SetCGACT(int state, int cid)
{
    int bufferSize = 32;
    char* full_cmd = new char[bufferSize];//full command buffer

    const char* at_cmd = "AT+CGACT="; //AT command

    char stateChar[5]; //create buffer for char version of state
    itoa(state, stateChar, 10); //convert int to char array

    char cidChar[5]; //create buffer for char version of the id of the pdp context
    itoa(cid, cidChar, 10); //convert int to char array

    strcpy(full_cmd, at_cmd); //copy at command to command buffer
    strcat(full_cmd, stateChar); //append this to command buffer
    strcat(full_cmd, ","); //append this to command buffer
    strcat(full_cmd, cidChar); //append this to command buffer

    int responseLen = 3; //two characters and one null character
    char responseBuffer[responseLen];

    SendCmdAndWait(full_cmd, responseBuffer, responseLen, false, false);  //send command to the modem 
    _devicePort->print("\nSetCGACT Response: ");
    _devicePort->print(responseBuffer);
}

void Modem::GetCEER()
{
    int bufferSize = 32;
    char* full_cmd = new char[bufferSize];//full command buffer

    const char* at_cmd = "AT+CEER?"; //AT command

    strcpy(full_cmd, at_cmd); //copy at command to command buffer

    int responseLen = 3; //two characters and one null character
    char responseBuffer[responseLen];

    SendCmdAndWait(full_cmd, responseBuffer, responseLen, false, false);  //send command to the modem 
    _devicePort->print("\nGetCEER Response: ");
    _devicePort->print(responseBuffer);
}

void Modem::SetCIICR()
{
    int bufferSize = 32;
    char* full_cmd = new char[bufferSize];//full command buffer

    const char* at_cmd = "AT+CIICR"; //AT command

    strcpy(full_cmd, at_cmd); //copy at command to command buffer

    int responseLen = 3; //two characters and one null character
    char responseBuffer[responseLen];

    SendCmdAndWait(full_cmd, responseBuffer, responseLen, false, false);  //send command to the modem 
    _devicePort->print("\nSetCIICR Response: ");
    _devicePort->print(responseBuffer);

}

void Modem::SetCIFSR()
{
    int bufferSize = 32;
    char* full_cmd = new char[bufferSize];//full command buffer

    const char* at_cmd = "AT+CIFSR"; //AT command

    strcpy(full_cmd, at_cmd); //copy at command to command buffer

    int responseLen = 16; //two characters and one null character
    char responseBuffer[responseLen];

    SendCmdAndWait(full_cmd, responseBuffer, responseLen, false, false);  //send command to the modem 
    _devicePort->print("\nSetCIFSR Response: ");
    _devicePort->print(responseBuffer);
}

void Modem::GetCOPS()
{
    int bufferSize = 32; //length of buffer
    char* full_cmd = new char[bufferSize]; //create buffer for command to be sent

    const char* at_cmd = "AT+CGREG=?"; //AT command

    strcpy(full_cmd, at_cmd); //copy the base command to the buffer    

    int responseLen = 20; //two characters and one null character
    char responseBuffer[responseLen];

    SendCmdAndWait(full_cmd, responseBuffer, responseLen, false, false);  //send command to the modem 
    _devicePort->print("\nGetCOPS Response: ");
    _devicePort->print(responseBuffer);
}

void Modem::GetCSQ()
{
    int bufferSize = 32; //length of buffer
    char* full_cmd = new char[bufferSize]; //create buffer for command to be sent

    const char* at_cmd = "AT+CSQ"; //AT command

    strcpy(full_cmd, at_cmd); //copy the base command to the buffer    

    int responseLen = 8; //two characters and one null character
    char responseBuffer[responseLen];

    SendCmdAndWait(full_cmd, responseBuffer, responseLen, false, false);  //send command to the modem 
    _devicePort->print("\nGetCSQ Response: ");
    _devicePort->print(responseBuffer);
}


void Modem::RefreshCIPSTATUS()
{
    int bufferSize = 32; //length of buffer
    char* full_cmd = new char[bufferSize]; //create buffer for command to be sent

    const char* at_cmd = "AT+CIPSTATUS"; //AT command

    strcpy(full_cmd, at_cmd); //copy the base command to the buffer        

    //Manipulating data returned by modem
    int bufferLen = 32;
    char wholeResponseBuffer[bufferLen];
    SendCmdAndWait(full_cmd, wholeResponseBuffer, bufferLen, true, false); //now we have the response without the command


    int stateBufferLen = 16; //state only length
    int startIndex = 13; //index to start from
    int endIndex = 0;
    char stateBuffer[stateBufferLen]; //buffer for state only char array

    trimChar(wholeResponseBuffer, stateBuffer, stateBufferLen, startIndex, endIndex); //trim the char to convert it to more appropriate terms for checking

    //_devicePort->print("\ntrimmed Response:\n");
    //_devicePort->print(stateBuffer);

    //setting enum variable based on returned value

    if (strcmp(stateBuffer, "IP INITIAL") == 0)
    {
        cipstatus = IPInitial;
    }
    else if (strcmp(stateBuffer, "IP START") == 0)
    {
        cipstatus = IPStart;
    }
    else if (strcmp(stateBuffer, "IP GPRSACT") == 0)
    {
        cipstatus = IPGprsAct;
    }
    else if (strcmp(stateBuffer, "IP STATUS") == 0)
    {
        cipstatus = IPStatus;
    }
    else if (strcmp(stateBuffer, "PDP DEACT") == 0)
    {
        cipstatus = PDPDeact;
    }
    else if (strcmp(stateBuffer, "IP CONFIG") == 0) //rare
    {
        cipstatus = IPConfig;
    }
    else if (strcmp(stateBuffer, "IP PROCESSING") == 0) //rare
    {
        cipstatus = IPProcessing;
    }
}

CIPSTATUS Modem::GetCIPSTATUS()
{
    RefreshCIPSTATUS();
    return cipstatus;
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
    _modemPort->write(cmd); //write command to stream
    _modemPort->write(endline); //write endline character to stream to end stream
    _modemPort->flush();
    delay(1000); //not sure why this delay is here
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

void Modem::trimChar(char inputBuffer[], char outputBuffer[], int outputBufferSize, int startIndex, int endIndex)
{
    char tempBuffer[outputBufferSize]; //create temporary buffer to store trimmed array
    int tempIndex = 0;

    if (startIndex == 0 && endIndex == 0) //means nothing must get trimmed
    {
        strcpy(outputBuffer, inputBuffer); 
    }
    else if (startIndex == 0 && endIndex > 0) //means end must get trimmed
    {
        for (int i = 0; i < strlen(inputBuffer); i++)
        {
            tempBuffer[tempIndex] = inputBuffer[i]; //setting value of new char array

            //increase index
            tempIndex++;
            if (tempIndex == outputBufferSize - 1) //means output buffer is full
            {
                break;
            }
            if (tempIndex == endIndex) //reached the index at which to stop at
            {
                break;
            }

        }
        tempBuffer[tempIndex] = '\0'; //add null character to end of array when at end of input or reached output size

        strcpy(outputBuffer, tempBuffer);


        return;
    }
    else if (startIndex > 0  && endIndex == 0) //means beginning must get trimmed
    {
        //_devicePort->write("Trimming Beginning:\n");
        for (int i = startIndex; i < strlen(inputBuffer); i++)
        {
            //printing to serial for debugging
            /*char indexChar[4];
            itoa(i, indexChar, 10);
            _devicePort->write("Char[");
            _devicePort->write(indexChar);
            _devicePort->write("]: ");
            _devicePort->write(inputBuffer[i]);
            _devicePort->write("\n");*/

            tempBuffer[tempIndex] = inputBuffer[i]; //setting value of new char array
            
            //increase index
            tempIndex++;
            if (tempIndex == outputBufferSize - 1) //means output buffer is full
            {                
                break;
            }
        }
        tempBuffer[tempIndex] = '\0'; //add null character to end of array when at end of input or reached output size
        
        strcpy(outputBuffer, tempBuffer);
        return;
    }
    else if (startIndex > 0 && endIndex > 0) //means some of the end and some of the beginnning must get trimmed
    {
        return;
    }
}

void Modem::SendCmdAndWait(const char* cmd, char returnBuffer[], int returnBufferSize, bool removeOKFromResponse, bool printResponse)
{
    //clear modemportBuffer before doing all this

    //send cmd
    SendAT(cmd);
    //now wait 

    //setting up entire response buffer
    unsigned int bufferLen = 100;
    char responseBuffer[bufferLen];
    int responseIndex = 0;

    //retrieve entire response and insert it into buffer
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

    //setting up buffer to return
    int responseOnlyBufferSize = returnBufferSize;
    char responseOnly[responseOnlyBufferSize];

    removeCmd(responseBuffer, cmd, responseOnly, responseOnlyBufferSize);
    
    strcpy(returnBuffer, responseOnly);

    //if print response bool is true, print response to serialMon 
    if (printResponse)
    {
        _devicePort->write("\nResponse of Cmd:\n");
        _devicePort->write(responseOnly);
        _devicePort->write(endline);
    }
}






