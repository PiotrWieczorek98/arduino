#include <SoftwareSerial.h>

SoftwareSerial hc(11, 12); // RX, TX

 //Bluetooth packet size definitions
#define SPEED_PACKET_SIZE     7     //Speed packet size
#define DIRECTION_PACKET_SIZE 7     //Direction packet size
#define BUTTON_PACKET_SIZE    4     //Button packet size
#define RANGE_FROM            0     //Set to the value selected in app settings (Settings->Joystick Value Range) 
#define RANGE_TO              512  //Set to the value selected in app settings (Settings->Joystick Value Range)

//L293 Connection 
  const int enA = 9;
  const int enB = 3;
  const int motorA1  = 7;  // Pin  2 of L293
  const int motorA2  = 6;  // Pin  7 of L293
  const int motorB1  = 5; // Pin 10 of L293
  const int motorB2  = 4;  // Pin 14 of L293


// Use it to make a delay... without delay() function!
  long previousMillis = -1000*10;// -1000*10=-10sec. to read the first value. If you use 0 then you will take the first value after 10sec.  
  long interval = 1000*10;       // interval at which to read battery voltage, change it if you want! (10*1000=10sec)
  unsigned long currentMillis;   //unsigned long currentMillis;
  int timeout_counter = 0; //Counter used for counting the time since the last received packet
//Useful Variables
  int i=0;
  int j=0;
  bool disable = false;
  int dir = 0;

  //LIGHTS
  const int frontLight = 13;
  const int topLight = 10;
  bool front = 0;
  bool top = 0;
  bool blinker = 0;

void motorAngle(int x)
{
  int mspeed = 0;
  if(x > 256)
  { //forward movement
    dir = 1; 
    mspeed = map(x, 256, 512, 140, 255); //Map the speed value to L293D_EN_MAX limit
        digitalWrite(motorB1, HIGH);    digitalWrite(motorB2, LOW);
        analogWrite(enB,mspeed);       
  } 
  else if(x < 256)
  { //backward movement
    dir = 2;
    mspeed = map(x, 0, 256, 255, 140); //Map the speed value to L293D_EN_MAX limit
        digitalWrite(motorB1, LOW);     digitalWrite(motorB2, HIGH);
        analogWrite(enB,mspeed);
  } 
  else
  {
    if(dir == 1)
    {
        digitalWrite(motorB1, LOW);     digitalWrite(motorB2, HIGH);
        analogWrite(enB,150);
        delay(10);
        analogWrite(enB,0);
        dir = 0;
    }
    else if(dir == 2)
    {
        digitalWrite(motorB1, HIGH);    digitalWrite(motorB2, LOW);
        analogWrite(enB,150);
        delay(10);
        analogWrite(enB,0);
        dir = 0;  
    }
    else
    {
      digitalWrite(motorB1, LOW);    digitalWrite(motorB2, LOW);
      analogWrite(enB,0);
    }   
  }
}

void motorStop()
{
        analogWrite(enA, 0);   analogWrite(enB, 0);
}

void motorDrive(int y)
{
  int mspeed = 0;
  if(y > 256)
  { //forward movement 
    mspeed = map(y, 256, 512, 140, 255); //Map the speed value to L293D_EN_MAX limit
        digitalWrite(motorA1, HIGH);   digitalWrite(motorA2, LOW);
        analogWrite(enA,mspeed);
  } 
  else if(y < 256)
  { //backward movement
    mspeed = map(y, 0, 256, 255, 140); //Map the speed value to L293D_EN_MAX limit
        digitalWrite(motorA1, LOW);        digitalWrite(motorA2, HIGH);
        analogWrite(enA,mspeed);
  } 
  else
  {
    digitalWrite(motorA1, LOW);        digitalWrite(motorA2, HIGH);
     analogWrite(enA,0);
  }
}

