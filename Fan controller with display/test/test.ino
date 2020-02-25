#include <PWM.h>
#define debounce 20 // ms debounce period to prevent flickering when pressing or releasing the button
#define holdTime 2000 // ms hold period: how long to wait for press+hold event

volatile unsigned long half_revolutions = 0;
unsigned int rpm = 0;

int outPWM = 10;
int inSensor = 3;

//BUTTON VARS
int inButton = 12;
long btnDnTime; // time the button was pressed down
long btnUpTime; // time the button was released
boolean ignoreUp = false; // whether to ignore the button release because the click+hold was triggered

const long interval = 3000; 
unsigned long previousMillis = 0;
int speedArr[4] = {50,80,100,120};
int index = 0;

int buttonVal;
int buttonValOld = HIGH;
bool testModeOn = false;
bool manualModeOn = false;

//////////////////////////////////////////////////////////////////
//                            SETUP
//////////////////////////////////////////////////////////////////
 void setup()
 {
   Serial.begin(9600);
   SetPinFrequencySafe(outPWM, 25000); //set frequency to 25kHz
   pinMode(inSensor,INPUT);
   pinMode(outPWM,OUTPUT);
   pinMode(inButton,INPUT_PULLUP);
   attachInterrupt(digitalPinToInterrupt(inSensor), rpm_fun, RISING);
   pwmWrite(outPWM, speedArr[index]);
 }

//////////////////////////////////////////////////////////////////
//                            MANUAL MODE
//////////////////////////////////////////////////////////////////
 void manualMode()
 {
   Serial.print("manual mode");
 }
 
//////////////////////////////////////////////////////////////////
//                            AUTOMATIC MODE
//////////////////////////////////////////////////////////////////
 void automaticMode()
 {
   Serial.print("automatic mode");
 }
 
//////////////////////////////////////////////////////////////////
//                            TEST MODE
//////////////////////////////////////////////////////////////////
 void testMode()
 {
    Serial.print("test mode");
 }

//////////////////////////////////////////////////////////////////
//                           READ RPM
//////////////////////////////////////////////////////////////////
void readRPM()
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) 
  {
    detachInterrupt(digitalPinToInterrupt(inSensor));
    
    previousMillis = currentMillis;
    rpm = half_revolutions *30;
    half_revolutions = 0;
    int percent = 100 * speedArr[index]/255;
    
    Serial.print(percent);
    Serial.print("%, ");
    Serial.print(rpm);
    Serial.println(" rpm");
    
    attachInterrupt(digitalPinToInterrupt(inSensor), rpm_fun, RISING);
  }
}
//////////////////////////////////////////////////////////////////
//                            LOOP
//////////////////////////////////////////////////////////////////
 void loop()
{
    // Read the state of the button
    buttonVal = digitalRead(inButton);
    
    // Test for button pressed and store the down time
    if (buttonVal == LOW && buttonValOld == HIGH && (millis() - btnUpTime) > long(debounce))
        btnDnTime = millis();
    
    // Test for button release and store the up time
    if (buttonVal == HIGH && buttonValOld == LOW && (millis() - btnDnTime) > long(debounce))
    {
        if (ignoreUp == false) 
          {
              if(manualModeOn == false)
                  manualModeOn = true;
              else
                  manualModeOn = false;
          }
        else
        {
          // TO DO TEST BUTTONS
        }
        btnUpTime = millis();
    }
    
    // Test for button held down for longer than the hold time
    if (buttonVal == LOW && (millis() - btnDnTime) > long(holdTime))
    {
      if(testModeOn == false)
      {
        testModeOn = true;
        ignoreUp = true;
      }
          
      else
      {
        testModeOn = false;
        ignoreUp = false;
      }
          
      btnDnTime = millis();
    }
    buttonValOld = buttonVal;

    if(testModeOn == true)
      testMode();
    else if(manualModeOn == true)
      manualMode();
    else if(manualModeOn == false)
      automaticMode();
      
    readRPM();
 }

 void rpm_fun(){half_revolutions++;}
