#include <Wire.h>
#include <SPI.h>

#include <Adafruit_PN532.h> // библиотека для работы с RFID/NFC

#include <Blynk.h>
#include <SimpleDHT.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>

#include "DFRobotDFPlayerMini.h" // player
#define PlayerSerial Serial3

// пин прерывания
#define PN532_IRQ 7
#define Potpin A0
#define PWMpin 3
#define DHTpin 2
#define BUTTON_PIN_1 22 // кнопка подключена к выводу 22
#define BUTTON_PIN_2 23 // кнопка подключена к выводу 23
#define S0 12
#define S1 11
#define S2 10
#define S3 9
#define sensorOut 8

int frequency = 0;
int frequency_r = 0;
int frequency_g = 0;
int frequency_b = 0;

byte temperature = 0;
byte humidity = 0;

int potValue;
int motorValue;

SimpleDHT11 dht11(DHTpin);
LiquidCrystal_I2C lcd(0x27, 16, 4);
Servo servo1;

BlynkTimer timer;

DFRobotDFPlayerMini myDFPlayer;

// создаём объект для работы со сканером и передаём ему два параметра
// первый — номер пина прерывания
// вторым — число 100
// от Adafruit был программный сброс шилда
// в cканере RFID/NFC 13,56 МГц (Troyka-модуль) этот пин не используется
// поэтому передаём цифру, большая чем любой пин Arduino
Adafruit_PN532 nfc(PN532_IRQ, 100);

// Массивы в которые необходимо записать ID карт:
uint8_t uidFirstCard[] = {0x34, 0xBF, 0xAB, 0x49, 0x28, 0xD2, 0xD7};
// uint8_t uidSecondCard[] = {0x04, 0xAB, 0xB4, 0xDA, 0xA3, 0x40, 0x80};

// функция которая сравнивает два переданных ID
// при совпадении возвращает значение true
// и значение false если ID разные
boolean comparisonOfUid(uint8_t uidRead[8], uint8_t uidComp[8], uint8_t uidLen)
{
  for (uint8_t i = 0; i < uidLen; i++)
  {
    if (uidRead[i] != uidComp[i])
    {
      return false;
    }
    if (i == (uidLen)-0x01)
    {
      return true;
    }
  }
}
// Описание класса обработки сигналов кнопок
class Button
{
public:
  boolean flagPress;                          // признак кнопка сейчас нажата
  boolean flagClick;                          // признак кнопка была нажата (клик)
  void scanState();                           // метод проверки состояние сигнала
  void setPinTime(byte pin, byte timeButton); // метод установки номера вывода и времени (числа) подтверждения

private:
  byte _buttonCount; // счетчик подтверждений стабильного состояния
  byte _timeButton;  // время подтверждения состояния кнопки
  byte _pin;         // номер вывода
};

Button button1; // создание объекта типа Button с именем button1
Button button2; // создание объекта типа Button с именем button2

int moveDirection = 0; // -1 = закрываемся, +1 = открываемся
bool lidClosed = false;

void setup()
{

  pinMode(PWMpin, OUTPUT);
  analogWrite(PWMpin, 0); 
  delay(3000);
  analogWrite(PWMpin, 255); 
  delay(3000);

  Serial.begin(115200);
 /*  PlayerSerial.begin(9600);

  Serial.println("Инициализация плеера");
  if (!myDFPlayer.begin(PlayerSerial))
  { // Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true)
    {
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println("Плеер готов.");

  myDFPlayer.volume(10); // Set volume value. From 0 to 30
  myDFPlayer.play(1);    // Play the first mp3 */

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("temperature: ");
  lcd.setCursor(0, 1);
  lcd.print("target: ");

  pinMode(Potpin, INPUT);

  button1.setPinTime(BUTTON_PIN_1, 15); // вызов метода установки объекта button1 с параметрами: номер вывода 22, число подтверждений 15
  button2.setPinTime(BUTTON_PIN_2, 15);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);
  // Setting frequency-scaling to 20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  servo1.attach(5);

  InitNFC();

  timer.setInterval(1500L, readTemp);
  timer.setInterval(100L, ScanColor);
  timer.setInterval(150L, printLCD);
  timer.setInterval(1000L, ScanNFC);
}

void loop()
{
  timer.run(); // проверяет, не пора ли вызывать

  button1.scanState();
  button2.scanState();

  potValue = map(analogRead(Potpin), 0, 1023, 17, 31);
  int clampedTemperature = constrain((int)temperature, potValue, 31);
  motorValue = map(clampedTemperature, potValue, 31, 0, 255) * 1.5;
  motorValue = constrain(motorValue, 0, 255); // ограничиваем значение шима мотора
  //Serial.println(motorValue);
  analogWrite(PWMpin, motorValue); // шим на вентиляторе

  if (button1.flagClick == true)
  {
    // было нажатие кнопки
    button1.flagClick = false; // сброс признака клика
    servo1.writeMicroseconds(1300);
  }
  if (button2.flagClick == true)
  {
    // было нажатие кнопки
    button2.flagClick = false; // сброс признака клика
    servo1.writeMicroseconds(1700);
  }

  delay(2);
}

void readTemp()
{
  // читаем температуру
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess)
  {
    Serial.print("Read DHT11 failed, err=");
    Serial.print(SimpleDHTErrCode(err));
    Serial.print(",");
    Serial.println(SimpleDHTErrDuration(err));
    return;
  }

  /*   Serial.print("Sample OK: ");
    Serial.print((int)temperature);
    Serial.print(" *C, ");
    Serial.print((int)humidity);
    Serial.println(" H"); */
}

