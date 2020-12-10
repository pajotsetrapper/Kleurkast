#include <Adafruit_NeoPixel.h>
#include <IRremote.h>

#define led_strip_pin   2
#define led_strip_aantal_leds 8

int IR_ONTVANGER_PIN = 11;

IRrecv IRontvanger(IR_ONTVANGER_PIN);
//Adafruit_NeoPixel led_strip(led_strip_aantal_leds, led_strip_pin, NEO_GRB + NEO_KHZ800);
decode_results IRResultaten;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);    
    Serial.begin(115200);
    //led_strip.begin();
    IRontvanger.enableIRIn();  // Start de IR ontvanger
    IRontvanger.blink13(true); // Feedback LED activeren
}

void loop() {
    if (IRontvanger.decode(&IRResultaten)) {
        Serial.println();
        IRontvanger.printIRResultRaw(&Serial);
        Serial.println(IRResultaten.value, HEX);
        byte rood =  map((IRResultaten.value & 0b0111110000000000)>>10, 0, 31, 0, 255); //kleurwaarde parsen & herschalen naar 0-255)
        byte groen = map((IRResultaten.value &   0b0000001111100000)>>5, 0, 31, 0, 255);
        byte blauw = map((IRResultaten.value &   0b0000000000011111)>>0, 0, 31, 0, 255);        
        Serial.print("R: "); Serial.print(rood); Serial.print(" G: "); Serial.print(groen); Serial.print(" B: "); Serial.println(blauw);
        /*
        for(int i=0; i<led_strip_aantal_leds; i++) {
            led_strip.setPixelColor(i, led_strip.Color(rood, groen, blauw));
        }
        led_strip.show();
        */
        IRontvanger.resume(); // Receive the next value
    }
}
