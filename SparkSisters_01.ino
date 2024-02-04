// Team: Spark Sisters
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>


// HARDWARE DEFINITIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MAX30105 particleSensor;
#define debug Serial

const byte RATE_SIZE = 4; //rate size for averaging
byte rates[RATE_SIZE]; //creating rray of heart rates
byte rateSpot = 0;
long lastBeat = 0; // time last beat occured

float beatsPerMinute;
int beatAvg;
unsigned long startTime;

// LED DEFINITIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define LED_PIN1 6  // red
#define LED_PIN2 5  // yellow
#define LED_PIN3 3  // green

int Pins[4] = {LED_PIN1, LED_PIN2, LED_PIN3};

// OLED DEFINITIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define OLED_DC 9
#define OLED_CS 10
#define OLED_RESET 8

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_DC, OLED_RESET, OLED_CS);

// SETUP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~`
void setup()
{
  debug.begin(9600);
  debug.println("Initializing Life Amplifying Life Analyzer...");

  // initialize digital pins
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(LED_PIN3, OUTPUT);
  
   //lcd.print("Spark Sisters");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    debug.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  debug.println("Please place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); // Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); // Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); // Turn off Green LED
  particleSensor.enableDIETEMPRDY(); // Enable the temp ready interrupt. This is required.

    // init OLED
    if(!display.begin()) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  //delay(2000);
  display.clearDisplay();

}

// MAIN LOOP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop()
{
  // OLED Display
  oledDisplay();

  // runs heart rate analysis for 25 seconds
    startTime = millis();  // recording start time

  while (millis() - startTime < 2000)  // runs analysis for 20 seconds
  {
    heartRate();
  }

  // running LED functions
  turnAllOff();
  Blink();

  if ((beatAvg>=60)&&(beatAvg<=100)){ // run red for bad hr
    turnOn(LED_PIN3);
    oledResults();
    delay(1000);
  }
  else if ((beatAvg<60)||(beatAvg>100)){ // run green for good hr
    turnOn(LED_PIN1);
    oledResults();
    delay(1000);
  }
  else {
    turnOn(LED_PIN2); // run yellow for no vitals found, try again
    oledResults();
    delay(1000);
  }

  delay(5000); // wait 5 seconds until running analyzer again

}

// FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// OLED
void oledDisplay()
{
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  display.setCursor(0, 0);

  String textToDisplay = F("Initializing Life Amplifying Life Analyzer...");
  for (int i = 0; i < textToDisplay.length(); i++) {
    display.print(textToDisplay.charAt(i));
    display.display();
    delay(50); // Adjust the delay for the typing speed
  }

  delay(1000); // Pause after complete text is displayed
  display.clearDisplay();

  //delay(1000);
  display.setCursor(0, 0);
  String textToDisplay2 = F("Place your index finger on the sensor to read vital signs");
  for (int i = 0; i < textToDisplay2.length(); i++) {
    display.print(textToDisplay2.charAt(i));
    display.display();
    delay(50); // Adjust the delay for the typing speed
  }

    delay(1000); // Pause after complete text is displayed
  display.clearDisplay();
}

void oledResults()
{
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  display.setCursor(0, 0);

  // check the state of LEDs and display corresponding messages
  if (digitalRead(LED_PIN1) == HIGH) {
    display.println("You have concerning heart rate. :C");
  } else if (digitalRead(LED_PIN2) == HIGH) {
    display.println("Vitals not found, please try again.");
  } else if (digitalRead(LED_PIN3) == HIGH) {
    display.println("You have a healthy heart rate! :D");
  }

  display.display();
  delay(2000);
  display.clearDisplay();
}

// PROGRESS BAR
void progressBar(int percentage)
{

}

// HEART RATE & TEMP DISPLAY
void heartRate()
{
  // temp
    long irValue = particleSensor.getIR();
    float temperature = particleSensor.readTemperature();
    debug.print("temperatureC=");
    debug.print(temperature, 4);

  if (checkForBeat(irValue) == true)
  {
    // sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; // store this reading in the array
      rateSpot %= RATE_SIZE; // wrap variable

      //average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  debug.print(", IR=");
  debug.print(irValue);
  debug.print(", BPM=");
  debug.print(beatsPerMinute);
  debug.print(", Avg BPM=");
  debug.print(beatAvg);

  if (irValue < 50000)
    debug.print(" No finger?");

  debug.println();
}

// LEDS
void turnOn (int a){

  digitalWrite(a, HIGH);

}

void turnOff (int b){

  digitalWrite(b, LOW);

}

void turnAllOff(){

   for (int i = 0; i <3; i++){
    turnOff(Pins[i]);
    delay(1000);
  }

}

void Blink(){

  for (int i = 0; i <3; i++){
    turnOn(Pins[i]);
    delay(1000);
    turnOff(Pins[i]);
  }

}


