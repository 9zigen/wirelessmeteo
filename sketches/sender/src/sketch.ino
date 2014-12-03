#include <Arduino.h>
#include <SHT1x.h>
#include <LowPower_Teensy3.h>
#include <ampline.h>


// Таймаут между посылками (не более 65535)
#define TIMEOUT 60000

// Количество попыток отправки посылки
#define ATTEMPTS 3
 
// Информационный пин передатчика
#define RF_PIN 5

// Пины датчика температуры и влажности
#define GND1_PIN 10
#define VCC1_PIN 11
#define GND2_PIN 7
#define VCC2_PIN 8
#define DATA_PIN 12
#define CLK_PIN  9
 

AmperkaLine rf(RF_PIN);
SHT1x sht1x(CLK_PIN, DATA_PIN);
 

void loop(void);
 

// Функция усыпления платы. Каждые TIMEOUT секунд
// будет вызываться функция loop_func.
TEENSY3_LP LP = TEENSY3_LP();
sleep_block_t* LP_config;

void sleep_mode(void)
{
    LP_config = (sleep_block_t*)calloc(1,sizeof(sleep_block_t));

    // Просыпаться будем по таймеру
    LP_config->modules = (LPTMR_WAKE);
    // Задаём таймаут для таймера
    LP_config->lptmr_timeout = TIMEOUT;
    // По истечении таймаута будет вызываться функция loop
    LP_config->callback = loop;

    LP.Hibernate(LP_config);
}
 

// Функция включения периферии
void periferial_start(void)
{
    // Включаем линию передачи данных
    pinMode(RF_PIN, OUTPUT);

    // Включаем питания и земли датчиков температуры и влажности
    pinMode(GND1_PIN, OUTPUT);
    pinMode(GND2_PIN, OUTPUT);
    pinMode(VCC1_PIN, OUTPUT);
    pinMode(VCC2_PIN, OUTPUT);
    digitalWrite(GND1_PIN, LOW);
    digitalWrite(GND2_PIN, LOW);
    digitalWrite(VCC1_PIN, HIGH);
    digitalWrite(VCC2_PIN, HIGH);

    // Включаем светодиод для индикации передачи
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    // Выбираем в качестве опорного напряжения внутренний
    // источник (=1.2 В)
    analogReference(INTERNAL);
}
 

// Функция выключения периферии
void periferial_stop(void)
{
    // Выключаем линию передачи данных
    pinMode(RF_PIN, INPUT);

    // Выключаем датчик температуры и влажности
    pinMode(GND1_PIN, INPUT);
    pinMode(GND2_PIN, INPUT);
    pinMode(VCC1_PIN, INPUT);
    pinMode(VCC2_PIN, INPUT);

    pinMode(18, INPUT_PULLUP);
    pinMode(19, INPUT_PULLUP);

    // Выключаем светодиод
    digitalWrite(LED_BUILTIN, LOW);
}

void setup(void)
{
    // Ничего не инициализируем, сразу засыпаем
    sleep_mode();
}
 
// Эта функция выполняется раз в TIMEOUT секунд
void loop(void)
{
    unsigned long msg;
    byte temp, humidity, voltage;

    // Включаем периферию
    periferial_start();

    // Подождём, пока включится датчик температуры и влажности
    delay(30);

    // Получаем входные данные с сенсоров
    temp = (byte)(sht1x.readTemperatureC() + 40.)*2;
    humidity = (byte)sht1x.readHumidity();
    voltage = analogRead(A0)/4;

    // Составляем из данных посылку
    msg = 0;
    msg |= voltage;
    msg <<= 8;
    msg |= humidity;
    msg <<= 8;
    msg |= temp;

    // Отправляем несколько раз посылку
    for(int i = 0; i < ATTEMPTS; i++) rf.send(msg);

    // Выключаем периферию
    periferial_stop();

    // После выхода из функции плата снова уснёт
}

