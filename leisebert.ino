/*

	Leisebert - a DIY Hoerbert for Headphones

	Author: June 2020 M. Schmutz

*/

#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

void printDetail(uint8_t type, int value);


// analog pins for buttons and volume
const int volumePin = A4;
const int buttonsPin = A5;

// current status variables
int numberOfFiles[] = {0, 0, 0, 0, 0, 0, 0};
int currentFolder = -1;
int currentFile = 1;
int currentVolume = -1;
int lastButtonPressed;

// event timing variables
unsigned long lastButtonEvent;
unsigned long lastVolumeEvent;



void setup()
{
  mySoftwareSerial.begin(9600);
  Serial.begin(115200);
  delay(5000);
  Serial.println();
  Serial.println("Leisebert - a DIY Hoerbert for Headphones");
  Serial.println("Initializing DFPlayer ... ");

  if (!myDFPlayer.begin(mySoftwareSerial)) 
    Serial.println("Unable to begin:");
    Serial.println("1.Please recheck the connection!");
    Serial.println("2.Please insert the SD card!");
    // TODO reset here ! ! !
	while(true);
  }

  pinMode(buttonsPin, INPUT);

  // get number of files per folder
  for (int i=1;i<8;i++) {
    numberOfFiles[i-1] = myDFPlayer.readFileCountsInFolder(i);
    Serial.println(numberOfFiles[i-1]);
  }
 
  // set volume value
  checkAndSetVolume();

  // play last song from eeprom here ! ! !
 
  Serial.println("Leisebert online.");
 
}

// check value of buttons and return folder number
int checkButtonPressed() {
  int value = 0;
  value = analogRead(buttonsPin);  
  if (value > 435) return 3;
  if (value > 250) return 2;
  if (value > 140) return 1;
  return -1;
}

// check if button has been pressed
boolean checkAndSetButtonPressed() {
	 
  if (millis() - lastButtonEvent < 100) { // skip if lastButtonEvent is too close
    return;
  }
  
  int newButtonPressed = checkButtonPressed(); // get value of button
  
  if (newButtonPressed == -1 or newButtonPressed == lastButtonPressed){ // skip if no button pressed or button has not been released since last check
    return false;
  }
  
  if (currentFolder == newButtonPressed) { // check what to do if button pressed
    
	if (currentFile < numberOfFiles[currentFolder-1]) { // play next file if not at the end
      currentFile++;
    } else { // play first file if at the end of folder
      currentFile = 1;       
    }
	
    Serial.println("playing song "+String(currentFile)+" in folder"+String(currentFolder));
    myDFPlayer.playFolder(currentFolder, currentFile); // play the file
  } else { // play first file from new folder
    currentFolder = newButtonPressed;
    Serial.println("playing folder "+String(currentFolder));
    myDFPlayer.playFolder(currentFolder, 1); // play the file
  }
  
  lastButtonEvent = millis();
  lastButtonPressed = newButtonPressed;
  
  // TODO save to eeprome here ! ! !
  //saveSongAndPositionInEeprom(0);
  return true;
}

// set volume
void checkAndSetVolume() {

  if (millis() - lastVolumeEvent < 100){ // skip if lastVolumeEvent is too close
    return;
  }
  
  int volume = analogRead(volumePin);
  volume = map(volume, 1023, 10, 0, 10);

  if (volume != currentVolume) { // check what to do if volume changed
    if (volume == 0) { // pause player if volume == 0
      myDFPlayer.pause();
      currentVolume = 0;
      return;
    }
    if (currentVolume == 0) { // resume player if volume turned up again
      myDFPlayer.start();
    }
    
    Serial.println(volume);
    myDFPlayer.volume(volume);  //Set volume value. From 0 to 30
    currentVolume = volume;
  }
  lastVolumeEvent = millis();
}

// MAIN
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
