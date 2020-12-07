//#define teste
#define celular

#include <Arduino.h>
#include <Servo.h>
#ifdef celular
#include <SoftwareSerial.h>
#endif

#define S2_Pin 7
#define S3_Pin 8
#define LED_Pin 11
#define OUT_Pin 12
#define Beep_Pin 4
#define Servo0_Pin 9
#define Servo1_Pin 10

#define pick_tubo 2100
#define pick_sensor 1450
#define pick_rampa 1300
#define rampa_laranja 1050
#define rampa_verde 1260
#define rampa_amarelo 1470
#define rampa_vermelho 1680
#define rampa_azul 1890
#define rampa_marrom 2100

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
#ifdef celular
SoftwareSerial bt0 (2, 3);
#endif

uint8_t liga = 0; //Começa ligado ou desligado
uint8_t ligado = 1;
uint8_t ligado2 = 0;
uint32_t instant[2] = {0, 0};
uint32_t instfreq;
uint32_t x;
uint32_t period[4];
uint32_t pwm[4];
uint8_t mmvec[10] = {7, 7, 7, 7, 7, 7, 7, 7, 7, 7};
uint8_t mmveccount[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t mm = 7;
uint8_t maior = 0;
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
  if (period[Blue] < period[Red] && period[Blue] < period[Green] && (period[Red]-period[Blue]) > 40){
    return Azul;
  }else if (period[Red] < period[Blue] && period[Blue] < period[Green] && (period[Green]-period[Blue]) > 35){
    return Vermelho;
  }else if (period[Red] < period[Blue] && period[Blue] < period[Green] && (period[Green]-period[Blue]) < 35){
    return Laranja;
  }else if (period[Red] < period[Green] && period[Green] < period[Blue]){
    return Amarelo;
  }else if (period[Green] < period[Red]){
    return Verde;
  }else{
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
      servo1.writeMicroseconds(rampa_laranja);
      break;
    case Vermelho:
      servo1.writeMicroseconds(rampa_vermelho);
      break;
    case Amarelo:
      servo1.writeMicroseconds(rampa_amarelo);
      break;
    case Verde:
      servo1.writeMicroseconds(rampa_verde);
      break;
    case Azul:
      servo1.writeMicroseconds(rampa_azul);
      break;
    case Marrom:
      servo1.writeMicroseconds(rampa_marrom);
      break;
    default:
      break;
      //Deu merda
  }
}
#ifdef celular
void desliga(){
  if (bt0.available()){
    if (bt0.read() == '0'){
      analogWrite (LED_Pin, 0x00);
        digitalWrite (Beep_Pin, HIGH);
        servo0.writeMicroseconds(pick_sensor);
        delay (2000);
        digitalWrite (Beep_Pin, LOW);
      liga = 0;
    }
  }
  while(!liga){
    if (bt0.available()){
      char rec = bt0.read();
      Serial.print(rec);
      if (rec == '1'){
        analogWrite (LED_Pin, 0xFF); //Intensidade do LED
        liga = 1;
        digitalWrite (Beep_Pin, HIGH);
        delay (1000);
        digitalWrite (Beep_Pin, LOW);
      }
    }
  }
}
#endif

void setup() {
  pinMode (LED_BUILTIN, OUTPUT);
  pinMode (S2_Pin, OUTPUT);
  pinMode (S3_Pin, OUTPUT);
  pinMode (LED_Pin, OUTPUT);
  pinMode (OUT_Pin, INPUT);
  pinMode (Beep_Pin, OUTPUT);
  Serial.begin(1000000);
  servo0.attach(Servo0_Pin);
  servo1.attach(Servo1_Pin);
  #ifdef celular
  bt0.begin(9600);
  bt0.println("Hello");
  #endif
  #ifdef teste
  analogWrite (LED_Pin, 0xFF);
  #endif

}

void loop() {
  //Set inicial
  maior = 0;
  for (int i=0; i<10; i++){
    mmvec[i] = 7;
  }
  for (int i=0; i<8; i++){
    mmveccount[i] = 0;
  }
  //Pisca início
  digitalWrite (LED_BUILTIN, ligado);
  ligado = !ligado;
  //Pisca fim
  #ifndef teste
  #ifdef celular
  desliga();
  #endif

  //Move o pick para baixo do sensor
  servo0.writeMicroseconds(pick_sensor);
  //Delay para esperar chegar (fixo)
  delay(300);
  //Detecta a cor (faz a média de 10 leituras)
  #endif

  for (int i=0; i<10; i++){
    for (int k=0; k<4; k++){
      period[k] = 0;
    }
    for (int j=0; j<10; j++){
      period[Red] += detecta(Red);
      period[Blue] += detecta(Blue);
      period[Clear] += detecta(Clear);
      period[Green] += detecta(Green);
    }
    period[0] /= 10;
    period[1] /= 10;
    period[2] /= 10;
    period[3] /= 10;
    mmvec[i] = cor(period);
  }
  for(int i=0; i<10; i++){
    mmveccount[mmvec[i]]++;
  }
  for(int i=0; i<8; i++){
    if(mmveccount[i] > maior){
      maior = mmveccount[i];
      mm = i;
    }
  }
  bt0.println(mm);
  #ifndef teste
  //Move a rampa para o pote escolhido
  moveRampa(cor(period));
  //Delay do De Wit (pode ser variável)
  delay(200);
  //Move o pick para a rampa
  servo0.writeMicroseconds(pick_rampa);
  //Delay para esperar chegar (pode ser variável)
  delay(200);
  //Move o pick para baixo do tubo
  servo0.writeMicroseconds(pick_tubo);
  //Delay para esperar chegar (fixo)
  delay(400);
  //Move o pick para tras do tubo
  servo0.writeMicroseconds(pick_tubo + 75);
  //Delay para esperar chegar (fixo)
  delay(100);
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
  sprintf(outbuf, "W%08lX-%08lX-%08lX-%08lX-%08lX-X", period[Clear], period[Red], period[Green], period[Blue], (uint32_t)mm);
  Serial.write(outbuf, 47);
  //Print fim
  delay(10);
}