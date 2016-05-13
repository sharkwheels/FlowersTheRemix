#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <elapsedMillis.h>


// Pattern types supported:
enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE };
// Patern directions supported:
enum  direction { FORWARD, REVERSE };
 
// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
    public:
 
    // Member Variables:  
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern
    
    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position
    
    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern
    
    void (*OnComplete)();  // Callback on completion of pattern
    
    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
    :Adafruit_NeoPixel(pixels, pin, type)
    {
        OnComplete = callback;
    }
    
    // Update the pattern
    void Update()
    {
        if((millis() - lastUpdate) > Interval) // time to update
        {
            lastUpdate = millis();
            switch(ActivePattern)
            {
                case RAINBOW_CYCLE:
                    RainbowCycleUpdate();
                    break;
                case THEATER_CHASE:
                    TheaterChaseUpdate();
                    break;
                case COLOR_WIPE:
                    ColorWipeUpdate();
                    break;
                case SCANNER:
                    ScannerUpdate();
                    break;
                case FADE:
                    FadeUpdate();
                    break;
                default:
                    break;
            }
        }
    }
	
    // Increment the Index and reset at the end
    void Increment()
    {
        if (Direction == FORWARD)
        {
           Index++;
           if (Index >= TotalSteps)
            {
                Index = 0;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
        else // Direction == REVERSE
        {
            --Index;
            if (Index <= 0)
            {
                Index = TotalSteps-1;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
    }
    
    // Reverse pattern direction
    void Reverse()
    {
        if (Direction == FORWARD)
        {
            Direction = REVERSE;
            Index = TotalSteps-1;
        }
        else
        {
            Direction = FORWARD;
            Index = 0;
        }
    }
    
    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 255;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
        }
        show();
        Increment();
    }
 
    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = THEATER_CHASE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
   }
    
    // Update the Theater Chase Pattern
    void TheaterChaseUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            if ((i + Index) % 3 == 0)
            {
                setPixelColor(i, Color1);
            }
            else
            {
                setPixelColor(i, Color2);
            }
        }
        show();
        Increment();
    }
 
    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = COLOR_WIPE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Color Wipe Pattern
    void ColorWipeUpdate()
    {
        setPixelColor(Index, Color1);
        show();
        Increment();
    }
    
    // Initialize for a SCANNNER
    void Scanner(uint32_t color1, uint8_t interval)
    {
        ActivePattern = SCANNER;
        Interval = interval;
        TotalSteps = (numPixels() - 1) * 2;
        Color1 = color1;
        Index = 0;
    }
 
    // Update the Scanner Pattern
    void ScannerUpdate()
    { 
        for (int i = 0; i < numPixels(); i++)
        {
            if (i == Index)  // Scan Pixel to the right
            {
                 setPixelColor(i, Color1);
            }
            else if (i == TotalSteps - Index) // Scan Pixel to the left
            {
                 setPixelColor(i, Color1);
            }
            else // Fading tail
            {
                 setPixelColor(i, DimColor(getPixelColor(i)));
            }
        }
        show();
        Increment();
    }
    
    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = FADE;
        Interval = interval;
        TotalSteps = steps;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Fade Pattern
    void FadeUpdate()
    {
        // Calculate linear interpolation between Color1 and Color2
        // Optimise order of operations to minimize truncation error
        uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
        uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
        uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;
        
        ColorSet(Color(red, green, blue));
        show();
        Increment();
    }
   
    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color)
    {
        // Shift R, G and B components one bit to the right
        uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
        return dimColor;
    }
 
    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
        for (int i = 0; i < numPixels(); i++)
        {
            setPixelColor(i, color);
        }
        show();
    }
 
    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
        return (color >> 16) & 0xFF;
    }
 
    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
        return (color >> 8) & 0xFF;
    }
 
    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
        return color & 0xFF;
    }
    
    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
        WheelPos = 255 - WheelPos;
        if(WheelPos < 85)
        {
            return Color(255 - WheelPos * 3, 0, WheelPos * 3);
        }
        else if(WheelPos < 170)
        {
            WheelPos -= 85;
            return Color(0, WheelPos * 3, 255 - WheelPos * 3);
        }
        else
        {
            WheelPos -= 170;
            return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
        }
    }
};

