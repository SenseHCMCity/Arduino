// Simple demo of four threads
// LED blink thread, sensor read, print thread, and main thread
#include <ChibiOS_AVR.h>

#include <DHT.h>

const uint8_t LED_PIN = 13;

//------------------------------------------------------------------------------
// thread 1 - high priority for blinking LED
// 64 byte stack beyond task switch and interrupt needs
static WORKING_AREA(waBlinkThread, 64);

int ledOnTime, ledOffTime;

static msg_t blinkThread(void *arg) {
  pinMode(LED_PIN, OUTPUT);
  
  // Flash led every 200 ms.
  while (1) {
    // Turn LED on.
    digitalWrite(LED_PIN, HIGH);
    
    chThdSleepMilliseconds(ledOnTime);
    
    // Turn LED off.
    digitalWrite(LED_PIN, LOW);
    
    chThdSleepMilliseconds(ledOffTime);
  }
  return 0;
}

//------------------------------------------------------------------------------
// thread 2 - low priority for reading DHT sensors
// 64 byte stack beyond task switch and interrupt needs
static WORKING_AREA(waDHTSensorThread, 64);

float humidity;
float temperature;
bool dhtSensorStatus;

static msg_t dhtSensorThread(void *arg) {
  DHT dht(A0, DHT22);
  float t, h;
  
  dht.begin();
  dhtSensorStatus = true;
  
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  // signal error if sensor not properly installed
  if (isnan(humidity) || isnan(temperature)) {
    dhtSensorStatus = false;  
  }
  
  // read value every 1000ms and smooth out values
  while (1) {
    chThdSleepMilliseconds(1000);
    t = dht.readTemperature(); h = dht.readHumidity();
    if (isnan(h) || isnan(t)) {
      dhtSensorStatus = false;
      continue;
    } else {
      dhtSensorStatus = true;
      humidity = (humidity + h)/2;
      temperature = (temperature + t)/2;
    }
  }
  
  return 0;
}

//------------------------------------------------------------------------------
// thread 3 - print sensor value every 5 seconds
// 100 byte stack beyond task switch and interrupt needs
static WORKING_AREA(waPrintThread, 100);

#define NUM_SENSORS 2 
static bool *sensorStatus[NUM_SENSORS] = {&dhtSensorStatus, 
                                          &dhtSensorStatus };
static float *sensorValue[NUM_SENSORS] = {&humidity,
                                          &temperature };
static char *sensorName[NUM_SENSORS] = { "humidity",
                                         "temperature" };
static msg_t printThread(void *arg) {

  // print sensor value every 5 seconds
  while (1) {
    chThdSleepMilliseconds(5000);
    
    // check the error in sensors, signal if needed
    for (int ii = 0; ii < NUM_SENSORS; ii++) {
      if (false == *sensorStatus[ii])
        ledOnTime = 100;
        ledOffTime = 900;
        continue;
      }
    } 
    
    // turn off signaling if sensor ok
    ledOnTime = ledOffTime = 500;  
    // no error in sensors, begin to print out data
    Serial.print('{');
    for (int ii = 0; ii < NUM_SENSORS; ii++) {
      Serial.print(F("{\'"));
      Serial.print(sensorName[ii]);
      Serial.print(F("\':"));
      Serial.print(*sensorValue[ii]);
      Serial.print('}');
      if (ii != NUM_SENSORS - 1)
        Serial.print(',');
    }
    
    Serial.println('}');
  }
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  // wait for USB Serial
  while (!Serial) {}
  
  // read any input
  delay(200);
  while (Serial.read() >= 0) {}
  
  ledOnTime = ledOffTime = 500;
  chBegin(mainThread);
  // chBegin never returns, main thread continues with mainThread()
  while(1) {}
}
//------------------------------------------------------------------------------
// main thread runs at NORMALPRIO
void mainThread() {

  // start blink thread
  chThdCreateStatic(waBlinkThread, sizeof(waBlinkThread),
                          NORMALPRIO + 2, blinkThread, NULL);

  // start print thread
  chThdCreateStatic(waPrintThread, sizeof(waPrintThread),
                          NORMALPRIO + 1, printThread, NULL);
  // start DHT sensor read job
  chThdCreateStatic(waDHTSensorThread, sizeof(waDHTSensorThread),
                          NORMALPRIO, dhtSensorThread, NULL);

  // nothing to do in the main thread yet
  while (1) {
    chThdSleepMilliseconds(5000);
  }
}
//------------------------------------------------------------------------------
void loop() {
 // not used
}
