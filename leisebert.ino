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
const int busyPin = A1;
const int volumePin = A2;
const int buttonsPin = A3;

// current status variables
int numberOfFiles[] = {0, 0, 0, 0, 0, 0, 0};
int currentFolder = -1;
int currentFile = 1;
int currentVolume = -1;
int lastButtonPressed;
int newButtonPressed;

// event timing variables
unsigned long lastButtonEvent;
unsigned long lastVolumeEvent;
unsigned long lastBusyEvent;

void setup()
{
  mySoftwareSerial.begin(9600);
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("Leisebert - a DIY Hoerbert for Headphones");
  Serial.println("Initializing DFPlayer ... ");

  if (!myDFPlayer.begin(mySoftwareSerial)) { 
    Serial.println("Unable to begin:");
    Serial.println("1.Please recheck the connection!");
    Serial.println("2.Please insert the SD card!");
    // TODO reset here ! ! !
    while(true);
  }

  pinMode(buttonsPin, INPUT);
  digitalWrite(buttonsPin, HIGH);
  
  // get number of files per folder
  for (int i=1;i<8;i++) {
    numberOfFiles[i-1] = myDFPlayer.readFileCountsInFolder(i);
    Serial.println(numberOfFiles[i-1]);
  }
 
  // set volume value
  checkAndSetVolume();

  // play last song from eeprom here ! ! !
 
  Serial.println("Leisebert online.");

   myDFPlayer.playFolder(1, 1);
 
}

// check value of buttons and return folder number
int checkButtonPressed() {
  int value = 1023;
  // take multiple samples to prevent some false readings of too high voltages
  for (int i=0;i<5;i++) {
    int tmp = analogRead(buttonsPin);
    // if (tmp > 1000) return; // return if button not pressed during any of the 5 samplings (=incomplete or too short button press)
    if (tmp < value) value = tmp;
  }
  return value;
  if (value < 120) return value; //1; // 793 (white)
  if (value < 200) return value; //2; // 528 (blue)
  if (value < 310) return value; //3; // 395 (grey)
  if (value < 350) return value; //4; // 315 (green)
  if (value < 400) return value; //5; // 262 (black)
  if (value < 425) return value; //6; // 225 (yellow)
  if (value < 175 and value > 165) return value; //7; // 196 (transparent)
  if (value < 100) return value; //8; // 174 (red)
  return -1;
}

// check if button has been pressed
boolean checkAndSetButtonPressed() {
	 
  if (millis() - lastButtonEvent < 1000) { // skip if lastButtonEvent is too close
    return;
  }
  
  newButtonPressed = checkButtonPressed(); // get value of button
  int value = newButtonPressed; 

  newButtonPressed = -1;
  if (value < 445 and value > 435) newButtonPressed = 8; // 174 (red)
  if (value < 415 and value > 405) newButtonPressed = 7; // 196 (transparent)
  if (value < 380 and value > 370) newButtonPressed = 6; // 225 (yellow)
  if (value < 340 and value > 325) newButtonPressed = 5; // 262 (black)
  if (value < 295 and value > 280) newButtonPressed = 4; // 315 (green)
  if (value < 235 and value > 225) newButtonPressed = 3; // 395 (grey)
  if (value < 175 and value > 165) newButtonPressed = 2; // 528 (blue)
  if (value < 150) newButtonPressed = 1; // 793 (white)
    
  if (newButtonPressed == -1 or newButtonPressed == lastButtonPressed){ // skip if no button pressed or button has not been released since last check  
    lastButtonPressed = newButtonPressed;
    return false;
  }

  Serial.println(millis() - lastButtonEvent);
  Serial.println(value);
  Serial.println(lastButtonPressed);
  Serial.println(newButtonPressed);
  
  if (currentFolder == newButtonPressed) { // check what to do if button pressed   
	  playNextSong();
  } else { // play first file from new folder
    currentFolder = newButtonPressed;
    currentFile = 1;
    Serial.println("playing folder "+String(currentFolder));
    myDFPlayer.playFolder(currentFolder, currentFile); // play the file
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
  volume = 10;

  if (volume != currentVolume) { // check what to do if volume changed
    if (volume == 0) { // pause player if volume == 0
      myDFPlayer.pause();
      currentVolume = 0;
      return;
    }
    if (currentVolume == 0) { // resume player if volume turned up again
      myDFPlayer.start();
    }
    
    // Serial.println(volume);
    myDFPlayer.volume(volume);  //Set volume value. From 0 to 30
    currentVolume = volume;
  }
  lastVolumeEvent = millis();
}

// play next song
void playNextSong() {
  if (currentFile < numberOfFiles[currentFolder-1]) { // play next file if not at the end
    currentFile++;
  } else { // play first file if at the end of folder
    currentFile = 1;       
  }
  
  Serial.println("playing song "+String(currentFile)+" in folder"+String(currentFolder));
  myDFPlayer.playFolder(currentFolder, currentFile); // play the file
  return;
}

// check if current song finished playing -> play next
void checkSongFinished() {
  if (millis() - lastBusyEvent > 1000 and millis() - lastButtonEvent > 1000) {
    int value = analogRead(busyPin);
    if (value > 1000) {
      Serial.println("End of Song. Playing next!");
      playNextSong();
    }
    lastBusyEvent = millis();
  }
  return;
}


// MAIN
void loop()
{
  checkAndSetButtonPressed();
  checkAndSetVolume();
  checkSongFinished();
  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }
}


// DEBUG
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
