#include <Wire.h>
#include "RTClib.h" //Pour le module RTC
#include <Adafruit_SSD1306.h> //OLED
#include <Adafruit_ADS1015.h> //Carte analog/I2C
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h> //Accès à l'EEPROM

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define LED_PIN 3
#define LED_COUNT 18

RTC_DS3231 rtc; //Module RTC

Adafruit_ADS1015 ads1015; //Carte analog/I2C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire); //OLED
Adafruit_NeoPixel strip (LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

//pins
#define buttonPin 4

//Variables
int potVal, pot, lastx = 0, lasty = 0, x = 0;
int menuIndex = 0, menuState = 0;
bool buttonState;
bool inMenu;
bool sensLed;
int led, bright;
bool mesure = 0;
long chrono1, chrono2, lastTime, BPMTime, displayTime, storeTime, lastMeasure;
float periode, BPM, periodeSeconde;
int pulseVal;
int annee;

//Time
unsigned long changeTime;
unsigned long ledTime;

void setup() {
  //Begin
  Serial.begin (9600);
  rtc.begin();
  ads1015.begin();
  strip.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  //pinMode
  pinMode (buttonPin, INPUT_PULLUP);
  pinMode (8, OUTPUT);

  //Init OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor (WHITE);
  display.display();

  //Init Variables
  changeTime = millis();
  ledTime = millis();
  inMenu = 0;
  led = 1;
  sensLed = 1;
  bright = 0;
  displayTime = millis();

  strip.show();

}

int potValue () {
  int16_t adc2;
  adc2 = ads1015.readADC_SingleEnded(2);
  pot = map(adc2, 0, 1231, 0, 1023);
  return pot;
}

void afficherMenu ()
{
  int potVal = potValue();

  char menu[5][20] {"Mesurer BPM", "Consulter donnees", "Date & heure", "Effacer memoire"};
  int i;


  display.clearDisplay();
  display.setTextSize (2);
  display.setTextColor (WHITE, BLACK);
  display.setCursor (40, 2);
  display.println ("Menu");
  display.setTextSize(1);

  if (potVal <= 255)
  {
    menuIndex = 0;
  }

  else if (potVal > 255 && potVal <= 512)
  {
    menuIndex = 1;
  }

  else if (potVal > 512 && potVal <= 766)
  {
    menuIndex = 2;
  }

  else if (potVal > 766 && potVal <= 1023)
  {
    menuIndex = 3;
  }

  for (i = 0 ; i <= 3 ; i++)
  {
    if (menuIndex == i)
    {
      display.setTextColor (BLACK, WHITE);
    }

    else
    {
      display.setTextColor (WHITE, BLACK);
    }

    display.println (menu[i]);
  }

  display.display();
}

void gereMenu()
{
  unsigned long timeNow;
  byte h, m, s;

  switch (menuIndex)
  {
    case 0:
      mesurerBPM();
      break;

    case 1:
      accesDonnees();
      break;

    case 2:
      afficherDate();
      break;

    case 3 :
      effacerEEPROM();
      break;
  }
}

void mesurerBPM()
{
  delay (100);
  int i = 0, j = 0, moyBPM = 0, sommeBPM = 0;
  x = 0;

  BPMTime = millis();
  displayTime = millis();
  storeTime = millis();
  chrono1 = millis();
  chrono2 = millis();

  display.clearDisplay();
  display.display();

  byte indexEEPROM = EEPROM.read (0);

  while (millis() - BPMTime <= 20000)
  {
    pulseVal = analogRead (A3);

    if (pulseVal >= 620 && mesure == 0 && millis() - lastTime >= 350)
    {
      chrono1 = millis();
      mesure = 1;
      lastTime = millis();

      digitalWrite (8, 1);
      delay (50);
      digitalWrite (8, 0);
      lastMeasure = millis();
    }

    if (pulseVal >= 620 && mesure == 1 && millis() - lastTime >= 350)
    {
      chrono2 = millis();
      mesure = 0;
      lastTime = millis();
      digitalWrite (8, 1);
      delay (50);
      digitalWrite (8, 0);
      lastMeasure = millis();
    }

    if (chrono2 - chrono1 >= 0)
    {
      periode = chrono2 - chrono1;
    }

    else
    {
      periode = chrono1 - chrono2;
    }

    if (millis() - lastMeasure >= 2000)
    {
      BPM = 0;
    }

    else if (periode != 0) {
      periodeSeconde = periode / 1000;
      BPM = 60 / periodeSeconde;
    }

    else
    {
      BPM = 0;
    }

    if (millis() - storeTime >= 500 && i <= 35)
    {
      if (BPM > 50 && BPM < 180) {
        sommeBPM = sommeBPM + BPM;
        j++;
      }
      storeTime = millis();

      i++;
    }

    Serial.println (BPM);

    if (BPM < 50)
    {
      for (led = 0 ; led <= 18 ; led ++)
      {
        strip.setPixelColor (led, 255, 0, 0);
        strip.show();
      }
    }

    if (BPM >= 50 && BPM < 60)
    {
      for (led = 0 ; led <= 18 ; led ++)
      {
        strip.setPixelColor (led, 255, 255, 0);
        strip.show();
      }
    }

    if (BPM >= 60 && BPM < 75)
    {
      for (int led = 0 ; led <= 18 ; led ++)
      {
        strip.setPixelColor (led, 0, 255, 0);
        strip.show();
      }
    }

    if (BPM >= 75 && BPM < 90)
    {
      for (int led = 0 ; led <= 18 ; led ++)
      {
        strip.setPixelColor (led, 255, 255, 0);
        strip.show();
      }
    }

    if (BPM >= 90)
    {
      for (led = 0 ; led <= 18 ; led ++)
      {
        strip.setPixelColor (led, 255, 0, 0);
        strip.show();
      }
    }

    if (x > 127)
    {
      display.clearDisplay();
      x = 0;
      lastx = x;
    }

    int pulseDraw = map (pulseVal, 0, 1023, 0, 10000);

    int y = 410 - (pulseDraw / 16);

    display.writeLine (lastx, lasty, x, y, WHITE);
    lasty = y;
    lastx = x;

    display.writeFillRect(0, 0, 128, 22, BLACK);
    display.setTextSize (2);
    display.setTextColor (WHITE);
    display.setCursor (5, 5);
    display.print (BPM);
    display.print (" BPM");
    display.display();
    displayTime = millis();

    int  potVal = potValue();

    int scale = map (potVal, 0, 1231, 1, 10);

    x = x + scale;
  }

  if (j != 0) {
    moyBPM = sommeBPM / j;
  }


  if (EEPROM.read (0) <= 450) {

    DateTime now = rtc.now();

    int currentDay, currentMonth, currentYear, currentHour, currentMinute, currentSecond;

    currentDay = now.day (), DEC;
    currentMonth = now.month (), DEC;
    currentYear = now.year (), DEC;
    currentHour = now.hour(), DEC;
    currentMinute = now.minute(), DEC;

    EEPROM.write (indexEEPROM + 1, currentDay);
    EEPROM.write (indexEEPROM + 2, currentMonth);
    EEPROM.put (indexEEPROM + 3, currentYear);
    EEPROM.write (indexEEPROM + 19, currentHour);
    EEPROM.write (indexEEPROM + 20, currentMinute);
    EEPROM.write (indexEEPROM + 21, moyBPM);
    EEPROM.write (0, indexEEPROM + 22);

    inMenu = 0;
  }
}

void accesDonnees()
{
  int valScroll;

  for (led = 0 ; led <= 18 ; led++)
  {
    strip.setPixelColor (led, 255, 100, 0);
    strip.show();
  }

  if (EEPROM.read (0) > 40)
  {
    valScroll = EEPROM.read (0) - 45;
  }

  else
  {
    valScroll = 0;
  }

  int potVal = potValue();
  int y = map (potVal, 0, 1023, 0, -valScroll);

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor (0, y);

  if (EEPROM.read (0) == 0)
  {
    display.clearDisplay();
    display.setTextSize (2);
    display.setTextColor (WHITE);
    display.setCursor (28, 23);
    display.print ("EMPTY !");
    display.display();
    inMenu = 0;
    clignoteRouge();
  }

  else {
    for (int i = 1 ; i <= EEPROM.read (0) ; i++)
    {
      if (EEPROM.read(i) < 10)
      {
        display.print ("0");
      }

      display.print(EEPROM.read(i));
      display.print("/");

      if (EEPROM.read(i + 1) < 10)
      {
        display.print ("0");
      }

      display.print(EEPROM.read(i + 1));
      display.print("/");
      display.print(EEPROM.get(i + 2, annee));
      display.print(", ");

      if (EEPROM.read(i + 18) < 10)
      {
        display.print ("0");
      }

      display.print(EEPROM.read(i + 18));
      display.print(":");

      if (EEPROM.read(i + 19) < 10)
      {
        display.print ("0");
      }

      display.print(EEPROM.read(i + 19));
      display.println(" :");
      display.print(EEPROM.read(i + 20));
      display.print(" BPM");

      i = i + 21;
      display.println ("\n");
    }

    display.display();
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor (WHITE);
  display.display();
}

void effacerEEPROM()
{
  for (led = 0 ; led <= 18 ; led++)
  {
    strip.setPixelColor (led, 255, 0, 0);
    strip.show();
  }

  display.clearDisplay ();
  display.setTextColor (WHITE);
  display.setCursor (37, 20);
  display.print ("Effacement");
  display.setCursor (20, 33);
  display.print ("de la memoire...");
  display.display();

  for (int i = 0 ; i <= 512 ; i++)
  {
    EEPROM.write (i, 0);
  }

  for (led = 0 ; led <= 18 ; led++)
  {
    strip.setPixelColor (led, 0, 255, 0);
    strip.show();
  }

  delay (500);
  inMenu = 0;
}

void clignoteRouge()
{
  for (byte i = 0 ; i <= 2 ; i++)
  {
    for (led = 0 ; led <= 18 ; led++)
    {
      strip.setPixelColor (led, 255, 0, 0);
      strip.show();
    }

    delay (250);

    for (led = 0 ; led <= 18 ; led++)
    {
      strip.setPixelColor (led, 0, 0, 0);
      strip.show();
    }

    delay (250);
  }

  for (led = 0 ; led <= 18 ; led++)
  {
    strip.setPixelColor (led, 255, 0, 0);
    strip.show();
  }

  delay (250);
}

void afficherDate()
{
  for (led = 0 ; led <= 18 ; led++) {
    strip.setPixelColor (led, 100, 255, 0);
    strip.show();
  }

  int currentDay, currentMonth, currentHour, currentMinute, currentSecond;

  DateTime now = rtc.now();

  display.clearDisplay();

  display.setTextColor (WHITE);
  display.setTextSize(1);
  display.setCursor (39, 10);

  currentDay = now.day(), DEC;

  if (currentDay < 10)
  {
    display.print ("0");
  }

  display.print (currentDay);
  display.print ("/");

  currentMonth = now.month(), DEC;

  if (currentMonth < 10)
  {
    display.print ("0");
  }

  display.print (currentMonth);
  display.print ("/");
  display.print (now.year(), DEC);

  display.setTextSize (2);
  display.setCursor (19, 28);

  currentHour = now.hour(), DEC;

  if (currentHour < 10)
  {
    display.print ("0");
  }

  display.print (currentHour);
  display.print (":");

  currentMinute = now.minute(), DEC;

  if (currentMinute < 10)
  {
    display.print ("0");
  }

  display.print (now.minute(), DEC);
  display.print (":");

  currentSecond = now.second(), DEC;

  if (currentSecond < 10)
  {
    display.print ("0");
  }

  display.print (currentSecond);
  display.display();
}

void inMenuState()
{
  buttonState = digitalRead (buttonPin);

  if (buttonState == 0 && inMenu == 0 && millis() - changeTime >= 200)
  {
    inMenu = 1;
    changeTime = millis();
  }

  else if (buttonState == 0 && inMenu == 1 && millis() - changeTime >= 200)
  {
    inMenu = 0;
    changeTime = millis();
  }
}

void loop() {
  inMenuState();

  if (inMenu == 0)
  {
    afficherMenu();

    for (led = 0 ; led <= 18 ; led++)
    {
      strip.setPixelColor (led, 0, 0, 255);
      strip.show();
    }
  }

  else {
    gereMenu();
  }
}
