//#include <IRremote.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define r_weerstand_pin A0
#define g_weerstand_pin A1
#define b_weerstand_pin A2

//IRsend irsend;
RF24 radio(7, 8);  // CE, CSN
const byte address[6] = "00001";
unsigned long vorige_kleur = 0;

void setup() {  
  //Serial.begin(115200);
  while (!radio.begin()){    
    delay(100);
    //Serial.println("Radio niet geinitialiseerd");
  }
  //Serial.println("Radio OK");
  radio.setChannel(90); //2490 Mhz, buiten Wifi & medisch device spectrum
  radio.setPALevel(RF24_PA_MAX);  //Max bereik
  radio.setDataRate(RF24_1MBPS);  //Datarate = 1Mbps
  radio.openWritingPipe(address); //Adres instellen
  radio.stopListening(); //Zet als zender
}

void loop() {  
  unsigned long kleur = 0;
  int r_spanning = analogRead(r_weerstand_pin);
  unsigned long rood = map(r_spanning, 0, 1023, 0, 255); //range omzetten 0-1023 naar 0-255
  
  int g_spanning = analogRead(g_weerstand_pin);
  unsigned long groen = map(g_spanning, 0, 1023, 0, 255);
  
  int b_spanning = analogRead(b_weerstand_pin);
  unsigned long blauw = map(b_spanning, 0, 1023, 0, 255);
  
  kleur = (rood<<16) + (groen<<8) + (blauw); //3 kleuren in 1 unsigned long encoden (rood en groen x bits naar links shiften)    
  if (kleur != vorige_kleur){ //Only send when value changes, otherwise batteries run out quickly
    //Serial.print("R: "); //Serial.println(rood, HEX);
    //Serial.print("G: "); //Serial.println(groen, HEX);
    //Serial.print("B: "); //Serial.println(blauw, HEX);
    //Serial.print("Kleur-code doorsturen: "); //Serial.println(kleur, HEX);	  
    radio.write(&kleur, sizeof(kleur));
    vorige_kleur = kleur;
  }
  delay(5);
}
