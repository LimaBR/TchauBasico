#include <Arduino.h>
#include <Servo.h>

#define S2_Pin 7
#define S3_Pin 8
#define LED_Pin 9
#define OUT_Pin 2

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

Servo servo0;

uint8_t ligado = 0;
uint8_t ligado2 = 0;
uint32_t instant[2] = {0, 0};
uint32_t period;
uint32_t instfreq;
uint32_t x;
uint32_t freq[4];
uint32_t pwm[4];
char outbuf[64];
char inbuf[64];
enum{
  Laranja,
  Vermelho,
  Amarelo,
  Verde,
  Azul,
  Marrom
};
enum{
  Red,
  Blue,
  Clear,
  Green
};

uint8_t cor(uint32_t* period){
  return 0;
}

uint32_t detecta(uint8_t State){
  //Função blocking (não é o ideal)
  digitalWrite (S2_Pin, State/2);
  digitalWrite (S3_Pin, State%2);
  while(digitalRead(OUT_Pin));
  while(!digitalRead(OUT_Pin));
  while(digitalRead(OUT_Pin));
  while(!digitalRead(OUT_Pin));
  instant[0] = micros();
  while(digitalRead(OUT_Pin));
  while(!digitalRead(OUT_Pin));
  instant[1] = micros();
  return instant[1] - instant[0];
}

void setup() {
  pinMode (LED_BUILTIN, OUTPUT);
  pinMode (12, OUTPUT);
  pinMode (S2_Pin, OUTPUT);
  pinMode (S3_Pin, OUTPUT);
  pinMode (LED_Pin, OUTPUT);
  pinMode (OUT_Pin, INPUT);
  Serial.begin(1000000);
  analogWrite (LED_Pin, 0xFF);
  servo0.attach(11);
}

void loop() {
  //código do pisca
  digitalWrite (LED_BUILTIN, ligado);
  ligado = !ligado;

  freq[0] = detecta(Clear);
  freq[1] = detecta(Red);
  freq[2] = detecta(Green);
  freq[3] = detecta(Blue);

  if(Serial.read() == 'Y'){
    inbuf[0] = 'Y';
    for(int i = 0; i<37; i++){
      inbuf[i+1] = Serial.read();
    }
  sscanf(inbuf, "Y%08lX-%08lX-%08lX-%08lX-Z", &pwm[0], &pwm[1], &pwm[2], &pwm[3]);
  servo0.writeMicroseconds(pwm[0]);
  }

  //Print
  sprintf(outbuf, "W%08lX-%08lX-%08lX-%08lX-X", freq[0], freq[1], freq[2], freq[3]);
  Serial.write(outbuf, 38);
  //delay para deixar devagar
  //delay (100);
}