#include <SHT1x.h>
#include <LowPower_Teensy3.h>
 
const int rfpin = 5;
 
const int gnd1_pin = 10;
const int gnd2_pin = 7;
 
const int vcc1_pin = 11;
const int vcc2_pin = 8;
 
const int data_pin = 12;
const int clk_pin = 9;
 
SHT1x sht1x(clk_pin, data_pin);
 
#define _BIT(a, b) (a&(1UL<<b))
bool flag = false;
void callbackhandler() {
 
  //flag = true;
 
  loop_func();
 
}
 
/*****************************************************
 * The first configuration sets up Low-Power Timer and
 * Digital Pin 2 as the wake up sources. 
 *****************************************************/
 TEENSY3_LP LP = TEENSY3_LP();
sleep_block_t* LP_config;// first Hibernate configuration
void sleep_mode() {
  // configure pin 2
  pinMode(2, INPUT_PULLUP);
  // config1 struct in memory
  LP_config = (sleep_block_t*) calloc(1,sizeof(sleep_block_t));
  // OR together different wake sources
  LP_config->modules = (LPTMR_WAKE);
  // Low-Power Timer wakeup in 60 secs
  //config1->rtc_alarm = 5;
  LP_config->lptmr_timeout = 65535;
  // set pin 7 or pin 9 as wake up
  //LP_config->gpio_pin = (PIN_2);
  // user callback function
  LP_config->callback = callbackhandler;
  // go to bed
  LP.Hibernate(LP_config);
}
 
void rf_raise() {
  digitalWrite(rfpin, HIGH);
}
 
void rf_fall() {
  digitalWrite(rfpin, LOW);
}
 
bool rf_pin() {
  return digitalRead(rfpin);
}
 
void rf_putdw(unsigned long dw)
{
  byte crc, *dt = (byte*)dw;
 
  rf_raise();
  delayMicroseconds(70000);
  rf_fall();
  delayMicroseconds(10000);
  rf_raise();
 
  crc = 0x00;
 
  for(int i = 0; i < 32; i++) {
    if(_BIT(dw, i)) {
      delayMicroseconds(1000);
    } else {
      delayMicroseconds(3000);
    }
    rf_fall();
    delayMicroseconds(1500);
    rf_raise();
  }
 
  crc ^= dw&0xFF;
  crc ^= ((dw&0xFF00)>>8);
  crc ^= ((dw&0xFF0000)>>16);
  crc ^= ((dw&0xFF000000)>>24);
 
  for(int i = 0; i < 8; i++) {
    if(_BIT(crc, i)) {
      delayMicroseconds(1000);
    } else {
      delayMicroseconds(3000);
    }
    rf_fall();
    delayMicroseconds(1500);
    rf_raise();
  }
 
  rf_fall();
}
 
bool rf_getdw(unsigned long* dw) {
  unsigned long tm;
 
  while(!rf_pin());
 
  tm = millis();
 
  while(rf_pin());
 
  if(!(millis() - tm > 15 && millis() - tm < 25)) {
    return 1;
  }
 
  while(!rf_pin());
  tm = millis();
 
  for(int i = 0; i < 32; i++) {
    while(rf_pin());
 
    if(abs(millis() - tm) < 7) {
      *dw |= _BIT(0xFFFFFFFF, i);
    } else if(abs(millis() - tm) < 13) {
      *dw &= ~_BIT(0xFFFFFFFF, i);
    } else {
      return 2;
    }
 
    while(!rf_pin());
    tm = millis();
  }
 
  return 0;
}
 
void periferial_start(void) {
  pinMode(rfpin, OUTPUT);
 
  pinMode(gnd1_pin, OUTPUT);
  pinMode(gnd2_pin, OUTPUT);
  pinMode(vcc1_pin, OUTPUT);
  pinMode(vcc2_pin, OUTPUT);
 
  digitalWrite(gnd1_pin, LOW);
  digitalWrite(gnd2_pin, LOW);
  digitalWrite(vcc1_pin, HIGH);
  digitalWrite(vcc2_pin, HIGH);
 
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
}
 
void periferial_stop(void) {
  pinMode(rfpin, INPUT);
 
  pinMode(gnd1_pin, INPUT);
  pinMode(gnd2_pin, INPUT);
  pinMode(vcc1_pin, INPUT);
  pinMode(vcc2_pin, INPUT);
 
  pinMode(18, INPUT_PULLUP);
  pinMode(19, INPUT_PULLUP);
 
  digitalWrite(gnd1_pin, LOW);
  digitalWrite(gnd2_pin, LOW);
  digitalWrite(vcc1_pin, LOW);
  digitalWrite(vcc2_pin, LOW);
 
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
}
 
void setup() {
  sleep_mode();
}
 
void loop() {
  if(flag) {
    flag = false;
      loop_func();  
  sleep_mode();
  }
}
 
void loop_func() {
  unsigned long msg;
  byte temp, humidity;
 
  periferial_start();
 
  delay(30);
 
  temp = (byte)(sht1x.readTemperatureC() + 40.)*2;
  humidity = (byte)sht1x.readHumidity();
 
  msg = 0;
  msg |= humidity;
  msg <<= 8;
  msg |= temp;
 
  for(int i = 0; i < 3; i++) {
    rf_putdw(msg);
  }
 
  periferial_stop();
}
