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
	bool isInitialized();

	void DeInit();
	

	void Connect();


private:

	void SendAT(const char* cmd);

	void removeCmd(char inputBuffer[], const char* cmd, char returnBuffer[], int returnBufferSize);

	void SendCmdAndWait(const char* cmd);

	void GetCGREG(CGREG* returnState);


	bool initializedComms = false;
	int baudRate;

	char APN[];
	char PIN[];
	HardwareSerial *_modemPort;
    HardwareSerial* _devicePort;
	//CGREG cgreg;



};