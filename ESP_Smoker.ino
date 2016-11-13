/**************************************************************
 * This is my intial test code for cloud controlling my home
 * built trashcan smoker with an ESP8266-12E
 * It uses a servo aimed to control a damper / 1/2"valve, 
 * a relay driven fan to juice up the heat and a thermistor
 * for feedback on smoker temperature.
 * later on I will try to implement PID for automatic temperature
 * controll
 * It should compile on ESP32 as well
 * Blunk Virtual port V0 pulls the temperature
 * Blunk Virtual port V1 controls the servo (0-180 degrees)
 * Blunk GIO16 enable Fan On/Off
 * // Agge
 **************************************************************


 **************************************************************
 * Blynk is a platform with iOS and Android apps to control
 * Arduino, Raspberry Pi and the likes over the Internet.
 * You can easily build graphic interfaces for all your
 * projects by simply dragging and dropping widgets.
 *
 *   Downloads, docs, tutorials: http://www.blynk.cc
 *   Blynk community:            http://community.blynk.cc
 *   Social networks:            http://www.fb.com/blynkapp
 *                               http://twitter.com/blynk_app
 *
 * Blynk library is licensed under MIT license
 * This example code is in public domain.
 *
 **************************************************************
 * You can use this sketch as a debug tool that prints all incoming values
 * sent by a widget connected to a Virtual Pin 1 in the Blynk App.
 *
 * App project setup:
 *   Slider widget (0...100) on V1
 *
 **************************************************************/

 
/*
 * Inputs ADC Value from Thermistor and outputs Temperature in Celsius
 *  requires: include <math.h>
 * Utilizes the Steinhart-Hart Thermistor Equation:
 *    Temperature in Kelvin = 1 / {A + B[ln(R)] + C[ln(R)]3}
 *    where A = 0.001129148, B = 0.000234125 and C = 8.76741E-08
 *
 * These coefficients seem to work fairly universally, which is a bit of a 
 * surprise. 
 *
 * Schematic:
 *   [Ground] -- [10k-pad-resistor] -- | -- [thermistor] --[Vcc (5 or 3.3v)]
 *                                               |
 *                                          Analog Pin 0
 *
 * In case it isn't obvious (as it wasn't to me until I thought about it), the analog ports
 * measure the voltage between 0v -> Vcc which for an Arduino is a nominal 5v, but for (say) 
 * a JeeNode, is a nominal 3.3v.
 *
 * The resistance calculation uses the ratio of the two resistors, so the voltage
 * specified above is really only required for the debugging that is commented out below
 *
 * Resistance = PadResistor * (1024/ADC -1)  
 *
 * I have used this successfully with some CH Pipe Sensors (http://www.atcsemitec.co.uk/pdfdocs/ch.pdf)
 * which be obtained from http://www.rapidonline.co.uk.
 *
 */



#include <Servo.h> 
 
Servo myservo;  // create servo object to control a servo 
                // twelve servo objects can be created on most boards

                
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h> 



// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "2d8e2d1df2af4d399d6axxxxxxxxx";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "xxxxxxx";
char pass[] = "xxxxxx";

#include <math.h>
#define ThermistorPIN 0                 // Analog Pin 0
    float temp;
    float vcc = 1.0;                       // only used for display purposes, if used
                                        // set to the measured Vcc.
    float pad = 9850;                       // balance/pad resistor value, set this to
                                        // the measured resistance of your pad resistor
    float thermr = 10000;                   // thermistor nominal resistance

void setup() 
{ 
  myservo.attach(2);  // attaches the servo on GIO2 to the servo object 
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
} 

BLYNK_READ(V0) // Read the tempsensor on ADC0
{
  // This command writes TempFunc calculated temperature on Virtual pin (0)
  Blynk.virtualWrite(V0, TempFunc());
}


// This function will be called every time Slider Widget
// in Blynk app writes values to the Virtual Pin 1
BLYNK_WRITE(V1)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  // You can also use:
  // String i = param.asStr();
  // double d = param.asDouble();
    myservo.write(pinValue);              // tell servo to go to position in variable 'pos' 
    delay(15);   // waits 15ms for the servo to reach the position
}


void loop() 
{ 
Blynk.run();
}

// ------------- Thermistor functions----------------
float Thermistor(int RawADC) {
  long Resistance;  
  float Temp;  // Dual-Purpose variable to save space.

  Resistance=pad*((1024.0 / RawADC) - 1); 
  Temp = log(Resistance); // Saving the Log(resistance) so not to calculate  it 4 times later
  Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp));
  Temp = Temp - 273.15;  // Convert Kelvin to Celsius                      

  // BEGIN- Remove these lines for the function not to display anything
  //Serial.print("ADC: "); 
  //Serial.print(RawADC); 
  //Serial.print("/1024");                           // Print out RAW ADC Number
  //Serial.print(", vcc: ");
  //Serial.print(vcc,2);
  //Serial.print(", pad: ");
  //Serial.print(pad/1000,3);
  //Serial.print(" Kohms, Volts: "); 
  //Serial.print(((RawADC*vcc)/1024.0),3);   
  //Serial.print(", Resistance: "); 
  //Serial.print(Resistance);
  //Serial.print(" ohms, ");
  // END- Remove these lines for the function not to display anything

  // Uncomment this line for the function to return Fahrenheit instead.
  //temp = (Temp * 9.0)/ 5.0 + 32.0;                  // Convert to Fahrenheit
  return Temp;                                      // Return the Temperature
}


float TempFunc()
{
  float temp;
  temp=Thermistor(analogRead(ThermistorPIN));       // read ADC and  convert it to Celsius
  Serial.print("Celsius: "); 
  Serial.print(temp,1);                             // display Celsius
  //temp = (temp * 9.0)/ 5.0 + 32.0;                  // converts to  Fahrenheit
  //Serial.print(", Fahrenheit: "); 
  //Serial.print(temp,1);                             // display  Fahrenheit
  Serial.println("");                                   
  //delay(5000);                                      // Delay a bit... 
  return temp;
}
