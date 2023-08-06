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

#include <SoftwareSerial.h>



//prototype function
void pt_print();
void time_print();
void launchWeb(void);
void get_time_from_ntp();
void tombol_config();
void printText1(char *pMsg, int LOS, bool CenterJustify);
void clean();
void printHelp();
void sendMP3Command(char c);
String decodeMP3Answer();
void sendCommand(byte command);
void sendCommand(byte command, byte dat1, byte dat2);
int shex2int(char *s, int n);
String sanswer(void);
void notification();
void display();


/* global flag */
bool time_sync = 0;
bool pp_button = 0;

/* Blynk */
#define BLYNK_FIRMWARE_VERSION        "0.1.0"
#define BLYNK_PRINT Serial
#define APP_DEBUG
#define USE_NODE_MCU_BOARD
#define BLYNK_TEMPLATE_ID "TMPL6eIJrPxBi"
#define BLYNK_TEMPLATE_NAME "Playlist"
#define BLYNK_AUTH_TOKEN "6WAMzoEwKF1zxjFvbv4PcmhBslS4dw2k"

#include <BlynkSimpleEsp8266.h>

/* Fill in information from Blynk Device Info here */

char ssid[] = "123";
char pass[] = "kikikiki";

WidgetLCD lcd(V2);

/* MP3 */
SoftwareSerial mp3(D3,D0);//Rx,Tx

static int8_t Send_buf[8] = {0}; // Buffer untuk kirim perintah. 
static uint8_t ansbuf[10] = {0}; // Buffer untuk respon. 

String mp3Answer;           // respon dari MP3.

String sanswer(void);
String sbyte2hex(uint8_t b);

/************ Command byte ************************************/
//dari tabel pada manual book
#define CMD_NEXT_SONG     0X01  // Play next song.
#define CMD_PREV_SONG     0X02  // Play previous song.
#define CMD_PLAY_W_INDEX  0X03
#define CMD_VOLUME_UP     0X04
#define CMD_VOLUME_DOWN   0X05
#define CMD_SET_VOLUME    0X06

#define CMD_SNG_CYCL_PLAY 0X08  // Single Cycle Play.
#define CMD_SEL_DEV       0X09
#define CMD_SLEEP_MODE    0X0A
#define CMD_WAKE_UP       0X0B
#define CMD_RESET         0X0C
#define CMD_PLAY          0X0D
#define CMD_PAUSE         0X0E
#define CMD_PLAY_FOLDER_FILE 0X0F

#define CMD_STOP_PLAY     0X16  // Stop playing continuously. 
#define CMD_FOLDER_CYCLE  0X17
#define CMD_SHUFFLE_PLAY  0x18 //
#define CMD_SET_SNGL_CYCL 0X19 // Set single cycle.

#define CMD_SET_DAC 0X1A
#define DAC_ON  0X00
#define DAC_OFF 0X01

#define CMD_PLAY_W_VOL    0X22
#define CMD_PLAYING_N     0x4C
#define CMD_QUERY_STATUS      0x42
#define CMD_QUERY_VOLUME      0x43
#define CMD_QUERY_FLDR_TRACKS 0x4e
#define CMD_QUERY_TOT_TRACKS  0x48
#define CMD_QUERY_FLDR_COUNT  0x4f

/************ Opitons **************************/
#define DEV_TF            0X02  


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

//Blynk
BLYNK_WRITE(V1) {   
  // Called when the datastream virtual pin V2 is updated 
  // by Blynk.Console, Blynk.App, or HTTP API. 

  String value = param.asStr();
  lcd.clear();
  // OR:
  //String value = param.asString();

  if (value == "play") {
    Serial.println("'play' button pressed");    
    lcd.print(0, 0, "play :");
    char c = 'p';
    sendMP3Command(c);
  } else if (value == "stop") {
    Serial.println("'stop' button pressed"); 
    lcd.print(0, 0, "pause :");
    char c = 'P';
    sendMP3Command(c);
  } else if (value == "prev") {
    Serial.println("'prev' button pressed");  
    lcd.print(0, 0, "prev");
    char c = '<';
    sendMP3Command(c);
  } else if (value == "next") {
    Serial.println("'next' button pressed"); 
    lcd.print(0, 0, "next");
    char c = '>';
    sendMP3Command(c);
  } else {
    Serial.print("V2 = '");
    Serial.print(value);
    Serial.println("'"); 
       
  }
}

// volume down
BLYNK_WRITE(V3)
{
  char c = '-';
  sendMP3Command(c);
}

//volume up
BLYNK_WRITE(V4)
{
  char c = '+';
  sendMP3Command(c);
}

