/*
   1/2 Вкл выкл свет
   3/4 откр закрыт дверь


*/




#define but_pin 13        // кнопка подключена сюда (PIN --- КНОПКА --- GND)
#define trig_pin A3
#define ir_pin 7
#define servo_pin 12
#define sv A0
#define kon_1 5 //Закорытое
#define kon_2 6 // открытое
#define pin_out 10

#define pin_in 2

#define rast 10

#include "GyverButton.h"
#include "IRremote.h"
#include <Servo.h>

GButton butt1(but_pin);
GButton ob(pin_in);
IRrecv irrecv(ir_pin);
decode_results results;
Servo servo;

bool opend = false, kon1, kon2, f = false, blok = false, dor = false;

boolean trig() ;
void setup() {
  irrecv.enableIRIn();
  pinMode(trig_pin, INPUT);
  pinMode(sv, OUTPUT);
  pinMode(kon_2, INPUT_PULLUP); pinMode(kon_1, INPUT_PULLUP);
  servo.attach(servo_pin);
    pinMode(pin_out, OUTPUT);
    digitalWrite(pin_out, 1);
  kon2 = !digitalRead(kon_2);
  kon1 = !digitalRead(kon_1);
  servo.write(90);
}

void loop() {
  butt1.tick();
  if (butt1.isSingle()) {
    if (!opend)opend = true;
    else opend = false;
  }
  if (butt1.isDouble()) {
    f = !f;
    digitalWrite(sv, f);
    if(f){digitalWrite(pin_out,0);delay(20);digitalWrite(pin_out,1);delay(20);}
     else{for(int i=0;i<2;i++){digitalWrite(pin_out,0);delay(20);digitalWrite(pin_out,1);delay(20);}}
  }
  if ( irrecv.decode( &results )) {
    switch (results.value) {
      case 0x1689D02F:
          opend = !opend;

          if(opend){for(int i=0;i<3;i++){digitalWrite(pin_out,0);delay(20);digitalWrite(pin_out,1);delay(20);}}
             else{ for(int i=0;i<4;i++){digitalWrite(pin_out,0);delay(20);digitalWrite(pin_out,1);delay(20);}}
        break;
      case 0x168938C7:
        f = !f;
        digitalWrite(sv, f);
        break;
    }

    irrecv.resume();
  }
  if (opend && !dor) {
    servo.write(0);
    while (digitalRead(kon_2) == 1) {}
    servo.write(90);
    dor = true;
    //  delay(20);
  }
  if (!opend && dor) {
    servo.write(180);
    while (digitalRead(kon_1) == 1) {}
    servo.write(90);
    dor = false;
    //   servo.detach();
    //    delay(200);
    //    servo.attach(servo_pin);
  }
//    if (butt1.isTriple())blok = !blok;
  /*
    if (!blok) {
      ob.tick();
      if (butt1.isSingle()) Serial.println("Single");     // проверка на один клик
      if (butt1.isDouble()) Serial.println("Double");     // проверка на двойной клик
    }
  */
  if (opend && trig()) {
    digitalWrite(sv, 1);
    f = true;
    //    tt=false;
        digitalWrite(pin_out,0);delay(20);digitalWrite(pin_out,1);delay(20);
  }


}
boolean trig() {
  float volts = analogRead(trig_pin) * 0.0048828125;
  // и в расстояние в см
  float distance = 32 * pow(volts, -1.10);
  if (distance < rast)return true;
  else return false;
}
void yield() {
  butt1.tick();
}
