#include <Adafruit_NeoPixel.h>
//#include <IRremote.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#define led_strip_pin   2
#define AANTAL_LEDS 8

//int IR_ONTVANGER_PIN = 11;
//IRrecv IRontvanger(IR_ONTVANGER_PIN);
//decode_results IRResultaten;
RF24 radio(7, 8);  // CE, CSN
//11 (MOSI), 12(MISO) and 13(SCK) would be taken as well by the built-in NRF module

const byte address[6] = "00001";
Adafruit_NeoPixel led_strip(AANTAL_LEDS, led_strip_pin, NEO_GRB + NEO_KHZ800);

void setup() {
    //pinMode(LED_BUILTIN, OUTPUT);    
    Serial.begin(115200);
    led_strip.begin();
    //IRontvanger.enableIRIn();  // Start de IR ontvanger
    //IRontvanger.blink13(true); // Feedback LED activeren
    while (!radio.begin()){
      delay(100);
      Serial.println("Radio niet geinitialiseerd");
    }
    Serial.println("Radio OK");
    radio.setChannel(90); //2490 Mhz, buiten Wifi & medisch device spectrum
    radio.setPALevel(RF24_PA_MAX);  //Max bereik
    radio.setDataRate(RF24_1MBPS);  //Datarate = 1Mbps
    radio.openReadingPipe(0, address);; //Adres instellen
    radio.startListening(); //Zet als ontvanger
}

void loop() {
  unsigned long kleur = 0;
  if (radio.available())
  {
    Serial.println("Data ontvangen");    
    radio.read(&kleur, sizeof(kleur));
    Serial.println(kleur);
    byte rood  = (kleur & 0b111111110000000000000000)>>16;
    byte groen = (kleur & 0b000000001111111100000000)>>8;
    byte blauw = (kleur & 0b000000000000000011111111)>>0;
    Serial.print("R: "); Serial.print(rood); Serial.print(" G: "); Serial.print(groen); Serial.print(" B: "); Serial.println(blauw);
    led_strip.fill(led_strip.Color(rood, groen, blauw), 0, AANTAL_LEDS);
    led_strip.show();
  }  
}
