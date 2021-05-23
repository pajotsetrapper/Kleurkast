/*
LED unit for the "command_and_colorkast"
A project made to add some cool-factor to a kid's wardrobe.
This allows to control the color / effects on a WS2812b LED strip remotely.
Communication interface used is an NRF24L01+.
Communication between remote to receiver is done using 1 unsigned long (4bytes).
3 bytes are used to specify the values for the red / green / blue components (1 byte each).
1 byte is is used for control characters

Protocol:
Control |Red     |Green   |Blue
CCCCCCCC|RRRRRRRR|GGGGGGGG|BBBBBBBB
||||||||_ Mode: 0: Manual / 1: Effect (Receiver to set color received if manual & to choose next effect if set)
|||||||__ On/Off toggle - 1: receiver to toggle on/off
||||||___ Future
|||||____ Future
||||_____ Future
|||______ Future
||_______ Future
|________ Future
 */

#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define LED_STRIP_PIN   2
#define NBR_LEDS 8
#define MANUAL 0
#define EFFECT 1
#define TOGGLE 2
#define UNDEFINED 255

RF24 radio(10, 9);  // CE, CSN
const byte address[6] = "00001";
Adafruit_NeoPixel led_strip(NBR_LEDS, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);
byte effect_number = 0;
byte mode = UNDEFINED;
bool leds_on = true; //flag to indicate whether LEDS should be on or off
unsigned long command_and_color = 0;
byte red  = 0;
byte green = 0;
byte blue = 0;
byte effectLoopCnt = 0;        // Number of times you got in the effect function
byte effectGenericCounterA = 0; // Generic couter that can be used by any effect
byte effectGenericCounterB = 0; // Generic couter that can be used by any effect



unsigned long timestamp = 0;

void setup() {
    Serial.begin(115200);
    led_strip.begin();
    while (!radio.begin()){
      delay(100);
    }
    radio.setChannel(90); //2490 Mhz, outside Wifi & medical device spectrum
    radio.setPALevel(RF24_PA_MAX);  //Max range    
    radio.setDataRate(RF24_250KBPS);  //Datarate = 250Kbps
    radio.openReadingPipe(0, address);; //Set address
    radio.startListening(); //Set as receiver
}

bool getAndDecodeCommand(void){
  //returns true if a command was received from remote, false otherwise
  if (radio.available()){
    radio.read(&command_and_color, sizeof(command_and_color));
    Serial.println(command_and_color, HEX);
    byte control = (command_and_color & 0b11111111000000000000000000000000)>>24;
    if ((control & 0b00000010) == TOGGLE){ //toggle requested
      leds_on = (not leds_on);    
    }
    else{
      red   =  (command_and_color & 0b00000000111111110000000000000000)>>16;
      green =  (command_and_color & 0b00000000000000001111111100000000)>>8;
      blue  =  (command_and_color & 0b00000000000000000000000011111111)>>0;
    }
    mode = (control & 0b00000001);
    
    Serial.print("R: "); Serial.print(red); Serial.print(" G: "); Serial.print(green); Serial.print(" B: "); Serial.println(blue);
    if (mode == MANUAL){
      Serial.println("Manual mode");
    }
    if (mode == EFFECT){
      effectLoopCnt = 0; //Reset the loop counter used in the effects.
      effectGenericCounterA = 0;
      effectGenericCounterB = 0;
      Serial.println("Effect mode");
      
    }
    if (leds_on){
      Serial.println("Leds ON");
    }
    else{
      Serial.println("Leds OFF");
    }
    
    return (true);
  }
  return (false);
}

void loop() {
    if (getAndDecodeCommand()){
      if (leds_on == false){        
        led_strip.fill(led_strip.Color(0, 0, 0), 0, NBR_LEDS);
        led_strip.show();
      }
      else{
        if (mode == MANUAL){
          //Send command to set the color)
          led_strip.fill(led_strip.Color(red, green, blue), 0, NBR_LEDS);
          led_strip.show();
        }       
      }
    }
    if (mode == EFFECT){
      if ((millis() - timestamp) > 125){
        //led_strip.fill(led_strip.Color(random(0,90), random(0,90), random(0,90)), 0, NBR_LEDS);
        // effectGenericCounterA used to keep track of the index
         if ((effectGenericCounterA) <= NBR_LEDS){
          for (int blackpixel = 0; blackpixel < effectGenericCounterA; blackpixel++){
            led_strip.setPixelColor(blackpixel, led_strip.Color(0, 0, 0));
          }
          led_strip.setPixelColor(effectGenericCounterA,  led_strip.Color(red, 0, 0));
          led_strip.setPixelColor(effectGenericCounterA+1,  led_strip.Color(0, green, 0));
          led_strip.setPixelColor(effectGenericCounterA+2,  led_strip.Color(0, 0, blue));
          for (int blackpixel = effectGenericCounterA+3; blackpixel < NBR_LEDS; blackpixel++){
            led_strip.setPixelColor(blackpixel, led_strip.Color(0, 0, 0));
          }
          led_strip.show();
          timestamp = millis();
          effectGenericCounterA +=1;
         }
         else{
          effectGenericCounterA = 0;
         }         
      }
    }
}
