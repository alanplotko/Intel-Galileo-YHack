#include <Wire.h>
#include <rgb_lcd.h>
#include <Time.h>

// Initalize object representing LCD
rgb_lcd lcd;

/* ------------------------------------------
    Configuration Variables & Constants
--------------------------------------------- */

// Light Sensor Setup
const int pinLight = A0;    // Light sensor connected to A0
int thresholdvalue = 400;   // The threshold to turn on or off the LED
int lightValue;

// Touch Sensor Setup
const int pinTouch = 2;   // Pin of touch sensor

// Button Setup
const int pinButton = 6;   // Pin of button

// Sound Sensor setup
const int pinSound = A3;    // Pin of sound sensor

// Buzzer Setup
int speakerPin = 4;    // Grove Buzzer connect to D4
const int notesLength = 100;  // the number of notes
int sounds[notesLength];  // We'll collect notesLength notes

// Temperature Sensor Setup
const int pinTemp = A2;  // Pin of temperature sensor
const int pinLed = 5;    // Pin of led
int B = 3975;            // B value of the thermistor
float resistance;
int temperature;
int lightsOn;

// Rotary Angle Sensor
const int potentiometer = A1; // Pin of rotary angle sensor

// Custom Characters
byte heart[8] = { 0b00000, 0b01010, 0b11111, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000 };
byte smiley[8] = { 0b00000,  0b10001,  0b00000,  0b00000,  0b10001, 0b01110,  0b00000 };
byte degree = B11011111;

boolean page = false;

/* ---------
    Setup
------------*/

void setup() {
    Serial.begin(9600);            // Start serial monitor
    pinMode(speakerPin, OUTPUT);   // Connect buzzer
    pinMode(pinTouch, INPUT);     // Connect touch sensor
    pinMode(pinButton, INPUT);     // Connect button
    pinMode(pinLed, OUTPUT);       // Connect LED
    pinMode(potentiometer, INPUT); // Connect rotary angle sensor
    
    lightsOn = analogRead(pinLight) > thresholdvalue ? 1 : 0;
    lcd.begin(16, 2);             // Prepare LCD
    lcd.createChar(0, heart);     // Create heart character to display on lcd later
    lcd.createChar(1, smiley);    // Create smiley character to display on lcd later
}

int calculateTemp() {
    int val = analogRead(pinTemp);        // Read in temperature
    resistance = (float)(1023 - val)      // Calculate resistance
      * 10000 / val;
    int tempC = 1 / (log(resistance /     // Get temperature in celsius from resistance
      10000) / B + 1 / 298.15) - 273.15;
    int tempF = ((tempC * 9) / 5) + 32;   // Convert celsius to fahrenheit
    return tempF;
}

void loop()
{
    lightValue = analogRead(pinLight); // Read in light intensity
    temperature = calculateTemp(); // Calculate temperature

    if(digitalRead(pinButton)) {
        page = !page;
    }

    // Check if the button was pressed
    if(digitalRead(pinTouch))
    {  
      turnOffLed(); // Keep LED off
      lcd.setRGB(255, 255, 255);
      lcd.noBlink();
      prepareForInput();
      
      for (int i = 0; i < notesLength; i++)
      {
        sounds[i] = analogRead(pinSound); // Read in each sound intensity
        Serial.println(sounds[i]);        // Write each sound intensity to the serial monitor
      }

      lcd.clear();        // Clear LCD
      waitingMessage();   // Show "processing" message to delay the playing of the recorded sound intensities
      playMusic(sounds);  // Play sounds
      lcd.setRGB(255, 255, 255);
    }
    
    // Check for abnormal temperature
    else if(temperature <= 40 || temperature >= 80) {
        turnOnLed();
        lcd.setRGB(255, 0, 0);
        lcd.blink();
        // Rebuild the message and update the lcd every 5 seconds
        lcd.clear();
        buildTempDisplay();
        lcd.display();  // Display on screen
        delay(5000);    // Message remains on screen for 5 seconds
    }
    
    // Check if the room is dark
    else if(lightValue > thresholdvalue)
    {
        turnOffLed(); // Keep LED off
        lcd.setRGB(255, 255, 255);
        lcd.noBlink();
        // Rebuild the message and update the lcd every 5 seconds
        lcd.clear();
        buildMessageDisplay(page); // Prepare appropriate message
        lcd.display();  // Display on screen
        if (lightsOn == 0) {
          lightsOn = 1;
          playMusic(sounds);
        }
        delay(5000); // Message remains on screen for 5 seconds
    }

    // If no conditions are met, don't display anything
    else
    {
        turnOffLed(); // Keep LED off
        lcd.noDisplay();
        lcd.setRGB(0, 0, 0);
    }
    lightsOn = lightValue > thresholdvalue ? 1 : 0;
}

