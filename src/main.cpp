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

#include <MD_MAX72xx.h>
#include <SPI.h>

//prototype function
void pt_print();
void time_print();
void launchWeb(void);
void get_time_from_ntp();
void tombol_config();
void printText1(char *pMsg, int LOS, bool CenterJustify);
void clean();


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

/* DOT MATRIX MAX */

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4  // Instruksi untuk mengatur berapa banyak dotmatrix yang digunakan
#define CLK_PIN   D5   // Deklarasi CLK pada pin D5 NodeMcu
#define DATA_PIN  D7   // Deklarasi Data pada pin D7 NodeMcu
#define CS_PIN    D4   // Deklarasi CS pada pin D7 NodeMcu
#define BUF_SIZE  75
#define CHAR_SPACING  1 // pixels between characters

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

char message[BUF_SIZE] = "Ge Creative";
char daysOfTheWeek[7][7] = {"MINGGU", "SENIN", "SELASA", "RABU", "KAMIS", "JUMAT", "SABTU"};
char sholat[7][8] = {"SUBUH", "TERBIT", "DHUHUR", "ASAR", " ", "MAGHRIB", "ISYA"};
char zero[2][2] = {"", "0"};
int zero1 = 0, zero2 = 0, zero3 = 0, hr, mn, sc, ds=0, yy, mm, dd, hh, nn, ss, h12=2, brg, pos=0;
long millis1, millis2;

MD_MAX72XX::fontType_t myFont[] PROGMEM = 
{
  0,  // 0            - 'Unused'
  0,  // 1            - 'Unused'
  0,  // 2            - 'Unused'
  0,  // 3            - 'Unused'
  0,  // 4            - 'Unused'
  0,  // 5            - 'Unused'
  0,  // 6            - 'Unused'
  0,  // 7            - 'Unused'
  0,  // 8            - 'Unused'
  0,  // 9            - 'Unused'
  0,  // 10            - 'Unused'
  0,  // 11            - 'Unused'
  0,  // 12            - 'Unused'
  0,  // 13            - 'Unused'
  0,  // 14            - 'Unused'
  0,  // 15            - 'Unused'
  0,  // 16            - 'Unused'
  0,  // 17            - 'Unused'
  0,  // 18            - 'Unused'
  0,  // 19            - 'Unused'
  0,  // 20            - 'Unused'
  0,  // 21            - 'Unused'
  0,  // 22            - 'Unused'
  0,  // 23            - 'Unused'
  0,  // 24            - 'Unused'
  0,  // 25            - 'Unused'
  0,  // 26            - 'Unused'
  0,  // 27            - 'Unused'
  0,  // 28            - 'Unused'
  0,  // 29            - 'Unused'
  0,  // 30            - 'Unused'
  0,  // 31            - 'Unused'
  1, 0,   // 32            - 'Space'
  0,  // 33            - '!'
  0,  // 34            - '"'
  0,  // 35            - '#'
  0,  // 36            - '$'
  5, 32, 16, 42, 4, 2,  // 37            - '%'
  0,  // 38            - '&'
  0,  // 39           
  0,  // 40            - '('
  0,  // 41            - ')'
  0,  // 42            - '*'
  0,  // 43            - '+'
  0,  // 44           '
  0,  // 45            - '-'
  0,  // 46            - '.'
  4, 32, 16, 8, 4,  // 47            - '/'
  3, 126, 66, 126,  // 48            - '0'
  3, 0, 126, 0,   // 49              - '1'
  3, 122, 74, 78,   // 50                 - '2'
  3, 74, 74, 126,   // 51              - '3'
  3, 14, 8, 126,  // 52            - '4'
  3, 78, 74, 122,   // 53              - '5'
  3, 126, 74, 122,  // 54            - '6'
  3, 2, 2, 126,   // 55               - '7'
  3, 126, 74, 126,  // 56            - '8'
  3, 78, 74, 126,   // 57               - '9'
  1, 36,  // 58            - ':'
  0,  // 59         
  3, 16, 40, 68,  // 60         
  0,  // 61         
  3, 68, 40, 16,  // 62         
  0,  // 63         
  0,  // 64         
  4, 126, 18, 18, 126,  // 65         
  4, 126, 74, 74, 118,  // 66         
  4, 126, 66, 66, 66,   // 67          
  4, 126, 66, 66, 60,   // 68         
  4, 126, 74, 74, 74,   // 69         
  0,  // 70         
  4, 126, 66, 82, 114,  // 71         
  4, 126, 8, 8, 126,  // 72         
  3, 66, 126, 66,   // 73         
  4, 96, 64, 64, 126,   // 74          
  4, 126, 24, 36, 66,   // 75           
  4, 126, 64, 64, 64,   // 76         
  5, 126, 4, 8, 4, 126,   // 77    
  4, 126, 4, 8, 126,  // 78         
  4, 126, 66, 66, 126,  // 79         
  0,  // 80         
  0,  // 81         
  4, 126, 18, 50, 78,   // 82           
  4, 78, 74, 74, 122,   // 83          
  5, 2, 2, 126, 2, 2,   // 84           
  4, 126, 64, 64, 126,  // 85         
  0,  // 86         
  0,  // 87         
  0,  // 88         
  5, 2, 4, 120, 4, 2,   // 89   
};

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
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 3);
  mx.setFont(myFont);

  millis1 = millis()+10000;
  // interupt tombol config
  attachInterrupt(digitalPinToInterrupt(buttonPin), tombol_config, CHANGE);

}

