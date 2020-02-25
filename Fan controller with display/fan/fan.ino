#include <LiquidCrystal.h>
#include <DallasTemperature.h>
#include <OneWire.h>
//pins 12, 11, 10,  8,  7 - 3,  A0
//     PB4,PB3,PB2, PB0,PD7-PD3,PC0

// Inputs
const int inTemp = 11;
const int inButton = 12;
const int inPot = A0;

// Outputs
const int outPWM = 10;

// Fan
int pwmVal = 0;

// For button
int buttonVal = 0;
int buttonValOld = HIGH;

// For time calculations
unsigned long currentMillis;
unsigned long currentMillis2;
unsigned long previousMillis = 0;
unsigned long previousMillis2 = 0;
unsigned long interval = 1000;
unsigned long interval2 = 5000;

bool fanMode = 0;   // fan mode: 0-automatic 1-manual
bool testMode = 0;  //test mode: 0-off 1-on
bool tgl = 0;

LiquidCrystal lcd(7,8,3,4,5,6); //LiquidCrystal lcd(RS,E,D4,D5,D6,D7);
OneWire oneWire(inTemp);
DallasTemperature tempSens(&oneWire);

void refresh()
{
    // Read temperature and calculate PWM
    tempSens.requestTemperatures();
    float tmp = tempSens.getTempCByIndex(0);

    // For automatic fanMode
    if(fanMode == 0)
    {
      pwmVal = (int)tmp;
      if      (pwmVal < 20)  pwmVal = 20;
      else if (pwmVal > 30)  pwmVal = 30;
      
      pwmVal = map(pwmVal, 20, 30, 0, 255);      
    }
    else
    {
      pwmVal = analogRead(inPot);
      pwmVal = map(pwmVal, 0, 1023, 0, 255);  
    }

    int perc = 100 * pwmVal/255;
    if(perc < 20) perc = 20;

    // Write to LCD
    String fanModeStr;
    if (fanMode == 0) fanModeStr = "AUT";
    else fanModeStr = "MAN";
    String firstRow = "Temp.: " + String(tmp,1) + "\337C";
    String secondRow = "Fan: " + String(perc,DEC) + "% ";
    lcd.clear();
    lcd.print(firstRow);
    lcd.setCursor(0,1);
    lcd.print(secondRow);
    lcd.setCursor(11,1);
    lcd.print("M:" + fanModeStr);
}

 void setup()
 {
   pinMode(outPWM, OUTPUT);
   pinMode(inButton,INPUT_PULLUP);
   pinMode(inPot, INPUT);
    
   tempSens.begin();
   lcd.begin(16,2); //col,row
   lcd.clear();
 }

 void testing()
 {
    lcd.clear();
    if(tgl)
    {
      lcd.print("0123456789ABCDEF");
      lcd.setCursor(0,1);
      lcd.print("0123456789ABCDEF");
      analogWrite(outPWM, 0);
    }
    else
    {
      lcd.print("XXXXXXXXXXXXXXXX");
      lcd.setCursor(0,1);
      lcd.print("XXXXXXXXXXXXXXXX");
      analogWrite(outPWM, 150);
    }
 }
 void loop()
 {
    // check if time passed
  currentMillis = millis();
  if(currentMillis - previousMillis > interval) 
  {
    if(!testMode)
        refresh();
    else
      testing();
      
    previousMillis = currentMillis;
    tgl = !tgl;
  }

  //read button
   buttonVal = digitalRead(inButton);
    if(buttonVal == LOW && buttonValOld == HIGH)  
        fanMode = !fanMode; // toggle bool

  // check if button is held
    if(buttonVal == LOW)
    {
      currentMillis2 = millis();
      if(currentMillis2 - previousMillis2 > interval2)
      {
        testMode = !testMode;
        previousMillis2 = millis();
      }
    }
    else previousMillis2 = millis();
    buttonValOld = buttonVal;
      
  if(!testMode)
  {
      analogWrite(outPWM, pwmVal);
  }
}
