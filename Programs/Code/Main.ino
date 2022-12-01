#include <SimpleDHT.h>
#include <TroykaIMU.h>
#include <arduino-timer.h>
#include "LcdMenu.h"
#include "ClimatSystem.h""

#define LCD_ROWS 4
#define LCD_COLS 20
#define DHT_PIN 5
#define COOLER_PIN 6

SimpleDHT11 dht(DHT_PIN);
Barometer barometer;

extern MenuItem ClimatMenu[];
ClimatSystem climatSystem(dht, barometer, COOLER_PIN);

MenuItem ClimatMenu[] = {ItemHeader(),
                       ItemLabel("Temperature:", "NaN"),
                       ItemLabel("Target t:", "NaN"),
                       ItemLabel("Humidity:", "NaN"),
                       ItemLabel("Pressure:", "NaN"),
                       ItemLabel("Cooler PWM:", "NaN"),
                       ItemFooter()};

LcdMenu menu(LCD_ROWS, LCD_COLS);

void setup() {
  
    Serial.begin(9600);
    // Initialize LcdMenu with the menu items
    menu.setupLcdWithMenu(0x27, ClimatMenu);
    //ClimatMenu[1].setValue("Press F");

}

void loop() {
}

void updateClimat(MenuItem menuItem){
    //menuItem.setText("sdededs");
    
}

