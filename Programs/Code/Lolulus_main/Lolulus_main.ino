#include <GyverButton.h>
//#include <TroykaDHT.h>
#include <TroykaIMU.h>
#include <TroykaRTC.h>
#include "LcdMenu.h"
#include <SimpleDHT.h>
#include <Servo.h>


#define LCD_ROWS 4
#define LCD_COLS 20
#define BTN_UP_PIN 2	
#define BTN_ENTER_PIN 3	
#define BTN_DOWN_PIN 4	

#define DHT11_PIN 5	
#define COOLER_PIN 6

#define STORAGE_PIN 9
#define STORAGE_PET_PIN 8

#define ALARM_PIN 53

byte temperature;  
int targetTemperature = 18; 
byte humidity;
int pressure;
byte coolerPWM;
bool isStorageOpen;
bool isPetStorageOpen;

int alarmSens = 250;
bool isAlarmOn=false;
bool isAlarmEnable = true;

bool isInSideLightOn = true;
bool isOutSideLightOn = true;

SimpleDHT11 dht11(DHT11_PIN);
Barometer barometer;
Gyroscope gyroscope;
Servo storageServo;
Servo petServo;

GButton buttUp(BTN_UP_PIN);
GButton buttEnter(BTN_ENTER_PIN);
GButton buttDown(BTN_DOWN_PIN);

extern MenuItem mainMenu[];
    extern MenuItem systemsMenu[];
        extern MenuItem climatMenu[];
        extern MenuItem lightingMenu[];
            extern MenuItem inSideLightingMenu[];
            extern MenuItem outSideLightingMenu[];
        extern MenuItem alarmMenu[];
    extern MenuItem settingsMenu[];
    extern MenuItem alarmClockMenu[];
    extern MenuItem storageMenu[];

void targTempsCallback(String value) {
    int i = value.toInt();
    setTargetTemperature(i);
    climatMenu[3].setValue(String(targetTemperature));
}
void toggleStorage(uint8_t isOn) {
    isStorageOpen = isOn;
    if (isOn) storageServo.write(180);
    else storageServo.write(0);
}
void togglePetStorage(uint8_t isOn) {
    isPetStorageOpen = isOn;
    if (isOn) petServo.write(180);
    else petServo.write(0);
}
void toggleAlarm(uint8_t isOn) {
    isAlarmEnable = isOn;
}

void toggleInSideLighting(uint8_t isOn) {
    isInSideLightOn = isOn;
}
void toggleOutSideLighting(uint8_t isOn) {
    isOutSideLightOn = isOn;
}


MenuItem mainMenu[] = {ItemHeader(),
                        MenuItem("<-"),
                        ItemSubMenu("Systems->", systemsMenu),
                        ItemSubMenu("Alarm clock->", alarmClockMenu),
                        ItemSubMenu("Settings->", settingsMenu),
                        ItemSubMenu("Storage->", storageMenu),
                    ItemFooter()};
MenuItem systemsMenu[] = {ItemHeader(mainMenu),
                        ItemSubMenu("<-", mainMenu),
                        ItemSubMenu("Climat control->", climatMenu),
                        MenuItem("Sound"),
                        ItemSubMenu("Lighting->", lightingMenu),
                        ItemSubMenu("Alarm->", alarmMenu),
                        MenuItem("Sleep Settings"),
                    ItemFooter()};
                    MenuItem climatMenu[] = {ItemHeader(systemsMenu),
                       ItemSubMenu("<-", systemsMenu),
                       ItemLabel("Temperature:", "NaN"),
                       ItemLabel("Target t:", "NaN", targTempsCallback),
                       ItemLabel("Humidity:", "NaN"),
                       ItemLabel("Pressure:", "NaN"),
                       ItemLabel("Cooler PWM:", "NaN"),
                    ItemFooter()};
                    MenuItem lightingMenu[] = {ItemHeader(systemsMenu),
                        ItemSubMenu("<..", systemsMenu),
                        ItemSubMenu("Inside->", inSideLightingMenu),
                        ItemSubMenu("Outside->", outSideLightingMenu),
                    ItemFooter()};
                            MenuItem inSideLightingMenu[] = {ItemHeader(lightingMenu),
                                ItemSubMenu("<..", lightingMenu),
                                ItemLabel("inside mode:", "red"),
                                ItemToggle("Status", toggleInSideLighting),
                            ItemFooter()};
                            MenuItem outSideLightingMenu[] = {ItemHeader(lightingMenu),
                                ItemSubMenu("<..", lightingMenu),
                                ItemLabel("outside mode:", "blue"),
                                ItemToggle("Status", toggleOutSideLighting),
                            ItemFooter()};
                    MenuItem alarmMenu[] = {ItemHeader(systemsMenu),
                        ItemSubMenu("<..", systemsMenu),
                        ItemLabel("Alarm sens:", String(alarmSens)),
                        ItemToggle("Alarm:", toggleAlarm),
                    ItemFooter()};
