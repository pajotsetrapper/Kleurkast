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
byte effect_number = 1;
byte total_effects = 5;
byte mode = UNDEFINED;
byte previous_mode = mode;
bool leds_on = true; //flag to indicate whether LEDS should be on or off
unsigned long command_and_color = 0;
byte red  = 0;
byte green = 0;
byte blue = 0;
byte effectLoopCnt = 0;     // Number of times you got in the effect function
long effectGenericVarA = 0; // Generic global var that can be used by any effect
long effectGenericVarB = 0; // Generic global var that can be used by any effect
long effectGenericVarC = 0; // Generic global var that can be used by any effect

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
    byte control = (command_and_color & 0b11111111000000000000000000000000)>>24;
    if ((control & 0b00000010) == TOGGLE){ //toggle requested
      leds_on = (not leds_on);
      leds_on ? Serial.println("TOGGLE received - leds ON"):Serial.println("TOGGLE received - leds OFF");
    }
    else{
      red   =  (command_and_color & 0b00000000111111110000000000000000)>>16;
      green =  (command_and_color & 0b00000000000000001111111100000000)>>8;
      blue  =  (command_and_color & 0b00000000000000000000000011111111)>>0;
    }
    mode = (control & 0b00000001);
    switch (mode){
      case MANUAL:
        Serial.println("Manual mode");
        previous_mode = MANUAL;
        break;
      case EFFECT:                
        if (previous_mode == EFFECT){
          (effect_number<total_effects) ? (effect_number+=1) : (effect_number=1); //condition ? expression-true : expression-false          
        }
        Serial.print("Effect mode, effect number: ");Serial.println(effect_number);
        previous_mode = EFFECT;
        break;
      case UNDEFINED:
        Serial.println("Undefined mode, should not be here");
        break;
    }
    Serial.print("R: "); Serial.print(red); Serial.print(" G: "); Serial.print(green); Serial.print(" B: "); Serial.println(blue);
    return (true);
  }
  return (false);
}

void effect1(){
  if ((millis() - timestamp) > 125){      
   // effectGenericVarA used to keep track of the index
   if ((effectGenericVarA) <= NBR_LEDS){
    for (int blackpixel = 0; blackpixel < effectGenericVarA; blackpixel++){
      led_strip.setPixelColor(blackpixel, led_strip.Color(0, 0, 0));
    }
    led_strip.setPixelColor(effectGenericVarA,  led_strip.Color(red, 0, 0));
    led_strip.setPixelColor(effectGenericVarA+1,  led_strip.Color(0, green, 0));
    led_strip.setPixelColor(effectGenericVarA+2,  led_strip.Color(0, 0, blue));
    for (int blackpixel = effectGenericVarA+3; blackpixel < NBR_LEDS; blackpixel++){
      led_strip.setPixelColor(blackpixel, led_strip.Color(0, 0, 0));
    }
    led_strip.show();
    timestamp = millis();
    effectGenericVarA +=1;
   }
   else{
    effectGenericVarA = 0;
   }
  }
}


void effect2(){ //// Very slow rainbow
  if ((millis() - timestamp) > 20){    
    if (effectGenericVarA < 5*65536){    
      for(int i=0; i<led_strip.numPixels(); i++) {
        int pixelHue = effectGenericVarA + (i * 65536L / led_strip.numPixels());
        led_strip.setPixelColor(i, led_strip.gamma32(led_strip.ColorHSV(pixelHue)));
      }    
      led_strip.show(); 
      effectGenericVarA += 256;
    } else {
      effectGenericVarA = 0;
    }
    timestamp = millis(); 
  }
}

void effect3(){ //// Slow rainbow
  if ((millis() - timestamp) > 10){    
    if (effectGenericVarA < 5*65536){    
      for(int i=0; i<led_strip.numPixels(); i++) {
        int pixelHue = effectGenericVarA + (i * 65536L / led_strip.numPixels());
        led_strip.setPixelColor(i, led_strip.gamma32(led_strip.ColorHSV(pixelHue)));
      }    
      led_strip.show(); 
      effectGenericVarA += 256;
    } else {
      effectGenericVarA = 0;
    }
    timestamp = millis(); 
  }
}

void effect4(){ // Normal rainbow
  if ((millis() - timestamp) > 5){    
    if (effectGenericVarA < 5*65536){    
      for(int i=0; i<led_strip.numPixels(); i++) {
        int pixelHue = effectGenericVarA + (i * 65536L / led_strip.numPixels());
        led_strip.setPixelColor(i, led_strip.gamma32(led_strip.ColorHSV(pixelHue)));
      }    
      led_strip.show(); 
      effectGenericVarA += 256;
    } else {
      effectGenericVarA = 0;
    }
    timestamp = millis(); 
  }
}

void effect5(){ // Fast rainbow
  if ((millis() - timestamp) > 0){    
    if (effectGenericVarA < 5*65536){    
      for(int i=0; i<led_strip.numPixels(); i++) {
        int pixelHue = effectGenericVarA + (i * 65536L / led_strip.numPixels());
        led_strip.setPixelColor(i, led_strip.gamma32(led_strip.ColorHSV(pixelHue)));
      }    
      led_strip.show(); 
      effectGenericVarA += 256;
    } else {
      effectGenericVarA = 0;
    }
    timestamp = millis(); 
  }
}

  /*
  for(effectGenericVarA; effectGenericVarA < 5*65536; effectGenericVarA += 256) {
    for(int i=0; i<led_strip.numPixels(); i++) {
      int pixelHue = effectGenericVarA + (i * 65536L / led_strip.numPixels());
      led_strip.setPixelColor(i, led_strip.gamma32(led_strip.ColorHSV(pixelHue)));
    }
    led_strip.show();    
  }
  effectGenericVarA = 0;
  */
  
void loop() {
    if (getAndDecodeCommand()){
      if (not leds_on){
        led_strip.fill(led_strip.Color(0, 0, 0), 0, NBR_LEDS);
        led_strip.show();
      }
      else{
        if (mode == MANUAL){
          led_strip.fill(led_strip.Color(red, green, blue), 0, NBR_LEDS);
          led_strip.show();
        }
      }
    }
    if (mode == EFFECT){
      switch (effect_number){
        case 1:
          effect1();
          break;
        case 2:
          effect2();
          break;
        case 3:
          effect3();
          break;
        case 4:
          effect4();
          break;
        case 5:
          effect5();
          break;
      }
    }
}