void loop() {

  RTC.read(tm);

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
    // sprintf(message,"%s", daysOfTheWeek[tm.Wday]);
    // printText1(message, sizeof(message), true);
    print_time();
    Serial.println(millis());
    Serial.println(millis1);
    Serial.println(tm.Year-2000);
    Serial.println(tm.Year%2000);
    Serial.println(tm.Year);




    if(pos==0)
    {
      hr = tm.Hour;
      mn = tm.Minute;
      sc = tm.Second;
    
      if(hr>12 && h12==1)
      {
        hr = hr-12;
      }
    
      zero1 = 0;
      if(hr<10)
        zero1 = 1;
    
      zero2 = 0;
      if(mn<10)
        zero2 = 1;
    
      zero3 = 0;
      if(sc<10)
        zero3 = 1;
        
      sprintf(message, "%s%d:%s%d:%s%d", zero[zero1], hr, zero[zero2], mn, zero[zero3], sc);
      printText1(message, sizeof(message), true);
      delay(500);
      sprintf(message, "%s%d %s%d %s%d", zero[zero1], hr, zero[zero2], mn, zero[zero3], sc);
      printText1(message, sizeof(message), true);
      delay(500);

      if(millis()>=millis1)
      {
        clean();
        
        pos=1;
      }

    }



    if(pos==1)
    {
      sprintf(message,"%s", daysOfTheWeek[tm.Wday]);
      printText1(message, sizeof(message), true);
      delay(1000);
      
      clean();
      pos=2;
    }
    
    if(pos==2)
    {
      sprintf(message, "%d/%d/%d", tm.Day, tm.Month,(tmYearToCalendar(tm.Year)%2000));
      printText1(message, sizeof(message), true);
      delay(1000);
      
      clean();
      pos=4;
      

    }

    if(pos==3)
    {
      sprintf(message, "%s", "SHOLAT");
      printText1(message, sizeof(message), true);
      delay(1000);
      
      clean();
      pos=4;
    }

    if(pos==4)
    {
      get_prayer_times(tmYearToCalendar(tm.Year), tm.Month, tm.Day, Latitude, Longitude, GMT, times);

      for (int i=0;i<sizeof(times)/sizeof(double);i++)
      {
        if(i != 4 && i != 7)
        {
          char tmp[10];
          int hours, minutes;
          get_float_time_parts(times[i], hours, minutes);
          
          sprintf(message, "%s", sholat[i]);
          printText1(message, sizeof(message), true);
          delay(1000);
          clean();

          zero1 = 0;
          if(hours<10)
            zero1 = 1;
      
          zero2 = 0;
          if(minutes<10)
            zero2 = 1;
        
          sprintf(message, "%s%d:%s%d", zero[zero1], hours, zero[zero2], minutes);
          printText1(message, sizeof(message), true);
          delay(1000);
          clean();
        }
      }
      millis1 = millis()+10000;
      pos=0;
    }
  }
  // get_prayer_times (2023, 7, 25, Latitude, Longitude, GMT, times); 
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
  setTime(epochTime);

  //reset wifi setting
  wifiManager.resetSettings();
  // time_sync = 1;
  ESP.restart();
  
}


// Print the text string to the LED matrix modules specified.
// Message area is padded with blank columns after printing.
// And center justified if third argument is "true"
void printText1(char *pMsg, int LOS, bool CenterJustify)                                   // copied and modified from library
{
  uint8_t modStart = 0;
  uint8_t modEnd = MAX_DEVICES - 1;
  uint8_t   state = 0;
  uint8_t   curLen;
  uint16_t  showLen;
  uint8_t   cBuf[8];
  int16_t   col = ((modEnd + 1) * COL_SIZE) - 1;
  int pixelcount = 0;
  int ccounter = LOS;

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  do     // finite state machine to print the characters in the space available
  {   switch (state)
    {
      case 0: // Load the next character from the font table
        // if we reached end of message, reset the message pointer
        if (*pMsg == '\0')
        {
          showLen = col - (modEnd * COL_SIZE);  // padding characters
          state = 2;
          break;
        }

        // retrieve the next character form the font file

        showLen = mx.getChar(*pMsg++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);
        if (ccounter > 0) {
          pixelcount = (pixelcount + showLen) + CHAR_SPACING;
          ccounter--;
        }
        curLen = 0;
        state++;
      // !! deliberately fall through to next state to start displaying

      case 1: // display the next part of the character
        mx.setColumn(col--, cBuf[curLen++]);

        // done with font character, now display the space between chars
        if (curLen == showLen)
        {
          showLen = CHAR_SPACING;
          state = 2;
        }
        break;

      case 2: // initialize state for displaying empty columns

        curLen = 0;

        state++;
      // fall through

      case 3:  // display inter-character spacing or end of message padding (blank columns)
        mx.setColumn(col--, 0);
        curLen++;
        if (curLen == showLen)
          state = 0;
        break;

      default:
        col = -1;   // this definitely ends the do loop
    }
  } while (col >= (modStart * COL_SIZE));

  if (CenterJustify) {
    for (int i = 1; i <= (((MAX_DEVICES * COL_SIZE) - pixelcount) / 2); i++) {
      mx.transform( MD_MAX72XX::TSR);
    }
  }
  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

void clean()
{
  for (uint8_t i = 0; i < 8; i++) {
    mx.transform(MD_MAX72XX::TSD);
    delay(50);
  }
}