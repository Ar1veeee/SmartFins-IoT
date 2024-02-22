#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

const char *ssid = "IoT";
const char *password = "innovillage";
float sensor_oksigen = 0, sensor_suhu = 0, sensor_ph = 0;
String uuid_device;
String mac_address;
unsigned long currentMillis = 0, prevMillis = 0, intervalMillis = 5000;

void setup() {
  Serial.begin(115200);
  connectToWifi();

  // Dapatkan alamat MAC
  uint8_t mac[6];
  WiFi.macAddress(mac);

  // Format alamat MAC sebagai string
  mac_address = "";
  for (int i = 0; i < 6; ++i) {
    mac_address += String(mac[i], HEX);
    if (i < 5) {
      mac_address += ":";
    }
  }

  Serial.println("Alamat MAC: " + mac_address);
}

void connectToWifi() {
  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
String splitString(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void kirimDataKeServer() {
  HTTPClient http;
  String postData;
  String url;
  uuid_device = mac_address;
  //Post Data
  postData = "uuid_device=";
  postData += uuid_device;
  postData += "&sensor_oksigen=";
  postData += sensor_oksigen;
  postData += "&sensor_suhu=";
  postData += sensor_suhu;
  postData += "&sensor_ph=";
  postData += sensor_ph;

  //ipaddress server apache mysql
  url = "https://backend-smartfins-qsy5codmja-as.a.run.app/api/smartfins/insertSensor";
  //WiFiClient client;
  WiFiClientSecure client;
  client.setInsecure();
  http.begin(client, url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.POST(postData);  //Send the request
  String payload = http.getString();   //Get the response payload

  Serial.println(httpCode);  //Print HTTP return code
  Serial.println(payload);   //Print request response payload

  http.end();
}

void loop() {
  if (Serial.available()) {
    String msg = "";
    while (Serial.available()) {
      msg += char(Serial.read());
      delay(50);
    }
    sensor_oksigen = splitString(msg, ';', 0).toFloat();
    sensor_suhu = splitString(msg, ';', 1).toFloat();
    sensor_ph = splitString(msg, ';', 2).toFloat();
  }

  currentMillis = millis();
  if ((currentMillis - prevMillis) >= intervalMillis) {
    kirimDataKeServer();
    prevMillis = currentMillis;
  }
}