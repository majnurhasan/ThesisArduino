#include "Wire.h"
#include "DHT.h"
#include "SoftwareSerial.h"
#include "SPI.h"
#include "SD.h"
#include "arduinoFFT.h"

#define DS3231_I2C_ADDRESS 0x68
#define DHTPIN 8
#define DHTTYPE DHT11
#define SAMPLES 128             //SAMPLES-pt FFT. Must be a base 2 number. Max 128 for Arduino Uno.
#define SAMPLING_FREQUENCY 2048 //Ts = Based on Nyquist, must be 2 times the highest expected frequency.
#define IR1 A1
#define IR2 A2
#define IR3 A3

DHT dht(DHTPIN, DHTTYPE);
arduinoFFT FFT = arduinoFFT();
File dataFile;
SoftwareSerial ESP8266(2, 3); // Rx,  Tx
String tempString;
String humString;
String timeString;
String fileName;
String myAPIkey = "LZACM7PEGT5USJ0S";
String myAPIkey2 = "SSNNT6WOXNOJQKAR";

String trapNumber = "2";
String geolocation = "7.081007";
String geolocation2 = "125.507435";
String OFreq;
String AFreq;
String CFreq;
String genus;
String species;
String sex;
int hourString;
int minuteString;
unsigned int samplingPeriod;
unsigned long microSeconds;
double vReal[SAMPLES]; //create vector of size SAMPLES to hold real values
double vImag[SAMPLES]; //create vector of size SAMPLES to hold imaginary values
long TR;

void setup() {
  Serial.begin(9600);
  ESP8266.begin(9600); //never forget to start ur ESP8266
  dht.begin();
  Wire.begin();
  SD.begin(53);
  samplingPeriod = round(1000000 * (1.0 / SAMPLING_FREQUENCY)); //Period in microseconds
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
  // detection block and processing of values block
  if ((analogRead(IR1) > 50) || (analogRead(IR2) > 50) || (analogRead(IR3) > 50)) {
    // get frequencies and readings block
    getPeakA();
    getPeakO();
    CFreq = String((OFreq.toInt() + AFreq.toInt()) / 2);

    // determine genus, species, and sex block
    // Aedes
    if (CFreq.toInt() > 503 && CFreq.toInt() < 907) {
      genus = "0";
      if (CFreq.toInt() > 503 && CFreq.toInt() < 584) {
        sex = "1";
        if (CFreq.toInt() > 511 && CFreq.toInt() < 584) {
          species = "0";
        }
        if (CFreq.toInt() > 503 && CFreq.toInt() < 572) {
          species = "1";
        }
      }
      if (CFreq.toInt() > 600 && CFreq.toInt() < 907) {
        sex = "0";
        if (CFreq.toInt() > 600 && CFreq.toInt() < 907) {
          species = "0";
        }
        if (CFreq.toInt() > 725 && CFreq.toInt() < 745) {
          species = "1";
        }
      }
    }

    // Anopheles
    if (CFreq.toInt() > 400 && CFreq.toInt() < 710) {
      genus = "1";
      if (CFreq.toInt() > 400 && CFreq.toInt() < 430) {
        sex = "1";
        species = "2";
      }
      if (CFreq.toInt() > 690 && CFreq.toInt() < 710) {
        sex = "0";
        species = "2";
      }
    }

    // Culex
    if (CFreq.toInt() > 310 && CFreq.toInt() < 500) {
      genus = "2";
      if (CFreq.toInt() > 310 && CFreq.toInt() < 384) {
        sex = "1";
        if (CFreq.toInt() > 310 && CFreq.toInt() < 343) {
          species = "4";
        }
        if (CFreq.toInt() > 330 && CFreq.toInt() < 384) {
          species = "3";
        }
      }
      if (CFreq.toInt() > 400 && CFreq.toInt() < 474) {
        sex = "0";
        if (CFreq.toInt() > 400 && CFreq.toInt() < 420) {
          species = "4";
        }
        if (CFreq.toInt() > 434 && CFreq.toInt() < 474) {
          species = "3";
        }
      }
    }

    // get time and environmental values block
    displayTime();
    getTempAndHumidity();

    // save to SD card and send to Thingspeak block
    ESP8266.println("AT+CIPSTART=\"TCP\",\"184.106.153.149\",80");
    delay(500);
    cipsendCommand();
    delay(500);

    ESP8266.println("AT+CIPSTART=\"TCP\",\"184.106.153.149\",80");
    delay(500);
    cipsendCommand2();
    delay(500);
    saveData();

    delay(2000); //data padding rest for 2 seconds
  }
  delay(100); // loop every 100 milliseconds
}

