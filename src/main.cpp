#include <Arduino.h>


#include <stdio.h>
#include <Wire.h>
// get this library from https://github.com/PaulStoffregen/Time
#include <TimeLib.h>
#include "PrayerTimes.h"

double times[sizeof(TimeName)/sizeof(char*)];

// //pengaturan bujur lingkar semarang
float Latitude = -7.13505; // lintang
float Longitude = 110.40876388888888 ; // bujur
int GMT = 7 ; // zona waktu WIB=7, WITA=8, WIT=9

void p(char *fmt, ... ){
        char tmp[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(tmp, 128, fmt, args);
        va_end (args);
        Serial.print(tmp);
}


void setup() {
  Serial.begin(9600);
  
}

void loop() {

  Serial.println("PTimes:");
  
  int dst=1;
  
  set_calc_method(ISNA);
  set_asr_method(Shafii);
  set_high_lats_adjust_method(AngleBased);
  set_fajr_angle(20);
  set_isha_angle(18);
  
  //MEKKA
  //float latitude=21.427378;
  //float longitude=39.814838;
  //get_prayer_times(year(), month(), day(), latitude, longitude, dst, times);
    get_prayer_times (2023, 7, 25, Latitude, Longitude, GMT, times);
    // get_prayer_times(2015, 5, 8, 46.9500f, 7.4458f, 2, times);
    
    Serial.print("YEAR:");     
    Serial.println(year());     
    Serial.print("MONTH:");     
    Serial.println(month());     
    Serial.print("DAY:");     
    Serial.println(day());     
      
  for (int i=0;i<sizeof(times)/sizeof(double);i++){
    char tmp[10];
    int hours, minutes;
    get_float_time_parts(times[i], hours, minutes);
    p("%d \t %10s %s \t %02d:%02d \n\r",i,TimeName[i],dtostrf(times[i],2,2,tmp),hours,minutes);
    /*
    Serial.print(i); 
    Serial.print(" \t "); 
    Serial.print(TimeName[i]); 
    Serial.print(" \t\t "); 
    Serial.print(times[i]);
    Serial.print(" \t ");     
    Serial.print(hours); 
    Serial.print(":"); 
    Serial.print(minutes); 
    Serial.println();
    */
  }
  delay(10000);
}
