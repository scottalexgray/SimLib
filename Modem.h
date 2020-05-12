/*
	Modem.h - Library for using a SIM800L modem
	Created by Scott A. Gray 2020
	Released into the public domain
*/
#ifndef Modem_h
#define Modem_h


#define DEFAULT_TIMEOUT     5



#include <Wire.h>
#include <Arduino.h>

class Modem
{
public:

    /*
    *
    *
    */


    Modem(HardwareSerial& modemPort, Stream& devicePort, char board[], char APN[], char PIN[]);

    int CIPSTATUS();


    /* send AT command to SIM800 module
     *  @param cmd  command array which will be send to GPRS module
     */
    void sendCmd(const char* cmd);

    /*send "AT" to SIM800 module
     */
    int sendATTest(void);


    /*  check SIM800 module response before time out
     *  @param  *resp   correct response which SIM800 module will return
     *  @param  *timeout    waiting seconds till timeout
     *  @returns
     *      0 on success
     *      -1 on error
     */
    int waitForResp(const char* resp, unsigned timeout);

    /** send AT command to GPRS module and wait for correct response
     *  @param  *cmd    AT command which will be send to GPRS module
     *  @param  *resp   correct response which GPRS module will return
     *  @param  *timeout    waiting seconds till timeout
     *  @returns
     *      0 on success
     *      -1 on error
     */
    int sendCmdAndWaitForResp(const char* cmd, const char* resp, unsigned timeout);






private:
	char APN[];
	char PIN[];
	HardwareSerial *_modemPort;
    Stream* _devicePort;


};



















#endif