void setup() {
  Serial.begin(9600);

  // MP3
  mp3.begin(9600);
  delay(500);
  sendCommand(CMD_SEL_DEV, 0, DEV_TF);
  delay(500);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
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
  // attachInterrupt(digitalPinToInterrupt(buttonPin), tombol_config, CHANGE);

}

void loop() {

  RTC.read(tm);
  Blynk.run();
  notification();

  if(time_sync == 0){
    //masuk mode config
    Serial.println("Mode Sikkronisasi Waktu Lewat NTP");
    wifiManager.startConfigPortal("Jam Waktu Sholat", "");
    server.begin();
    get_time_from_ntp();
  }
  else{
    //running normal
    if (mp3.available()){
      Serial.println(decodeMP3Answer());
      String str = decodeMP3Answer();
      String sub = "Completed";
      if(str.indexOf(sub) != -1){
        char c = '>';
        sendMP3Command(c);
      }
    }
    display();
  }
  // get_prayer_times (2023, 7, 25, Latitude, Longitude, GMT, times); 
}

/* interupt */

// ICACHE_RAM_ATTR void tombol_config() {
//   // Simpan waktu saat tombol ditekan
//   static unsigned long buttonPressStartTime = 0;
  
//   // Membaca status tombol
//   bool buttonState = digitalRead(buttonPin);

//   // Tombol ditekan
//   if (buttonState == LOW) {
//     // Catat waktu penekanan tombol
//     buttonPressStartTime = millis();
//       Serial.println("TOMBOL DITEKAN ");

//   } else {
//     // Tombol dilepas
//     // Tombol ditekan selama 3 detik atau lebih
//     if (millis() - buttonPressStartTime >= longPressDuration) {
//       // Eksekusi fungsi yang diinginkan, dalam contoh ini kita hanya akan mengaktifkan LED
//       Serial.println(" ");
//       Serial.println("TOMBOL DITEKAN SELAMA 3 DETIK");

//       time_sync = 0;
//       // Tambahkan fungsi lain yang ingin dieksekusi di sini

//       // Set tombolPressed menjadi false untuk mencegah eksekusi berulang saat tombol tetap ditekan
//       buttonPressed = false;
//     }
//   }
// }

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

void display(){

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
    sprintf(message,"%s", daysOfTheWeek[tm.Wday-1]);

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
    millis1 = millis()+300000;
    pos=0;
  }
}


// fungsi MP3
void printHelp(){
    Serial.println("HELP  ");
    Serial.println(" h = Print again this Massage");
    Serial.println(" p = Play");
    Serial.println(" P = Pause");
    Serial.println(" > = Next");
    Serial.println(" < = Previous");
    Serial.println(" s = Stop Play"); 
    Serial.println(" + = Volume UP");
    Serial.println(" - = Volume DOWN");
    Serial.println(" c = Query current file");
    Serial.println(" q = Query status");
    Serial.println(" v = Query volume");
    Serial.println(" x = Query folder count");
    Serial.println(" t = Query total file count");
    Serial.println(" f = Play folder 1.");
    Serial.println(" S = Sleep");
    Serial.println(" W = Wake up");
    Serial.println(" r = Reset");
  }
//perintah dari karakter pada serial monitor
void sendMP3Command(char c) {
  switch (c) {
    case '?':
    case 'h':
      printHelp();
      break;

    case 'p':
      Serial.println("Play ");
      sendCommand(CMD_PLAY);
      break;

    case 'P':
      Serial.println("Pause");
      sendCommand(CMD_PAUSE);
      break;

    case '>':
      Serial.println("Next");
      sendCommand(CMD_NEXT_SONG);
      sendCommand(CMD_PLAYING_N); // cek nomor file yang di play
      break;

    case '<':
      Serial.println("Previous");
      sendCommand(CMD_PREV_SONG);
      sendCommand(CMD_PLAYING_N); // cek nomor file yang di play
      break;

    case 's':
      Serial.println("Stop Play");
      sendCommand(CMD_STOP_PLAY);
      break;


    case '+':
      Serial.println("Volume Up");
      sendCommand(CMD_VOLUME_UP);
      break;

    case '-':
      Serial.println("Volume Down");
      sendCommand(CMD_VOLUME_DOWN);
      break;

    case 'c':
      Serial.println("Query current file");
      sendCommand(CMD_PLAYING_N);
      break;

    case 'q':
      Serial.println("Query status");
      sendCommand(CMD_QUERY_STATUS);
      break;

    case 'v':
      Serial.println("Query volume");
      sendCommand(CMD_QUERY_VOLUME);
      break;

    case 'x':
      Serial.println("Query folder count");
      sendCommand(CMD_QUERY_FLDR_COUNT);
      break;

    case 't':
      Serial.println("Query total file count");
      sendCommand(CMD_QUERY_TOT_TRACKS);
      break;

    case 'f':
      Serial.println("Playing folder 1");
      sendCommand(CMD_FOLDER_CYCLE, 1, 0);
      break;

    case 'S':
      Serial.println("Sleep");
      sendCommand(CMD_SLEEP_MODE);
      break;

    case 'W':
      Serial.println("Wake up");
      sendCommand(CMD_WAKE_UP);
      break;

    case 'r':
      Serial.println("Reset");
      sendCommand(CMD_RESET);
      break;
  }
}


