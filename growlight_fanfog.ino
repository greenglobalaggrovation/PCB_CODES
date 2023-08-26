// Define pins for fogger, fan, and grow light
const int foggerPin = 5;   // Replace with the actual fogger pin number
const int fanPin = 4;      // Replace with the actual fan pin number
const int growLightPin = 3; // Replace with the actual grow light pin number

// Define time intervals in milliseconds
const unsigned long foggerOnDuration = 18000;  // 5 minutes
const unsigned long foggerOffDuration = 180000;  // 1 minute
const unsigned long fanOnDuration = 30000;     // 5 minutes
const unsigned long fanOffDuration = 60000;     // 1 minute
const unsigned long growLightOnDuration = 60000; // 5 minutes
const unsigned long growLightOffDuration = 60000; // 1 minute

unsigned long foggerStartTime = 60;
unsigned long fanStartTime = 60;
unsigned long growLightStartTime = 60;

bool growLightOn = false;

void setup() {
  pinMode(foggerPin, OUTPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(growLightPin, OUTPUT);
  
  // Turn on fogger and fan immediately
  digitalWrite(foggerPin, HIGH);
  digitalWrite(fanPin, HIGH);

  // Delay for the initial duration
  delay(foggerOnDuration);

  // Turn them off
  digitalWrite(foggerPin, LOW);
  digitalWrite(fanPin, LOW);
}


void loop() {
  unsigned long currentTime = millis();

  // Control the fogger
  if (currentTime - foggerStartTime >= (foggerOnDuration + foggerOffDuration)) {
    foggerStartTime = currentTime;
  }

  if (currentTime - foggerStartTime < foggerOnDuration) {
    digitalWrite(foggerPin, HIGH); // Turn fogger on
  } else {
    digitalWrite(foggerPin, LOW); // Turn fogger off
  }

  // Control the fan
  if (currentTime - fanStartTime >= (fanOnDuration + fanOffDuration)) {
    fanStartTime = currentTime;
  }

  if (currentTime - fanStartTime < fanOnDuration) {
    digitalWrite(fanPin, HIGH); // Turn fan on
  } else {
    digitalWrite(fanPin, LOW); // Turn fan off
  }

  // Control the grow light
  if (currentTime - growLightStartTime >= (growLightOn ? growLightOnDuration : growLightOffDuration)) {
    growLightOn = !growLightOn; // Toggle the state
    growLightStartTime = currentTime;
  }

  digitalWrite(growLightPin, growLightOn ? HIGH : LOW); // Turn grow light on or off
}