// Dependencies
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <Stepper.h>

// Hard coded definitions (pins and values)
#define NUMPIXELS 22
#define relayPin 27
#define touchPin 4
#define touchSensitivity 10
#define hallPin 39
#define usrLed 2
#define rsvr 12
const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution

// ULN2003 Motor Driver Pins
#define IN1 26
#define IN2 25
#define IN3 33
#define IN4 32

bool theShow = false;
bool waterLevel = false;
bool manualMode = false;
bool relayState = false;
bool stateChange = false;  // turns true upon async call - ensures the relay initializes in ON state of the cycle (always)
bool waterVal = false;

// Neopixel Variables
int brightness = 200;
int brightnessDirection = 1;
unsigned long previousMillis = 0;
int colorStep = 0;
byte color = 0;
int currentPixel = 0;
int rainbowStep = 0;
int direction = 1;
int currentEffect = 0;


byte touchVal = 0;
short rotation = 0;

String hexString = "Color(0xff00ff57)";
int alphaVal = 200;
int hexVal = 16516088;
int fogCycle = 1;

unsigned long previousMillis2 = 0;  // will store last time LED was updated
unsigned long OffTime = 180000;      // defualt fog cycle
unsigned long OnTime = 8000;


const char *wifi_network_ssid = "Induct Technologies";
const char *wifi_network_password = "maas-1004";
const char *soft_ap_ssid = "Venus32";
const char *soft_ap_password = "Venus@32";

// Initialize neopixels
Adafruit_NeoPixel pixels(NUMPIXELS, 13, NEO_GRB + NEO_KHZ800);
AsyncWebServer server(80);  // accessible on port 80
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

void serverCalls() {
  server.on("/hello", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "FH18 Mini Reporting");
  });

  server.on("/mg", HTTP_GET, [](AsyncWebServerRequest *request) {
    color = 0;
    request->send(200, "text/plain", "magenta");
  });

  server.on("/cy", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "cyan");
    color = 1;
  });

  server.on("/ledon", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "NEO ON");
    color = 0;
  });

  server.on("/ledoff", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "NEO OFF");
    color = 2;
  });

  server.on("/hex", HTTP_GET, [](AsyncWebServerRequest *request) {
    hexString = request->getParam("hexCode")->value();
    alphaVal = strtol(hexString.substring(8, 10).c_str(), NULL, 16);
    hexVal = strtol(hexString.substring(10, 16).c_str(), NULL, 16);

    request->send(200, "text/plain", "magenta");
  });

  server.on("/mode/false", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Mini now in manual mode");
    manualMode = true;
  });

  server.on("/mode/true", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Mini now in auto mode");
    manualMode = false;
  });


  server.on("/spin/2", HTTP_GET, [](AsyncWebServerRequest *request) {
    rotation = 1;
    request->send(200, "text/plain", "clockwise");
  });

  server.on("/spin/0", HTTP_GET, [](AsyncWebServerRequest *request) {
    rotation = -1;
    request->send(200, "text/plain", "anti-clockwise");
  });

  server.on("/spin/1", HTTP_GET, [](AsyncWebServerRequest *request) {
    rotation = 0;
    request->send(200, "text/plain", "halt");
  });

  server.on("/fog/69", HTTP_GET, [](AsyncWebServerRequest *request) {
    relayState = 1;
    request->send(200, "text/plain", "relay on");
  });  // inverted logic (Common anode)

  server.on("/fog/96", HTTP_GET, [](AsyncWebServerRequest *request) {
    relayState = 0;
    request->send(200, "text/plain", "relay off");
  });

  server.on("/fog", HTTP_GET, [](AsyncWebServerRequest *request) {
    fogCycle = (request->getParam("fogCycle")->value()).toInt();
    Serial.println("Fog variable is : ");
    Serial.print(fogCycle);
    String fogString = String(fogCycle);
    request->send(200, "text/plain", fogString);
  });

  server.on("/fog/1", HTTP_GET, [](AsyncWebServerRequest *request) {
    OnTime = 5000;
    OffTime = 5000;
    stateChange = true;
    request->send(200, "text/plain", "2 second cycle set");
  });

  server.on("/fog/2", HTTP_GET, [](AsyncWebServerRequest *request) {
    OnTime = 120000;
    OffTime = 1200000 - OnTime;
    stateChange = true;
    request->send(200, "text/plain", "4 min cycle set");
  });

  server.on("/fog/3", HTTP_GET, [](AsyncWebServerRequest *request) {
    OnTime = 180000;
    OffTime = 1200000 - OnTime;
    stateChange = true;
    request->send(200, "text/plain", "6 min cycle set");
  });

  server.on("/fog/4", HTTP_GET, [](AsyncWebServerRequest *request) {
    OnTime = 240000;
    OffTime = 1200000 - OnTime;
    stateChange = true;
    request->send(200, "text/plain", "8 min cycle set");
  });

  server.on("/fog/5", HTTP_GET, [](AsyncWebServerRequest *request) {
    OnTime = 300000;
    OffTime = 1200000 - OnTime;
    stateChange = true;
    request->send(200, "text/plain", "10 min cycle set");
  });

  server.on("/fog/6", HTTP_GET, [](AsyncWebServerRequest *request) {
    OnTime = 360000;
    OffTime = 1200000 - OnTime;
    stateChange = true;
    request->send(200, "text/plain", "12 min cycle set");
  });

  server.on("/fog/7", HTTP_GET, [](AsyncWebServerRequest *request) {
    OnTime = 420000;
    OffTime = 1200000 - OnTime;
    stateChange = true;
    request->send(200, "text/plain", "14 min cycle set");
  });

  server.on("/fog/8", HTTP_GET, [](AsyncWebServerRequest *request) {
    OnTime = 480000;
    OffTime = 1200000 - OnTime;
    stateChange = true;
    request->send(200, "text/plain", "16 min cycle set");
  });

  server.on("/fog/9", HTTP_GET, [](AsyncWebServerRequest *request) {
    OnTime = 540000;
    OffTime = 1200000 - OnTime;
    stateChange = true;
    request->send(200, "text/plain", "18 min cycle set");
  });

  server.on("/fog/10", HTTP_GET, [](AsyncWebServerRequest *request) {
    OnTime = 600000;
    OffTime = 1200000 - OnTime;
    stateChange = true;
    request->send(200, "text/plain", "20 min cycle set");
  });

  server.on("/effect/0", HTTP_GET, [](AsyncWebServerRequest *request) {
    currentEffect = 0;
    request->send(200, "text/plain", "Breathe Effect");
  });

  server.on("/effect/1", HTTP_GET, [](AsyncWebServerRequest *request) {
    currentEffect = 1;
    request->send(200, "text/plain", "Comet Effect");
  });

  server.on("/effect/2", HTTP_GET, [](AsyncWebServerRequest *request) {
    currentEffect = 2;
    request->send(200, "text/plain", "Rainbow Effect");
  });

  server.on("/effect/3", HTTP_GET, [](AsyncWebServerRequest *request) {
    currentEffect = 3;
    request->send(200, "text/plain", "Static Effect");
  });
}

