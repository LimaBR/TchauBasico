#include <Arduino.h>
#include <Servo.h>

#define teste

#define S2_Pin 7
#define S3_Pin 8
#define LED_Pin 11
#define OUT_Pin 2
#define Servo0_Pin 9
#define Servo1_Pin 10

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
Servo servo1;

uint8_t liga = 0; //Começa ligado ou desligado
uint8_t ligado = 0;
uint8_t ligado2 = 0;
uint32_t instant[2] = {0, 0};
uint32_t instfreq;
uint32_t x;
uint32_t period[4];
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
  if (period[Blue] < period[Red] && period[Blue] < period[Green] && (period[Red]-period[Blue]) > 20){
    return Azul;
  }else if (period[Red] < period[Blue] && period[Blue] < period[Green] && (period[Blue]-period[Red]) > 10 && (period[Green]-period[Blue]) > 14){
    return Vermelho;
  }else if (period[Red] < period[Blue] && period[Blue] < period[Green] && (period[Blue]-period[Red]) > 10 && (period[Green]-period[Blue]) < 14){
    return Laranja;
  }else if (period[Red] < period[Blue] && period[Green] < period[Blue] && (period[Blue]-period[Red]) > 10){
    return Amarelo;
  }else if (period[Green] < period[Blue] && ((period[Blue]-period[Red]) < 10 || (period[Red]-period[Blue]) < 10)){
    return Verde;
  }else if (period[Blue] < period[Green] && ((period[Blue]-period[Red]) < 10 || (period[Red]-period[Blue]) < 10)){
    return Marrom;
  }
  return 7;
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

void moveRampa(uint8_t cor){
  //Ajustar as constantes com a posição da rampa (testar)
  switch (cor){
    case Laranja:
      servo1.writeMicroseconds(1200);
      break;
    case Vermelho:
      servo1.writeMicroseconds(1300);
      break;
    case Amarelo:
      servo1.writeMicroseconds(1400);
      break;
    case Verde:
      servo1.writeMicroseconds(1600);
      break;
    case Azul:
      servo1.writeMicroseconds(1700);
      break;
    case Marrom:
      servo1.writeMicroseconds(1800);
      break;
    default:
      break;
      //Deu merda
  }
}

void desliga(){
  while(!liga){
    if (Serial.available()){
      if (Serial.read() == '1'){
        liga = 1;
      }
    }
  }
  if (Serial.available()){
    if (Serial.read() == '0'){
      liga = 0;
    }
  }
}

void setup() {
  pinMode (LED_BUILTIN, OUTPUT);
  pinMode (S2_Pin, OUTPUT);
  pinMode (S3_Pin, OUTPUT);
  pinMode (LED_Pin, OUTPUT);
  pinMode (OUT_Pin, INPUT);
  Serial.begin(1000000);
  analogWrite (LED_Pin, 0xFF);
  servo0.attach(Servo0_Pin);
  servo1.attach(Servo1_Pin);

  #ifdef celular
  while 
  #endif
}

void loop() {
  //Set inicial
  period[0] = 0;
  period[1] = 0;
  period[2] = 0;
  period[3] = 0;
  #ifndef teste
  desliga();
  //Pisca início
  digitalWrite (LED_BUILTIN, ligado);
  ligado = !ligado;
  //Pisca fim

  //Move o pick para baixo do sensor
  servo0.writeMicroseconds(1530);
  //Delay para esperar chegar (fixo)
  delay(1000);
  //Detecta a cor (faz a média de 10 leituras)
  #endif
  for (int i=0; i<10; i++){
    period[Red] += detecta(Red);
    period[Blue] += detecta(Blue);
    period[Clear] += detecta(Clear);
    period[Green] += detecta(Green);
  }
  period[0] /= 10;
  period[1] /= 10;
  period[2] /= 10;
  period[3] /= 10;
  #ifndef teste
  //Move a rampa para o pote escolhido
  moveRampa(cor(period));
  //Delay (pode ser variável)
  delay(1);
  //Move o pick para a rampa
  servo0.writeMicroseconds(1000);
  //Delay para esperar chegar (pode ser variável)
  delay(300);
  //Move o pick para baixo do tubo
  servo0.writeMicroseconds(2400);
  //Delay para esperar chegar (fixo)
  delay(600);
  #else
  //Teste dos servos início
  if(Serial.read() == 'Y'){
    inbuf[0] = 'Y';
    for(int i = 0; i<37; i++){
      inbuf[i+1] = Serial.read();
    }
  sscanf(inbuf, "Y%08lX-%08lX-%08lX-%08lX-Z", &pwm[0], &pwm[1], &pwm[2], &pwm[3]);
  servo0.writeMicroseconds(pwm[0]);
  servo1.writeMicroseconds(pwm[1]);
  }
  //Teste dos servos fim
  #endif

  //Print início
  sprintf(outbuf, "W%08lX-%08lX-%08lX-%08lX-%08lX-X", period[Clear], period[Red], period[Green], period[Blue], (uint32_t)cor(period));
  Serial.write(outbuf, 47);
  //Print fim
  delay(10);
}