/* 
 * Modified from: http://www.instructables.com/id/make-a-wifi-air-monitor/step4/Code/ 
 * 
 * Removed the Wifi server and access point.
 * Added a software serial for the nova and logging values to the main serial.
 */

/*
 * PM Sensor spec here http://inovafitness.com/upload/file/20150311/14261262164716.pdf
 * 
 * packet format: AA C0 pm2_5_Low pm2_5_High pm10_Low pm10_High 0 0 CRC AB
 * Check-sum: DATA1+DATA2+...+DATA6
 * PM2.5 value: PM2.5(ug/m3) = ((PM2.5 highbyte * 256) + PM2.5 lowbyte) / 10
 * pm10 value: pm10(ug/m3) = ((pm10 highbyte * 256) + pm10 lowbyte) / 10
 */
#include <SoftwareSerial.h>
#include <Time.h>

// use software serial to capture sensor data
const int RX_PIN = 8; 
const int TX_PIN = 9; 
SoftwareSerial pmSerial(RX_PIN, TX_PIN, false);

void ProcessSerialData()
{
  uint8_t mData = 0;
  uint8_t i = 0;
  uint8_t mPkt[10] = {0};
  uint8_t mCheck = 0;
 while (pmSerial.available() > 0) 
  {  
    // packet format: AA C0 pm2_5_Low pm2_5_High pm10_Low pm10_High 0 0 CRC AB
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
            pmSerial.flush();

            // PM2.5 value: PM2.5(ug/m3) = ((PM2.5 highbyte * 256) + PM2.5 lowbyte) / 10
            // pm10 value: pm10(ug/m3) = ((pm10 highbyte * 256) + pm10 lowbyte) / 10
            unsigned int pm2_5 = (uint16_t)mPkt[2] | (uint16_t)(mPkt[3]<<8);
            unsigned int pm10 = (uint16_t)mPkt[4] | (uint16_t)(mPkt[5]<<8);
            if(pm2_5 > 9999)
             pm2_5 = 9999;
            if(pm10 > 9999)
             pm10 = 9999;        
            
            float finalPm2_5 = pm2_5 / 10;
            float finalPm10 = pm10 / 10;

            Serial.print(now());
            Serial.print(",");
            Serial.print(finalPm2_5);
            Serial.print(",");
            Serial.println(finalPm10);
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
  delay(29000);
}

