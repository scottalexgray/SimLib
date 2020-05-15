/*
	Modem.h - Library for using a SIM800L modem
	Created by Scott A. Gray 2020
	
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
	
	void Init();
	void DeInit();
	

	void ConnectGPRS();


private:


	void SetCFUN(int val);

	void WaitForResponse(int bufferLen, char response[], unsigned long timeoutTime);

	void ModemRDYCheck();

	void SendAT(const char* cmd);

	void removeCmd(char inputBuffer[], const char* cmd, char returnBuffer[], int returnBufferSize);

	void SendCmdAndWait(const char* cmd);

	void GetCGREG(CGREG* returnState);



	
	bool RDY = false;


	int baudRate;

	char APN[];
	char PIN[];
	HardwareSerial *_modemPort;
    HardwareSerial* _devicePort;
	//CGREG cgreg;



};