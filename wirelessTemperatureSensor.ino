/*
 Name:		wirelessTemperatureSensor.ino
 Created:	3/24/2021 10:50:53 PM
 Author:	DM
*/
#include <BMP280_DEV.h>
#include <VirtualWire.h>
#include <PMU.h>

#define RF_PIN 4
#define EN_PIN 2
#define LED 13

#define MINUTES 5

BMP280_DEV bmp280;

typedef union transmission_packet 
{
    float value;
    byte packet[sizeof(float)];
};

transmission_packet temperature;
transmission_packet pressure;
transmission_packet altitude;

byte sleepCounter = 0;


void setup() 
{
    pinMode(LED, OUTPUT);
    pinMode(EN_PIN, OUTPUT);
    pinMode(RF_PIN, OUTPUT);
    digitalWrite(LED, LOW);
    //Serial.begin(9600);
    bmp280.begin(FORCED_MODE, BMP280_I2C_ALT_ADDR);
    vw_set_tx_pin(RF_PIN);
    vw_set_ptt_pin(EN_PIN);
    vw_set_ptt_inverted(false);                 // false! иначе передатчик останется включенным
    vw_setup(1200);                             // Bits per sec
    sendPacket();
}

void loop() 
{
    if (sleepCounter < MINUTES * 2)
    {
        sleepCounter++;
        PMU.sleep(PM_POFFS0, SLEEP_32S);
    }
    else
    {
        sendPacket();
        sleepCounter = 0;
    }
}

void sendPacket()
{
    bmp280.startForcedConversion();
    while (!bmp280.getMeasurements(temperature.value, pressure.value, altitude.value)) delay(32);
    bmp280.stopConversion();
    byte bytes[sizeof(float) * 3];
    memcpy(bytes, temperature.packet, sizeof(float));
    memcpy(bytes + sizeof(float), pressure.packet, sizeof(float));
    memcpy(bytes + sizeof(float)*2, altitude.packet, sizeof(float));
    digitalWrite(LED, HIGH);
    vw_send((byte*)bytes, sizeof(bytes));
    vw_wait_tx();   // Wait until the whole message is gone    
    digitalWrite(LED, LOW);
    /*
    Serial.print(F("Temperature = "));
    Serial.print(temperature.value);
    Serial.println(" *C");

    Serial.print(F("Pressure = "));
    Serial.print(pressure.value * 0.75006156);
    Serial.println(" mmHg");

    Serial.println("");
    */
}