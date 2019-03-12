#include <ESP8266WiFi.h>
#include <time.h>

const char* ssid="Domotica-IF";
const char* password="iotsenha123";

int ledVerificaWifi = 16;
int ledPin = 5;

//Tempo
int timezone = -3 * 3600;
int dst = 0;

void setup() {
  //Conexao a internet
  pinMode(ledVerificaWifi,OUTPUT);
  digitalWrite(ledVerificaWifi,LOW);
  
  //Led de teste
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin,LOW);

  Serial.begin(115200);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println( ssid );
  
  WiFi.begin(ssid,password);

  Serial.println();
  Serial.print("Conectando");

  while( WiFi.status() != WL_CONNECTED ){
    delay(500);
    Serial.print(".");
  }
  
  digitalWrite( ledVerificaWifi , HIGH);
  Serial.println();

  Serial.println("Wifi conectado com sucesso!");
  Serial.print("Endereco IP: ");
  Serial.println(WiFi.localIP());

  //Tempo
  configTime(timezone, dst, "pool.ntp.org","time.nist.gov");
  Serial.println("\nEsperando o tempo da internet");

  while(!time(nullptr)){
    Serial.print("*");
    delay(1000);
  }
  Serial.println("\nTempo de resposta....OK");
}

void loop() {

  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);

  //Liga LED
  if( (p_tm->tm_hour == 13) && (p_tm->tm_min == 48) && (p_tm->tm_sec == 20) ){
    digitalWrite(ledPin,HIGH);
  }

  //Desliga LED
  if( (p_tm->tm_hour == 13) && (p_tm->tm_min == 49) ){
    digitalWrite(ledPin,LOW);
  }

  delay(1000);
}
