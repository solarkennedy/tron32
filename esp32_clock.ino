#define DEBUG_ON
#include <WiFi.h>
#include <NTPtimeESP.h>
#include <Wire.h>
#include "RTClib.h"


HardwareSerial Serial2(2);

RTC_DS3231 rtc;

#define threshold 35

NTPtime NTP("pool.ntp.org");

//change those
const char* ssid = "essid";
const char* password = "password";
float timezone = 2.0;

byte err = 0;
byte lastMin = -1;

int prev = 45;

strDateTime dateTime;
DateTime now;

const char Day[7][10] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char Month[12][6] = {"Jan.", "Feb.", "March", "Apr.", "May", "June", "July", "Aug.", "Sept.", "Oct", "Nov", "Dec."};

void setup() {
  Serial.begin(115200);

  Serial2.begin(9600);

  delay(100);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC!!!");
  } else if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time back to the UNIX EPOCH!");
    rtc.adjust(DateTime(1970, 1, 1, 0, 0, 0));
  }
  DateTime now = rtc.now();

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin (ssid, password);
  WiFi.softAP("essidap", "password", 1, 13);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1);
  }

}

void printDateTime() {
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(Day[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  Serial.print(" since midnight 1970-01-01 = ");
  Serial.print(now.unixtime());
  Serial.print("s = ");
  Serial.print(now.unixtime() / 86400L);
  Serial.println("d");
}


void printDateTimeObject(strDateTime dt) {
  Serial.println(dt.hour);
  Serial.println(dt.minute);
  Serial.println(dt.second);
  Serial.println(dt.year);
  Serial.println(dt.month);
  Serial.println(dt.day);
  Serial.println(dt.dayofWeek);
  Serial.println(dt.valid);

}

void setClockFromNTP() {
  dateTime = NTP.getNTPtime(timezone, 1);
  //NTP.printDateTime(dateTime);
  if (dateTime.valid == 0) {
    err++;
    if (err > 1500) {
      Serial.println("Could not get\ndate and time,\nreset router");
      ESP.deepSleep(1000000 * 60 * 24 * 10);
    }
    return;
  }
  if (dateTime.valid && dateTime.minute != lastMin) {

    NTP.printDateTime(dateTime);
    Serial.println("Got a valid time. Setting the RTC:");

    printDateTimeObject(dateTime);
    rtc.adjust(DateTime(dateTime.year, dateTime.month, dateTime.day, dateTime.hour, dateTime.minute, dateTime.second));
    now = rtc.now();
    printDateTime();
    err = 0;
    lastMin = dateTime.minute;
    String time;
    if (dateTime.hour < 10)
      time += '0';
    time += String(dateTime.hour) + ':';
    if (dateTime.minute < 10)
      time += '0';
    time += String(dateTime.minute);
    Serial.println(time);
    String date;
    if (dateTime.day < 10)
      date += '0';
    date += dateTime.day;
    date += Month[dateTime.month - 1];
    date += dateTime.year;
    Serial.println(String(Day[dateTime.dayofWeek - 1]));
    printDateTime();

  }
}




void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.println("No WiFiconnection\nretrying...");
  }
  while (WiFi.status() != WL_CONNECTED)
    delay(1);

  setClockFromNTP();
  delay(10000);


  wait_for_rfid();

}

