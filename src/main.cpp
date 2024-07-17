#include <Arduino.h>
#include <Servo.h>

#define L298N_PIN_ENA  2
#define L298N_PIN_IN1  8
#define L298N_PIN_IN2  9

#define ZED_SERVO_SIG  3

#define LED_PIN_RED    7
#define LED_PIN_BLUE   6
#define LED_PIN_GREEN  5
#define LED_PIN_YELLOW 4

enum PacketState
{
  BEGIN,    //0x01
  DEVICE,   //0x02-0x04
  PAN,
  LED,
  EEF
};

enum DeviceID
{
  PAN=0x02, //0x02
  LED,      //0x03
  EEF       //0x04
};

enum LEDCommand
{
  NONE,
  RED=0x02, //0x02
  GREEN,    //0x03
  BLUE,     //0x04
  YELLOW    //0x05
};

enum EEFCommand
{
  OPEN=0x02,
  CLOSE
};

PacketState curr_state = BEGIN;
Servo zed_servo;

void handlePan(char c);
void handleLED(char c);
void handleEEF(char c);

void setup()
{
  Serial.begin(9600);
  pinMode(L298N_PIN_ENA, OUTPUT);
  pinMode(L298N_PIN_IN1, OUTPUT);
  pinMode(L298N_PIN_IN2, OUTPUT);
  pinMode(ZED_SERVO_SIG, OUTPUT);
  pinMode(LED_PIN_RED, OUTPUT);
  pinMode(LED_PIN_BLUE, OUTPUT);
  pinMode(LED_PIN_GREEN, OUTPUT);
  pinMode(LED_PIN_YELLOW, OUTPUT);
  zed_servo.attach(ZED_SERVO_SIG);
  zed_servo.write(0);
  digitalWrite(L298N_PIN_ENA, LOW);
  digitalWrite(L298N_PIN_IN1, LOW);
  digitalWrite(L298N_PIN_IN2, LOW);
  digitalWrite(ZED_SERVO_SIG, LOW);
  digitalWrite(LED_PIN_RED, LOW);
  digitalWrite(LED_PIN_BLUE, LOW);
  digitalWrite(LED_PIN_GREEN, LOW);
  digitalWrite(LED_PIN_YELLOW, LOW);
}

void loop()
{
  if (Serial.available() < 1) return;
  char c = Serial.read();
  if (c == 0xFF)
  {
    Serial.write(0xFF);
    return;
  }

  switch (curr_state)
  {
  case PacketState::BEGIN:
    if (c == 0x01)
    {
      curr_state = PacketState::DEVICE;
    }
    return;
  case PacketState::DEVICE:
    switch ((DeviceID) c)
    {
    case DeviceID::PAN:
      curr_state = PacketState::PAN;
      break;
    case DeviceID::LED:
      curr_state = PacketState::LED;
      break;
    case DeviceID::EEF:
      curr_state = PacketState::EEF;
      break;
    default:
      curr_state = PacketState::BEGIN;
      break;
    }
    return;
  case PacketState::PAN:
    handlePan(c);
    break;
  case PacketState::LED:
    handleLED(c);
    break;
  case PacketState::EEF:
    handleEEF(c);
    break;
  default:
    break;
  }
  curr_state = PacketState::BEGIN;
}

void handlePan(char c)
{
  if (c < 0x02 || c > 0xB6)
  {
    curr_state = PacketState::BEGIN;
    return;
  }
  int angle = c - 2;
  zed_servo.write(angle);
}

void handleLED(char c)
{
  switch (c)
  {
  case LEDCommand::RED:
    digitalWrite(LED_PIN_RED, HIGH);
    digitalWrite(LED_PIN_BLUE, LOW);
    digitalWrite(LED_PIN_GREEN, LOW);
    digitalWrite(LED_PIN_YELLOW, LOW);
    break;
  case LEDCommand::BLUE:
    digitalWrite(LED_PIN_RED, LOW);
    digitalWrite(LED_PIN_BLUE, HIGH);
    digitalWrite(LED_PIN_GREEN, LOW);
    digitalWrite(LED_PIN_YELLOW, LOW);
  case LEDCommand::GREEN:
    digitalWrite(LED_PIN_RED, LOW);
    digitalWrite(LED_PIN_BLUE, LOW);
    digitalWrite(LED_PIN_GREEN, HIGH);
    digitalWrite(LED_PIN_YELLOW, LOW);
  case LEDCommand::YELLOW:
    digitalWrite(LED_PIN_RED, LOW);
    digitalWrite(LED_PIN_BLUE, LOW);
    digitalWrite(LED_PIN_GREEN, LOW);
    digitalWrite(LED_PIN_YELLOW, HIGH);
  default:
    digitalWrite(LED_PIN_RED, LOW);
    digitalWrite(LED_PIN_BLUE, LOW);
    digitalWrite(LED_PIN_GREEN, LOW);
    digitalWrite(LED_PIN_YELLOW, LOW);
    break;
  }
}

void handleEEF(char c)
{
  switch (c)
  {
  case EEFCommand::OPEN:
    digitalWrite(L298N_PIN_IN1, HIGH);
    digitalWrite(L298N_PIN_IN2, LOW);
    break;
  case EEFCommand::CLOSE:
    digitalWrite(L298N_PIN_IN1, LOW);
    digitalWrite(L298N_PIN_IN2, HIGH);
    break;
  default:
    return;
  }
  for (size_t i = 0; i < 200; i++)
  {
    digitalWrite(L298N_PIN_ENA, HIGH);
    delayMicroseconds(2000);
    digitalWrite(L298N_PIN_ENA, LOW);
    delayMicroseconds(2000);
  }
  digitalWrite(L298N_PIN_IN1, LOW);
  digitalWrite(L298N_PIN_IN2, LOW);
}