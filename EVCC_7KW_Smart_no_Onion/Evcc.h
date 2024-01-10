#ifndef Evcc_h
#define Evcc_h

#include "Arduino.h"
#include "EmonLib.h"

class Evcc
{
public:
    Evcc(
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
        );

    void Plug_start();
    void Plug_stop();
    int PPread();
    int CPread();
    void Timer_init();
    void CPout_on();
    void CPout_off();

    void AC_on();
    void AC_off();
    bool Smart_reg(int max_amps, int min_amps, bool line_splitted, int hys);

    void Led_set_color(int r, int g, int b);
    void Led_off();
    void Led_green();
    void Led_blue();
    void Led_red();
    void Led_white();
    void Led_yellow();
    void Led_orange();
    void Led_grey();
    void UpdateDuty(int newValue);

private:
    int _ID;
    int _PP;
    int _CP;
    int _CP_F;
    int _samples;
    int _serial_duty;
    String _chrg_state;
    int _RL;
    int _GL;
    int _BL;
    int _REL1;
    int _REL2;
    int _REL3;
    int _CT1;
    int _CT_EXT;
    EnergyMonitor _IL1, _IL1_EXT;
};
#endif
