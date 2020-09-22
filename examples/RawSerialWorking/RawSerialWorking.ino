/*
 * Code Written By: Scott Gray (GRYSCO003)
 * Date Started: 14 May 2020 
 * 
 * This code is meant to use raw arduino functions to connect to device and modem
 * As well as send AT commands and receive responses, without GSM libraries
 * 
 */



#include "mail-google-com.h" //file that contains the certificate
#define CERT_FILE "C:\\USER\\CERT.CRT" //directory in modem of certificate we want to create



// TTGO T-Call pins
//#define MODEM_RST            5
//#define MODEM_PWKEY          4
//#define MODEM_POWER_ON       23
//#define MODEM_TX             27
//#define MODEM_RX             26
//#define I2C_SDA              21
//#define I2C_SCL              22

//#define endline "\r\n\0"
#define modemBaudRate 115200
#define serialBaudRate 115200

// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to SIM800 module)
#define SerialAT Serial1

#include <Wire.h>
#include <Modem.h>

//Simcard Configuration Settings
const char APN[] = "internet";
const char PIN[] = "";

//Modem State Variables
CIPSTATUS cipState;
CGREG cgregState;

//Modem modem(SerialAT, SerialMon);
Modem modem(SerialAT, SerialMon, modemBaudRate);

void setup() 
{
  SerialMon.begin(serialBaudRate);
  modem.Init();
  modem.SetSimSettings(APN, PIN);  
  modem.ConnectGPRSBlocking(); //this is the blocking version of the GPRSConnect
  

  
  cipState = modem.GetCIPSTATUS();
  cgregState = modem.GetCGREG();
  
  SerialMon.print("\nCIPSTATUS: ");
  SerialMon.println(cipState);
}


void loop() 
{  
  
  while(true) {
    if (SerialAT.available()) {      
      SerialMon.write(SerialAT.read());
    }
    if (SerialMon.available()) {
      SerialAT.write(SerialMon.read());
    }    
    delay(0);
  }
}

void SetCert()
{
  /*

  modem.sendAT(GF("+FSCREATE=" CERT_FILE));
  if (modem.waitResponse() != 1) return;

  const int cert_size = sizeof(cert);

  modem.sendAT(GF("+FSWRITE=" CERT_FILE ",0,"), cert_size, GF(",10"));
  if (modem.waitResponse(GF(">")) != 1) {
    return;
  }

  for (int i = 0; i < cert_size; i++) {
    char c = pgm_read_byte(&cert[i]);
    modem.stream.write(c);
  }

  modem.stream.write(GSM_NL);
  modem.stream.flush();

  if (modem.waitResponse(2000) != 1) return;
  
  modem.sendAT(GF("+SSLSETCERT=\"" CERT_FILE "\""));
  if (modem.waitResponse() != 1) return;
  if (modem.waitResponse(5000L, GF(GSM_NL "+SSLSETCERT:")) != 1) return;
  const int retCode = modem.stream.readStringUntil('\n').toInt();


  SerialMon.println();
  SerialMon.println();
  SerialMon.println(F("****************************"));
  SerialMon.print(F("Setting Certificate: "));
  SerialMon.println((0 == retCode) ? "OK" : "FAILED");
  SerialMon.println(F("****************************"));
*/
}