void wifiSetup() {
  WiFi.begin(wifi_network_ssid, wifi_network_password);

  // check if connection established in 5 sex, blink red led in the meantime
  while (WiFi.status() != WL_CONNECTED && millis() <= 5000) {
    Serial.println("Connecting to WiFi..");
    delay(1000);
  }

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.softAP(soft_ap_ssid, soft_ap_password);
    Serial.print("Connect to Mini via WiFi: ");
    Serial.println(WiFi.softAPIP());
  }

  else {
    Serial.print("Mini connected to WiFI: ");
    Serial.println(WiFi.localIP());
  }
}

// Initialize light setup
void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, color);
    pixels.show();
    delay(wait);
  }
}

void startPodAnimation(int wait) {
  // Define the colors from violet to red
  uint32_t colors[] = {pixels.Color(148, 0, 211),  // Violet
                       pixels.Color(75, 0, 130),   // Indigo
                       pixels.Color(0, 0, 255),    // Blue
                       pixels.Color(0, 255, 0),    // Green
                       pixels.Color(255, 255, 0),  // Yellow
                       pixels.Color(255, 127, 0),  // Orange
                       pixels.Color(255, 0, 0)};   // Red

  // Turn on each color segment one by one from bottom to top
  for (int i = 0; i < pixels.numPixels(); i++) {
    colorWipe(colors[i], wait);
    delay(100); // Optional delay between colors
    pixels.clear();
  }

  // Rainbow animation
  rainbowCyycle(5); // Adjust the delay as needed
}

