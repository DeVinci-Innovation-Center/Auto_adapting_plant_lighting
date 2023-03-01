#include <WiFi.h>  //Used to connect to a WiFi network
#include <HTTPClient.h> //Used to send HTTP requests
#include <ArduinoJson.h> //Used to process json documents

// Necessary parameters for use of PWM on ESP32 boards
const int channel = 0; //Number of the channel on which the PWM signal will be transmitted
const int freq = 1000; //PWM frequency of 1kHz
const int res = 8; //8-bit resolution, 256 values
const int ledPin = 23; //Pin of the ESP32 connected to the MOSFET

// WiFi and meteo API configurations  
const char* ssid = "YOUR_WIFI_NAME_HERE";
const char* password = "YOUR_WIFI_PASSWORD_HERE";
const char* apiKey = "YOUR_API_KEY_HERE"; //API key provided by OpenWeatherMap
const char* server = "api.openweathermap.org"; //URL to request the meteorological data

// Working hours and time API configurations
const char* timeUrl = "http://worldtimeapi.org/api/timezone/Europe/Paris"; // URL to request the time of a city, the city and region can be changed
const int startHour = 7; // Starting hour of the lighting, can be changed here
const int endHour = 21; // Ending hour of the lighting, can be changed here

void setup() 
{
  ledcSetup(channel, freq, res); // Creating a PWM signal with a frequency of freq and a resolution of res  
  ledcAttachPin(ledPin, channel); // Connecting a pin of the ESP32 to the PWM generating channel

  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) // Verifying that the device is connected to WiFi
  {
    Serial.println("Connecting to WiFi...");
    delay(2000);
  }
  Serial.println("Connected to WiFi");
}

void loop() 
{
  if(WiFi.status() == WL_CONNECTED) // Verify the WiFi connection
  {
    int hour = 0;
    HTTPClient httpTime;
    httpTime.begin(timeUrl);
    int httpTimeCode = httpTime.GET();
    if(httpTimeCode>0) // Verifying that the HTTP request for time was correctly processed
    {
      String payload = httpTime.getString();
      DynamicJsonDocument time(1024);
      deserializeJson(time, payload);
      String datetime = time["datetime"];
      hour = datetime.substring(11,13).toInt();
    }
    else
    {
      Serial.println("There was an error in the time API request.");
    }

    if(hour>=startHour && hour<endHour) // Verifying if the time corresponds to the activation time zone
    {
      HTTPClient http;
      String url = String("http://") + server + "/data/2.5/weather?lat=YOUR_LATITUDE&lon=YOUR_LONGITUDE&appid=" + apiKey; // Change the latitude and longitude by yours
      http.begin(url);
      int httpCode = http.GET();
      delay(2000);

      if(httpCode>0) // Verifying that the HTTP request for meteo was correctly processed
      {
        String payload = http.getString();
        Serial.println(payload);
        DynamicJsonDocument meteo(1024);
        deserializeJson(meteo, payload);
        int clouds = meteo["clouds"]["all"];
        int pwm = map(clouds, 0, 100, 50, 255);
        ledcWrite(channel, pwm);
      }
      else
      {
        Serial.println("There was an error in the meteo API request.");
      }
      http.end();
    }
    else
    {
      ledcWrite(channel, 0);
      Serial.println("Outside activation time zone");
    } 
    delay(1800000); //delay 30 minutes before the next api calls    
  }
}
