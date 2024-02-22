#define TWO_POINT_CALIBRATION 0

#include <Arduino.h>
#include <Wire.h> // Library komunikasi I2C 
#include <LiquidCrystal_I2C.h> // Library modul I2C LCD
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// #include <ESP8266WiFi.h>
// #include <ESP8266HTTPClient.h>


#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

float sensor_suhu = 0;
float Fahrenheit = 0;

LiquidCrystal_I2C lcd(0x27, 20, 4);

#define DO_PIN A1
#define VREF 5000    //VREF (mv)
#define ADC_RES 1024 //ADC Resolution

#define READ_TEMP (sensor_suhu) //Suhu temperatur air saat ini ℃, Or temperature sensor function
#define CAL1_V 1024 //mv
#define CAL1_T sensor_suhu //℃


int pH_Value;
float sensor_ph;

SoftwareSerial ss(2, 3);

const uint16_t DO_Table[41] = {
    14460, 14220, 13820, 13440, 13090, 12740, 12420, 12110, 11810, 11530,
    11260, 11010, 10770, 10530, 10300, 10080, 9860, 9660, 9460, 9270,
    9080, 8900, 8730, 8570, 8410, 8250, 8110, 7960, 7820, 7690,
    7560, 7430, 7300, 7180, 7070, 6950, 6840, 6730, 6630, 6530, 6410};

uint8_t Temperaturet;
uint16_t ADC_Raw;
uint16_t ADC_Voltage;
uint16_t DO;

int16_t readDO(uint32_t voltage_mv, uint8_t temperature_c)
{

    uint16_t V_saturation = (uint32_t)CAL1_V + (uint32_t)35 * temperature_c - (uint32_t)CAL1_T * 35;
    return (voltage_mv * DO_Table[temperature_c] / V_saturation);
}

void setup()
{
    Serial.begin(9600);
    ss.begin(115200);
    pinMode(pH_Value, INPUT);
    lcd.init();
    lcd.backlight();
    sensors.begin();  // Start up the library
}

void loop()
{
    Serial.println("Hello Boss");
    delay(1500);
    Temperaturet = (uint8_t)READ_TEMP;
    ADC_Raw = analogRead(DO_PIN);
    ADC_Voltage = uint32_t(VREF) * ADC_Raw / ADC_RES;

    // Mengambil nilai yang sama yang ditampilkan di LCD
    int sensor_oksigen = (int)((ADC_Voltage + Temperaturet) / 1000);

    DO = readDO(ADC_Voltage, Temperaturet); // Update DO value

    Serial.print("Temperaturet:\t" + String(Temperaturet) + "\t");
    Serial.print("ADC RAW:\t" + String(ADC_Raw) + "\t");
    Serial.print("ADC Voltage:\t" + String(ADC_Voltage) + "\t");
    Serial.println("DO:\t" + String(DO) + "\t");

    sensors.requestTemperatures();

    sensor_suhu = sensors.getTempCByIndex(0);
    if (sensor_suhu == DEVICE_DISCONNECTED_C) {
        Serial.println("Sensor not connected!");
    } else {
        Fahrenheit = sensors.toFahrenheit(sensor_suhu);

        // Print out the raw sensor data
        Serial.print("Raw sensor data (sensor_suhu):\t");
        Serial.println(sensor_suhu);
        Serial.print("Raw sensor data (Fahrenheit):\t");
        Serial.println(Fahrenheit);
    }

    Serial.print(sensor_suhu);
    Serial.print(" C  ");
  

    lcd.setCursor(0, 0);
    lcd.print("Suhu : ");
    lcd.print(sensor_suhu, 1); // Menampilkan nilai yang sama di LCD
    lcd.print(" C");
    
    lcd.setCursor(0, 1);
    Serial.print(sensor_oksigen);
    lcd.print("DO : ");
    lcd.print(sensor_oksigen); // Menampilkan nilai yang sama di LCD
    lcd.print(" mg/L");

    lcd.setCursor(0, 2);
    pH_Value = analogRead(A0);
    sensor_ph = pH_Value * (5.0 / 1023.0);
    lcd.print("PH : ");
    lcd.print(sensor_ph);

    String sendToESP = "";
    sendToESP += sensor_oksigen;
    sendToESP += ";";
    sendToESP += sensor_suhu;
    sendToESP += ";";
    sendToESP += sensor_ph; // Menggunakan nilai yang ditampilkan di LCD
    ss.println(sendToESP);

    delay(4000);
}
