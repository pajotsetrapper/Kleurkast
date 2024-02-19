/*
Remote control for the "Kleurkast"
A project made to add some cool-factor to a kid's wardrobe.
This allows to control the color / effects on a WS2812b LED strip remotely.
Communication interface used is an NRF24L01+.

Pinout  
  - A0: Potentiometer ROOD  (20k)
  - A1: Potentiometer GROEN (20k)
  - A2: Potentiometer BLAUW (20k)
  - A3:
  - A4:
  - A5:
  - A6:
  - A7:
  - D2 (IRQ) - Button: Power
  - D3 (IRQ) - Button: Manual
  - D4 (IRQ) - Button: Effect
  - D5
  - D6
  - D7
  - D8
  - D9:  CSN
  - D10: CE 
  - D11: MOSI
  - D12: MISO
  - D13: SCK

Communication from this remote to receiver is done using 1 unsigned long (4bytes).
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


- Note: initially, control register is set to FF. In case receiver gets this, no action should be taken

*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "Button2.h";

#define R_RESISTOR_PIN  A0
#define G_RESISTOR_PIN  A1
#define B_RESISTOR_PIN  A2
#define BTN_MODE_MANUAL 2
#define BTN_MODE_EFFECT 3
#define BTN_ONOFF       4

#define MANUAL 0
#define EFFECT 1
#define TOGGLE 2
#define UNDEFINED 255

RF24 radio(10, 9);  // CE, CSN
const byte address[6] = "00001";
unsigned long previous_color_and_controls = 0;
byte previous_mode = UNDEFINED;
byte mode  = UNDEFINED;
bool effect_button_pressed = false;
bool manual_button_pressed = false;
Button2 btn_mode_manual = Button2(BTN_MODE_MANUAL);
Button2 btn_mode_effect  = Button2(BTN_MODE_EFFECT);
Button2 btn_onoff   = Button2(BTN_ONOFF);

void setup() {  
  //Serial.begin(115200);
  while (!radio.begin()){    
    delay(100);
    //Serial.println("Failed to initialize radio");
  }
  //Serial.println("Radio OK");
  radio.setChannel(90); //2490 Mhz, buiten Wifi & medisch device spectrum
  radio.setPALevel(RF24_PA_MAX);  //Max bereik  
  radio.setDataRate(RF24_1MBPS);  //Datarate = 250Kbps
  radio.openWritingPipe(address); //Adres instellen
  radio.stopListening(); //Zet als zender
  btn_mode_manual.setTapHandler(btn_mode_manualClick);
  btn_mode_effect.setTapHandler(btn_mode_effectClick);
  btn_onoff.setTapHandler(btn_onoffClick);
}

void loop() {  
  unsigned long color_and_controls = 0;
  btn_mode_manual.loop();
  btn_mode_effect.loop();
  btn_onoff.loop();
  color_and_controls = getColorAndControls();
  switch (mode){
    case UNDEFINED:
      break;
    case MANUAL:
      if ((color_and_controls != previous_color_and_controls) or manual_button_pressed) {
        previous_color_and_controls = color_and_controls;
        radio.write(&color_and_controls, sizeof(color_and_controls));
        manual_button_pressed = false;     
      }
      break;
    case EFFECT:
      if (effect_button_pressed){
        previous_color_and_controls = color_and_controls;
        radio.write(&color_and_controls, sizeof(color_and_controls));
        effect_button_pressed = false;
      }
      break;
  }
  delay(5);
}

unsigned long getColorAndControls(void){
  unsigned long color_and_controls = 0;
  unsigned long controls = 0;

  controls += mode;

  int r_voltage = analogRead(R_RESISTOR_PIN);
  unsigned long red = 255 - map(r_voltage, 0, 1023, 0, 255); //Adjust range from 0-1023 to 0-255 (and inversed as I inverted +/- on potentiometers)  

  int g_voltage = analogRead(G_RESISTOR_PIN);
  unsigned long green = 255 - map(g_voltage, 0, 1023, 0, 255);
  
  int b_voltage = analogRead(B_RESISTOR_PIN);
  unsigned long blue = 255 - map(b_voltage, 0, 1023, 0, 255);
  
  color_and_controls = (controls<<24) + (red<<16) + (green<<8) + (blue); //Encode controls (most significant byte) + color components (r,g,b) to the 3 least significant bytes  
  return (color_and_controls);
}

void btn_mode_manualClick(Button2 &btn){
  //Serial.println("Button manual mode pressed");
  mode = MANUAL;
  manual_button_pressed = true; //Will force a command to be sent to the receiver
}
void btn_mode_effectClick(Button2 &btn){
  //Serial.println("Button effect mode pressed");
  mode = EFFECT;
  effect_button_pressed = true; //Will force a command to be sent to the receiver
}
void btn_onoffClick(Button2 &btn){
  //Send 'toggle' command to receiver, up to receiver to decide wether it should swith on/off.
  unsigned long toggle_cmd = TOGGLE;
  unsigned long color_and_controls = 0;
  color_and_controls += (toggle_cmd<<24);  
  radio.write(&color_and_controls, sizeof(color_and_controls));
  //Serial.print("Button toggle on/off pressed ");Serial.println(color_and_controls, HEX);
}