void setup() 
{
    // Set pins as outputs:
    pinMode(enA, OUTPUT);
    pinMode(enB, OUTPUT);
    pinMode(motorA1, OUTPUT);
    pinMode(motorA2, OUTPUT);
    pinMode(motorB1, OUTPUT);
    pinMode(motorB2, OUTPUT);
    pinMode(topLight, OUTPUT);
    pinMode(frontLight, OUTPUT);
    //Set PWM frequency of the EN pin to 32768Hz (above the frequency a human can hear, otherwise the motor is generating some noise).
    setPwmFrequency(enA, 1); 
    setPwmFrequency(enB, 1);

    // Initialize serial communication at 9600 bits per second:
    Serial.begin(9600);
    hc.begin(9600);
}
 
void loop() 
{
  if(hc.available() && disable == false)
  { //Check if there is serial data ready to be read
    while(hc.available()) handleSerial(hc.read()); //If there is data ready to be read, pass the first character to the handleSerial function 
    timeout_counter = 0; //Reset the counter when a packet is received
  }
  else
  {  
    if(timeout_counter > 200)
    { //If no data was received in the last 500ms stop the motor, so that the car doesn't drive away when the connection is lost
      motorStop();
    }
    else
    {
      timeout_counter++; //Increment the counter
    }
  }
  if(top)
  {
    if(blinker)
      digitalWrite(topLight,HIGH);
    else
      digitalWrite(topLight,LOW);
    blinker = !blinker;
  }
  else
    digitalWrite(topLight,LOW);
delay(100); //Wait for 25ms
    
}

void handleSerial(char c){
  switch(c){  //The first received character defines the packet type (speed, direction, button)
    case 'S':  //'S' indicates that the speed packet was received
    {
      if(hc.available() < SPEED_PACKET_SIZE) break; // Speed packet not fully received break and wait for it
      int y = hc.parseInt(); //Parse the speed
      if(hc.read() == 13){  //Check if the packet is complete, each packet ends with "\r\n" (13 and 10 in ANSII)
        if(hc.read() == 10){ 
          motorDrive(y); //If the packet is valid, drive the motor
        }
        else clearSerialBuffer(); //Clear the Serial buffer if the packet is invalid
      }
      else clearSerialBuffer(); //Clear the Serial buffer if the packet is invalid
    } break;
    case 'D':  //'D' indicates that the direction packet was received
    {
      if(hc.available() < DIRECTION_PACKET_SIZE) break; // Direction packet not fully received break and wait for it
      int x = hc.parseInt(); //Read the direction (the received direction value is between 0 and RANGE_TO)
      if(hc.read() == 13){ //Check if the packet is complete, each packet ends with "\r\n" (13 and 10 in ANSII)
        if(hc.read() == 10){
          motorAngle(x); //If the packet is valid, drive servo
        }
        else clearSerialBuffer; //Clear the Serial buffer if the packet is invalid
      }
      else clearSerialBuffer();//Clear the Serial buffer if the packet is invalid
    } break;
    case 'B':  //'B' indicates that a button packet was received
    {
      if(hc.available() < BUTTON_PACKET_SIZE) break; // Button packet not fully received break and wait for it
      switch(hc.parseInt()){
        case 1: //Button 1 pressed
        {
          if(hc.read() == 13){
            if(hc.read() == 10){
              onButton1Press();
            }
          } 
        } break;
        case 2: //Button 2 pressed
        {
          if(hc.read() == 13){
            if(hc.read() == 10){
              onButton2Press(); 
            }
          } 
        } break;
        case 3: //Button 3 pressed
        {
          if(hc.read() == 13){
            if(hc.read() == 10){
              onButton3Press();
            }
          } 
        } break;
        default: //Unrecognized button command
        {
          clearSerialBuffer();
        } break;
      } 
    } break;
    default: // Unrecognized packet
    {
      clearSerialBuffer();
    } break;
  }
}

void clearSerialBuffer(){
  while(hc.available()){
    hc.read();
  }
}

void onButton1Press()
{
  front = !front;
  if(front)
  digitalWrite(frontLight,HIGH);
  else
  digitalWrite(frontLight,LOW);
}

void onButton2Press()
{
  top = !top;
}

void onButton3Press()
{
  
}

void setPwmFrequency(int pin, int divisor) 
{
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) 
  {
    switch(divisor) 
    {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) 
    {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else 
    {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } 
  else if(pin == 3 || pin == 11) 
  {
    switch(divisor) 
    {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}
