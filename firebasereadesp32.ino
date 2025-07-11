#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>

// Network and Firebase credentials
#define WIFI_SSID "FAWSTECH INNOVATION"
#define WIFI_PASSWORD "JERSARFAWS"

#define Web_API_KEY "AIzaSyDIrKwPF-7TjtOf1v62NlXqayKMVqrqeZw"
#define DATABASE_URL "https://homeautomation-94e12-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "riyarv7723@gmail.com"
#define USER_PASS "ri041004"

// User function
void processData(AsyncResult &aResult);

// Authentication
UserAuth user_auth(Web_API_KEY, USER_EMAIL, USER_PASS);

// Firebase components
FirebaseApp app;
WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);
RealtimeDatabase Database;

// Timer variables for sending data every 10 seconds
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 10000; // 10 seconds in milliseconds

// Variables to save values from the database
int intValue;
float floatValue;
String stringValue;

void setup(){
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)    {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Configure SSL client
  ssl_client.setInsecure();
  ssl_client.setConnectionTimeout(1000);
  ssl_client.setHandshakeTimeout(5);

  // Initialize Firebase
  initializeApp(aClient, app, getAuth(user_auth), processData, "🔐 authTask");
  app.getApp<RealtimeDatabase>(Database);
  Database.url(DATABASE_URL);
}

void loop(){
  // Maintain authentication and async tasks
  app.loop();

  // Check if authentication is ready
  if (app.ready()){
    // Periodic data sending every 10 seconds
    unsigned long currentTime = millis();
    if (currentTime - lastSendTime >= sendInterval){
      // Update the last send time
      lastSendTime = currentTime;

      // GET VALUES FROM DATABASE (and save the data in a variable)
      int intValue =  Database.get<int>(aClient, "/test/int");
      check_and_print_value (intValue);

      float floatValue = Database.get<float>(aClient, "/test/float");
      check_and_print_value(floatValue);

      String stringValue = Database.get<String>(aClient, "/test/string");
      check_and_print_value(stringValue);

      Serial.println("Requested data from /test/int, /test/float, and /test/string");
    }
  }
}

template <typename T>
void check_and_print_value(T value){
  // To make sure that we actually get the result or error.
  if (aClient.lastError().code() == 0){
    Serial.print("Success, Value: ");
    Serial.println(value);
  }
  else {
    Firebase.printf("Error, msg: %s, code: %d\n", aClient.lastError().message().c_str(), aClient.lastError().code());
  }
}

void processData(AsyncResult &aResult){
  if (!aResult.isResult())
    return;

  if (aResult.isEvent())
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());

  if (aResult.isDebug())
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());

  if (aResult.isError())
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());

  if (aResult.available()){
    // Log the task and payload
    Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
  }
}