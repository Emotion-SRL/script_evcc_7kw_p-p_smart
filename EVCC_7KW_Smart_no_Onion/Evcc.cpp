#include "Arduino.h"
#include "Evcc.h"
#include "EmonLib.h"
#include <ArduinoJson.h>
#include "TimerOne.h"
#include <string.h>

Evcc::Evcc(
    int ID,
    int PP,
    int CP,
    int CP_F,
    int samples,
    int RL,
    int GL,
    int BL,
    int REL1,
    int CT1,
    int CT_EXT
    )

{
    _ID = ID;
    _PP = PP;
    _CP = CP;
    _CP_F = CP_F;
    _samples = samples;
    _RL = RL;
    _GL = GL;
    _BL = BL;
    _REL1 = REL1;
    _CT1 = CT1;
    _CT_EXT = CT_EXT;

    pinMode(CP, OUTPUT);
    pinMode(RL, OUTPUT);
    pinMode(GL, OUTPUT);
    pinMode(BL, OUTPUT);
    pinMode(REL1, OUTPUT);

    //Serial.begin(9600);
    Timer1.initialize(1000); // Frequency, 1000us = 1khz

    
    _IL1.current(_CT1, 60.6); // Current: input pin, calibration: (40A / 0.02A) / 33 Ohms = 60.6  
    _IL1_EXT.current(_CT_EXT, 55.5); // Current: input pin, calibration: (100A / 0.05A) / 36 Ohms = 55.5      
}


void Evcc::Timer_init()
{  
Timer1.initialize(1000); // Frequency, 1000us = 1khz
delay(100);
}


void Evcc::Plug_start()
{
    
    int PPstate = PPread(); // Read PP state
    //Serial.print(PPstate);
    //Serial.println("%    ");
    delay(20);

    CPout_on();

    int CPstate = CPread(); // Read CP state
    delay(20);

    if ((CPstate < 973) && (CPstate > 853))
    {
        AC_off();
        for(int i=0; i<255; i++) {
        Led_set_color(i, 255-i, 0);
         delay(5);
        }

        // fade from red to blue
        for(int i=0; i<255; i++) {
        Led_set_color(255-i, 0, i);
        delay(5);
        }

        // fade from blue to green
        for(int i=0; i<255; i++) {
        Led_set_color(0, i, 255-i);
        delay(2);
        }
        _chrg_state = "EV NOT CONNECTED";
    }

    else if ((CPstate < 853) && (CPstate > 758))
    {
        AC_off();
        Led_green();
        _chrg_state = "EV CONNECTED TO WALLBOX ";
        //Serial.println(_ID);
    }

    else if ((CPstate < 758) && (CPstate > 628))
    {
        AC_on();
        Led_blue(); // Function for LED blinking (It slows the entire loop!)
        _chrg_state = "CHARGING";
        //Serial.println(_ID);
    }

    else
    {
        AC_off();
        Led_red(); // ERROR
        _chrg_state = "ERROR WALLBOX ";
        //Serial.println(_ID);
    }

}


void Evcc::Plug_stop()
{
    
    int PPstate = PPread(); // Read PP state
    //Serial.print(PPstate);
    //Serial.println("%    ");
    delay(20);

    CPout_off();

    int CPstate = CPread(); // Read CP state
    delay(20);

    if ((CPstate < 973) && (CPstate > 853))
    {
      AC_off();
        for(int i=0; i<255; i++) {
        Led_set_color(i, 255-i, 0);
         delay(5);
        }

        // fade from red to blue
        for(int i=0; i<255; i++) {
        Led_set_color(255-i, 0, i);
        delay(5);
        }

        // fade from blue to green
        for(int i=0; i<255; i++) {
        Led_set_color(0, i, 255-i);
        delay(2);
        }
        _chrg_state = "WALLBOX STOPPED, EV NOT CONNECTED";
    }

    else if ((CPstate < 853) && (CPstate > 758))
    {
        AC_off();
        Led_green();
        _chrg_state = "WALLBOX STOPPED, EV CONNECTED TO WALLBOX ";
        //Serial.println(_ID);
    }

    else if ((CPstate < 758) && (CPstate > 628))
    {
        AC_off();
        Led_blue(); // Function for LED blinking (It slows the entire loop!)
        _chrg_state = "WALLBOX STOPPED, ILLEGAL STATE ";
        //Serial.println(_ID);
    }

    else
    {
        AC_off();
        Led_red(); // ERROR
        _chrg_state = "WALLBOX STOPPED, ERROR WALLBOX ";
        //Serial.println(_ID);
    }

}



