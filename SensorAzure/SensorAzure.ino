//PROJECT SENSEHCMC at FABLAB SAIGON
//Work done by Mai Nguyen on Sat 2 May 2014
//Test successfully with humi&temp, air quality, sound sensors
//Issue: Data is received intermittently! even though sending seems OK.

#include "DHT.h"
#include <SoftwareSerial.h>
//#include "AirQuality.h"

#define UV_PORT A0
#define AQLT_PORT A1
#define SOUND_PORT A2
#define TEMPHUM_PORT A3

#define channelID 32615
#define APIKey 0AZUPUIQWSFIHP0R
#define IP "184.106.153.149" //ThingSpeak
String GET = "GET /update?api_key=0AZUPUIQWSFIHP0R";

SoftwareSerial wifiSerial(2, 3); // RX, TX

void panic(char *reason) {
  Serial.print("Panic: ");
  Serial.println(reason);
 
  while(1) {
   
  } 
}

/*
    Part A: Temperature and Humidity Sensor
 */

float thHumidity;
float thTemp;
DHT dht(TEMPHUM_PORT, DHT11);

void thSetup() {
  //Serial.println("DHT setup");
  thHumidity = 0.0f;
  thTemp = 0.0f;
  dht.begin();
}

void thReadTemp() {
  //Serial.println("DHT readTemp");
  thTemp = dht.readTemperature();
  //if (isnan(thTemp))
    //thTemp = 0.0f;
}

void thReadHumidity() {
   //Serial.println("DHT readHumidity");
   thHumidity = dht.readHumidity();
   if (isnan(thHumidity))
     thHumidity = 0.0f;  
}

/*
    Part B: Air Quality Sensor
 */
float currentQuality;
void AirQualityLoop()
{
 int val;
 val = analogRead(AQLT_PORT);
 currentQuality = val;
 //Serial.println(val);
}

/*
    Part C: Sound Sensor
 */
 float currentSound;
 void SoundLoop()
 {
   int sensorsum = 0;
   for (int i = 0; i < 32; i++) {
      int sensorValue = analogRead(SOUND_PORT);
      sensorsum = sensorsum + sensorValue;        
   }
   currentSound = sensorsum / 32;
   //Serial.print("sensorsum ");
   //Serial.println(sensorsum);  
 }
 
 /*
    Part D: UV Sensor
 */
 float currentUV;
 
 void UVLoop()
{  
  int sensorValue;
  long  sum=0;
  for(int i=0;i<1024;i++)
   {  
      sensorValue=analogRead(UV_PORT);
      sum=sensorValue+sum;
      delay(2);
   }   
  sum = sum >> 10;
  currentUV = sum*4980.0/1023.0;
 //Serial.print("The voltage value:");
 //Serial.print(sum*4980.0/1023.0);
 //Serial.println("mV");
 //delay(20);
 //Serial.print("\n");
 
}
 
//--------------------------------------------
#define NUMBER_OF_VAR 5

float* dataToSend[NUMBER_OF_VAR] = {
  &thHumidity,
  &thTemp,
  &currentQuality,
  &currentSound,
  &currentUV,
};

//char* stringLiteral[NUMBER_OF_VAR] = {
//  "\"humi\"",
//  "\"temp\"",
//  "\"aqlt\"",
//  "\"sound\"",
//  "\"uv\"",    
//};

char* stringLiteral[NUMBER_OF_VAR] = {
  "\"Sensor1\"", //Humidity
  "\"Sensor2\"", //Temperature
  "\"Sensor3\"", //Air Quality
  "\"Sensor4\"", //Sound
  "\"Sensor5\"", //UV   
}; 

 
/*
    Part 2: Communication part
 */

int readline(int readch, char *buffer, int len)
{
  static int pos = 0;
  int rpos;

  if (readch > 0) {
    switch (readch) {
      case '\n': // Ignore new-lines
        break;  
      case '\r': // Return on CR
        rpos = pos;
        pos = 0;  // Reset position index ready for next time
        return rpos;
      default:
        if (pos < len-1) {
          buffer[pos++] = readch;
          buffer[pos] = 0;
        }
    }
  }
  else { 
    //Serial.println("readch is 0 or less");
  }  
  // No end of line has been found, so return -1.
  return -1;
} 

char buffer[80];
int n;
void wifiSetup() {
  wifiSerial.begin(9600);
  
  Serial.println("Reset module");
  wifiSerial.println("AT+RST");

  while(1) {
    if (readline(wifiSerial.read(), buffer, 80) > 0) {
      Serial.println(buffer);
      int len = strlen(buffer);
      if (0 == strcmp(buffer+len-5, "ready"))
      //if (0 == strcmp(buffer, "ready"))

        break;
      if (0 == strcmp(buffer+len-4, "FAIL"))
      //if (0 == strcmp(buffer, "FAIL"))

        panic("Error");
    }
  }

  Serial.println("Change mode");
  wifiSerial.println("AT+CWMODE=1");
  
  //memset(buffer, 0, 80);
  while(1) {
    if (readline(wifiSerial.read(), buffer, 80) > 0) {
      Serial.println(buffer);
      if (0 == strcmp(buffer, "OK"))
        break;
      if (0 == strcmp(buffer, "FAIL"))
        panic("Error");
    }
  }
    
  Serial.println("Join accesspoint");
  wifiSerial.println("AT+CWJAP=\"Fablab Saigon\",\"011235813\"");

  //memset(buffer, 0, 80);
  while(1) {
    if (readline(wifiSerial.read(), buffer, 80) > 0) {
      Serial.println(buffer);
      if (0 == strcmp(buffer, "OK"))
        break;
      if (0 == strcmp(buffer, "FAIL"))
        panic("Error");
    }
  }
  
  Serial.println("Start connection");

//  if(wifiSerial.find("Error")){
//    Serial.println("AT+CIPSTART error");
//    return;
//  }


}

