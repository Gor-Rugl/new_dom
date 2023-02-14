// Ссылка для менеджера плат:
// http://arduino.esp8266.com/stable/package_esp8266com_index.json
/*
Библиотека Firebase Arduino содержит ссылку на отпечаток пальца SSL-сертификата Firebase. Этот отпечаток пальца может не совпадать с текущим отпечатком пальца.
Этот отпечаток пальца находится в FirebaseHttpClient.h (обычно в C:\Users\<User>\Documents\Arduino\libraries\firebase-arduino-<version>\src\FirebaseHttpClient.h).
Чтобы найти и изменить текущий отпечаток пальца:
Перейти к https://www.grc.com/fingerprints.htm
Войти "test.firebaseio.com "
Запишите отпечаток пальца (например, в данный момент он 04:E0:BD:B0:F8:63:22:3C:3A:19:7D:92:B6:79:2A:44:BF:77:FC:DA
ОткрытьC:\Users\<User>\Documents\Arduino\libraries\firebase-arduino-<version>\src\FirebaseHttpClient.h
Заменить значение kFirebaseFingerprint с отпечатком пальца (без двоеточий)
Перекомпилировать

*/

#include <ESP8266WiFi.h>                            // esp8266 library
#include <FirebaseArduino.h>                        // Библиотека для работы с базой данных
#include <NewPing.h>                          //Библиотека для датчика препядствия

#define  FIREBASE_HOST "smarthouse-43fc3-default-rtdb.firebaseio.com"    // адрес сайта firebase
#define  FIREBASE_AUTH "CLl9k32wxjrXJ7UlPj8uMp6puSFZbxeJaeW031Ah"               // ключ доступа
#define WIFI_SSID "Rugl2"                                                 //provide ssid (wifi name)
#define WIFI_PASSWORD "Wi-1234567890"                                           //wifi password

//пины подключения 
#define TRIGGER_PIN  12
#define ECHO_PIN     14
#define piezoPin 4                 //Пин подключение пеьезлизлучателя

#define MAX_DISTANCE 40//максимально фиксируемое растояние

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);//инициализация датчика перпядствия


 bool flag_ev=0,flagg=false,flag_open=false;//Логические переменные
 int fiks,data;
 char key;
 unsigned long fot;

 
void setup() {
  Serial.begin(115200);//Общение с UNO
  Serial.setTimeout(50);
 WiFi.begin(WIFI_SSID, WIFI_PASSWORD);               //Производим подключение к WiFi сети
  delay(300);
      while (WiFi.status() != WL_CONNECTED) { 
        tone(piezoPin,500,100);
        delay(300);//Вывод ошибок о подключении
  }
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  
  Firebase.setInt("num", 40);
  // handle error
  if (Firebase.failed()) {
      tone(piezoPin,1000);//Если Firebasa не доступна подать сигнал 
     // Serial.println(Firebase.error());  
      
  }

  delay(100);

  fiks=sonar.ping_cm();// Закрытое полож двери

 
Firebase.setString("sost","Не открывалась");//Очистка предидущих сессий
Firebase.setString("event","Утечки нет");
Firebase.setInt("consol",0);
}
 
void loop() {
  if((!flag_open&&sonar.ping_cm()<fiks)){//Если есть препядствие то открыть дверь
  Serial.println("n1");fot=millis();flag_open=true;}  
                                                    
  if(flag_open&&(millis()>(fot+10000))){Serial.println("n0");flag_open=false;}//Таймер(10с) чтоб дверь сама закрывалась
 
 if(Firebase.getInt("consol")){flagg=true;tone(piezoPin, 500, 500);//Если был запрос дистанционного открытия
                              Serial.print("d1");Firebase.setString("sost","Открыто дистанционно");}//Уведомить обратно и открыть сервер
else if(flagg){Serial.print("d0");flagg=false;}

if(Serial.available()>1){
  key=Serial.read();
  data=Serial.parseInt();
  switch(key){
    case 's'://Состояние по открытию серверной
if(data==0)Firebase.setString("sost","Закрыто");
if(data==1){
          Firebase.setString("sost","Открытие по карте");
          tone(piezoPin,500,100);delay(120);
          tone(piezoPin,700,300);delay(320);
          tone(piezoPin,500,100);delay(120);
}
if(data==2){
            Firebase.setString("sost","Отказано в доступе");
            for(int gg=0;gg<5;gg++){tone(piezoPin,300,100);delay(120);}
      }
    case 'p'://влажность
    Firebase.setInt("pet",data);//Вывод в базу
    break;
    case 't'://Температура
    Firebase.setInt("temp",data);//Вывод в базу
    break;
    case 'e'://Эвакуация
    Firebase.setString("event",data ? "Утечка газа":"Утечки нет");//Вывод в базу
    if(data){
    for(int i=0;i<10;i++){
  tone(piezoPin, 1000, 500);
  delay(550);
 
  tone(piezoPin, 2000, 500);
  delay(550);
 
}}
    break;
    case 'f'://Ошибка контроля доступа
    tone(piezoPin,1500);
    break;
    
  }
}

}