// FFT Library Functions
// Get peak acoustic frequency
void getPeakA() {
  for (int i = 0; i < SAMPLES; i++)
  {
    microSeconds = micros();

    vReal[i] = analogRead(0);
    vImag[i] = 0;

    while (micros() < (microSeconds + samplingPeriod))
    {
      //do nothing
    }
  }

  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

  double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
  AFreq = String(peak);
  Serial.print("Frequency: ");
  Serial.println(peak);     //Print out the most dominant frequency.
  Serial.println("\n");
  delay(3000);
}

// Get peak optical frequency, default at A1
void getPeakO() {
  for (int i = 0; i < SAMPLES; i++)
  {
    microSeconds = micros();

    vReal[i] = analogRead(1);
    vImag[i] = 0;


    while (micros() < (microSeconds + samplingPeriod))
    {
      //do nothing
    }
  }

  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);


  double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
  OFreq = String(peak);
  Serial.print("Frequency: ");
  Serial.println(peak);     //Print out the most dominant frequency.
  Serial.println("\n");
  delay(3000);
}


// DHT11 Functions
// Get temperature and humidity
void getTempAndHumidity () {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  tempString = String(temp);
  humString = String(hum);
}

// RTC Functions
// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val) {
  return ( (val / 10 * 16) + (val % 10) );
}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val) {
  return ( (val / 16 * 10) + (val % 16) );
}

// Set time
void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte
                   dayOfMonth, byte month, byte year) {
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}

// Read time
void readDS3231time(byte *second,
                    byte *minute,
                    byte *hour,
                    byte *dayOfWeek,
                    byte *dayOfMonth,
                    byte *month,
                    byte *year) {
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

// Display time
void displayTime() {
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
                 &year);
  // send it to the serial monitor
  timeString = String(hour, DEC);
  hourString = hour;
  // convert the byte variable to a decimal number when displayed
  timeString = timeString + ":";
  if (minute < 10) {
    timeString = timeString + "0";
  }
  timeString = timeString + String(minute, DEC) + ":";
  minuteString = minute;
  if (second < 10) {
    //Serial.print("0");
  }
  timeString = timeString + String(second, DEC) + " " + String(dayOfMonth, DEC) + "/" + String(month, DEC) + "/" + String(year, DEC);

  switch (month) {
    case 1:
      fileName = "jan" + String(dayOfMonth, DEC);
      break;
    case 2:
      fileName = "feb" + String(dayOfMonth, DEC);
      break;
    case 3:
      fileName = "mar" + String(dayOfMonth, DEC);
      break;
  }
}

// SD Functions
// Saves data to SD Card
void saveData () {
  dataFile = SD.open(fileName + ".csv", FILE_WRITE);
  String record = trapNumber + "," + geolocation + "," + geolocation2 + "," + OFreq + "," + AFreq + "," +
                  CFreq + "," + timeString + "," + tempString + "," + humString + "," +
                  genus + "," + species + "," + sex;
  Serial.println(record);
  dataFile.println(record);
  dataFile.close();
}

// ESP8266 Functions
// Writes data to channel 1
void cipsendCommand(void)
{
  String getStr = "GET /update?api_key=";
  getStr += myAPIkey;
  getStr += "&field1=";
  getStr += geolocation;
  getStr += "&field2=";
  getStr += geolocation2;
  getStr += "&field3=";
  getStr += tempString;
  getStr += "&field4=";
  getStr += humString;
  getStr += "&field5=";
  getStr += "1";
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

// Writes data to channel 2
void cipsendCommand2(void)
{
  String getStr = "GET /update?api_key=";
  getStr += myAPIkey2;
  getStr += "&field1=";
  getStr += OFreq;
  getStr += "&field2=";
  getStr += AFreq;
  getStr += "&field3=";
  getStr += CFreq;
  getStr += "&field4=";
  getStr += genus;
  getStr += "&field5=";
  getStr += species;
  getStr += "&field6=";
  getStr += sex;
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
