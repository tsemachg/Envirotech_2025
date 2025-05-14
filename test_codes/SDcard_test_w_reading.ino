#include <SD.h>
#include <SPI.h>

File file;

int pinCS = 10; // for Uno

void setup() {
  Serial.begin(9600);
  pinMode(pinCS, OUTPUT); 

  if (SD.begin())
  {
    Serial.println("SD card good");
  } else {
    Serial.println("SD card failed!");
    return;
  }

  // Reading the file
  file = SD.open("FILE.TXT");
  if(file) {
    Serial.print("------- Reading "); Serial.print(file.name()); Serial.println(": -------");
    Serial.print("File siez (bytes): "); Serial.println(file.size());
    Serial.println("File Content: ");
    Serial.write(file.read()); Serial.println("");
    Serial.println("---- Finished reading, closing file ----");
  }

}

void loop() {
  // put your main code here, to run repeatedly:

}
