
#include <Servo.h>

// Define various ADC prescaler:
const unsigned char PS_32 = (1 << ADPS2) | (1 << ADPS0);
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

int IntArray[256]; // <-- the array where the readout of the photodiodes is stored, as integers

#define CLKpin 2     // Arduino digital output pin delivering the clock pulses to pin 3 (CLK) of the TSL1402R
#define SIpin 6      // Arduino digital output pin delivering the SI (serial-input) pulse to pin 2 of the TSL1402R
#define AOpin 3      // Arduino analog input pin connected to pin 8 (analog output 2)of the TSL1402R
#define IRSensor 2   // Arduino analog input pin connected to the IR sensor
#define SERVO 5      // Arduino analog output pin connected to the Servo motor
#define LEDs 7       // Arduino digital output pin switching spectrometer LEDs on and off

#define WHEEL_LEFT 11
#define WHEEL_RIGHT 3

#define DIRECTION_LEFT 13
#define DIRECTION_RIGHT 12

#define BREAK_LEFT 8
#define BREAK_RIGHT 9

#define SERVO_RIGHT 20  // maximum servo position right
#define SERVO_LEFT 128  // maximum servo position left

#define BAUDRATE 9600   // RS 232 / serial baudrate

Servo SensorServo;

void setup() {
  //initialize servo motor
  SensorServo.attach(SERVO);
  SensorServo.write(90);
  
  Serial.begin(BAUDRATE);
  
  // Initialize output pins
  pinMode(CLKpin, OUTPUT);
  digitalWrite(CLKpin, LOW);
  
  pinMode(SIpin, OUTPUT);
  digitalWrite(SIpin,LOW);
  
  pinMode(LEDs, OUTPUT);
  digitalWrite(LEDs,LOW);

  pinMode(WHEEL_LEFT, OUTPUT);
  digitalWrite(WHEEL_LEFT,LOW);
 
  pinMode(WHEEL_RIGHT, OUTPUT);
  digitalWrite(WHEEL_RIGHT,LOW);

  pinMode(BREAK_LEFT, OUTPUT);
  digitalWrite(BREAK_LEFT,LOW);

  pinMode(BREAK_RIGHT, OUTPUT);
  digitalWrite(BREAK_RIGHT,LOW);
  
  pinMode(DIRECTION_LEFT, OUTPUT);
  digitalWrite(BREAK_LEFT,LOW);

  pinMode(BREAK_RIGHT, OUTPUT);
  digitalWrite(DIRECTION_RIGHT,LOW);

  // To set up the ADC, first remove bits set by Arduino library, then choose
  // a prescaler: PS_16, PS_32, PS_64 or PS_128:
  ADCSRA &= ~PS_128;  
  ADCSRA |= PS_32; // <-- Using PS_32 makes a single ADC conversion take ~30 us

  // Next, assert default setting:
  analogReference(DEFAULT);

  // Clock out any existing SI pulse through the ccd register:
  for(int i=0;i< 260;i++)
  {
      clockPulse();
  }

  // Create a new SI pulse and clock out that same SI pulse through the sensor register:
  digitalWrite(SIpin, HIGH);
  clockPulse();
  digitalWrite(SIpin, LOW);
  for(int i=0;i< 260;i++)
  {
      clockPulse();
  }
}

bool asPause ( int ms)
{
  static unsigned long old_ms;
  if ( (millis() - old_ms) >= ms)
  {
    old_ms = millis ();
    return 1;
  }
  return 0;
}
  
void driveForward()
{
  digitalWrite (DIRECTION_LEFT, HIGH);
  digitalWrite (DIRECTION_RIGHT, HIGH);
  digitalWrite (BREAK_LEFT, LOW);
  digitalWrite (BREAK_RIGHT, LOW);
  analogWrite (WHEEL_RIGHT, 110);
  analogWrite (WHEEL_LEFT, 110);
}

void driveBackward()
{
  digitalWrite (DIRECTION_LEFT, LOW);
  digitalWrite (DIRECTION_RIGHT, LOW);
  digitalWrite (BREAK_LEFT, LOW);
  digitalWrite (BREAK_RIGHT, LOW);
  analogWrite (WHEEL_RIGHT, 110);
  analogWrite (WHEEL_LEFT, 110);
}

