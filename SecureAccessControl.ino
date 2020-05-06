/* Error Codes
 *  3 - Failed to initialise SD Module
*   4 - failed to write to Log 
*/
#define AccessGrantedTime 3000 // Time to keep the door unlocked for in milliseconds
#define AccessDeniedTime 1000 // Time to wait after an incorrect unlock attempt in milliseconds
// Setting up the NeoPixel Strip
#include <Adafruit_NeoPixel.h>
#define NeoPixelPin 2 // Which pin on the Arduino is connected to the NeoPixels?
#define NumNeoPixels 1 // How many NeoPixels are attached to the Arduino?
#define NeoPixelNotify 0 // Determine which LED to use as a notification LED
Adafruit_NeoPixel pixels(NumNeoPixels, NeoPixelPin, NEO_GRB + NEO_KHZ800);
static uint32_t Red   = pixels.Color(255,   0,   0);  // Setting easy name for common colours for the LED's 
static uint32_t Green = pixels.Color(  0, 255,   0);  // use pixels.setPixelColor(LED#, Color); to set a LED color (Red/ Green/ Blue/ Off)
static uint32_t Blue  = pixels.Color(  0,   0, 255);  // pixels.clear(); will set all LEDs to off
static uint32_t Off   = pixels.Color(  0,   0,   0);  // and then use pixels.show();
// Setting up the RFID and SD modules
#include <SPI.h> // for the RFID and SD card module
#include <SD.h> // for the SD card
#include <MFRC522.h> // for the RFID
#define CS_RFID 8    // define pins for RFID
#define RST_RFID 9    // define pins for RFID
#define CS_SD 4   // define select pin for SD card module
MFRC522 rfid(CS_RFID, RST_RFID); // Instance of the class for RFID
File myFile; // Create a file to store the data
String LogFile = "log.txt"; // name of Log File
String uidString;// Variable to hold the tag's UID

//*****************************************************************************************//

void setup() {
  // Setup NeoPixels
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(30); // Set BRIGHTNESS (max = 255)
  pixels.clear(); // Set all pixel colors to 'off'
  //pixels.show();
  
  Serial.begin(9600); // Init Serial port
//  while(!Serial); // Wait for computer serial box
  SPI.begin(); // Init SPI bus for RFID and SD modules
  
  // Setup RFID module
  rfid.PCD_Init(); // Init MFRC522
  //rfid.PCD_SetAntennaGain(rfid.RxGain_max); // increases the range of the RFID module

  // Setup SD card module
  Serial.print("Initializing SD card...");
  if(!SD.begin(CS_SD)) {
    Serial.println("initialization failed!");
    ErrorCode(3);  
  }
  Serial.println("initialization done.");
  FlashNeoPixel(NeoPixelNotify, 1, 250, Red);
  FlashNeoPixel(NeoPixelNotify, 1, 250, Blue);
  FlashNeoPixel(NeoPixelNotify, 1, 250, Green);
}

//*****************************************************************************************//

void loop() {
  //look for new cards
  if(rfid.PICC_IsNewCardPresent()) {
    readRFID();
    if(uidString != "0000"){  // ignore if it didn't read the card properly
      pixels.setPixelColor(NeoPixelNotify, Blue);
      pixels.show();
      verifyRFIDCard();
      ResetRFIDReadVariables();
    }
  }
  delay(10);
}

//*****************************************************************************************//

void readRFID() {
  rfid.PICC_ReadCardSerial();
  Serial.print("Tag UID Scanned: ");
  uidString = String(rfid.uid.uidByte[0], HEX) + String(rfid.uid.uidByte[1], HEX) + 
    String(rfid.uid.uidByte[2], HEX) + String(rfid.uid.uidByte[3], HEX);
  Serial.println(uidString);
  delay(100);
}

//*****************************************************************************************//

void verifyRFIDCard(){
  if(SD.exists(uidString + ".txt")){
    AccessGranted(AccessGrantedTime);
  }
  else{
    AccessDenied(AccessDeniedTime);
  }
  Serial.println("");
}

//*****************************************************************************************//

void LogToSD(String DataToLogToSD){
        // Open file
      myFile=SD.open(LogFile, FILE_WRITE);
      // If the file opened ok, write to it
      if (myFile) {
        Serial.println("Log opened ok");
        myFile.println(DataToLogToSD);
        Serial.println("sucessfully written to log");
        myFile.close();
        Serial.println("Log closed");
      }
      else {
        Serial.println("error opening " + LogFile);
        ErrorCode(4);  
      }
  }

//*****************************************************************************************//
  
void AccessGranted(int TimeDoorOpen){
  pixels.setPixelColor(NeoPixelNotify, Green);
  pixels.show();
  // activate relay
  Serial.println("Access Granted");
  LogToSD(uidString + ", Access Granted");
  delay(TimeDoorOpen);
  // de-activate relay
  pixels.setPixelColor(NeoPixelNotify, Off);
  pixels.show();
}

void AccessDenied(int LockoutTime){
  pixels.setPixelColor(NeoPixelNotify, Red);
  pixels.show();
  Serial.println("Access Denied");
  LogToSD(uidString + ", Access Denied");
  delay(LockoutTime);
  pixels.setPixelColor(NeoPixelNotify, Off);
  pixels.show();
}

//*****************************************************************************************//

void ErrorCode(int ErrorNum){
  Serial.println("Error Code: " + ErrorNum);
  pixels.clear();
  delay(500);
  FlashNeoPixel(NeoPixelNotify, 2, 500, Blue);
  FlashNeoPixel(NeoPixelNotify, ErrorNum, 500, Red);
  FlashNeoPixel(NeoPixelNotify, 2, 500, Blue);
}


void FlashNeoPixel(int PixelNum, int NumberOfTimesToFlah, int FlashDelayTime, uint32_t FlashColor){
  
  for(int i = 0; i < NumberOfTimesToFlah; i++){
    pixels.setPixelColor(PixelNum, FlashColor);
    pixels.show();
    delay(FlashDelayTime);
    pixels.setPixelColor(PixelNum, Off);
    pixels.show();
    delay(FlashDelayTime);
  }
}

//*******************************************************************************************//

void ResetRFIDReadVariables(){
    uidString = "0";
    rfid.uid.uidByte[0] = 00;
    rfid.uid.uidByte[1] = 00;
    rfid.uid.uidByte[2] = 00;
    rfid.uid.uidByte[3] = 00;
}
