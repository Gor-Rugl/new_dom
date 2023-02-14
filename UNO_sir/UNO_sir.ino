//------------------- Библиоткеки ---------------------//
#include <TroykaDHT.h>
#include <TroykaMQ.h>
#include <Adafruit_PN532.h>
#include <LiquidCrystalRus.h>
#include <TroykaLight.h>
#include <Servo.h>
#include <Wire.h>
#include <SPI.h>
#include "GyverButton.h"
#include <microLED.h>   // подключаем библу светодиодки
//------------------- Пины подключкини --------------------//
#define PIN_MQ5  A1           //Пин для подключения датчика газа
#define PIN_DHT  3           //Пин для подключения датчика влажности и температуры
#define PIN_LIH  A0           //Пин подключения фоторезистора
#define PIN_PT1  A3           //Пин подключения 1 потанцеометра
#define PIN_PT2  A2           //Пин подключения 2 потанциометра
#define PN532_IRQ  2          // Прин подключениея RFID/NFC трекера от прерывания
#define key A5                // кнопка подключена сюда (PIN --- КНОПКА --- GND)
#define STRIP_PIN 11     // пин ленты
#define NUMLEDS 30      // кол-во светодиодов

                   //Пины для дисплея
#define DB7 4             //DB7 
#define DB6 5           //DB6
#define DB5 7             //DB5
#define DB4 6         //DB4
#define RS 9            //RS
#define E 8          //E

#define ser_cpp 13     //Дверь с датчиком препядствия          
#define ser_qq 10        //Управление штрами 
#define ser_serr 12    // Дверь серверной   

//------------------- Обявление объектов --------------------//
MQ5 mq5(PIN_MQ5);                       //Инициализация датчика газа
DHT dht(3, DHT11);                  //Инициализация датчика температуры и влажности
TroykaLight sensorLight(PIN_LIH);     //Инициализация датчика освещености
Adafruit_PN532 nfc(PN532_IRQ, 100);//Инициализация RFID/NFC сканера
LiquidCrystalRus lcd(RS, E, DB4, DB5, DB6, DB7);//Инициализация LCD Дисплея
microLED<NUMLEDS, STRIP_PIN, MLED_NO_CLOCK, LED_WS2818, ORDER_GRB, CLI_AVER> strip;//Инициализация адресной ленты
GButton butt(key);//Инициализация кнопки
//Инициализация сервоприводов
Servo ms_q;// шторы
Servo ms_cp;//Комнота с дисплеем
Servo ms_serv; //Вход в серверную


//------------------- Переменные --------------------//

int hag,met,tem,prg,lig,pt1,pt2,data;        //Постаянно обьявляемые 
int hag_ol,tem_ol,ppt1=60,ppt2;        //Переменные выданных значений 
char keys;                      //Переменная для приема команд от Wi-Fi
bool flag_s=false,flag_g=false,flag_op=false,flag_lig = false;//Логичские переменные
int pd_cp[2]={ 180,90},pd_ex[2]={ 0,90},pd_ser[2]={60,160};//Положение закр/откррыт дверей Серверные

//uint8_t uidFirstCard[] = {0xA6, 0xA0, 0x1A, 0xD0};
uint8_t uidFirstCard[] = {0x69, 0x19, 0xB0, 0x3F};// Данные карты допуска

uint8_t success;
uint8_t uid[8];
uint8_t uidLength;
mData svet[2]={mRGB(0, 0, 0),mRGB(255, 255, 255)};

//------------------- Объявление функций --------------------//
void dth();//Темп и Влажность
void gas();//Газ 2
void lih();//Свет
void pts();//Потанциометры
void card();// Карта
void disp();//Вывод информ на дисплей
boolean comparisonOfUid(uint8_t uidRead[8], uint8_t uidComp[8], uint8_t uidLen);// функция которая сравнивает два переданных ID

//------------------- Запуск --------------------//
void setup() {
  Serial.begin(115200);   //Общение с платой Wi-Fi
      dht.begin();            //Инициализация датчика влажности и температуры
      mq5.calibrate();    //Калибровка датчика газа
      lcd.begin(8, 2);          //Инициализация lcd дисплея 8х2
      
        ms_q.attach(ser_qq); //Подключение сервопприводов
        ms_cp.attach(ser_cpp);
        ms_serv.attach(ser_serr); 
        
        
      pinMode(PIN_PT1,INPUT);//Подкючение потанциометров
      pinMode(PIN_PT2,INPUT);
      
      
     butt.setDebounce(50);        // настройка антидребезга
     butt.setTimeout(2000);        // настройка таймаута на удержание 
     butt.setType(HIGH_PULL);     //Потдяжка конпки в высокому сигналу те pin->конпка->gnd
     butt.setDirection(NORM_OPEN); //Задание тип кнопки

     
      nfc.begin();                                  //Инициализация и проверка RFID/NFC модуля
          int versiondata = nfc.getFirmwareVersion();
          if (!versiondata) {
            //Serial.println("ER_D_RFID");
            while(1) {Serial.print("f1");}}         //при ошибке подаем сигнал
            nfc.SAMConfig();
            
          
          ms_cp.write(pd_cp[0]);//Закрытие всех дверей
          ms_serv.write(pd_ser[0]);
          ms_q.write(0);
          
    strip.setBrightness(ppt1);//Установка якроси ленты и ее очистка 
    strip.clear();
    strip.show(); // вывод изменений на ленту

}
//------------------- Повотряющаяся --------------------//
void loop() {
if(Serial.available()>1){
  keys=Serial.read();
  data=Serial.parseInt();
  switch(keys){
    case 'd'://Открытие двери серверной удаленно
      ms_serv.write(pd_ser[data]);
      flag_op=true;
    break;

    case 'n'://Открытие двери у леснечной
      ms_cp.write(pd_cp[data]);
    break;
 } 
}
 
   butt.tick();                  //Проверка нажатия конпки
   dth();                 // Запуск опроса на темперетуру(tem) и влажность(hag) 
   gas();                             // Значение газа
   lih();                   // Значение освешенности
   pts();                           // Значение потанциометров и распределение на нов знач
   disp();                     //Вывод значений на дисплей
//Serial.println(met);Serial.println(prg); //кор. данные
   if(butt.isHolded())card();// Проверка карт и открытие или нет серверной
   if(butt.isSingle()&&flag_op){ms_serv.write(pd_ser[0]);Serial.print("s0");flag_op=false;}
}
//------------------- Функции --------------------//
void dth(){
  dht.read();
      tem=dht.getTemperatureC();  //Значениея температуры
      hag=dht.getHumidity();      //Заначение влажноти
  }
