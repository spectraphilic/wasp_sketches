/*
Send Iridium SBD message
 
 (C) John Hulth 2016
 john.hulth@geo.uio.no
 */



#include <IridiumSBD.h>
// #include <SoftwareSerial.h> //not in use


//SoftwareSerial nss(18, 19); //not in use

//IridiumSBD isbd(nss, 10); //not in use, no sleeping pin


//static const int ledPin = 13; //Arduino led thing?

void setup()
{

  PWR.setSensorPower(SENS_5V,SENS_ON); // Sets the 5V switch ON to power RockBlock


  Utils.setMuxAux2(); // Set AUX SERIAL 2RX/TX in waspmote
  beginSerial(19200,1); // Set Boudrate to 19200
  serialFlush(1);
  delay(100);

  int signalQuality = -1;

  // pinMode(ledPin, OUTPUT);

  // Serial.begin(115200);
  // nss.begin(19200);

  isbd.attachConsole(Serial);
  isbd.setPowerProfile(1);
  isbd.begin();

  int err = isbd.getSignalQuality(signalQuality);
  if (err != 0)
  {
    USB.print("SignalQuality failed: error ");
    USB.println(err);
    return;
  }

  USB.print("Signal quality is ");
  USB.println(signalQuality);

  err = isbd.sendSBDText("Hello, world!");
  if (err != 0)
  {
    USB.print("sendSBDText failed: error ");
    USB.println(err);
    return;
  }

  USB.println("Hey, it worked!");
  USB.print("Messages left: ");
  USB.println(isbd.getWaitingMessageCount());
}

void loop()
{
  digitalWrite(ledPin, HIGH);
}

bool ISBDCallback()
{
  digitalWrite(ledPin, (millis() / 1000) % 2 == 1 ? HIGH : LOW);
  return true;
}



