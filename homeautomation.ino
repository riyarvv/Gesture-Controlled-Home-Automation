#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>

const int light = 2;

// Network and Firebase credentials
#define WIFI_SSID "ENTER YOUR WIFI SSID NAME"
#define WIFI_PASSWORD "ENTER YOUR WIFI PASSWORD"

#define Web_API_KEY "PASTE YOUR WEB API KEY"
#define DATABASE_URL "PASTE YOUR DATABASE URL"
#define USER_EMAIL "USER EMAIL"
#define USER_PASS "USER PASSWORD"

// Firebase components
FirebaseApp app;
WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);
RealtimeDatabase Database;
UserAuth user_auth(Web_API_KEY, USER_EMAIL, USER_PASS);

// Relay output pins
#define LIGHT_RELAY_PIN 12
#define FAN_RELAY_PIN 14

void processData(AsyncResult &aResult);

// Timer
unsigned long lastReadTime = 0;
const unsigned long readInterval = 5000; // every 5 seconds

void setup() {
  Serial.begin(115200);

  // Output pin setup
  pinMode(light, OUTPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  digitalWrite(LIGHT_RELAY_PIN, LOW);
  digitalWrite(FAN_RELAY_PIN, LOW);

  // Wi-Fi connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.println("Wi-Fi Connected");

  // Firebase initialization
  ssl_client.setInsecure();  // skip certificate validation
  initializeApp(aClient, app, getAuth(user_auth), processData, "authTask");
  app.getApp<RealtimeDatabase>(Database);
  Database.url(DATABASE_URL);
}

void loop() {
  app.loop(); // keep Firebase async tasks alive

  if (app.ready()) {
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime >= readInterval) {
      lastReadTime = currentTime;

      // Get Light status
      String lightState = Database.get<String>(aClient, "/light");
      if (aClient.lastError().code() == 0) {
        Serial.print("Light state: ");
        Serial.println(lightState);
        if (lightState == "ON") {
          digitalWrite(light, HIGH);
        } else {
          digitalWrite(light, LOW);
        }
      } else {
        Firebase.printf("Light read error: %s\n", aClient.lastError().message().c_str());
      }

      // Get Fan status
      String fanState = Database.get<String>(aClient, "/fan");
      if (aClient.lastError().code() == 0) {
        Serial.print("Fan state: ");
        Serial.println(fanState);
        if (fanState == "ON") {
          digitalWrite(FAN_RELAY_PIN, HIGH);
        } else {
          digitalWrite(FAN_RELAY_PIN, LOW);
        }
      } else {
        Firebase.printf("Fan read error: %s\n", aClient.lastError().message().c_str());
      }

      Serial.println("--------");
    }
  }
}

void processData(AsyncResult &aResult) {
  if (!aResult.isResult())
    return;

  if (aResult.isEvent())
    Firebase.printf("Event: %s, msg: %s, code: %d\n",
                    aResult.uid().c_str(),
                    aResult.eventLog().message().c_str(),
                    aResult.eventLog().code());

  if (aResult.isDebug())
    Firebase.printf("Debug: %s, msg: %s\n",
                    aResult.uid().c_str(),
                    aResult.debug().c_str());

  if (aResult.isError())
    Firebase.printf("Error: %s, msg: %s, code: %d\n",
                    aResult.uid().c_str(),
                    aResult.error().message().c_str(),
                    aResult.error().code());

  if (aResult.available())
    Firebase.printf("Task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
}
