#include <SPI.h>
#include <Ethernet.h>
 
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0F, 0xBC, 0x71 };
 
char server[] = "narodmon.ru";
 
EthernetClient client;
 
const int rfpin = 7;
 
#define _BIT(a, b) (a&(1UL<<b))
 
 
void rf_raise() {
  digitalWrite(rfpin, HIGH);
}
 
void rf_fall() {
  digitalWrite(rfpin, LOW);
}
 
bool rf_pin() {
  return digitalRead(rfpin);
}
 
bool rf_getdw(unsigned long* dw) {
  unsigned long tm;
  byte crc, rcrc;
 
  do {
  while(!rf_pin());
  tm = micros();
  while(rf_pin());
  } while(!((micros() - tm) > 45000 && (micros() - tm) < 72000));
 
  crc = 0;
 
  while(!rf_pin());
  tm = micros();
 
  for(int i = 0; i < 32; i++) {
    while(rf_pin());
 
    if(abs(micros() - tm) < 1500) {
      *dw |= _BIT(0xFFFFFFFF, i);
    } else if(abs(micros() - tm) < 4000) {
      *dw &= ~_BIT(0xFFFFFFFF, i);
    } else {
      //Serial.println("!!!");
      return 2;
    }
 
    while(!rf_pin());
    tm = micros();
  }
 
  crc ^= (*dw)&0xFF;
  crc ^= (((*dw)&0xFF00)>>8);
  crc ^= (((*dw)&0xFF0000)>>16);
  crc ^= (((*dw)&0xFF000000)>>24);
  rcrc = 0;
 
  for(int i = 0; i < 8; i++) {
    while(rf_pin());
 
    if(abs(micros() - tm) < 1500) {
      rcrc |= _BIT(0xFFFFFFFF, i);
    } else if(abs(micros() - tm) < 4000) {
      rcrc &= ~_BIT(0xFFFFFFFF, i);
    } else {
      //Serial.println("!!!");
      return 2;
    }
 
    while(!rf_pin());
    tm = micros();
  }
 
  if(rcrc != crc) { /*Serial.print(rcrc, HEX); Serial.print(" - "); Serial.println(crc, HEX);*/ return 3; }
 
  return 0;
}
 
 
void setup() {
  pinMode(rfpin, INPUT);
  pinMode(6, OUTPUT);
 
  Serial.begin(9600);
  Serial.println("Started.");
}
 
void loop() {
  unsigned long msg;
  static unsigned long pushtimeout = 0;
  static float temp, humidity, voltage;
  int res;
 
  if((res = rf_getdw(&msg)) == 0) {      
      temp = ((float)(msg&0xFF))/2. - 40.;
      msg >>= 8;
      humidity = (float)(msg&0xFF);
      msg >>= 8;
      voltage = (float)(msg&0xFF) / 256. * 1.2 * 10 * 1.1;
 
      digitalWrite(6, HIGH);
      Serial.print("Temp: ");
      Serial.print(temp);
      Serial.print(", humidity: ");
      Serial.print(humidity);
      Serial.print(", voltage: ");
      Serial.println(voltage);
      digitalWrite(6, LOW);
  }
  else Serial.println('E');
 
  if(millis() - pushtimeout > 60000*5) {
    pushtimeout = millis();
 
    Serial.println("Starting Ethernet...");
 
    if (Ethernet.begin(mac) == 0) {
      Serial.println("Failed to configure Ethernet using DHCP");
      while(1) { }
    }
    delay(1000);
    Serial.println("connecting...");
 
    if (client.connect(server, 8283)) {
      Serial.println("connected");
 
      client.println("#90-A2-DA-0F-BC-71#AmperkaWeather#55.7466#37.6625#40.0");
 
      client.print("#90A2DA0FBC7101#");
      client.print(temp, DEC);
      client.println("#In");
 
      client.print("#90A2DA0FBC7102#");
      client.print(humidity, DEC);
      client.println("#Humidity");
 
      client.print("#90A2DA0FBC7103#");
      client.print(voltage, DEC);
      client.println("#Voltage");
 
      client.println("##");
    } 
    else {
      Serial.println("connection failed");
    }
 
    {
      unsigned long tm = millis();
 
      while(millis() - tm < 5000) {
        if (client.available()) {
          char c = client.read();
          Serial.print(c);
        }
      }
    }
 
    client.stop();
  }
}
