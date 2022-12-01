#pragma once
#include <Arduino.h>
#include <SimpleDHT.h>
#include <TroykaIMU.h>

class ClimatSystem {

public:
    ClimatSystem(SimpleDHT11 dht, byte coolerPin){
        _dht = dht;
        _coolerPin = coolerPin;
        pinMode(_coolerPin, OUTPUT);
    };

    void update(){
        _temperature = _dht.getTemperatureC()+_barometer.readTemperatureC()/2;
        _humidity = _dht.getHumidity();
        _pressure = _barometer.readPressureMillibars();
        int value=0;
        value = getTargetTemperature() - getTemperature();
        setCoolerPWM(value*50);
    };

    int getTemperature(){return _temperature;};
    String getTemperatureString(){return String(_temperature);};

    int getTargetTemperature(){return _targetTemperature;};
    String getTargetTemperatureString(){return String(_targetTemperature);}; 
    void setTargetTemperature(int targetTemperature){
        _targetTemperature = constrain(targetTemperature, 18, 28);
    };

    int getHumidity(){return _humidity;};
    String getHumidityString(){ return String(_humidity);};

    int getPressure(){return _pressure;};
    String getPressureString(){return String(_pressure);};

    byte getCoolerPWM(){return _coolerPWM;};
    void setCoolerPWM(byte value){
        _coolerPWM = constrain(value, 0, 255);
        analogWrite(_coolerPin, _coolerPWM);
    };

private:
    int _temperature;  
    int _targetTemperature; 
    int _humidity;
    int _pressure;
    byte _coolerPWM;
    SimpleDHT11 _dht;
    Barometer _barometer;
    byte _coolerPin;
};
