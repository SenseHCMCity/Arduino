/* 
 * Modified from: http://www.instructables.com/id/make-a-wifi-air-monitor/step4/Code/ 
 * 
 * Removed the Wifi server and access point.
 * Added a software serial for the nova and logging values to the main serial.
 */

/*
 * Spec here http://inovafitness.com/upload/file/20150311/14261262164716.pdf
 * 
 * Check-sum: DATA1+DATA2+...+DATA6
 * PM2.5 value: PM2.5(ug/m3) = ((PM2.5 highbyte * 256) + PM2.5 lowbyte) / 10
 * PM10 value: PM10(ug/m3) = ((PM10 highbyte * 256) + PM10 lowbyte) / 10
 */
#include <SoftwareSerial.h>

SoftwareSerial pmSerial(8, 9, false);

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
 while (pmSerial.available() > 0) 
  {  
    // from www.inovafitness.com
    // packet format: AA C0 PM25_Low PM25_High PM10_Low PM10_High 0 0 CRC AB
     mData = pmSerial.read();     delay(2);
    if(mData == 0xAA)//head1 ok
     {
	delay(400);//wait until packet is received
        mPkt[0] =  mData;
        mData = pmSerial.read();
        if(mData == 0xc0)//head2 ok
        {
          mPkt[1] =  mData;
          // checksum: DATA1+DATA2+...+DATA6
          mCheck = 0;
          for(i=0;i < 6;i++)//data recv and crc calc
          {
             mPkt[i+2] = pmSerial.read();
             delay(2);
             mCheck += mPkt[i+2];
          }
          mPkt[8] = pmSerial.read();
          delay(1);
          mPkt[9] = pmSerial.read();
          if(mCheck == mPkt[8]) //crc ok
          {
            Serial.print("checksum=");
            Serial.println(mCheck);
            pmSerial.flush();
            //pmSerial.write(mPkt,10);

            // PM2.5 value: PM2.5(ug/m3) = ((PM2.5 highbyte * 256) + PM2.5 lowbyte) / 10
            // PM10 value: PM10(ug/m3) = ((PM10 highbyte * 256) + PM10 lowbyte) / 10
            Pm25 = (uint16_t)mPkt[2] | (uint16_t)(mPkt[3]<<8);
            Pm10 = (uint16_t)mPkt[4] | (uint16_t)(mPkt[5]<<8);
            if(Pm25 > 9999)
             Pm25 = 9999;
            if(Pm10 > 9999)
             Pm10 = 9999;            
            //get one good packet
            Pm25IsNew = 1;
            Serial.print(Pm25);
            Serial.print(",");
            Serial.println(Pm10);
             return;
          }
        }      
     }
   } 
}

void setup() {
	delay(1000);
  Serial.begin(9600);
  pmSerial.begin(9600);
	Serial.println("setup pmSerial.");
}

void loop() {
  ProcessSerialData();
}