//arti respon dari modul
String decodeMP3Answer() {
  String decodedMP3Answer = "";

  decodedMP3Answer += sanswer();

  switch (ansbuf[3]) {
    case 0x3A:
      decodedMP3Answer += " -> Memory card inserted.";
      break;

    case 0x3D:
      decodedMP3Answer += " -> Completed play num " + String(ansbuf[6], DEC);
      break;

    case 0x40:
      decodedMP3Answer += " -> Error";
      break;

    case 0x41:
      decodedMP3Answer += " -> Data recived correctly. ";
      break;

    case 0x42:
      decodedMP3Answer += " -> Status playing: " + String(ansbuf[6], DEC);
      break;

    case 0x48:
      decodedMP3Answer += " -> File count: " + String(ansbuf[6], DEC);
      break;

    case 0x4C:
      decodedMP3Answer += " -> Playing: " + String(ansbuf[6], DEC);
      break;

    case 0x4E:
      decodedMP3Answer += " -> Folder file count: " + String(ansbuf[6], DEC);
      break;

    case 0x4F:
      decodedMP3Answer += " -> Folder count: " + String(ansbuf[6], DEC);
      break;
  }

  return decodedMP3Answer;
}

//kirim perintah 
void sendCommand(byte command){
  sendCommand(command, 0, 0);
}

void sendCommand(byte command, byte dat1, byte dat2){
  delay(20);
  Send_buf[0] = 0x7E;    //
  Send_buf[1] = 0xFF;    //
  Send_buf[2] = 0x06;    // Len
  Send_buf[3] = command; //
  Send_buf[4] = 0x01;    // 0x00 tanpa respon, 0x01 ada respon
  Send_buf[5] = dat1;    // data1
  Send_buf[6] = dat2;    // data2
  Send_buf[7] = 0xEF;    //
  Serial.print("Sending: ");
  for (uint8_t i = 0; i < 8; i++){
    mp3.write(Send_buf[i]) ;
    Serial.print(sbyte2hex(Send_buf[i]));
  }
  Serial.println();
}


//konversi byte ke hex
String sbyte2hex(uint8_t b){
  String shex;

  shex = "0X";

  if (b < 16) shex += "0";
  shex += String(b, HEX);
  shex += " ";
  return shex;
}


//konversi hex ke interger
int shex2int(char *s, int n){
  int r = 0;
  for (int i=0; i<n; i++){
     if(s[i]>='0' && s[i]<='9'){
      r *= 16; 
      r +=s[i]-'0';
     }else if(s[i]>='A' && s[i]<='F'){
      r *= 16;
      r += (s[i] - 'A') + 10;
     }
  }
  return r;
}


/********************************************************************************/
/*Function: sanswer. Returns a String answer from mp3 UART module.          */
/*Parameter:- uint8_t b. void.                                                  */
/*Return: String. If the answer is well formated answer.                        */

String sanswer(void){
  uint8_t i = 0;
  String mp3answer = "";

  // Get only 10 Bytes
  while (mp3.available() && (i < 10)){
    uint8_t b = mp3.read();
    ansbuf[i] = b;
    i++;

    mp3answer += sbyte2hex(b);
  }

  // if the answer format is correct.
  if ((ansbuf[0] == 0x7E) && (ansbuf[9] == 0xEF)){
    return mp3answer;
  }

  return "???: " + mp3answer;
}


void notification(){

  get_prayer_times(tmYearToCalendar(tm.Year), tm.Month, tm.Day, Latitude, Longitude, GMT, times);
  
  for (int i=0;i<sizeof(times)/sizeof(double);i++)
  {
    if(i != 4 && i != 7)
    {
      int hours, minutes;
      get_float_time_parts(times[i], hours, minutes);
      if(tm.Minute == minutes && tm.Hour == hours){
        //push notif
        Blynk.logEvent("SHOLAT");

      }
    }
  }
}