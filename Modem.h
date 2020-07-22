/*
	Modem.h - Library for using a SIM800L modem
	Created by Scott Alexander Gray - 2020	
*/
#include <Wire.h>
#include <Arduino.h>


enum CGREG
{
	NotRegistered = 0,
	HomeRegistered = 1,
	Searching = 2,
	RegistrationDenied = 3,
	Roaming = 5
};




class Modem
{
public:


    //Constructor
    Modem(HardwareSerial& modemSerialConn, HardwareSerial& deviceSerialConn, int bRate);
	
	void Init(); //initialise modem with power and comms etc
	void DeInit();
	

	void SetSimSettings(const char* apn, const char* pin); //set the APN and PIN of simcard
	void ConnectGPRS(); //connect to the GPRS Network (IP INITIAL -> IP STATUS)


private:

	//Comands used to tranverse the different CIPSTATUS states

	void SetCFUN(int val); //AT+CFUN=val //Sets modem functionality

	//state = IP INITIAL

	void SetCSTT(const char* apn); //AT+CSTT=APN //sets APN to use with the simcard, i.e. "internet"

	void SetCPIN(char pin[]); //AT+CPIN=pin //set the pincode for the simcard	

	void GetCPIN(); //AT+CPIN? //gets the pin status of the simcard(does it still need a pin)

	void GetCGREG(CGREG* returnState); //AT+CGREG? //is the sim registered on the network?	

	void SetCGATT(int val); //AT+CGATT=val //Attach to network 

	void SetCGDCONT(int PDPNum, char ipMode[], char APN[]);//AT+CGDCONT=PDPNum,ipMode,APN //Set the PDP context

	/*activate PDP context
	* Command = AT+CGACT=state,cid
	* state = 1 : Activate
	* state = 0 : Deactivate
	* cid : PDP number (if omitted first one is used)
	*/
	void SetCGACT(int state, int cid);

	/*Check if there are any errors
	* Command = AT+CEER?
	*check AT command manual for error numbers
	*/	
	void GetCEER();
	
	//state = IP START

	/*Bring up wireless connection with GPRS or CSD
	* Command = AT+CIICR
	*/
	void SetCIICR();

	//state = IP GPRSACT

	/*Get or Aquire local IP address
	* Command = AT+CIFSR
	*/
	void SetCIFSR();

	//state = IP STATUS, can now register at a bearer and then do HTTP connection

	
	
	//Other Commands useful for debugging

	//AT+COPS? //get network info

	//AT+CSQ //gets signal strength

	//AT+CIPSTATUS //Query current connection status and status of modem
	


	void WaitForResponse(int bufferLen, char response[], unsigned long timeoutTime);

	void ModemRDYCheck();

	void SendAT(const char* cmd);

	void removeCmd(char inputBuffer[], const char* cmd, char returnBuffer[], int returnBufferSize);

	void SendCmdAndWait(const char* cmd);

	



	
	bool RDY = false;


	int baudRate;

	const char* APN;
	const char* PIN;
	HardwareSerial *_modemPort;
    HardwareSerial* _devicePort;
	//CGREG cgreg;



};