#include <Modem.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <MPU9250.h>

//GSM---------------------------------------------------------------------------
#define GSMPowerPin 23
#define serialBaudRate 115200
// Set serial for debug console (to Serial Monitor, default speed 115200)
HardwareSerial SerialMon(0);

#define modemBaudRate 115200
// Set serial for AT commands (to SIM800 module)
HardwareSerial SerialAT(1);
//Simcard Configuration Settings
const char APN[] = "internet";
const char PIN[] = "";

//Modem State Variables
CIPSTATUS cipState;
CGREG cgregState;

Modem modem(SerialAT, SerialMon, modemBaudRate); //Modem object

//GPS-----------------------------------------------------------------------
#define gpsBaudRate 9600
HardwareSerial SerialGPS(2);//Set serial connection for BN220 (GPS)

#define GPSPowerPin 14
#define GPS_powerCycleTime 500

#define GPSRX 18
#define GPSTX 19

TinyGPSPlus gps;

//IMU--------------------------------------------------------------------------
#define IMUPowerPin 25
#define IMU_powerCycleTime 1000

MPU9250 IMU(Wire,0x68);
int imu_status;



void setup() 
{
  SerialMon.begin(serialBaudRate); //begin serial connection to PC
  InitPinModes(); //initialize power pin modes
  
  InitGSM();
  InitGPS();   
  InitIMU();
}
void InitGSM()
{
  //Connect To GSM  
  modem.Init();
  modem.SetSimSettings(APN, PIN);  
  modem.ConnectGPRSBlocking(); //this is the blocking version of the GPRSConnect
  
  cipState = modem.GetCIPSTATUS();  
  
  SerialMon.print("\nCIPSTATUS: ");
  SerialMon.println(cipState);
}

void InitGPS()
{
  //GPS
  SerialMon.print("Turning GPS on... ");
  digitalWrite(GPSPowerPin, HIGH); //BN220 (GPS)
  delay(GPS_powerCycleTime);  
  SerialGPS.begin(gpsBaudRate, SERIAL_8N1, GPSRX, GPSTX);
  SerialMon.println("OK");
    
}

void InitIMU()
{
  //IMU
    SerialMon.print("Turned IMU on... ");
    digitalWrite(IMUPowerPin, HIGH); //MPU9250 (IMU)
    delay(IMU_powerCycleTime);

    imu_status = IMU.begin();
    if (imu_status < 0) 
    {
      SerialMon.println("IMU initialization unsuccessful");
      SerialMon.println("Check IMU wiring or try cycling power");
      SerialMon.print("Status: ");
      SerialMon.println(imu_status);
      //while(1) {}
    }
    else
    {
        SerialMon.println("OK");
    }
}

void loop() 
{ 
    PrintGSMData();
    PrintGPSData();
    PrintIMUData();
    delay(200);
}

void PrintGSMData()
{
  SerialMon.println("--------------------GSM-------------------");
  int signalStrength = modem.GetCSQ();
  
  SerialMon.println("Signal Strength: " + (String)signalStrength );

  
}

void PrintGPSData()
{
    while (SerialGPS.available() > 0) 
    {
        gps.encode(SerialGPS.read());        
    }

    if (millis() > 5000 && gps.charsProcessed() < 10)
    {
      SerialMon.println(F("No GPS detected: Trying Again"));      
    }

    
    SerialMon.println("--------------------GPS-------------------");
    SerialMon.print("LAT:");  SerialMon.print(gps.location.lat(), 6);
    SerialMon.print(", LONG:"); SerialMon.print(gps.location.lng(), 6);
    SerialMon.print(", ALT:");  SerialMon.print(gps.altitude.meters());
    SerialMon.print(", Sats: " + (String)gps.satellites.value() + "\n");
}

void PrintIMUData()
{
  SerialMon.println("--------------------IMU-------------------");
  IMU.readSensor();  
  // display the data
  SerialMon.print("AccelX: ");
  SerialMon.print(IMU.getAccelX_mss(),6);
  SerialMon.print("");
  SerialMon.print("AccelY: ");  
  SerialMon.print(IMU.getAccelY_mss(),6);
  SerialMon.print("");
  SerialMon.print("AccelZ: ");  
  SerialMon.println(IMU.getAccelZ_mss(),6);
 
  SerialMon.print("GyroX: ");
  SerialMon.print(IMU.getGyroX_rads(),6);
  SerialMon.print("");
  SerialMon.print("GyroY: ");  
  SerialMon.print(IMU.getGyroY_rads(),6);
  SerialMon.print("");
  SerialMon.print("GyroZ: ");  
  SerialMon.println(IMU.getGyroZ_rads(),6);
 
  SerialMon.print("MagX: ");  
  SerialMon.print(IMU.getMagX_uT(),6);
  SerialMon.print("");  
  SerialMon.print("MagY: ");
  SerialMon.print(IMU.getMagY_uT(),6);
  SerialMon.print("");
  SerialMon.print("MagZ: ");  
  SerialMon.println(IMU.getMagZ_uT(),6);
 
  SerialMon.print("Temperature in C: ");
  SerialMon.println(IMU.getTemperature_C(),6);
  SerialMon.println();
}





void InitPinModes()
{
  pinMode(GPSPowerPin,OUTPUT);//BN220 (GPS)
  pinMode(GSMPowerPin,OUTPUT);//SIM800L (GSM)
  pinMode(IMUPowerPin,OUTPUT);//MPU9250 (IMU)  
}
