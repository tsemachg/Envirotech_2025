/* 

  +----------------------------------------------------------------------------------+
  |  Temp, RH and CO2 monitoring and logging.                                        |
  |                                                                                  |
  |  Gill Tsemach                                                                    |
  |  2025-04-10 (begin date)                                                         |
  |  Envirotech course 2025 Spring semester, midterm assignment.                     |
  |  Code was built with help from tutorials: Arduino IDE's built-in "Blink" & "SD"  |
  |  sketches, Adafruit's SHTC3 & RTClib and Sensirion's SCD4x libraries.            |                                       | 
  +----------------------------------------------------------------------------------+
  
*/



// Loading libraries
#include "Adafruit_SHTC3.h" // SHTC3 sensor
#include <SensirionI2cScd4x.h> // SCD40 (w CO2) sensor
#include <Wire.h> // for SCD40 (w CO2) sensor, as seen in the example
#include <SD.h>  // SD
#include <SPI.h>  // SD
#include "RTClib.h"  // Real-Time-Clock

// definition of "NO_ERROR" for SCD40 from thier documentation
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

// ---------- Declaring variables ----------
//// SDcard related
int pinCS = 10; // The CS pin connected to SD card, pin #10 is for Uno.
File file; // Creating file object for read/write to SD card.

//// Sensors related
Adafruit_SHTC3 shtc3 = Adafruit_SHTC3(); // SHTC3 temp & RH sensor 
SensirionI2cScd4x sensor; // SCD40 temp, RH & CO2 sensor
static char errorMessage[64]; // debugging for SCD40 sensor 
static int16_t error; // debugging for SCD40 sensor 

//// Real-Time-Clock
RTC_DS3231 rtc; // DS3231 clock, decided to ommit the use of days in character format.
// char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//// Calculations, Other...
bool operational = true; // for the whole system to be checked at setup, will determine if loop section runs
bool broke = false; // if something breaks while loop is running print message 
unsigned long time; // set variable named 'time', of type long & unsigned (only positive)
const unsigned long wait = 2000; // initial time to wait currently 2 sec, 'const' to save memory space