int Evcc::PPread()
{
    int duty;
    int PP_reading = analogRead(_PP);

    if (PP_reading > 743)
    {

        duty = 10;
        //Serial.print("6A    ");
    }

    else if ((PP_reading < 743) && (PP_reading > 508))
    {

        duty = 21;
        //Serial.print("13A   ");
    }

    else if ((PP_reading < 508) && (PP_reading > 254))
    {

        duty = 33;
        //Serial.print("20A    ");
    }

    else if ((PP_reading < 254) && (PP_reading > 131))
    {

        duty = 53;
        //Serial.print("32A   ");
    }

    else if ((PP_reading < 131)) //&& (amps > 46)
    {

        duty = 92;
        //Serial.print("70A   ");
    }

    return duty;
}

int Evcc::CPread()
{
    int maximum = 0;
    int minimum = 1023;
    int minvaluePilot = 0;
    int value;
    for (int i = 0; i <= _samples; i++)
    {
        value = analogRead(_CP_F); // pilotPin or currentSensPin
        if (value <= minimum)
        {
            minimum = value;
            minvaluePilot = minimum;
        }
        if (value >= maximum)
        {
            maximum = value;
        }
    }

    return maximum;
}

void Evcc::CPout_on() {

  //int duty = PPread();                 // int duty = PPread();   for current setted by PP resistor reading
  int limited_duty = constrain(_serial_duty, 10, 90);
  int mapped_duty = map(limited_duty, 0, 100, 2, 1023);
  Timer1.pwm(_CP, mapped_duty);
  //Serial.println(mapped_duty);
  
}

void Evcc::UpdateDuty(int newValue){
     _serial_duty = newValue;
     //Serial.println(_serial_duty);
 }


void Evcc::CPout_off() {

  digitalWrite(_CP, HIGH);
  
}

// CONTACTOR AND CURRENT SENSOR CONTROL FUNCTIONS

void Evcc::AC_on()
{
    digitalWrite(_REL1, HIGH);
}

void Evcc::AC_off()
{
    digitalWrite(_REL1, LOW);
}

bool Evcc::Smart_reg(int max_amps, int min_amps, bool line_splitted, int hys)
{ 
  bool started = true;
  int amps;  
  float EXT_amps = (_IL1_EXT.calcIrms(1480));
  delay(5);
  float EV_amps = (_IL1.calcIrms(1480));
  delay(5);

  // caso linea splittata
  if(line_splitted){
    amps = max_amps - EXT_amps;
    if(amps >= min_amps && amps <= max_amps){
      UpdateDuty(int(amps/0.6));
      started = true;
    } else {
      started = false; 
    }
       
  } else {
    amps = max_amps - (EXT_amps - EV_amps);
    if(min_amps <= amps && amps <= max_amps){
      UpdateDuty(int(amps/0.6));
      started = true;
    } else {  
      started = false;     
    }      
  }
  
  delay(10);
  int hysteresis = min_amps + hys;;
  // se amps è fuori dall' intervallo e amps è maggiore di min_amps + hys riparte la ricarica
  if (started == false && amps >= hysteresis) {
    //Plug_start();
    started = true;
  }
// se amps è fuori dall'intervallo è minore4 di min_amps + hys la ricarica non parte
  else if(started == false && amps <= hysteresis){
    started = false;
    }
  // altrimenti continua a mantenere started true
  else{
    //Plug_stop(); // Esegui evcc.Plug_stop() se non è stato dato il comando "start"
    started = true;
  }  
    
    //Serial.print("EXT_amps: "); Serial.print(EXT_amps);
    //Serial.print("      amps: "); Serial.print(amps);  
    //Serial.print("      EV_amps: "); Serial.println(EV_amps); 
  return started;
}






// RGB LED CONTROL FUNCTIONS

void Evcc::Led_off()
{
    analogWrite(_RL, 0);
    analogWrite(_GL, 0);
    analogWrite(_BL, 0);
}

void Evcc::Led_set_color(int r, int g, int b)
{
    analogWrite(_RL, r);
    analogWrite(_GL, g);
    analogWrite(_BL, b);
}

void Evcc::Led_red()
{
    analogWrite(_RL, 255);
    analogWrite(_GL, 0);
    analogWrite(_BL, 0);
}

void Evcc::Led_green()
{
    analogWrite(_RL, 0);
    analogWrite(_GL, 255);
    analogWrite(_BL, 0);
}

void Evcc::Led_blue()
{
    analogWrite(_RL, 0);
    analogWrite(_GL, 0);
    analogWrite(_BL, 255);
}

void Evcc::Led_white()
{
    analogWrite(_RL, 255);
    analogWrite(_GL, 255);
    analogWrite(_BL, 255);
}

void Evcc::Led_yellow()
{
    analogWrite(_RL, 255);
    analogWrite(_GL, 255);
    analogWrite(_BL, 0);
}

void Evcc::Led_orange()
{
    analogWrite(_RL, 255);
    analogWrite(_GL, 165);
    analogWrite(_BL, 0);
}

void Evcc::Led_grey()
{
    analogWrite(_RL, 127);
    analogWrite(_GL, 127);
    analogWrite(_BL, 127);
}
