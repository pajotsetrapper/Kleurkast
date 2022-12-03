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

void rainbowEffect(int delay){
  if ((millis() - timestamp) > delay){    
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

void fadingColorEffect(){
  /* The variables byte red, green and blue will be used to tune effect (as such controllable via potentiometers on remote)
      - red: value (brightness)
      - green: saturation
      - blue: speed
  */
  byte saturation = green;
  byte brightness = red;
  byte speed = 255-blue;
  if ((millis() - timestamp) > speed){
    if (effectGenericVarA < 65536){      
      led_strip.fill(led_strip.gamma32(led_strip.ColorHSV(effectGenericVarA, saturation, brightness)));
      led_strip.show();
      effectGenericVarA += 256;
    }
    else {
      effectGenericVarA = 0;
    }
    timestamp = millis();
  }
}

void stroboEffect(int delay){
  byte rgbcomponent = 0;
  if ((millis() - timestamp) > delay){
    effectGenericVarA==0 ? rgbcomponent=0 : rgbcomponent=200;
    effectGenericVarA+=1;   
    led_strip.fill(led_strip.Color(rgbcomponent, rgbcomponent, rgbcomponent));
    led_strip.show();    
    timestamp = millis();
    if (effectGenericVarA > 1){
      effectGenericVarA = 0;
    }
  }
}

void effect1(){
  if ((millis() - timestamp) > 100){    
    int rpos=0; int gpos=0; int bpos=0;
    led_strip.fill(led_strip.Color(0, 0, 0)); //Clear all    
    ((effectGenericVarA)<led_strip.numPixels()) ? rpos=effectGenericVarA : rpos=((effectGenericVarA)%led_strip.numPixels());
    ((effectGenericVarA+1)<led_strip.numPixels()) ? gpos=effectGenericVarA+1 : gpos=((effectGenericVarA+1)%led_strip.numPixels());
    ((effectGenericVarA+2)<led_strip.numPixels()) ? bpos=effectGenericVarA+2 : bpos=((effectGenericVarA+2)%led_strip.numPixels());
    led_strip.setPixelColor(rpos, led_strip.Color(red, 0, 0));
    led_strip.setPixelColor(gpos, led_strip.Color(0, green, 0));
    led_strip.setPixelColor(bpos, led_strip.Color(0, 0, blue));
    (effectGenericVarA+1 < led_strip.numPixels()) ? effectGenericVarA+=1 : effectGenericVarA=0; //Start index
    led_strip.show();
    timestamp = millis();
  }
}
  
void loop() {
    if (getAndDecodeCommand()){
      if (not leds_on){
        led_strip.fill(led_strip.Color(0, 0, 0), 0, led_strip.numPixels());
        led_strip.show();
      }
      else{
        if (mode == MANUAL){
          led_strip.fill(led_strip.Color(red, green, blue), 0, led_strip.numPixels());
          led_strip.show();
        }
      }
    }
    if (mode == EFFECT){
      switch (effect_number){
        case 1:
          fadingColorEffect();          
          break;
        case 2:
          rainbowEffect(50); //Very slow
          break;
        case 3:
          rainbowEffect(25); //Slow
          break;
        case 4:
          rainbowEffect(5); //Normal
          break;
        case 5:
          effect1();
          break;
      }
    }
}
