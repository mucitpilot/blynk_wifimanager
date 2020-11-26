//Mucitpilot 2020
//Blynk ve ESP8266 Kullanımında Ağ Adı-Şifresi ve Blynk Token'in dışarıdan Girilerek
//Sonrasında EPROM'da saklanarak kullanılabileceği uygulama
//Hazır kütüphanelerde sorun yaşadığım için normal Wifimanager kütüphanesini kullanıp, token verisini EPROM'a yazıp okuyan bir çözüm geliştirdim.
//ayrıca butona fonk atayıp 3sn den fazla basınca tüm ayarları resetletiyorum.

//Gerekli kütüphaneler

#define BLYNK_PRINT Serial
//#ifndef UNIT_TEST
#include <Arduino.h>
//#endif
#include <EEPROM.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <BlynkSimpleEsp8266.h>

//gerekli değişkenler
#define led D2
bool resetbutton=false;
int butondurumu=0;
bool res; //wifimanager oto bağlantı durumu takibi için
int sayac, buttonSayac=0;
bool resetle = false;

WiFiManager wm; //bir adet wifimanager nesnesi oluşturduk

BlynkTimer timer; //senkronu sağlamak için bir sayaç nesnesi oluşturuluyor.



char blynk_token[] = "Blynk Token Giriniz";//Wifi manager ekranında ilk seferde görünecek 

//blynk token'in ilk seferde kayıt edilmesi için gerekli mantıksal değişken tanımladık
bool kaydedilsinmi = false;

//Token'in kaydedilmesi gerektiği durumda kaydedilsin mi verisini true yapacak fonksiyon. Bunu daha sonra aşağıda çağıracağız.
void konfigGuncelleme () {
  Serial.println("Veri Kaydedilmeli");
  kaydedilsinmi = true;
}