// Show YHack message
void buildMessageDisplay(boolean page) {
    if(!page) {
        lcd.setCursor(0, 0);
        lcd.print("Made w/  @YHack!");
        lcd.setCursor(7, 0);
        lcd.write(byte(0));
        lcd.setCursor(0, 1);
        lcd.print(temperature);
        lcd.setCursor(2, 1);
        lcd.write(degree);
        lcd.setCursor(3, 1);
        lcd.print("F | Welcome!");
    }
    else {
        lcd.setCursor(0, 0);
        lcd.print("CoRE was here!");
        lcd.setCursor(15, 0);
        lcd.write(byte(1));
        lcd.setCursor(0, 1);
        lcd.print("YHack, Fall 2014");
    }
}

/* ------------------------
    Temperature functions
---------------------------*/

// Show temperature
void buildTempDisplay() {
    lcd.print("Abnormal Weather");
    lcd.setCursor(0, 1);
    lcd.print("Detected:    F");
    lcd.setCursor(10, 1);
    lcd.print(temperature);
    lcd.setCursor(12, 1);
    lcd.write(degree);
    lcd.setCursor(14, 1);
}

void turnOnLed() {
    analogWrite(pinLed, 255);
}

void turnOffLed() {
   analogWrite(pinLed, 0);
}

/* -------------------
    Timers, Messages
======================*/

// Show a timer to allow the end user to prepare to hum tunes
void prepareForInput() {
    lcd.setRGB(230, 255, 0);
    lcd.blink();
    for(int i = 5; i > 0; i--) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Ready to record:");
      lcd.setCursor(0, 1);
      lcd.print(i);
      lcd.setCursor(1, 1);
      lcd.print(" seconds...");
      lcd.display();
      delay(1000);
    }
    lcd.clear();
    lcd.setCursor(6, 0);
    lcd.noBlink();
    lcd.setRGB(0, 255, 0);
    lcd.print("Go!");
    lcd.display();
    delay(3000);
}

// Processing message to delay playing of sounds after recording sounds
void waitingMessage() {
    lcd.clear();
    lcd.setRGB(230, 255, 0);
    lcd.setCursor(2, 0);
    lcd.blink();
    lcd.print("Processing");
    lcd.setCursor(1, 1);
    lcd.print("Please wait...");
    lcd.display();
    delay(3000);
    lcd.clear();
    lcd.setRGB(19, 204, 225);
    lcd.noBlink();
    lcd.setCursor(1, 0);
    lcd.print("Playing music!");
    lcd.display();
    delay(1000);
}


/* -----------------
    Sound functions
--------------------*/

void playTone(int tone, int duration) {
    for (long i = 0; i < duration * 1000L; i += tone * 2) {
        digitalWrite(speakerPin, HIGH);
        delayMicroseconds(tone);
        digitalWrite(speakerPin, LOW);
        delayMicroseconds(tone);
    }
}

void playNote(char note, int duration) {
    char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
    int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };

    // play the tone corresponding to the note name
    for (int i = 0; i < 8; i++) {
        if (names[i] == note) {
            playTone(tones[i], duration);
        }
    }
}

void playMusic(int sounds[]) {    
    char note;
    for (int i = 0; i < notesLength; i += 4) 
    {
        if (sounds[i]  < 125) { note = 'c'; }
        else if (sounds[i] < 250) { note = 'd'; }
        else if (sounds[i] < 375) { note = 'e'; }
        else if (sounds[i] < 500) { note = 'f'; }
        else if (sounds[i] < 625) { note = 'g'; }
        else if (sounds[i] < 750) { note = 'a'; }
        else if (sounds[i] < 875) { note = 'b'; }
        else { note = 'C'; }
        playNote(note, 150);

        // pause between notes
        delay((0.1 * analogRead(potentiometer)) + 1);
    } 
}