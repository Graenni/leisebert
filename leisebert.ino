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
const int busyPin = 12;
const int volumePin = A2;

// define buttons pins: TODO here colors series
const int buttonPins[] = {2, 3, 4, 5, 6, 7, 8, 9};

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

// settings
int maxVolume = 10; // 0..30

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
    while (true);
  }

  // set up button pins as digital input with pullup on
  for (int i = 0; i < 8; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  // set busy pin as digital input with pullup on
  pinMode(busyPin, INPUT_PULLUP);

  // get number of files per folder
  for (int i = 1; i < 8; i++) {
    numberOfFiles[i - 1] = myDFPlayer.readFileCountsInFolder(i);
    Serial.println(numberOfFiles[i - 1]);
  }

  // set initial volume value
  myDFPlayer.volume(5);

  // TODO play last song from eeprom here ! ! !
  // currently just start playing button 1
  setSong(1);

  Serial.println("Leisebert online.");

}

// check value of buttons and return folder number
int checkButtonPressed() {
  for (int i = 0; i < 8; i++) {
    int buttonVal = digitalRead(buttonPins[i]);

    // button pressed
    if (buttonVal == LOW) {
      newButtonPressed = buttonVal;

      // check some early return conditions
      if (millis() - lastButtonEvent < 500) return false; // last button event too close -> skip
      if (newButtonPressed == lastButtonPressed) return false; // button has not been released since last check -> skip

      // button event ok -> play song
      setSong(i + 1);

      // save button event timing
      lastButtonEvent = millis();

      // save button event
      lastButtonPressed = newButtonPressed;

      return true;
    }
  }
  lastButtonPressed = -1; // no button pressed since last check
  return false;
}

boolean setSong(int buttonVal) {

  // check what to play
  // - next file if same button
  // - new folder if new button
  if (currentFolder == buttonVal) {

    // specifiy next file to be played within folder:
    // - next file if not yet at the end of folder
    // - first file if at the end of the folder
    if (currentFile < numberOfFiles[currentFolder - 1]) {
      currentFile++;
    } else {
      currentFile = 1;
    }

    Serial.println("playing song " + String(currentFile) + " in folder" + String(currentFolder));

  } else {

    // set new button as current folder
    currentFolder = buttonVal;

    // reset file counter to first file
    currentFile = 1;

    Serial.println("playing folder " + String(currentFolder));

  }

  // play the selected file
  myDFPlayer.playFolder(currentFolder, currentFile);

  // TODO save to eeprome here ! ! !
  // saveSongAndPositionInEeprom(0);
  return true;
}

// set volume
void checkAndSetVolume() {

  if (millis() - lastVolumeEvent < 500) { // skip if lastVolumeEvent is too close
    return;
  }

  int volume = analogRead(volumePin);
  volume = map(volume, 10, 1023, 0, maxVolume);

  if (volume != currentVolume) { // check what to do if volume changed
    Serial.println(volume);
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

// check if current song finished playing -> play next
void checkSongFinished() {
  if (millis() - lastBusyEvent > 1000 and millis() - lastButtonEvent > 1000 and currentFolder > 0 and currentVolume != 0) {
    int busyVal = digitalRead(busyPin);
    if (busyVal == HIGH) {
      Serial.println("End of Song. Playing next!");
      
      // emulate button press with button of current folder
      setSong(currentFolder);
    }
    lastBusyEvent = millis();
  }
  return;
}


// MAIN
void loop()
{
  checkButtonPressed(); // check if button pressed and play corresponding song
  checkAndSetVolume(); // check if volume has been adjusted
  checkSongFinished(); // check if song has finished playing
  
  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }
}


// DEBUG
void printDetail(uint8_t type, int value) {
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
