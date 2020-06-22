/***************************************************
DFPlayer - A Mini MP3 Player For Arduino
 <https://www.dfrobot.com/index.php?route=product/product&product_id=1121>

 ***************************************************
 This example shows the basic function of library for DFPlayer.

 Created 2016-12-07
 By [Angelo qiao](Angelo.qiao@dfrobot.com)

 GNU Lesser General Public License.
 See <http://www.gnu.org/licenses/> for details.
 All above must be included in any redistribution
 ****************************************************/

/***********Notice and Trouble shooting***************
 1.Connection and Diagram can be found here
 <https://www.dfrobot.com/wiki/index.php/DFPlayer_Mini_SKU:DFR0299#Connection_Diagram>
 2.This code is tested on Arduino Uno, Leonardo, Mega boards.
 ****************************************************/

#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

const int volumePin = A4;
const int buttonsPin = A5;

int currentFolder = -1;
int currentFile = 1;

int numberOfFiles[] = {0, 0, 0, 0, 0, 0, 0};

unsigned long lastButtonEvent;
unsigned long lastVolumeEvent;

int currentVolume = -1;

void setup()
{
  mySoftwareSerial.begin(9600);
  Serial.begin(115200);
  delay(5000);
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }

  pinMode(buttonsPin, INPUT);

  Serial.println(F("DFPlayer Mini online."));

  for (int i=1;i<8;i++) {
    numberOfFiles[i-1] = myDFPlayer.readFileCountsInFolder(i);
    Serial.println(numberOfFiles[i-1]);
  }
  
 
  myDFPlayer.volume(5);  //Set volume value. From 0 to 30

  // play last song from eeprom here ! ! !
  
}

// Prueft welcher Knopf gedrueckt wurde
// Gibt -1 zurueck falls kein Knopf gedrueckt wurde
int checkButtonPressed() {
  int value = 0;
  value = analogRead(buttonsPin);  
  if (value > 435) return 3;
  if (value > 250) return 2;
  if (value > 140) return 1;
  return -1;
}

// Gibt true zurueck falls ein neuer Knopf gedrueckt wurde
boolean checkAndSetButtonPressed() {
  if (millis() - lastButtonEvent < 100) {
    return;
  }
  
  int newButtonPressed = checkButtonPressed();
  if (newButtonPressed == -1){
    return false;
  }
  
  if (currentFolder == newButtonPressed) {      
    if (currentFile < numberOfFiles[currentFolder-1]) {
      currentFile++;
    } else {
      currentFile = 1;       
    }
    Serial.println("playing song "+String(currentFile)+" in folder"+String(currentFolder));
    myDFPlayer.playFolder(currentFolder, currentFile);
  } else { 
    currentFolder = newButtonPressed;
    Serial.println("playing folder "+String(currentFolder));
    myDFPlayer.playFolder(currentFolder, 1);
  }
  lastButtonEvent = millis();
  //saveSongAndPositionInEeprom(0);
  return true;
}

void checkAndSetVolume() {
  if (millis() - lastVolumeEvent < 100){
    return;
  }
  int volume = analogRead(volumePin);
  volume = map(volume, 1023, 10, 0, 10);

  if (volume != currentVolume) {
    if (volume == 0) {
      myDFPlayer.pause();
      currentVolume = 0;
      return;
    }
    if (currentVolume == 0) {
      myDFPlayer.start();
    }
    
    Serial.println(volume);
    myDFPlayer.volume(volume);  //Set volume value. From 0 to 30
    currentVolume = volume;
  }
  lastVolumeEvent = millis();
}

void loop()
{
  checkAndSetButtonPressed();
  checkAndSetVolume();
  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}