void driveStand()
{
  digitalWrite (DIRECTION_LEFT, LOW);
  digitalWrite (DIRECTION_RIGHT, LOW);
  digitalWrite (BREAK_LEFT, HIGH);
  digitalWrite (BREAK_RIGHT, HIGH);
  analogWrite (WHEEL_RIGHT, 0);
  analogWrite (WHEEL_LEFT, 0);
}

void driveLeft()
{
  digitalWrite (DIRECTION_LEFT, HIGH);
  digitalWrite (DIRECTION_RIGHT, HIGH);
  digitalWrite (BREAK_LEFT, LOW);
  digitalWrite (BREAK_RIGHT, LOW);
  analogWrite (WHEEL_RIGHT, 110);
  analogWrite (WHEEL_LEFT, 25);
}

void driveRight()
{
  digitalWrite (DIRECTION_LEFT, HIGH);
  digitalWrite (DIRECTION_RIGHT, HIGH);
  digitalWrite (BREAK_LEFT, LOW);
  digitalWrite (BREAK_RIGHT, LOW);
  analogWrite (WHEEL_RIGHT, 25);
  analogWrite (WHEEL_LEFT, 110);
}

void leds_on ()
{
 digitalWrite(LEDs,HIGH); 
}

void leds_off ()
{
 digitalWrite(LEDs,LOW); 
}

//TSL1402R clock pulse
void clockPulse()
{
  delayMicroseconds(1);
  digitalWrite(CLKpin, HIGH);
  digitalWrite(CLKpin, LOW);
}

void readSpectrum()
{
  byte c;
  while( Serial.available () == 0 )
  {
      // wait until a byte is available, byte is interpreted as integer between 0 and 255. this is corresponding to exposure times between 0 and 255 x 100 ms
  }  
  c = Serial.read ();
  int exposure_time;
  exposure_time = ((int) c) * 100;

  // stop the ongoing light integration
  digitalWrite(SIpin, HIGH);
  clockPulse();
  digitalWrite(SIpin, LOW);
  
  // loop through register to start new integration
  for(int i = 0; i < 260; i++)
  {
    clockPulse();
  }    
  
  // change exposure time, initial exposure due to arduino speed is around 15 ms. any delay is added on top
  delay(exposure_time);
  
  // stop exposure
  digitalWrite(SIpin, HIGH);
  clockPulse();
  digitalWrite(SIpin, LOW);
  
  // read pixel values
  for(int i=0; i < 256; i++)
  {
    delayMicroseconds(20);// <-- We add a delay to stabilize the AO output from the sensor
    IntArray[i] = analogRead(AOpin);
    clockPulse();
  }
  
  // transmit data via serial interface
  for(int i = 0; i < 256; i++)
  {
    Serial.print(IntArray[i]); Serial.print(";");
  }
  Serial.println("");
}

void readDistance() {
  //analogRead( IR_Sensor );
  int ServoPosition;
  for(ServoPosition = 0; ServoPosition <= 127; ServoPosition++)
  {
    if(ServoPosition >= SERVO_RIGHT && ServoPosition <= SERVO_LEFT)
    {
      SensorServo.write(ServoPosition);
      Serial.print(analogRead( IRSensor )); Serial.print(";");
      delay(10);
    }
    else
    {
      Serial.print(0); Serial.print(";");
    }
  }
  Serial.println("");
}

void loop() {

  if ( Serial.available () > 0 )
  {
    char c;
    c = Serial.read ();

    switch (c)
    {
      case 'U' : driveForward ();
        break;
      case 'D' : driveBackward ();
        break;
      case 'C' : driveStand ();
        break;
      case 'L' : driveLeft ();
        break;
      case 'R' : driveRight ();
        break;
      case 'E' : readSpectrum ();
        break;
      case 'A' : readDistance ();
        break;
      case 'F' : leds_on ();
        break;
      case 'G' : leds_off ();
        break;
    }
  }
}
