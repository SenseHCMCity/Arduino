/* from: http://www.instructables.com/id/make-a-wifi-air-monitor/step4/Code/ */

/* Create a WiFi access point and provide a web server for pm2.5 on it. */

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

/* Set these to your desired credentials. */
const char *ssid = "AirMonitor";
const char *password = "inovafitness";

ESP8266WebServer server(80);

// this is a demo for Arduino PM2.5 sensor test
// PM2.5 sensor is from www.inovafitness.com SDS011
unsigned int Pm25 = 0;//used for result pm2.5
unsigned int Pm10 = 0;//used for result pm10
unsigned char Pm25IsNew = 0;//show if pm25 is refreshed


void ProcessSerialData()
{
  uint8_t mData = 0;
  uint8_t i = 0;
  uint8_t mPkt[10] = {0};
  uint8_t mCheck = 0;
 while (Serial.available() > 0) 
  {  
    // from www.inovafitness.com
    // packet format: AA C0 PM25_Low PM25_High PM10_Low PM10_High 0 0 CRC AB
     mData = Serial.read();     delay(2);
    if(mData == 0xAA)//head1 ok
     {
	delay(400);//wait until packet is received
        mPkt[0] =  mData;
        mData = Serial.read();
        if(mData == 0xc0)//head2 ok
        {
          mPkt[1] =  mData;
          mCheck = 0;
          for(i=0;i < 6;i++)//data recv and crc calc
          {
             mPkt[i+2] = Serial.read();
             delay(2);
             mCheck += mPkt[i+2];
          }
          mPkt[8] = Serial.read();
          delay(1);
	  mPkt[9] = Serial.read();
          if(mCheck == mPkt[8])//crc ok
          {
            Serial.flush();
            //Serial.write(mPkt,10);

            Pm25 = (uint16_t)mPkt[2] | (uint16_t)(mPkt[3]<<8);
            Pm10 = (uint16_t)mPkt[4] | (uint16_t)(mPkt[5]<<8);
            if(Pm25 > 9999)
             Pm25 = 9999;
            if(Pm10 > 9999)
             Pm10 = 9999;            
            //get one good packet
            Pm25IsNew = 1;
             return;
          }
        }      
     }
   } 
}

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot() {
       char pm25_str[100];
       char *pm25_format_red = "<head><meta http-equiv=\"refresh\" content=\"5\"></head><h1>Pm2.5=<font color=\"red\">%d.%d</font></h1>";
       char *pm25_format_green = "<head><meta http-equiv=\"refresh\" content=\"5\"></head><h1>Pm2.5=<font color=\"green\">%d.%d</font></h1>";       
       char *pm25_format_blue = "<head><meta http-equiv=\"refresh\" content=\"5\"></head><h1>Pm2.5=<font color=\"blue\">%d.%d</font></h1>";              
       if (Pm25<150)//15.0
       sprintf(pm25_str,pm25_format_green, Pm25/10,Pm25%10);
       else if (Pm25<500)//pm2.5<50.0
       sprintf(pm25_str,pm25_format_blue, Pm25/10,Pm25%10);
       else
       sprintf(pm25_str,pm25_format_red, Pm25/10,Pm25%10);
       	server.send(200, "text/html", pm25_str);
}

void setup() {
	delay(1000);
	Serial.begin(9600);
	Serial.println();
	Serial.print("Configuring access point...");
	/* You can remove the password parameter if you want the AP to be open. */
	WiFi.softAP(ssid, password);
        delay(500); 
        delay(500); 
	//while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }

	Serial.println("done");
	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);

	server.on("/", handleRoot);
	server.begin();
	Serial.println("HTTP server started");
}

void loop() {
	server.handleClient();
        ProcessSerialData();
}

