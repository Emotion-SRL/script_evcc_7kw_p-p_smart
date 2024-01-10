
#include "Evcc.h"

// Define Digital Pin
#define CP 11
#define RL 45
#define GL 46
#define BL 44
#define REL 30


// Define Analog Pin
#define PPreadA A2
#define CPreadA A0
#define CT1 A3
#define CT_EXT A4


bool isPluggedIn = true;
bool reg_toggle = true;
bool line_splitted = true;

int max_amps = 21; //corrente massima della linea in Ampere
int min_amps = 5; //corrente minima
int hysteresis = 3;

unsigned long loop_interval = 50; //intervallo di ripetizioni in ms del loop principale (funzionalità base della evcc)
unsigned long reg_interval = 30000; //intervallo di ripetizioni in ms dell'aggiornamento di corrente dal parte della funzione Smart_reg
unsigned long previousMillis0 = 0; //ms
unsigned long previousMillis1 = 0; //ms  


int samples = 500;

Evcc evcc(1, PPreadA, CP, CPreadA, samples, RL, GL, BL, REL, CT1, CT_EXT);


void setup() {

  //Serial.begin(9600);
  evcc.UpdateDuty(max_amps);
  evcc.Timer_init();
  evcc.AC_off(); // Utilizzo del metodo off() della classe Plug
  delay(1000);
}

void loop() {

unsigned long currentMillis = millis();  // Ottieni l'attuale valore di millis()

if (currentMillis - previousMillis0 >= loop_interval) {
  previousMillis0 = currentMillis; 
// Esegui evcc.Plug_start() se è stato dato il comando "start"
  if (reg_toggle) {
    evcc.Plug_start();
  } else {
    evcc.Plug_stop(); // Esegui evcc.Plug_stop() se non è stato dato il comando "start"
  } 
}

if (currentMillis - previousMillis1 >= reg_interval) {
  previousMillis1 = currentMillis; 
  reg_toggle = evcc.Smart_reg(max_amps, min_amps, line_splitted, hysteresis);
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      
}
