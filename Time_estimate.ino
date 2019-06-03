#include <SD.h>
#include <SPI.h>
#include <TMRpcm.h>

#define SD_ChipSelectPin 10
#define buzzerPin 5

TMRpcm audio;
unsigned short audiofile = 0;
bool recmode = false;
unsigned int secondsPassed = 0;
long beepIndex = 3;
unsigned int recordedSeconds = 0;
unsigned int currentFileIndex = 0;
char currentFileName[10];
bool toneIsPlaying = false;
unsigned long oldTime = 0;
unsigned long newTime = 0;

void intToString(unsigned int num, char *str) {
  for(int i = 3; i >= 0; i--) {
    str[i] = (num%10) + '0';
    num /= 10;
  }
}

int getHighestFileNumber() {
  int maxFileNum = 0;
  File root = SD.open("/");
  while(true) {
    File entry = root.openNextFile();
    if(!entry) {
      break;
    }
    if(!entry.isDirectory()) {
      int currFileNum = intFromFileName(entry.name());
      maxFileNum = currFileNum > maxFileNum ? currFileNum : maxFileNum;
    }
    entry.close();
  }
  root.close();  
}

int intFromFileName(char *fileName) { //0123
  char c;
  int sum = 0;
  int factor = 1;
  for(int i = 3; i >= 0; i--) {
    c = fileName[i];
    int numVal = c - '0';
    if(numVal < 0 || numVal > 9) {
      sum = 0;
      break;
    }
    sum += (c - '0')*factor;
    factor *= 10;
  }

  return sum;
}

void getCurrentFileName(char *str) {
  intToString(currentFileIndex, str);
  str[4] = '.';
  str[5] = 'w';
  str[6] = 'a';
  str[7] = 'v';
  str[8] = '\0';
}

void newBeepIndex() {
  beepIndex = random(8,20); //Randomiserar int 8-19 s
  File timefile = SD.open ("time.txt", FILE_WRITE);
  timefile.print(currentFileIndex);
  timefile.print(" : ");
  timefile.println(beepIndex);
  timefile.close();
}

void setup() {
  pinMode(A0, INPUT);
  pinMode(6, OUTPUT);
  // pinMode(2, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  SD.begin(SD_ChipSelectPin);
  currentFileIndex = getHighestFileNumber() + 1;
  //Skapa textfil på SD-kort om den inte finns
  if (!SD.exists("time.txt")) {
   File timefile = SD.open ("time.txt", FILE_WRITE);
   timefile.close();
  }
  audio.CSPin = SD_ChipSelectPin;
  oldTime = millis();
  newTime = millis();
  newBeepIndex();
  delay (10000);
}

void loop() {
  newTime = millis();
  if(newTime - oldTime > 1000) { // Om en sekund har passerat
    
    if(recmode)
      recordedSeconds++;
      
    if(toneIsPlaying) { // Stops beep and starts recording audio
      noTone(buzzerPin);
      toneIsPlaying = false;
    }
     
    //While audio.startrec = true 
      //sleep.thread
    if(recmode && recordedSeconds >= 3) {
      audio.stopRecording(currentFileName);
      digitalWrite(6, LOW);
      currentFileIndex++;
      recmode = false;
    }
    
    if(secondsPassed == beepIndex) {
      tone(buzzerPin, 500);
      toneIsPlaying = true;
      newBeepIndex();
      secondsPassed = 0;
      recordedSeconds = 0;
      getCurrentFileName(currentFileName);
      audio.startRecording(currentFileName, 16000, A0);
      digitalWrite(6, HIGH);
      recmode = true;
    }
    else {
      secondsPassed++;
    }

    oldTime = newTime;
  }
}