//----------------------Газ---------------------------//
void gas(){
  prg=mq5.readLPG();              //Значение Природных газов
  met=mq5.readMethane();        //Значение Метан
}
//------------------Свет вкл/выкл-------------------------------//
void lih(){
  sensorLight.read();           // Значение яркости                                
  lig=sensorLight.getLightLux();
  if(lig<40 && !flag_lig){strip.fill(svet[1]);strip.show();flag_lig=true;ms_q.write(0);}//Включение света и закрыте штор
  if(lig>150 && flag_lig){strip.fill(svet[0]);strip.show();flag_lig=false;ms_q.write(70);}//Выключение и открытие штор
}
//----------------------шторы+яркость---------------------------//
void pts(){
  pt1=analogRead(PIN_PT1);
  pt1=map(pt1,0,1023,0,245);//Данные с потанциометра подстраиваем под значения яркости ленты
  if((ppt1<pt1+5 && ppt1<pt1-5)||(ppt1>pt1+5 && ppt1>pt1-5)){strip.setBrightness(pt1);ppt1=pt1;if(flag_lig){strip.fill(svet[1]);strip.show();}}
  pt2=analogRead(PIN_PT2);
  pt2=map(pt2,0,1023,0,70);//Данные с потанциометра подстраиваем под значения сервопривода для штор
  if((ppt2<pt2+5 && ppt2<pt2-5)||(ppt2>pt2+5 && ppt2>pt2-5)){ppt2=pt2;ms_q.write(pt2);}
}

//----------------------Аунтификация карт---------------------------//
void card(){
  
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success) {
    if (comparisonOfUid(uid, uidFirstCard, uidLength)){
      Serial.println("s1");      // Открытие серверной и передача на базу
      delay(700);
      ms_serv.write(pd_ser[1]);
      flag_op=true;
    }else{
      Serial.println("s2");      //Уведомление о попытке открытия
      delay(700);
      ms_serv.write(pd_ser[0]);
    }
 }
}
//-----------------------Вывод на дисплей и проверка утечки--------------------------//
void disp(){
  if((tem!=tem_ol||hag!=hag_ol)){   //Если данные отличаются от выведенных, выводим новые
    tem_ol=tem;hag_ol=hag;Serial.print("p");Serial.print(hag);Serial.print("t");Serial.print(tem);
    lcd.clear();//очистка дисплея и последующий вывод
    lcd.setCursor(0, 0);
    lcd.print("t=");
    lcd.print(tem_ol);
    lcd.print("\x99""C ");
    lcd.setCursor(1, 1);
    lcd.print("p=");
    lcd.print(hag_ol);
    lcd.print("\x25");
}

 if(prg>20||met>20){//Если параметры газа завыгены начать тревогу
     if(!flag_g){ms_cp.write(pd_cp[1]);strip.fill(mRGB(255,0,0));strip.show();}
     Serial.print("e1");
     
  flag_g=true;
 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print("Warning");
 lcd.setCursor(0,1);
 lcd.print("Danger");
     
delay(11000);


}else if(flag_g){//Если опасности больше нет, включить обычное функционирование системы
  flag_g=false;
  Serial.print("e0");
  ms_cp.write(pd_cp[0]);
    if(lig<50){strip.fill(svet[1]);strip.show();}else{strip.fill(svet[0]);strip.show();}
    }

}
//-------------------Проверка для карт------------------------------//
boolean comparisonOfUid(uint8_t uidRead[8], uint8_t uidComp[8], uint8_t uidLen) {
  for (uint8_t i = 0; i < uidLen; i++) {
    if (uidRead[i] != uidComp[i]) {
      return false;
    }
    if (i == (uidLen)-0x01) {
      return true;
     }
   }
}

//--------------------Опрос во время задержек-----------------------------//
void yield(){
  //gas();
  butt.tick();
  if(butt.isStep())card();// Проверка карт и открытие или нет серверной
}
