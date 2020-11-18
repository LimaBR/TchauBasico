#include <Arduino.h>

#define S2_Pin 7
#define S3_Pin 8
#define LED_Pin 9
#define OUT_Pin 2
#define tSensor 100 //Pra não dar merda

/* 
 * S0 S1  
 * L  L   Desliga
 * L  H   2%
 * H  L   20%
 * H  H   100%
 * 
 * S2 S3
 * L  L   Red
 * L  H   Blue
 * H  L   Clear
 * H  H   Green
 */

uint8_t ligado = 0;
uint64_t instant[2] = {0, 0};
uint64_t period;
uint64_t freq;

void callback(){
  instant[0] = instant[1];
  instant[1] = micros();
  period = instant[1] - instant[0];
}

void setup() {
  pinMode (LED_BUILTIN, OUTPUT);
  pinMode (S2_Pin, OUTPUT);
  pinMode (S3_Pin, OUTPUT);
  pinMode (LED_Pin, OUTPUT);
  pinMode (OUT_Pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(2), callback, RISING);
  Serial.begin(115200);
  analogWrite (LED_Pin, 0xFF);
}

void loop() {
  //código do pisca
  digitalWrite (LED_BUILTIN, ligado);
  ligado = !ligado;
  //Clear
  digitalWrite (S2_Pin, HIGH);
  digitalWrite (S3_Pin, LOW);
  delay(tSensor);
  freq = 1000000/period;
  Serial.print((unsigned int)freq);
  Serial.print(" ");
  //Red
  digitalWrite (S2_Pin, LOW);
  digitalWrite (S3_Pin, LOW);
  delay(tSensor);
  freq = 1000000/period;
  Serial.print((unsigned int)freq);
  Serial.print(" ");
  //Green
  digitalWrite (S2_Pin, HIGH);
  digitalWrite (S3_Pin, HIGH);
  delay(tSensor);
  freq = 1000000/period;
  Serial.print((unsigned int)freq);
  Serial.print(" ");
  //Blue
  digitalWrite (S2_Pin, LOW);
  digitalWrite (S3_Pin, HIGH);
  delay(tSensor);
  freq = 1000000/period;
  Serial.println((unsigned int)freq);
  //delay para deixar devagar
  //delay (100);
}