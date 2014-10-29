====== Беспроводная метеостанция ======

{{ :projects:meteo_overview.jpg?direct&700 }}

  * Платформы: Teensy, Arduino Uno
  * Языки программирования: [[wp>Wiring_(development_platform) | Wiring (C++)]]
  * Проект на [[wpru>GitHub]]: http://github.com/amperka-projects/wirelessmeteo
  * Тэги: метеостанция, погода, температура, влажность, прогноз

===== Что это такое? =====

В этой статье мы расскажем о том, как собрать полноценную метеостанцию,
передающую данные о погоде на широко известный сервис
«[[http://narodmon.ru/|народный мониторинг]]».

Наша метеостанция будет состоять из двух устройств: компактного автономного
устройства, измеряющего погодные показатели, и устройства-ретранслятора,
получающего эти показатели и отправляющего их на «народный мониторинг».
Устройства будут связываться по беспроводному каналу связи на частоте 433 МГц.
Автономная часть будет питаться от трёх пальчиковых батареек и сможет
просуществовать на одном комплекте батарей до года (при периоде опроса датчиков
в 20 мин).

Такая конструкция позволяет не сверлить стены для прокладки проводов с улицы
(где необходимо производить измерения) в помещение (где результатами этих
измерений надо пользоваться).

===== Что для этого необходимо? =====

{{ :projects:meteo_collage.jpg?direct&700 }}

Для изготовления автономного передатчика нам понадобятся:
  -[[amp>product/teensy-31|Teensy 3.1]]
  -[[amp>product/rf-433-transmitter|Передатчик на 433 МГц]]
  -[[amp>product/temperature-humidity-sensor-sht1x|Датчик температуры и влажности]]
  -[[amp>product/breadboard-mini|Макетная плата Mini]]
  -[[amp>product/battery-holder-3-aa|Держатель пальчиковых батареек на x3 AA]]
  -[[amp>product/wire-mm|Провода папа-папа ×3 шт]]
  -[[amp>product/resistor|Резистор на 100 кΩ]]
  -[[amp>product/resistor|Резистор на 10 кΩ]]

Для изготовления ретранслятора нам понадобятся:
  -[[amp>product/arduino-uno|Arduino Uno]]
  -[[amp>product/arduino-ethernet|Arduino Ethernet]]
  -[[amp>product/rf-433-receiver|Приёмник на 433 МГц]]
  -[[amp>product/breadboard-mini|Макетная плата Mini]]
  -[[amp>product/wire-mm|Провода папа-папа ×4 шт]]

Так же удобно установить два светодиода для индикации процессов:
  -[[amp>product/wire-mm|Провода папа-папа ×2 шт]]
  -[[amp>product/led-5mm|Светодиод красный ×1 шт]]
  -[[amp>product/led-5mm|Светодиод зелёный ×1 шт]]
  -[[amp>product/resistor|Резисторы на 220 Ω x2 шт]]

Для звуковой индикации разряда батареи автономной части удобно использовать
пьезо-пищалку:
  -[[amp>product/wire-mm|Провод папа-папа ×1 шт]]
  -[[amp>product/piezo-buzzer|Пьезо-пищалка]]

===== Как это собрать? =====

==== Сборка автономной части ====

  - Комментарий {{ :projects:meteo_build_sender_0.jpg?direct&700 }}
  - Комментарий {{ :projects:meteo_build_sender_1.jpg?direct&700 }}
  - Комментарий {{ :projects:meteo_build_sender_2.jpg?direct&700 }}
  - Комментарий {{ :projects:meteo_build_sender_3.jpg?direct&700 }}
  - Комментарий {{ :projects:meteo_build_sender_4.jpg?direct&700 }}

==== Сборка ретранслятора ====

  - Комментарий {{ :projects:meteo_build_receiver_0.jpg?direct&700 }}
  - Комментарий {{ :projects:meteo_build_receiver_1.jpg?direct&700 }}
  - Комментарий {{ :projects:meteo_build_receiver_2.jpg?direct&700 }}
  - Комментарий {{ :projects:meteo_build_receiver_3.jpg?direct&700 }}

===== Исходный код =====

===== Код автономной части =====

<code cpp meteo_sensor.ino>
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


</code>

===== Код платы, работающей в помещении =====

<code cpp receiver.ino>

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

</code>

===== Регистрация метеостанции в «Народном мониторинге» =====

Чтобы данные, передаваемые нашим устройством, корректно отображались на народном
мониторинге, необходимо выполнить следующее:

  - Установить уникальный MAC-адрес устройства.
  - Зарегистрироваться на сайте «Народного мониторинга».{{ :projects:meteo_register.png?direct&700 }}
  - Авторизоваться.
  - Открыть список датчиков и установить номиналы передаваемых данных.{{ :projects:meteo_sensorsfix.png?direct&700 }}


===== Демонстрация работы устройства =====

{{youtube>R7ILpfYvDNc?large}}

===== Что ещё можно сделать? =====

  - Мы установили только сенсор температуры и влажности. Но у Teensy остаётся
    ещё много свободных ножек, т.ч. можно добавить разных датчиков:
    освещённости, атмосферного давления, скорости ветра и т.д..
  - Teensy прямо на борту имеет часы реального времени (RTC). Для их
    работоспособности не хватает только кварца. Можно купить кварц на 32,768
    КГц и припаять его. Тогда можно пробуждать Teensy по будильнику RTC.
    Достоинство в том, что можно будить устройство чаще в те часы, когда нужны
    более точные показания. Например, в рабочее время будить устройство каждые
    5 минут, а в остальное — каждые полчаса.
