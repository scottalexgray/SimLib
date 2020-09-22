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
	Unknown = 4,
	Roaming = 5
};

enum CIPSTATUS
{
	IPInitial = 0,
	IPStart = 1,
	IPConfig = 2,
	IPGprsAct = 3,
	IPStatus = 4,
	IPProcessing = 5,
	PDPDeact = 9
};




class Modem
{
public:

	

    //Constructor
    Modem(HardwareSerial& modemSerialConn, HardwareSerial& deviceSerialConn, int bRate);
	
	void Init(); //initialise modem with power and comms etc
	void DeInit();
	

	void SetSimSettings(const char* apn, const char* pin); //set the APN and PIN of simcard
	void ConnectGPRSBlocking(); //connect to the GPRS Network (IP INITIAL -> IP STATUS)

	//Other Commands useful for debugging
	void GetCSQ();//AT+CSQ //gets signal strength

	enum CGREG GetCGREG(); //AT+CGREG? //is the sim registered on the network?

	void SetCGATT(int val); //AT+CGATT=val //Attach to network 

	enum CIPSTATUS GetCIPSTATUS();


private:

	//Comands used to tranverse the different CIPSTATUS states

	void SetCFUN(int val); //AT+CFUN=val //Sets modem functionality

	//state = IP INITIAL

	void SetCSTT(const char* apn); //AT+CSTT=APN //sets APN to use with the simcard, i.e. "internet"

	void SetCPIN(const char* pin); //AT+CPIN=pin //set the pincode for the simcard		

	void GetCPIN(); //AT+CPIN? //gets the pin status of the simcard(does it still need a pin)

		

	

	void SetCGDCONT(int PDPNum, const char* ipMode, const char* apn);//AT+CGDCONT=PDPNum,ipMode,APN //Set the PDP context

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

	void GetCOPS();//AT+COPS? //get network info

	
	void RefreshCGREG();
	void RefreshCIPSTATUS();//AT+CIPSTATUS //Query current connection status and status of modem
	


	void WaitForResponse(int bufferLen, char response[], unsigned long timeoutTime); //wait for a response from modem. At the moment only used for startup

	void ModemRDYCheck();

	void SendAT(const char* cmd); //send command

	//remove command prefix from response buffer
	void removeCmd(char inputBuffer[], const char* cmd, char returnBuffer[], int returnBufferSize); 

	//trim function
	void trimChar(char inputChar[], char outputChar[], int outputBufferSize, int startIndex = 0, int endIndex = 0);


	void SendCmdAndWait(const char* cmd, char returnBuffer[], int returnBufferSize, bool removeOKFromResponse, bool printResponse); //send command, wait for response and print response

	



	
	bool RDY = false;


	int baudRate;

	const char* APN;
	const char* PIN;
	HardwareSerial *_modemPort;
    HardwareSerial* _devicePort;


	CGREG cgreg;
	CIPSTATUS cipstatus;



};