//////////////////////////////////////// INITIALIZE //////////////////////////////////////////////

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define servoMin  190 // this is the 'minimum' pulse length count (out of 4096)
#define servoMax  500 // this is the 'maximum' pulse length count (out of 4096)

int pixelPins[] = {11,10};
int servoPins[] = {15,14};

const size_t n = sizeof(pixelPins) / sizeof(pixelPins[0]);
const size_t o = sizeof(servoPins) / sizeof(servoPins[0]);

/// end of demo display wait
elapsedMillis timeElapsed; 
bool endDemo = false;
unsigned int wait = 5000;


elapsedMillis strip1RestElapsed;
bool strip1RestBool = false;
unsigned int strip1RestInterval = 30000; // 30 seconds

elapsedMillis strip2RestElapsed;
bool strip2RestBool = false;
unsigned int strip2RestInterval = 90000; // 1.5 minutes

elapsedMillis timer0;
boolean timer0Fired;

elapsedMillis timer1;
boolean timer1Fired;

unsigned int interval = 5000;


// NeoPatterns //////////////////////////

NeoPatterns Strip1(6, 0, NEO_GRB + NEO_KHZ800, &strip1Complete);
NeoPatterns Strip2(6, 0, NEO_GRB + NEO_KHZ800, &strip2Complete);

bool strip1Active = false;
bool strip2Active = false;

//// Serial and Python ///////////////

byte incomingByte; // from python
int command = 0; // command (1 = open, 2 = close)
int startServo = 0; // the incoming command shit


void setup()
{
    Serial.begin(9600);
    Serial.flush();

    pwm.begin();
    pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
    yield();

    // close all the servos
    pwm.setPWM(servoPins[0], 0, servoMin);
    pwm.setPWM(servoPins[1], 0, servoMin);

    Strip1.begin();
    Strip2.begin();
    
    // Kick off a pattern
    Strip1.ColorWipe(Strip1.Color(40,0,0), 20); //red
    Strip2.ColorWipe(Strip2.Color(0,40,0), 20); // green


    Serial.println("setup done");

}

void loop()
{
    /// Strip Update and Master Pins //////////////////////
    Strip1.setPin(pixelPins[0]);
    Strip2.setPin(pixelPins[1]);

    Strip1.Update();
    Strip2.Update();

    if (Serial.available() > 0) 
    {
        //incomingByte = Serial.parseInt(); // use if testing from arduino input
        incomingByte = Serial.read(); // use if live
        command = incomingByte;
        startServo = ServoGo(command);
        timeElapsed = 0; 
        
    }

    /// LOGIC ///////////////////////////////

    if (startServo == 11){
        strip1Active = true;
        openFlowers(pixelPins[0]);  // open pixelpin 11 and call servo 10

    } 
    else if(startServo == 12) 
    {
        strip1Active = false;
        closeFlowers(pixelPins[0]);
        
    }

    if (startServo == 21){
        strip2Active = true;
        openFlowers(pixelPins[1]);
    } 
    else if(startServo == 22) 
    {
        strip2Active = false;
        closeFlowers(pixelPins[1]);
        
    }

    if (startServo == 70){
        endDemo = true;
        openFlowers(pixelPins[0]); 
        openFlowers(pixelPins[1]); 

        if (timeElapsed >= wait) 
        {   
            endDemo = false; // this will reset shit
            resetDemo();
        }
    }

    
}

int ServoGo(int com)
{
    Serial.println("!inServoGo");
    Serial.println(com);
    return com;
}

