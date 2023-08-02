#include <Arduino.h>


#include <stdio.h>
#include <Wire.h>
// get this library from https://github.com/PaulStoffregen/Time
#include <TimeLib.h>
#include <DS1307RTC.h>
#include "PrayerTimes.h"


#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <NTPClient.h>


//prototype function
void pt_print();
void time_print();
void launchWeb(void);
void get_time_from_ntp();
void tombol_config();


/* global flag */
bool time_sync = 0;

/* PrayerTime */
double times[sizeof(TimeName)/sizeof(char*)];

//pengaturan bujur lingkar semarang
float Latitude     = -7.13505;            // lintang
float Longitude    = 110.40876388888888 ; // bujur
int GMT            = 7 ;                  // zona waktu WIB=7, WITA=8, WIT=9

void p(char *fmt, ... ){
        char tmp[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(tmp, 128, fmt, args);
        va_end (args);
        Serial.print(tmp);
}

/* RTC */
tmElements_t tm;


/* Web Server */

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

WiFiManager wifiManager;


/* Interput */
const int buttonPin = D3;   // Pin tempat tombol dihubungkan
const int longPressDuration = 3000; // Durasi ketika tombol harus ditekan selama 3 detik

volatile bool buttonPressed = false; // Variabel untuk menyimpan status tombol yang ditekan








/////////////////////// FUNGSI-FUNGSI //////////////////////////////

//cek RTC
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

void print_time(){
  Serial.print("Ok, Time = ");
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tm.Day);
    Serial.write('/');
    Serial.print(tm.Month);
    Serial.write('/');
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.println();
}
void rtc_check(){

  //RTC Internal
  Serial.print(year()); 
  time_sync = 0;
  if(year() >= 2023) time_sync = 1;
  else if(RTC.read(tm)) {
    time_sync = 1;
    print_time();
  }
}


void setup() {
  Serial.begin(9600);

  pinMode(buttonPin, INPUT_PULLUP); // Set pin tombol sebagai INPUT_PULLUP (internal pull-up resistor)
  
  rtc_check();

  setTime(23,13,0,25,7,2023);
  set_calc_method(ISNA);
  set_asr_method(Shafii);
  set_high_lats_adjust_method(AngleBased);
  set_fajr_angle(20);
  set_isha_angle(18);

  //MAX72xx
      

  // interupt tombol config
  attachInterrupt(digitalPinToInterrupt(buttonPin), tombol_config, CHANGE);

}

void loop() {

  if(time_sync == 0){
    //masuk mode config
    Serial.println("Mode Sikkronisasi Waktu Lewat NTP");
    wifiManager.startConfigPortal("Jam Waktu Sholat", "");
    server.begin();
    get_time_from_ntp();
  }
  else{
    //running normal
    Serial.println("waktu sinkron");


  }





  
  

  

  



  // get_prayer_times (2023, 7, 25, Latitude, Longitude, GMT, times);

    

  delay(10000);
}

/* interupt */

ICACHE_RAM_ATTR void tombol_config() {
  // Simpan waktu saat tombol ditekan
  static unsigned long buttonPressStartTime = 0;
  
  // Membaca status tombol
  bool buttonState = digitalRead(buttonPin);

  // Tombol ditekan
  if (buttonState == LOW) {
    // Catat waktu penekanan tombol
    buttonPressStartTime = millis();
      Serial.println("TOMBOL DITEKAN ");

  } else {
    // Tombol dilepas
    // Tombol ditekan selama 3 detik atau lebih
    if (millis() - buttonPressStartTime >= longPressDuration) {
      // Eksekusi fungsi yang diinginkan, dalam contoh ini kita hanya akan mengaktifkan LED
      Serial.println(" ");
      Serial.println("TOMBOL DITEKAN SELAMA 3 DETIK");

      time_sync = 0;
      // Tambahkan fungsi lain yang ingin dieksekusi di sini

      // Set tombolPressed menjadi false untuk mencegah eksekusi berulang saat tombol tetap ditekan
      buttonPressed = false;
    }
  }
}

/* RTC */

void get_time_from_ntp(){
  //cek koneksi wifi

  Serial.print("terkoneksi dengan wifi");

  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP, "pool.ntp.org"); 

  timeClient.begin();
  timeClient.setTimeOffset(25200);

  while(!timeClient.update()){
    Serial.print("time update false");
  }

  time_t epochTime = timeClient.getEpochTime();
  Serial.print("Epoch Time: ");
  Serial.println(epochTime);
  
  String formattedTime = timeClient.getFormattedTime();
  Serial.print("Formatted Time: ");
  Serial.println(formattedTime);  

  //simpan waktu ke RTC
  
  while (!Serial) ; // wait for serial
  delay(200);
  Serial.println("DS1307RTC Read Test");
  Serial.println("-------------------");
  RTC.set(epochTime);

  //reset wifi setting
  wifiManager.resetSettings();
  ESP.restart();
  
}