// ---------------- Setup, run once on startup -----------------
void setup() {
  Serial.begin(9600); // to match the baud
  Serial.println(F("------------------------- Initializing Sensor -------------------------"));
  Serial.println(F("\n** Cheking components:")); // F() is for fixed string to save space

  // # Blink: initialize digital pin LED_BUILTIN as an output.
  // pinMode(LED_BUILTIN, OUTPUT);

  // # SD card - define the correct pin as output for card. 
  pinMode(pinCS, OUTPUT);
  // SPI.begin(); // commented out, was trying to operate LED 

  /*  +-------------------------------------------------------------+
      |  CAUTION: The SD card and built-in LED use the same pin!    |
      |      Discarding the use of blink on LED.                    |
      |                                                             |
      |  Found possible solution if using extrrnal LED -> Arduino   | 
      |      Forums: "Is it possible to use pin 13 for both the     |
      |      built-in LED and as SPI clock pin?"                    |
      |      The solution is not ideal, turning SPI on and off.     |
      |      It is advised to find other solution as external LED.  | 
      +-------------------------------------------------------------+
  */

  /* This sction might be deleted -> chack what happens when connected to 
  wall power but not computer. 

  while (!Serial)
    delay(10);    // will pause Zero, Leonardo, etc until serial console opens
  */
  
  // # Initialize SD card
  Serial.print(F("Checking SD card:       "));
  if (SD.begin())
  {
    Serial.println("SD card good to go");
  } else {
    Serial.println("SD card failed!");
    operational = false;
    return; // if error occured - abort & exit
  }

  // # Initializing RTC DS3231
  Serial.print(F("RTC test:               "));
  if (rtc.begin()) {
    Serial.println("RTC Found");
    if (rtc.lostPower()) { // if first time using or battery power lost 
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // read & set time from when the sketch was compiled (computer)
    }
  } else {
    Serial.println("Couldn't find RTC");
    operational = false;
    return; // if error occured - abort & exit    
  }

  // # Initializing Adafruit SHTC3 temp & RH
  Serial.print(F("SHTC3 test:             "));
  if (shtc3.begin()) {
    Serial.println("Found SHTC3 sensor");
  } else {
    Serial.println("Couldn't find SHTC3");
    operational = false;
    return; // if error occured - abort & exit    
  }

  // # Initializing SCD40 temp, RH & CO2
  Serial.print("SCD40 test:             ");
  Wire.begin(); // for I2C
  sensor.begin(Wire, SCD41_I2C_ADDR_62);
  /* All commented section removed due to taking too much memory space, considering 
     the scope of the exercise there's not enough time to figure out how to optimze 
     it in a meaningful way. (+ it's not the focus...)  

  // All error tests by Sensirion example code
  uint64_t serialNumber = 0;
  error = sensor.wakeUp();
  if (error != NO_ERROR) {
      Serial.print(F("Error on wakeUp(): "));
      errorToString(error, errorMessage, sizeof errorMessage);
      Serial.println(errorMessage);
  }
  
  error = sensor.stopPeriodicMeasurement();
  if (error != NO_ERROR) {
      Serial.print(F("Error on stopPeriodicMeasurement(): "));
      errorToString(error, errorMessage, sizeof errorMessage);
      Serial.println(errorMessage);
  }
  error = sensor.reinit();
  if (error != NO_ERROR) {
      Serial.print(F("Error on reinit(): "));
      errorToString(error, errorMessage, sizeof errorMessage);
      Serial.println(errorMessage);
  }
  // Read out information about the sensor
  error = sensor.getSerialNumber(serialNumber);
  if (error != NO_ERROR) {
      Serial.print(F("Error on getSerialNumber(): "));
      errorToString(error, errorMessage, sizeof errorMessage);
      Serial.println(errorMessage);
      return;
  }
  */ 
  
  sensor.reinit(); // reads the sensor variant
  // set SCD40 to periodic measurement mode (not single)
  if (sensor.startPeriodicMeasurement() != NO_ERROR) {
    Serial.println(F("Found SCD40 sensor"));
  } else {
    Serial.println(F("Problem found in SCD40's process!"));
    operational = false;
    return;
  }
  
  // just in case
  if (operational == true) {
    Serial.println(F("All components are functioning! (or at least silently failing ;))\n"));
  } else {
    Serial.println(F("Found problem in one of the components! \nTerminating sensor operation."));
  } 

  // SDcard, creating file with headers for measurements
  Serial.println("** Creating 'midterm.csv':");
  file = SD.open("midterm.csv", FILE_WRITE);
  if (file) {
    file.println("RTC,millis,DS3231_Temp (°C),SHTC3_Temp (°C),SHTC_RH (%),SCF40_Temp (°C),SCD40_RH (%),SCD40_CO2 (ppm)");
    file.close();
    Serial.println("File created successfully");
  } else {
    Serial.println("Error occured while trying to create the file!");
    operational = false;
    return;
  }
}