void shuffle()
{
    // this shuffles the starting point of the pixel pins ////////

    Serial.print("Shuffling pixel pins");
    for (size_t i = 0; i < n - 1; i++)
    {
        size_t j = random(1, n - i);
        //size_t j = i + rand() / (RAND_MAX / (n - i) + 1); // ?? look up how this actually works
        int t = pixelPins[i];
        pixelPins[i] = pixelPins[j];
        pixelPins[j] = t;  
    }

    Serial.println("------------pixelPins--------------");
    for(size_t y = 0; y < n; y++) 
    {
        Serial.println(pixelPins[y]);
    }

}

void resetDemo(){

    // close all the servos
    pwm.setPWM(servoPins[0], 0, servoMin);
    pwm.setPWM(servoPins[1], 0, servoMin);
    
    // flush the serial port
    Serial.flush();

    //clear all the booleans
    startServo = 0;
    timeElapsed = 0;

    // shuffle the pins
    shuffle();

}

///// SERVO CONTROL //////////////////////////////////

void openFlowers(int p){
    if(p == 11) {
        pwm.setPWM(servoPins[0], 0, servoMax); // grab servo 15
    } 
    else if(p == 10)
    {
        pwm.setPWM(servoPins[1], 0, servoMax); // grab servo 14
    }

}

void closeFlowers(int p){
    if(p == 11)
    {
        pwm.setPWM(servoPins[0], 0, servoMin); // grab servo 15
    }
    else if(p == 10){
        pwm.setPWM(servoPins[1], 0, servoMin); // grab servo 14
    }
}



///// NEOPATTERNS CALLBACK //////////////////////////////////

void strip1Complete() {
    
    if (!endDemo)
    {
        if (strip1Active)
        {           
            Strip1.ActivePattern = THEATER_CHASE;
            Strip1.Interval = 50;
            Strip1.Color1 = Strip1.Color(40,0,0); // red      
            Strip1.Color2 = Strip1.Color(40,0,40); // purple
            
            strip1RestBool = false;
            strip1RestElapsed = 0; 
            
        } else if(!strip1Active)
        {   
            
            if (!strip1RestBool)
            {
                Strip1.ColorWipe(Strip1.Color(40,0,0), 50); //red 
            }else
            {
                Strip1.Scanner(Strip1.Color(40,0,0), 50);
            }

            if (strip1RestElapsed >= strip1RestInterval) {
                // if 30 seconds has passed...
                //Serial.println("30 seconds has passed.");
                strip1RestBool = true; // flip the strip to scanner
                timer0Fired = true;
                strip1RestElapsed = 0;
                timer0 = 0;
            }

            if(timer0Fired && (timer0>=interval))
            {
                //Serial.println("timer start fired");
                strip1RestBool = false; 
                
            }

            
        }

    } else {
        Strip1.ActivePattern = RAINBOW_CYCLE;
        Strip1.TotalSteps = 255;
        Strip1.Interval = min(10, Strip1.Interval);
    }
    
}

void strip2Complete(){

    if(!endDemo){
        if (strip2Active){
            Strip2.ActivePattern = THEATER_CHASE;
            Strip2.Interval = 50;
            Strip2.Color1 = Strip2.Color(0,40,0);  //green  
            Strip2.Color2 = Strip2.Color(0,0,40); // blue

            strip2RestBool = false;
            strip2RestElapsed = 0;
            
        }
        else if(!strip2Active)
        {   
            if(!strip2RestBool){
              Strip2.ColorWipe(Strip2.Color(0,40,0), 50); // green  
            } 
            else 
            {
                Strip2.Scanner(Strip1.Color(0,40,0), 50);
            }

            if (strip2RestElapsed >= strip2RestInterval) {
                // if 30 seconds has passed...
                Serial.println("60 seconds has passed.");
                strip2RestBool = true; // flip the strip to scanner
                timer1Fired = true;
                strip2RestElapsed = 0;
                timer1 = 0;
            }

            if(timer1Fired && (timer1>=interval))
            {
                //Serial.println("timer start fired");
                strip2RestBool = false; 
                
            }
            
        }
        
    } else {

        Strip2.ActivePattern = RAINBOW_CYCLE;
        Strip2.TotalSteps = 255;
        Strip2.Interval = min(10, Strip2.Interval);
    
        
    }
    
}
