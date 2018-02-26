/*
    ------ Waspmote Pro Code Example --------

*/

// Put your libraries here (#include ...)

void setup()
{
  // set DIGITAL1 pin as output
  pinMode(14, OUTPUT);
  pinMode(DIGITAL2 , OUTPUT);

}


void loop()
{
  // set DIGITAL3  LOW

  USB.print("OFF...");
  digitalWrite(14, LOW);
  digitalWrite(DIGITAL2, HIGH);
  delay(5000);

  // set DIGITAL3  HIGH
  USB.println("ON");
  digitalWrite(14, HIGH);
  digitalWrite(DIGITAL2, LOW);

  delay(5000);
}