void rainbowCyycle(int wait) {
  int j;
  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (int i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.

void setup() {
  pixels.begin();
  pixels.show();    // Initialize all pixels to 'off'
  startPodAnimation(25);  // Change the '10' to control the animation speed
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  pinMode(rsvr, OUTPUT);
  pinMode(hallPin, INPUT);
  pinMode(usrLed, OUTPUT);
  digitalWrite(usrLed, HIGH);
  myStepper.setSpeed(5);
  wifiSetup();  // Establish connection via WiFi

  serverCalls();  //Async request handler

  server.begin();
}

void pixelColor(void) {
  pixels.setBrightness(brightness);
  switch (color) {
    case 0:  // Magenta
      pixels.fill(pixels.Color(255, 0, 255));
      pixels.show();
      break;
    case 1:  // CYAN
      pixels.fill(pixels.Color(0, 255, 255));
      pixels.show();
    case 2:  // led OFF
      pixels.clear();
      pixels.show();
      break;
    case 10:  // RED
      pixels.fill(pixels.Color(255, 5, 5));
      pixels.show();
      break;
  }
}

void waterAlert() {
  manualMode = false;            // get out of Manual mode
  digitalWrite(relayPin, HIGH);  // turn relay OFF if ON
  pixels.begin();
  pixels.clear();
  pixels.setBrightness(brightness);
  for (int i = 255; i >= 0; i -= 5) {
    pixels.setBrightness(i);
    pixels.fill(pixels.Color(255, 5, 5));
    pixels.show();
    delay(5);
  }
  delay(200);
  for (int i = 0; i < 255; i += 5) {
    pixels.setBrightness(i);
    pixels.fill(pixels.Color(255, 5, 5));
    pixels.show();
    delay(5);
  }
  color = 10;
}

void miniShow() {
  digitalWrite(relayPin, LOW);  // turn relay ON
  pixels.setBrightness(100);
  for (int i = 0; i < 5; i++) {
    pixels.fill(pixels.Color(255, 255, 255));
    pixels.show();
    delay(100);
    pixels.clear();
    pixels.show();
    delay(100);
  }
  digitalWrite(relayPin, HIGH);  // turn relay OFF before exiting the function
}

// Neopixel Effects

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}


void breathe() {
  int interval = 50;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    pixels.setBrightness(brightness);
    pixels.fill(hexVal, 0, NUMPIXELS);
    pixels.show();

    brightness += brightnessDirection;

    if (brightness == 255 || brightness == 0) {
      brightnessDirection *= -1;
    }
  }
}


void comet() {
  int interval = 50;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    for (int i = 0; i < NUMPIXELS; i++) {
      if (abs(i - currentPixel) <= 2) {
        pixels.setPixelColor(i, hexVal);
      } else {
        pixels.setPixelColor(i, 0);
      }
    }
    pixels.show();

    currentPixel += direction;

    if (currentPixel == NUMPIXELS - 2 || currentPixel == 0) {
      direction *= -1;
    }
  }
}


void rainbowCycle() {
  int interval = 50;  // effect speed
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, Wheel((i + colorStep) & 255));
    }
    pixels.show();

    colorStep++;
  }
}

void staticColor() {
  pixels.setBrightness(alphaVal);
  pixels.fill(hexVal, 0, NUMPIXELS);
  pixels.show();
}


void manualModeF() {
  switch (currentEffect) {
    case 0:
      breathe();
      break;
    case 1:
      comet();
      break;
    case 2:
      rainbowCycle();
      break;
    case 3:
      staticColor();
      break;
  }

  if (relayState)
    digitalWrite(relayPin, HIGH);
  else
    digitalWrite(relayPin, LOW);

  myStepper.step(rotation);
}



void loop() {

  //Serial.println("Effect index : ");
  //Serial.println(currentEffect);

  // check for water level
  waterLevel = digitalRead(hallPin);
  if (waterLevel == 1) {          //waterLevel == 1
    digitalWrite(relayPin, LOW);  // literally fogger off
    Serial.println("Water no");
    digitalWrite(rsvr, HIGH);
    digitalWrite(usrLed, LOW);
    pixels.setBrightness(100);
    pixels.fill(pixels.Color(255, 0, 0));
    pixels.show();
    // fill with reservoir
  } else {
  
    digitalWrite(rsvr, LOW);
    if (manualMode) {
      manualModeF();
    } else {
      //Serial.println("auto mode");
      //Serial.println(waterLevel);

      myStepper.step(rotation);

      // growlight-like light spectrum

      pixels.setBrightness(100);
      pixels.fill(pixels.Color(255, 0, 255));
      pixels.show();

      // relay cycle
      unsigned long currentMillis = millis();

      if ((relayState == false) && (((currentMillis - previousMillis2) >= OffTime) || stateChange)) {
        relayState = HIGH;                   // Turn relay ON
        previousMillis2 = currentMillis;     // Remember the time
        digitalWrite(relayPin, relayState);  // Update the actual LED
        stateChange = false;                 // set stateChange to false
      } else if ((relayState == true) && (((currentMillis - previousMillis2) >= OnTime) && !stateChange)) {
        relayState = LOW;  // Turn relay OFF
        previousMillis2 = currentMillis;
        digitalWrite(relayPin, relayState);
        stateChange = false;
      }
    }
  }
}