#include "DHT.h"
#include "SoftwareSerial.h"

#define DHTPIN 8
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial ESP8266(2, 3); // Rx,  Tx
String tempString;
String humString;
String myAPIkey = "YKZYBLA9ME42FSL8";

void setup() {
  Serial.begin(9600);
  ESP8266.begin(9600); //never forget to start ur ESP8266
  dht.begin();
  // Setup data code block for ESP8266
  ESP8266.println("AT+RST");
  delay(2000);
  ESP8266.println("AT+CIPMUX=0");
  delay(2000);
  ESP8266.println("AT+CWMODE=3");
  delay(2000);
  ESP8266.println("AT+CWQAP");
  delay(2000);
  ESP8266.print("AT+CWJAP=\"Adamantoise\",\"maj12346\"\r\n");
  delay(2000);
  Serial.println("ESP8266 Setup done!");
  // end block
}

void loop() {
  getTempAndHumidity();
  // Sending data code block
  ESP8266.println("AT+CIPSTART=\"TCP\",\"184.106.153.149\",80");
  delay(5000);
  cipsendCommand();
  Serial.println("Data uploaded");
  // end block
  delay(60000);
}

// DHT11 Functions
// Get temperature and humidity
void getTempAndHumidity () {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  tempString = String(temp);
  humString = String(hum);
}

// ESP8266 Functions
// Writes data to API
void cipsendCommand(void)
{
  String getStr = "GET /update?api_key=";
  getStr += myAPIkey;
  getStr += "&field1=";
  getStr += tempString;
  getStr += "&field2=";
  getStr += humString;
  // add more fields here for data
  getStr += "\r\n\r\n";

  String command = "AT+CIPSEND=";
  command += String(getStr.length());
  ESP8266.println(command);
  Serial.println(command);
  delay(5000);

  ESP8266.println(getStr);
  Serial.println(getStr);
  delay(5000);

  ESP8266.println("AT+CIPCLOSE");
}