void wifiLoop() {
  wifiSerial.println("AT+CIPSTART=\"UDP\",\"sensehcmc.cloudapp.net\",5005");
  
  while(1) {
    if (readline(wifiSerial.read(), buffer, 80) > 0) {
      //Serial.println(buffer);
      if (0 == strcmp(buffer, "OK"))
      { Serial.println("Cloudapp connected");
        break;
      }
      if (0 == strcmp(buffer, "FAIL"))
        panic("Error");
    }
  }
  
  Serial.println("Send data over wifi");
  
// Modifications for ThingSpeak 
/* String cmd = GET + "&field1=" + *dataToSend[0] +
                "&field2="+ dataToSend[1] + 
                "&field3=" + dataToSend[2] +"\r\n";
                "&field4=" + *dataToSend[3] + 
                "&field5=" + *dataToSend[4] + "\r\n";
//  String  cmd = GET + "&field1=" + "10" + "\r\n"; */

  //String  cmd = String("{\"Sensor1\":") + (10+*dataToSend[0]) + "}" + "\r\n";

  String cmd = String("{\"Sensor1\":") + *dataToSend[0] + "," + 
                String("\"Sensor2\":") + *dataToSend[1] + "," + 
                String("\"Sensor3\":") + *dataToSend[2] + "," + 
                String("\"Sensor4\":") + *dataToSend[3] + "," + 
                String("\"Sensor5\":") + *dataToSend[4] + "}" + "\r\n";

  Serial.print(cmd);
  
  wifiSerial.print("AT+CIPSEND=");
  Serial.print("AT+CIPSEND=");
  //wifiSerial.println(cmd.length());
  wifiSerial.println(n);
  //Serial.println(n);
  

  Serial.println("Send OK");

  while (1) {
    if (wifiSerial.available() > 0) {
      char c = wifiSerial.read();
      Serial.print(c);
      if (c == '>')  
        break;
    }
  }
  
  //wifiSerial.print(cmd);
  wifiSerial.print("{");
  for (int ii = 0; ii < NUMBER_OF_VAR; ii++) {
    wifiSerial.print(stringLiteral[ii]);
    wifiSerial.print(":");
    wifiSerial.print(*dataToSend[ii]);
    if (ii != NUMBER_OF_VAR - 1) {
      wifiSerial.print(","); 
    }
  }
  wifiSerial.println("}");
  Serial.println("Send OK2");

  while (1) {
    if (readline(wifiSerial.read(), buffer, 80) > 0) {
      //Serial.println(buffer);
      if (0 == strcmp(buffer, "SEND OK"))
      { 
        break;
      }
      if (0 == strcmp(buffer, "FAIL"))
        panic("Error");
    }
  }
  
  Serial.println("AT+CIPCLOSE");
  wifiSerial.println("AT+CIPCLOSE");
  
  while(1) {
    if (readline(wifiSerial.read(), buffer, 80) > 0) {
      //Serial.println(buffer);
      if (0 == strcmp(buffer, "OK"))
      { 
        Serial.println("Cloudapp disconnected");
        break;
      }
      if (0 == strcmp(buffer, "FAIL"))
        panic("Error");
    }
  }
  
  //delay(15000);

}

/*
    Part 3: Debugging components
 */
String sOut;

void serialDebugSetup() {
  Serial.begin(115200);
  Serial.println("Hello, world"); 
}

void serialDebugSend() {
  n = 0;
  n += Serial.print("{");
  for (int ii = 0; ii < NUMBER_OF_VAR; ii++) {
    n += Serial.print(stringLiteral[ii]);
    n += Serial.print(":");
    n += Serial.print(*dataToSend[ii]);
    if (ii != NUMBER_OF_VAR - 1) {
      n += Serial.print(","); 
    }
  }
  n += Serial.println("}");
  
  Serial.println(n);
}

struct sensor {
  void (*startup)();
  void (*periodic)();
  unsigned long period;
};

/*
    Setup to tie everything up
 */
static const struct sensor sensors[] = {
   {serialDebugSetup, serialDebugSend, 10000},
   {wifiSetup, wifiLoop, 10000},
   {thSetup, thReadTemp, 1000},
   {NULL, thReadHumidity, 1000},
   {NULL, AirQualityLoop, 1000},
   {NULL, SoundLoop, 1000},   
   {NULL, UVLoop, 1000},    
};

static const int nsensors = sizeof(sensors)/sizeof(struct sensor);

unsigned long lastTriggerTime[nsensors];
unsigned long timeCurrent;

void setup() {
  // run all the startup function:
  for (int ii = 0; ii < nsensors; ii++) {
    if (sensors[ii].startup)
      (*sensors[ii].startup)();           
  }
  
  timeCurrent = millis();
  // setup the next timer
  for (int ii = 0; ii < nsensors; ii++) {
    lastTriggerTime[ii] = 0;  
  }
}



void loop() {
  // check if any timeout happen and execute the appropriate handler
  timeCurrent = millis();
  for (int ii = 0; ii < nsensors; ii++) {
    if (timeCurrent - lastTriggerTime[ii] >= sensors[ii].period) {
      lastTriggerTime[ii] = timeCurrent;
      if (sensors[ii].periodic)
        (*sensors[ii].periodic)();           
    }
  }  
}
