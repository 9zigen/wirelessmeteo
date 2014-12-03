#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <ampline.h>
 

byte mac[] = { 0x95, 0xA4, 0xDA, 0x0F, 0xBC, 0x72 };
 
char server[] = "narodmon.ru";
 
EthernetClient client;
 
const int rfpin = 7;
AmperkaLine rf(rfpin);
 
void setup(void)
{
    pinMode(rfpin, INPUT);
    pinMode(6, OUTPUT);
   
    Serial.begin(9600);
    Serial.println("Started.");
}
 
void loop(void)
{
    static unsigned long pushtimeout = 0;
    static float temp, humidity, voltage;
    unsigned long msg;
    int res;
   
    if((res = rf.receive(&msg)) == 0)
    {
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
   pushtimeout = millis();
    if(millis() - pushtimeout > 60000*5)
    {
        pushtimeout = millis();
   
        Serial.println("Starting Ethernet...");
   
        if (Ethernet.begin(mac) == 0)
        {
            Serial.println("Failed to configure Ethernet using DHCP");
            while(1) { }
        }
        delay(1000);
        Serial.println("connecting...");
   
        if (client.connect(server, 8283))
        {
            Serial.println("connected");
   
            client.println("#95-A4-DA-0F-BC-72#AmperkaWeather#55.746888#37.660425#40.0");
   
            client.print("#95A4DA0FBC7201#");
            client.print(temp, DEC);
            client.println("#In");
   
            client.print("#95A4DA0FBC7202#");
            client.print(humidity, DEC);
            client.println("#Humidity");
   
            client.print("#95A4DA0FBC7203#");
            client.print(voltage, DEC);
            client.println("#Voltage");
   
            client.println("##");
        } 
        else Serial.println("connection failed");
   
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

