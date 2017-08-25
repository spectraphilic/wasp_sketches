
/*
    ------ Waspmote Pro Code Example --------

Code to test the photodiodes
Simon Filhol

*/

// Put your libraries here (#include ...)



// define folder and file to store data
char filename[] = "DATA.TXT";

// buffer to write into Sd File
uint8_t sd_answer;

const int pinDigi47 = DIGITAL1;
const int pinDigi100 = DIGITAL2;
const int pinDigi220 = DIGITAL3;
const int pinDigi330 = DIGITAL4;
const int pinDigi470 = DIGITAL5;
const int pinDigi680 = DIGITAL6;
const int pinDigi2200 = DIGITAL7;

const int pinRead47 = ANALOG1;
const int pinRead100 = ANALOG2;
const int pinRead220 = ANALOG3;
const int pinRead330 = ANALOG4;
const int pinRead470 = ANALOG5;
const int pinRead680 = ANALOG6;
const int pinRead2200 = ANALOG7;

int val47;
int val100;
int val220;
int val330;
int val470;
int val680;
int val2200;
int val3300;




void setup() // put your setup code here, to run once:
{
  // open USB port
  USB.ON();

  // Set SD ON
  RTC.ON();
  SD.ON();

  USB.println(RTC.getTime()); // Get RTC time/date and print to USB
  USB.println("here");
  pinMode(DIGITAL1 , OUTPUT); // Ext for SW_in
  digitalWrite(DIGITAL1, LOW); // Ext. on
  pinMode(DIGITAL2, OUTPUT); // Ext for SW_in
  digitalWrite(DIGITAL2, LOW); // Ext. on
  pinMode(DIGITAL3, OUTPUT); // Ext for SW_in
  digitalWrite(DIGITAL3, LOW); // Ext. on
  pinMode(DIGITAL4, OUTPUT); // Ext for SW_in
  digitalWrite(DIGITAL4, LOW); // Ext. on
  pinMode(DIGITAL5, OUTPUT); // Ext for SW_in
  digitalWrite(DIGITAL5, LOW); // Ext. on
  pinMode(DIGITAL6, OUTPUT); // Ext for SW_in
  digitalWrite(DIGITAL6, LOW); // Ext. on
  pinMode(DIGITAL7, OUTPUT); // Ext for SW_in
  digitalWrite(DIGITAL7, LOW); // Ext. on


  // create path


  // Create file for Waspmote Frames
  sd_answer = SD.create(filename);
  USB.println("here");
  if (sd_answer == 1){
    USB.println(F("log created"));
    SD.appendln(filename, "YY,MM,DD,hh,mm,ss,SW_47,SW_100,SW_220,SW_330,SW_470,SW_680,SW_2200");
  }
  else{
    USB.println(F("log not created"));
  }
  SD.OFF();
}



void loop()  // put your main code here, to run repeatedly:
{

  PWR.deepSleep("00:00:05:00", RTC_OFFSET, RTC_ALM1_MODE1, ALL_OFF); // 10 min for sampling

  USB.ON();
  RTC.ON();
  USB.println(F("==== Wasp awake ===="));

  val47 = readPhotodiode(pinDigi47, pinRead47);
  val100 = readPhotodiode(pinDigi100, pinRead100);
  val220 = readPhotodiode(pinDigi220, pinRead220);
  val330 = readPhotodiode(pinDigi330, pinRead330);
  val470 = readPhotodiode(pinDigi470, pinRead470);
  val680 = readPhotodiode(pinDigi680, pinRead680);
  val2200 = readPhotodiode(pinDigi2200, pinRead2200);

  write2SD(filename, val47, val100, val220, val330, val470, val680, val2200);
}

void write2SD(char file[], int SW_47, int SW_100, int SW_220, int SW_330, int SW_470, int SW_680, int SW_2200){
  RTC.ON();
  RTC.getTime();
  char dataStr[200];

  // Create data string to write on SD file
  sprintf(dataStr, "%u,%u,%u,%u,%u,%u,%d,%d,%d,%d,%d,%d,%d", RTC.year, RTC.month, RTC.date, RTC.hour, RTC.minute, RTC.second, SW_47, SW_100, SW_220, SW_330, SW_470, SW_680, SW_2200);
  USB.println(dataStr);
  // Appends 'dataStr' at the end of the file
  SD.ON();
  sd_answer = SD.appendln(file, dataStr);
  SD.OFF();

  if (sd_answer == 1 )
  {
    USB.print("Appended 'dataStr' into SD-file ");
    USB.println(dataStr);
  }
  else
  {
    USB.println(F("Append 'dataStr' into SD-file: Error!!!"));
  }
  delay(100);

}

int readPhotodiode(int pinON, int pinRead){

  // Reading Solar short wave incoming connected to Digital- and Analog 2
  digitalWrite(pinON, HIGH); // Ext. on
  delay(100);
  int SW_in = analogRead(pinRead);
  delay(10);
  digitalWrite(pinON, LOW); // Ext. off
  delay(100);
  return SW_in;
}