// ------------ Loop, repeate this section ------------
void loop() {
  if (operational == true) {

      bool dataReady = false;        // SCD40 variables
      uint16_t co2Concentration = 0; // SCD40 variables
      float temperature = 0.0;       // SCD40 variables
      float relativeHumidity = 0.0;  // SCD40 variables
      
      



/////// ---- Change section of if{}else{} to count running for 3 days. 

// if 100 passed then do this sequence:
    if (millis() >= wait) {
      // Serial.println("LED on");
      // digitalWrite(LED_BUILTIN, HIGH); // turn on LED 
  
      // making the code "more interesting" instead of just 5 sec delay
      Serial.print("Pause: ");
      for (int i = 1; i <= 5; i++) {
        delay(1000); 
        Serial.print(i);
      }
      Serial.println("");

      // # Read RTC
      DateTime now = rtc.now();
      
      // # Reading both sensors
      // ## SHTC3
        sensors_event_t humidity, temp;
      
      shtc3.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data
      
      // ## SCD40
      //// Start measuring, wait for data to be ready
      error = sensor.getDataReadyStatus(dataReady);
      if (error != NO_ERROR) {
          Serial.println(F("Error on getDataReadyStatus()"));
          operational = false;
          return; 
      }
      while (!dataReady) {
          delay(100); // delay for proper operation, possible resulting inaccuracies in time not critical for case study
          error = sensor.getDataReadyStatus(dataReady);
          if (error != NO_ERROR) {
              Serial.println(F("Error on getDataReadyStatus()"));
              operational = false;
              return;
          }
      }
  
      sensor.readMeasurement(co2Concentration, temperature, relativeHumidity); // the actual reading of SCD40...
      
      Serial.println(F("------------------------------------------------"));
      //// # Serial print the readings
      //// ## RTC one-line print YYYY/MM/DD hh:mm:ss
      Serial.print(F("[DS3231] Date:               "));
      Serial.print(now.year(), DEC);
      Serial.print('/');
      Serial.print(now.month(), DEC);
      Serial.print('/');
      Serial.print(now.day(), DEC);
      Serial.print(" ");
      Serial.print(now.hour(), DEC);
      Serial.print(':');
      Serial.print(now.minute(), DEC);
      Serial.print(':');
      Serial.println(now.second(), DEC);
      Serial.print(F("[DS3231] Temperature:        ")); Serial.print(rtc.getTemperature()); Serial.println(" °C");

      //// ## SHTC3
      Serial.println(F("------------------------------------------------"));
      Serial.print(F("[SHTC3]  Temperature:        ")); Serial.print(temp.temperature); Serial.println(" °C");
      Serial.print(F("[SHTC3]  Humidity:           ")); Serial.print(humidity.relative_humidity); Serial.println("% RH");
      // Serial.println("");

      //// ## SCD40
      Serial.println(F("------------------------------------------------"));
      Serial.print(F("[SCD40]  Temperature:        ")); Serial.print(temperature); Serial.println(" °C");
      Serial.print(F("[SCD40]  Humidity:           ")); Serial.print(relativeHumidity); Serial.println("% RH");
      Serial.print(F("[SCD40]  CO2 concentratio)n: ")); Serial.print(co2Concentration); Serial.println(" ppm");
      Serial.println(F("------------------------------------------------"));

      // # Log data to SD card CSV format
      Serial.print(F("Logging... "));
      //("RTC,milis,RTC_Temp,SHTC3_Temp,SHTC_RH,SCF40_Temp,SCD40_RH,SCD40_CO2")
      file = SD.open("midterm.csv", FILE_WRITE);
      if (file) {
        // Time RTC
        file.print(now.year(), DEC);
        file.print('/');
        file.print(now.month(), DEC);
        file.print('/');
        file.print(now.day(), DEC);
        file.print(" ");
        file.print(now.hour(), DEC);
        file.print(':');
        file.print(now.minute(), DEC);
        file.print(':');
        file.print(now.second(), DEC); file.print(",");

        // time in millis()
        file.print(millis()); file.print(",");

        // DS3231 Temp
        file.print(rtc.getTemperature()); file.print(",");
  
        //// ## SHTC3
        file.print(temp.temperature); file.print(",");
        file.print(humidity.relative_humidity); file.print(",");
  
        //// ## SCD40
        file.print(temperature); file.print(",");
        file.print(relativeHumidity); file.print(",");
        file.println(co2Concentration); 
        
        file.close(); // closing file after finish
        Serial.println("Finished writing to file");
      } else {
        Serial.println("Error while writing to file");
      }
      
      // Serial.println("LED off");
      // digitalWrite(LED_BUILTIN, LOW); // turn LED off
      
      Serial.print("Pause: ");
      for (int i = 1; i <= 5; i++) {
        delay(1000); 
        Serial.print(i);
      }
      Serial.println("");
      Serial.println("");
      
    } else {
      Serial.println(millis()); // inital wait 2 sec, just to see it works
      delay(1000);
    }



  } else { // if something breaks while loop is running print message only once 
    if (broke != true) {
      Serial.println("Something broke while running!");
      broke = true;
      return;
    }
  }
  
}
