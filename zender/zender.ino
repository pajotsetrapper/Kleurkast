#include <IRremote.h>

#define r_weerstand_pin A0
#define g_weerstand_pin A1
#define b_weerstand_pin A2

IRsend irsend;
uint32_t vorige_kleur = 0;

void setup() {  
  Serial.begin(115200);
}
void loop() {  
  uint32_t kleur = 0;    
  int r_spanning = analogRead(r_weerstand_pin);
  byte rood = map(r_spanning, 0, 1023, 0, 31); //range omzetten 0-1023 naar 0-31 (5bit, wegens 3 keuren als unsigned int (16b) versturen)
  int g_spanning = analogRead(g_weerstand_pin);
  byte groen = map(g_spanning, 0, 1023, 0, 31);
  int b_spanning = analogRead(b_weerstand_pin);
  byte blauw = map(b_spanning, 0, 1023, 0, 31);
  kleur = (rood<<10) + (groen<<5) + (blauw); //3 kleuren in 1 unsigned int encoden (rood en groen x bits naar links shiften)    
  if (kleur != vorige_kleur){ //Only send when value changes, otherwise batteries run out quickly
    Serial.print("Kleur-code via IR doorsturen: "); Serial.println(kleur, HEX);
	  irsend.sendRC5(kleur, 32);
    vorige_kleur = kleur;
  }
  delay(100);
}
