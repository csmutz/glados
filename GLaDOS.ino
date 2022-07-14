//Greenhouse Liquidity and Degrees Oversight System (GLaDOS)
//Uses esp32 and DHT22 to monitor temp and humidity
//Blue light status:
//  blinking during startup: connecting to Wifi/starting server
//  solid when Wifi is connnected
//  off when disconnected from wifi
//  blinks off rapidly for web server activity
//Serves json data via HTTP
//Uses DHT library: https://github.com/adafruit/DHT-sensor-library
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <Syslog.h>
#include "DHT.h"

#define DHTPIN 15
#define DHTTYPE DHT22
#define HTTPPORT 80

const char* ssid     = "wifi_ssid";
const char* password = "wifi_pass";
const char* hostname = "GLaDOS";
unsigned int wifi_last_connected = 0;

WebServer server(HTTPPORT);
DHT dht(DHTPIN, DHTTYPE);

void handle_req() {
  char response[100];
  Serial.print("Request from: ");
  Serial.println(server.client().remoteIP());
  digitalWrite(LED_BUILTIN, LOW);
  snprintf(response, sizeof(response), "{\"temperature\": %i, \"humidity\": %i, \"uptime\": %u}" , (int)round(1.8*dht.readTemperature()+32), (int)dht.readHumidity(), time(NULL));
  server.send(200, "application/json", response);
}

//blink LED and output to Serial while connecting to WIFI
void wifi_blink_while_connecting()
{
  Serial.println("");
  Serial.println("Connecting to Wifi");

  int i = 2;
  while (WiFi.status() != WL_CONNECTED) {
    i++;
    delay(250);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, i % 2);
  }
  
  Serial.println("");
  Serial.print("Connected to Wifi: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  wifi_last_connected = time(NULL);
  digitalWrite(LED_BUILTIN, HIGH);
}


void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200);
  
  dht.begin();
  
  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname);
  WiFi.begin(ssid, password);

  wifi_blink_while_connecting();

  server.on("/", handle_req);
  server.onNotFound(handle_req);
  server.begin();

  Serial.print("HTTP server started on port: ");
  Serial.println(HTTPPORT);
}


void loop(void) {
  //turn led on if Wifi is still connected
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    wifi_last_connected = time(NULL);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
    if ((time(NULL) - wifi_last_connected) > 600)
    {
      WiFi.reconnect();
      wifi_blink_while_connecting();
    }
  }
  server.handleClient();
  delay(20);
}