void ScanColor()
{
  // Setting red filtered photodiodes to be read
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  // Reading the output frequency
  frequency = pulseIn(sensorOut, LOW);
  frequency_r = frequency;
  // Printing the value on the serial monitor
  Serial.print("R= ");     // printing name
  Serial.print(frequency); // printing RED color frequency
  Serial.print("  ");

  // Setting Green filtered photodiodes to be read
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  // Reading the output frequency
  frequency = pulseIn(sensorOut, LOW);
  frequency_g = frequency;
  // Printing the value on the serial monitor
  Serial.print("G= ");     // printing name
  Serial.print(frequency); // printing RED color frequency
  Serial.print("  ");

  // Setting Blue filtered photodiodes to be read
  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  // Reading the output frequency
  frequency = pulseIn(sensorOut, LOW);
  frequency_b = frequency;
  // Printing the value on the serial monitor
  Serial.print("B= ");     // printing name
  Serial.print(frequency); // printing RED color frequency
  Serial.print("  ");

  int minFreq = min(frequency_r, frequency_b);
  minFreq = min(minFreq, frequency_g);

  if (minFreq == frequency_r)
  {
    Serial.println("Red");
    if (moveDirection == 1)
    {
      // открылись
      servo1.writeMicroseconds(1500);
      moveDirection = 0;
      lidClosed = false;
    }
  }
  if (minFreq == frequency_g)
  {
    Serial.println("Green");
    if (moveDirection == -1)
    {
      // закрылись
      servo1.writeMicroseconds(1500);
      moveDirection = 0;
      lidClosed = true;
    }
  }
  if (minFreq == frequency_b)
  {
    Serial.println("Blue");
  }
}

void printLCD()
{
  // lcd.clear();
  lcd.setCursor(12, 0);
  lcd.print("   ");
  lcd.setCursor(7, 1);
  lcd.print("       ");

  lcd.setCursor(13, 0);
  lcd.print((int)temperature);
  lcd.setCursor(8, 1);

  lcd.print(potValue);
  lcd.print(" ");
  lcd.print(motorValue);
}

void Button::scanState()
{

  if (flagPress == (!digitalRead(_pin)))
  {
    // состояние сигнала осталось прежним
    _buttonCount = 0; // сброс счетчика состояния сигнала
  }
  else
  {
    // состояние сигнала изменилось
    _buttonCount++; // +1 к счетчику состояния сигнала

    if (_buttonCount >= _timeButton)
    {
      // состояние сигнала не менялось заданное время
      // состояние сигнала стало устойчивым
      flagPress = !flagPress; // инверсия признака состояния
      _buttonCount = 0;       // сброс счетчика состояния сигнала

      if (flagPress == true)
        flagClick = true; // признак клика на нажатие
    }
  }
}

// метод установки номера вывода и времени подтверждения
void Button::setPinTime(byte pin, byte timeButton)
{

  _pin = pin;
  _timeButton = timeButton;
  pinMode(_pin, INPUT_PULLUP); // определяем вывод как вход
}

void InitNFC()
{
  // инициализация RFID/NFC сканера
  nfc.begin();
  int versiondata = nfc.getFirmwareVersion();
  if (!versiondata)
  {
    while (1)
    {
      Serial.print("Didn't find RFID/NFC reader");
      delay(1000);
    }
  }
  Serial.println("Found RFID/NFC reader");
  // настраиваем модуль
  nfc.SAMConfig();
  Serial.println("Waiting for a card ...");
}

void ScanNFC()
{
  uint8_t success;
  // буфер для хранения ID карты
  uint8_t uid[8];
  // размер буфера карты
  uint8_t uidLength;
  // слушаем новые метки
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50U);
  // если найдена карта
  if (success)
  {
    // ID вернёт true иначе оставляем всё как есть
    if (comparisonOfUid(uid, uidFirstCard, uidLength))
    {
      Serial.println("FirstTAG");
      if (lidClosed == true)
      {
        // открываем
        moveDirection = 1;
        servo1.writeMicroseconds(1900);
      }
      else
      {
        // закрываем
        moveDirection = -1;
        servo1.writeMicroseconds(1300);
      }
    }
  }
}

// log for player
void printDetail(uint8_t type, int value)
{
  switch (type)
  {
  case TimeOut:
    Serial.println(F("Time Out!"));
    break;
  case WrongStack:
    Serial.println(F("Stack Wrong!"));
    break;
  case DFPlayerCardInserted:
    Serial.println(F("Card Inserted!"));
    break;
  case DFPlayerCardRemoved:
    Serial.println(F("Card Removed!"));
    break;
  case DFPlayerCardOnline:
    Serial.println(F("Card Online!"));
    break;
  case DFPlayerUSBInserted:
    Serial.println("USB Inserted!");
    break;
  case DFPlayerUSBRemoved:
    Serial.println("USB Removed!");
    break;
  case DFPlayerPlayFinished:
    Serial.print(F("Number:"));
    Serial.print(value);
    Serial.println(F(" Play Finished!"));
    break;
  case DFPlayerError:
    Serial.print(F("DFPlayerError:"));
    switch (value)
    {
    case Busy:
      Serial.println(F("Card not found"));
      break;
    case Sleeping:
      Serial.println(F("Sleeping"));
      break;
    case SerialWrongStack:
      Serial.println(F("Get Wrong Stack"));
      break;
    case CheckSumNotMatch:
      Serial.println(F("Check Sum Not Match"));
      break;
    case FileIndexOut:
      Serial.println(F("File Index Out of Bound"));
      break;
    case FileMismatch:
      Serial.println(F("Cannot Find File"));
      break;
    case Advertise:
      Serial.println(F("In Advertise"));
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}
