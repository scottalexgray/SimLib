/*
	Modem.h - Library for using a SIM800L modem
	Created by Scott A. Gray 2020
	Released into the public domain
*/
#ifndef Modem_h
#define Modem_h

//#define SerialAT Serial1

#include <Wire.h>
#include <Arduino.h>
//#include <SoftwareSerial.h>
class Modem
{
public:
	Modem(HardwareSerial& port, char board[],char APN[], char PIN[]);
	void connect();
	void CIPSTATUS();
	void BearerStatus();
	bool isConnected();

private:
	char APN[];
	char PIN[];
	HardwareSerial *_serialPort;
	


};



















#endif