MenuItem settingsMenu[] = {ItemHeader(mainMenu),
                        ItemSubMenu("<..", mainMenu),
                        MenuItem("Settings Line 1"),
                        MenuItem("Settings Line 2"), 
                    ItemFooter()};                  
MenuItem alarmClockMenu[] = {ItemHeader(mainMenu),
                        ItemSubMenu("<..", mainMenu),
                        MenuItem("Alarm Clock Line 1"),
                        MenuItem("Alarm Clock Line 2"), 
                    ItemFooter()};                  
MenuItem storageMenu[] = {ItemHeader(mainMenu),
                        ItemSubMenu("<..", mainMenu),
                        ItemToggle("Storage:", toggleStorage),
                        ItemToggle("Pet storage:", togglePetStorage),
                    ItemFooter()};

LcdMenu menu(LCD_ROWS, LCD_COLS);

void setup() {
    Serial.begin(9600);
    barometer.begin();
    gyroscope.begin();
    storageServo.attach(STORAGE_PIN);
    petServo.attach(STORAGE_PET_PIN);
    pinMode(ALARM_PIN, OUTPUT);
    menu.setupLcdWithMenu(0x27, climatMenu);

    buttonSetup(buttUp);
    buttonSetup(buttEnter);
    buttonSetup(buttDown);
}

long timer = 0;
long alarmTimer = 0;
void loop() {

    if (isAlarmEnable){
        if (isAlarmOn){
            if (millis() - alarmTimer >=5000){
                alarmTimer = millis();
                setAlarmStatus(false);
            }
        }else{
            setAlarmStatus(checkGyro());
        }
    }else{
        if (isAlarmOn) setAlarmStatus(false);
    }

    if (millis() - timer >= 2500){
        //Serial.println(timer);
        timer = millis();
        updateClimatSystem();
        menu.update();
    }

    buttUp.tick();
    buttEnter.tick();
    buttDown.tick();

    if (menu.isInEditMode()){
        if (buttUp.isClick()) menu.right();
        if (buttDown.isClick()) menu.left(); 
    } else {
        if (buttUp.isClick()) menu.up(); 
        if (buttDown.isClick()) menu.down(); 
    }
   
    if (buttEnter.isClick()) {
        menu.enter();   
        if (menu.isInEditMode()){
            menu.setCursorIcon(42);
        } else {
            menu.setCursorIcon(0x7E);
        }
        
    }   
}

void buttonSetup(GButton button){
    button.setDebounce(50);        // настройка антидребезга (по умолчанию 80 мс)
    button.setTimeout(300);        // настройка таймаута на удержание (по умолчанию 500 мс)
    button.setClickTimeout(600);   // настройка таймаута между кликами (по умолчанию 300 мс)

    button.setType(HIGH_PULL);

    button.setDirection(NORM_OPEN);
}


//Climat
void updateClimatSystem(){
    dht11.read(&temperature, &humidity, NULL);
    pressure = barometer.readPressureMillibars();
    int value=0;
    value = abs(targetTemperature - temperature);
    setCoolerPWM(value*50);

    climatMenu[2].setValue(String(temperature));
    climatMenu[3].setValue(String(targetTemperature));
    climatMenu[4].setValue(String(humidity));
    climatMenu[5].setValue(String(pressure));
    climatMenu[6].setValue(String(coolerPWM));
};

void setTargetTemperature(int value){
    targetTemperature = constrain(value, 18, 28);
    menu.update();
};

void setCoolerPWM(byte value){
    coolerPWM = constrain(value, 0, 255);
    analogWrite(COOLER_PIN, coolerPWM);
};

bool checkGyro(){
    int temp=0;
    int x = abs(gyroscope.readRotationDegX());
    int y = abs(gyroscope.readRotationDegY());
    int z = abs(gyroscope.readRotationDegZ());
    temp = x + y + z;
    
    return (temp>alarmSens);
}
void setAlarmStatus(bool isOn){
    isAlarmOn = isOn;
    if (isOn){
        digitalWrite(ALARM_PIN, HIGH);
    }else{
        digitalWrite(ALARM_PIN, LOW);
    }
}