/////////////////Blynk üzerinden ESP'ye komut gönderme/////////////////////////////////////
BLYNK_WRITE(V1) //uygulamada tanımladığımız değişken ismi olmalı (ör:V1, V2)
{
  int pinValue = param.asInt(); // Sanal v1 değişkeninden aldığımız veriyi bir değişkene atıyoruz.
  // Ayrıca veri tipine göre şunlar da kullanılabilir.:
  // String i = param.asStr();
  // double d = param.asDouble();

  if (pinValue==1){
    Serial.println(" AÇIK");
    digitalWrite(led,HIGH);
    butondurumu=1;
    }
  else {
      Serial.println(" KAPALI");
      digitalWrite(led,LOW);
       butondurumu=0;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////

////////////////ESP'den BLYNK uygulamasına veri gönderme//////////////////////////////////////


// aşağıdaki fonksiyon ile sunucuya her 1 sn'de bir veri göndereceğiz.
// Uygulama kısmında gösterge nesnesinin özelliği PUSH olarak ayarlanacak.
// Çok sık aralıkla veri göndermeniz (max sn'de 10 veri) sistem tarafından engellendiği için tasarıma dikkat etmek gerek.
void veriGonder()
{
  String h;
  if (butondurumu ==1){h="LED AÇIK";}
  else {h="LED KAPALI";}
  Blynk.virtualWrite(V2, h);

}
///////////////////////////////////////////////////////////////////////////////////////////////


void setup()
{
  EEPROM.begin(512);
  Serial.begin(115200);
  pinMode(D2, OUTPUT);//D2 pinini çıkış olarak tanımladık
  pinMode(D1, INPUT_PULLUP);//D1 i buton olarak tanımladık
  
  //////////////WIFI MANAGER AYAR BLOĞU//////////////////////////////////////////////////
   
   //wm.resetSettings(); //bu satırı açarsak hafızadaki wifi ayarlarını temizler.
   wm.setSaveConfigCallback(konfigGuncelleme); //ilk kayıtta yukarıda tanımladığımız kaydedilsin mi ile ilgili fonk'u çağıracak

  //bu satırda wifimanager'a blynk token hanesini ekliyoruz
   WiFiManagerParameter custom_blynk_token("Blynk", "blynk token", blynk_token, 34);
   wm.addParameter(&custom_blynk_token);

  
  res = wm.autoConnect("blynk_test", "12345678"); // Wifimanager bağlanma satırı. Ağ adı olarak görünmesini istediğimiz
  // ismi ve belirleyeceğimiz şifreyi tanımladık. İstersek şifre de girmeyebiliriz. Korumasız ağ olarak görünür.
  
  if (!res) {
    Serial.println("Ağ Bağlantı Sağlanamadı");
    // ESP.restart();
  }
  else {
    //buraya gelmişse WiFi'ya bağlanmış demektir.
    Serial.println("Ağ Bağlantısı Kuruldu");
  }


  //wifimanager sistemine alınan blynk token'ı çekip kullanmak üzere blynk_token char nesnesine kopyalıyoruz
   strcpy(blynk_token, custom_blynk_token.getValue()); 
   Serial.println("blink token için ilk değer:");
   Serial.println(blynk_token);

  
   //////////////////////////EPROMA YAZDIRMA BÖLÜMÜ////////////////////////////////
   
   //Eğer kaydedilsinmi true olmuş ise Token'i EPROM'a kaydedeceğiz
  if (kaydedilsinmi) { //yukarıda true'ya çekmişsek girecek. Yani sadece ilk değişiklikte true yaptığımız için o zaman girecek ve token'ı EPROM'a kayedetmiş olacağız
    Serial.println("EPROM'a kaydediliyor:");
    String yazilacak=String(blynk_token); //blynk_token char nesnesini stringe çevirmemiz lazım
    writeStringToEEPROM(100, yazilacak);
    Serial.print("eproma yazdırıldı:");
    Serial.println(yazilacak);
   
    kaydedilsinmi = false; //tekrar false'a çekelim ki her seferinde girmesin
  }


 ///////////////////EPROMDAKİ VERİYİ OKUMA BÖLÜMÜ/////////////////////////////////////// 
   String token =readStringFromEEPROM(100);
   token.toCharArray(blynk_token,34); 
   
   Serial.print("epromdan okudu:");
   Serial.println(blynk_token);
  
////////////////OKUNAN TOKEN İLE BLYNK'E BAĞLANMA///////////////////////////////////  
   Blynk.config(blynk_token);
   delay(100);
  
   bool result = Blynk.connect();

  if (result != true)
    {
      Serial.println("BLYNK'e bağlanılamadı !!!");
//      wm.resetSettings(); //eğer istenirse aktif edilip sistem resetlenebilir.
//      ESP.reset();
//       delay (5000);
    }
    else
    {
     Serial.println("BLYNK' Bağlantı Başarılı");
    }

  //D1 deki butonu interrupt olarak tanımladık
  attachInterrupt(digitalPinToInterrupt(D1), wifiReset, CHANGE);

  //timer nesnesinin veri gönderme aralığını  ve hangi fonksiyon ile veri gönderileceğini tanımlıyoruz.
  timer.setInterval(1000L, veriGonder); 
}

void loop()
{
  if (resetle) { //buton ile resetlemeyi denetliyor
      resetle = false;
      wm.resetSettings();//tüm ayarları sil
      ESP.reset(); //esp'yi baştan başlat
      delay(3000);
  }

  
  Blynk.run(); //Burası mümkün olduğunca boş olmalı. Blynk'in çalışma mantığı genel olarak bu şekilde. Loop içinde başka işlemler olunca Blynk sunucusu ile olan senkron bozulabiliyor.
  timer.run(); //zamanlamayı kontrol ediyor.
}

//////////////EPROMA VERİ YAZMAK İÇİN FONKSİYONUMUZ////////////////////////////////////
//eproma verileri her bir hücreye karakter karakter yazabiliriz. Yani her bir hücre 1 byte veri alır. 1024 hücre yani 1024 kapasitemiz var.
//O yüzden eğer diyelim ki 32 harfli bir token yazacak isek çağırırken uzunluğunu bilmemiz gerekeceğinden 100 numaralı hücreye önce metnin uzunluğunu yazarız
//sonra da sırayla her bir göze bir karakteri yazarız.
// sonra o uzunluğu okuyup devamında kaç hücreye bakacağımızı bilir ve tek tek karakterleri çekip birleştirerek metni tekrar oluşturabiliriz
void writeStringToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
   EEPROM.commit();
   delay(500);
}
//////////////EPROMDAN VERİ OKUMAK İÇİN FONKSİYONUMUZ////////////////////////////////////
String readStringFromEEPROM(int addrOffset)
{
int newStrLen = EEPROM.read(addrOffset);
char data[newStrLen + 1];
for (int i = 0; i < newStrLen; i++)
{
data[i] = EEPROM.read(addrOffset + 1 + i);
}
data[newStrLen] = '\0';
return String(data);
}

/////////RESET BUTONU İÇİN GEREKLİ INTERRUPT FONKSİYONU////////////////////////
//3snden fazla basınca wifi ayarları resetlenecek
ICACHE_RAM_ATTR void wifiReset() {
  if (!resetbutton) {
    if (digitalRead(D1) == HIGH) {
      buttonSayac = millis();
      resetbutton = true;
    }
  } else {
    if (digitalRead(D1) == LOW) {
      resetbutton = false;
      if ((millis() - buttonSayac) >= 50) {
        if ((millis() - buttonSayac) >= 3000) {
          Serial.println(">3 sn");
          resetle = true;
        } else {
          //butona eklenmek isteyen farklı bir fonksiyon varsa buradan çağrılabilir.
          sayac = 0;
          Serial.println("<3 sn");
        }
      }
    }
  }

}
