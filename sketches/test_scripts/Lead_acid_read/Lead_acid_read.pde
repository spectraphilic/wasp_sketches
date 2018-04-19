/*
    Test-code to read lead acid battery voltage on Lemming Power board

    (JH)
*/

// Put your libraries here (#include ...)

int analog5;
float LeadAcid_V;

float R1 = 10;  // 10k resistor
float R2 = 2.2; // 2k2 resistor

void setup()
{
  USB.ON();
  USB.println(F("------------------------------------------"));
  USB.println(F("Analog output (0 - 3.3V): from 0 to 1023"));
  USB.println(F("------------------------------------------"));


  pinMode(16, OUTPUT); //ANALOG3 12V_SW ON/OFF SWITCH
  pinMode(17, OUTPUT); //ANALOG4 Lead-Acid meassurement ON/OFF SWITCH
  pinMode(ANALOG5, INPUT);  //ANALOG5 Read data

}


void loop()
{
  digitalWrite(16, HIGH);
  digitalWrite(17, HIGH);
  delay(1000);

  analog5 = analogRead(ANALOG5);

  digitalWrite(16, LOW);
  digitalWrite(17, LOW);

  LeadAcid_V = analog5  * (R1 + R2) / R2 * 3.3 / 1023 ;

  USB.print(F("ANALOG5: "));
  USB.println(analog5);
  USB.print(F("Battery voltage: "));
  USB.println(LeadAcid_V);

  delay(1000);

}
