// 2025-04-10 (begin date) Gill Tsemach
// Envirotech course 2025 Spring semester, sensor task. - improve this section after code works. 
// built from Arduino IDE's built-in sketch "Blink" and Adafruit's SHTC3 sensor library. 



//     +--------------------------------------------------------+
//     | Change to correct time delay requirements for homework |
//     +--------------------------------------------------------+



// declaring variables 
#include "Adafruit_SHTC3.h"

Adafruit_SHTC3 shtc3 = Adafruit_SHTC3();

unsigned long time; // set variable named 'time', of type long & unsigned (only positive)
const unsigned long wait = 7000; // initial time to wait currently 2 sec, 'const' to save memory space

// the setup 
void setup() {
  Serial.begin(9600); // to match the baud

  // Blink: initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Sensor SHTC3 - try to find if sensor is connected properly, taken from Adafruit's tutorial
  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("SHTC3 test");
  if (! shtc3.begin()) {
    Serial.println("Couldn't find SHTC3");
    while (1) delay(1);
  }
  Serial.println("Found SHTC3 sensor");
  Serial.println("Initial Count: (miliseconds)");
}


// Loop section
void loop() {

  // time = millis();
// if 100 passed then do this sequence:
  if (millis() >= wait) {
    // do the thing
    Serial.println("LED on");
    digitalWrite(LED_BUILTIN, HIGH); // turn on LED 

    // making the code "more interesting" instead of just 5 sec delay
    Serial.print("Break: ");
    for (int i = 1; i <= 5; i++) {
      delay(1000); 
      Serial.print(i);
    }
    Serial.println("");

      sensors_event_t humidity, temp;
    
    shtc3.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data
    
    Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" °C");
    Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% RH");
    Serial.println("");

    Serial.println("LED off");
    digitalWrite(LED_BUILTIN, LOW); // turn LED off
    
    Serial.print("Break: ");
    for (int i = 1; i <= 5; i++) {
      delay(1000); 
      Serial.print(i);
    }
    Serial.println("");
    Serial.println("");

  } else {
    Serial.println(millis()); // just to see it works
    delay(1000);
  }

}
