// DESENVOLVIMENTO E IMPLANTAÇÃO DA DOMÓTICA NO IFRN CAMPUS MOSSORÓ
// Autores: Ailson Ferreira,Clayton Maciel, Fábio Lucas, Lariza Maria, Marcos Vinícius, Michel Santana e Sara Melo.
// 07 de Setembro de 2018 --- IFRN - Campus Mossoró

// BIBLIOTECA MQTT
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// BIBLIOTECA IRremote
#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <IRremoteESP8266.h>
#include <IRsend.h>

// TÓPICOS
#define TOPICO_SUBSCRIBE "sala1Envia"
#define TOPICO_PUBLISH "sala1Recebe"

// ID DO NODE
#define ID_MQTT "NodeMcu-3"

// PINAGEM
#define Relay1      /*D0*/      16  // Relé 1
#define Botao1      /*D1*/      5   // Botão 1 - Botoeira
#define Botao2      /*D2*/      4   // Botão 2  - Liga/Desliga
#define WiFiBroker  /*D3*/      0   // LED 2 - WiFi e Broker
#define Botao4      /*D4*/      2   // Botão 4 - Diminui Temperatura
#define Relay2      /*D5*/      14  // Relé 2
#define Ar1         /*D6*/      12  // LED Emissor - Ar 1
#define Ar2         /*D7*/      13  // LED Emissor - Ar 2
#define Botao3      /*D8*/      15  // Botão 3 - Aumenta Temperatura 
#define LedAr1      /*D9/RX*/   3   // LED 3 - Ar 1
#define LedAr2      /*D10/TX*/  1   // LED 4 - Ar 2

// SSID E SENHA DO ROTEADOR
const char* SSID = "DomoticaNode";
const char* PASSWORD = "iotsenha123";

// BROKER E PORTA
const char* BROKER_MQTT = "10.0.0.8";
int BROKER_PORT = 1883;

// INSTANCIANDO O OBJETO
WiFiClient espClient;
PubSubClient MQTT(espClient);

// DEFININDO FUNÇÕES
void conectaWiFi();
void conectaMQTT();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void enviaEstado();

// PINOS QUE ENVIARÁ O SINAL INFRA-VERMELHO
IRsend irsend_Ar1(Ar1);
IRsend irsend_Ar2(Ar2);

// VETORES QUE ARMAZENARAM O RAW IR
uint16_t liga_Ar1[348] = {3020,9032,482,528,478,1518,486,526,472,532,430,574,480,526,478,528,478,528,474,530,478,1520,480,532,474,530,474,1474,530,530,474,530,476,1518,486,1516,484,1518,484,1518,484,1520,484,530,474,530,472,536,472,530,476,530,472,534,468,536,472,534,474,530,472,532,472,534,472,532,472,532,474,532,472,532,474,530,474,530,474,532,474,530,474,530,474,530,472,532,472,534,472,534,476,530,474,530,470,536,472,532,476,530,474,530,472,532,474,532,474,1514,486,1516,482,1520,484,1518,488,3022,2974,9032,486,1516,488,530,476,528,476,528,476,530,476,528,474,530,476,530,474,532,476,1514,484,532,474,530,476,1514,488,528,476,1514,490,1514,484,1518,488,1514,488,1514,486,1516,482,532,474,530,478,528,478,526,476,530,476,528,476,528,476,528,478,528,478,528,476,528,478,526,478,526,480,526,478,528,476,526,478,528,478,528,476,530,478,526,474,530,476,528,478,528,478,526,480,526,478,526,478,526,478,528,478,526,480,526,478,528,478,526,478,526,478,526,478,526,478,526,478,2518,2976,9028,490,1514,490,528,476,528,478,526,478,526,480,526,480,526,480,526,478,526,480,1512,486,528,480,526,480,526,476,1516,490,526,480,1512,490,534,464,1518,490,1514,488,1512,490,1514,490,1514,488,1514,490,1512,490,1514,490,526,478,526,478,528,476,1514,490,1514,490,1514,490,526,478,526,478,526,478,526,478,526,478,1514,488,1514,488,1514,490,526,478,1512,490,1514,488,528,478,1514,490,1514,488,528,476,528,478,528,478,528,478,526,478,526,478,528,476,1516,488,1514,488,1514,488,1516,486};
uint16_t desliga_Ar1[348] = {2966,9038,486,526,478,1518,482,530,474,532,430,574,432,574,430,574,478,526,478,526,432,1566,486,526,476,528,478,1518,484,1478,478,572,478,1520,484,1518,486,1528,474,1518,482,1520,486,524,480,528,476,524,482,524,474,532,480,526,478,524,476,530,482,522,478,528,474,532,470,534,480,524,480,524,478,526,476,528,480,526,480,524,480,524,480,526,480,524,480,526,480,524,482,522,482,524,480,524,482,524,478,526,482,522,480,524,480,524,474,532,478,528,478,526,482,1514,488,1516,488,2520,2974,9030,486,1516,486,526,480,522,480,524,484,522,482,522,484,522,482,524,482,524,482,1514,490,522,482,522,484,1512,484,528,482,1514,488,1516,488,1514,490,1512,488,1516,486,1516,490,522,482,530,468,526,486,520,484,520,484,520,484,520,484,520,484,520,484,522,484,520,484,520,484,520,484,520,484,520,482,522,484,522,484,520,484,520,484,520,484,520,486,520,486,520,486,520,486,520,486,518,486,518,486,518,486,518,486,518,486,518,488,516,488,518,486,520,486,518,486,518,486,2014,2978,9028,490,1512,490,520,484,520,486,518,486,520,484,520,484,522,486,518,486,520,486,1512,492,518,486,518,486,518,486,518,486,1512,492,1510,492,520,486,1512,490,1512,492,1512,490,1512,492,1512,492,1512,490,1512,490,1512,490,522,482,522,484,520,484,1514,490,1512,490,1514,490,522,482,524,512,492,482,524,482,524,514,1480,522,1486,516,1482,522,492,512,1482,522,1482,522,490,482,1514,520,1480,490,522,482,522,484,522,482,524,482,524,482,522,482,524,482,522,482,522,482,1512,490,1512,490};
uint16_t liga_Ar2[200] = {4368,4424,500,1680,500,596,496,1684,500,1678,502,596,496,594,498,1680,500,598,550,540,498,1678,500,594,496,598,494,1678,502,1678,502,594,498,1682,558,538,498,594,500,1676,500,1678,500,1676,502,1678,502,1676,502,1678,556,1620,502,1678,500,596,498,594,498,596,498,596,496,598,496,598,550,540,496,1682,500,1680,500,1678,502,596,496,596,496,596,496,598,476,1696,500,598,490,600,498,596,496,1682,502,1678,502,1676,500,1678,526,5264,4408,4370,500,1680,500,596,494,1686,490,1686,500,596,496,600,494,1678,500,600,516,576,494,1678,492,606,496,598,494,1678,496,1682,500,596,496,1684,526,568,496,596,496,1678,502,1678,502,1678,502,1678,500,1678,502,1678,524,1652,502,1676,500,596,498,594,498,594,498,596,496,596,498,598,520,572,496,1682,502,1676,502,1678,502,594,498,596,496,596,498,596,522,1652,502,596,498,596,496,596,498,1682,502,1678,500,1678,502,1680,498};
uint16_t desliga_Ar2[200] = {4352,4398,538,1676,548,546,546,1590,592,1632,548,546,544,546,544,1632,548,548,504,586,546,1630,548,546,546,546,546,1590,588,1632,546,556,536,1638,504,588,500,1678,548,1596,584,1586,592,1630,548,536,548,1642,542,1596,546,1634,584,548,546,546,546,546,546,546,546,1586,592,548,544,550,506,1668,548,1630,546,1632,546,546,546,546,546,548,544,548,544,550,508,582,544,548,544,548,544,1632,544,1634,546,1596,582,1632,546,1592,546,5236,4402,4372,550,1636,548,588,504,1650,534,1628,550,556,534,568,526,1622,556,570,522,588,504,1620,558,570,522,566,528,1624,552,1644,536,562,530,1626,556,560,532,1632,552,1618,560,1618,558,1620,560,556,536,1622,560,1622,554,1628,552,544,580,524,530,574,554,526,568,1588,590,516,576,530,532,1622,588,1590,588,1588,590,528,564,528,564,528,566,526,566,530,530,556,568,526,566,526,566,1588,590,1584,558,1620,558,1624,554,1622,556};
uint16_t aumenta[] = {};
uint16_t diminuir[] = {};

// VARIÁVEIS DOS BOTÕES
int estadoRelay2 = HIGH;
int estadoBotao1 = LOW;
int estadoAnteriorB1 = LOW;
int estadoBotao2 = LOW;
int estadoAnteriorB2 = LOW;
int estado = LOW;

void setup()
{
  pinMode(LedAr1, OUTPUT);
  pinMode(LedAr2, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(WiFiBroker, OUTPUT);
  pinMode(Botao1, INPUT);
  pinMode(Botao2, INPUT);
  
  digitalWrite(Relay2, HIGH);

  irsend_Ar1.begin();
  irsend_Ar2.begin();

  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(mqtt_callback);
}

void conectaWiFi()
{
  if(WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(WiFiBroker, HIGH);
    delay(50);
    digitalWrite(WiFiBroker, LOW);
    delay(50);
  }
  digitalWrite(WiFiBroker, HIGH);
  Serial.print("conectado");
}

void conectaMQTT()
{
  while (!MQTT.connected())
  {
    digitalWrite(WiFiBroker, HIGH);
    delay(200);
    digitalWrite(WiFiBroker, LOW);
    delay(200);
    if (MQTT.connect(ID_MQTT))
    {
      MQTT.subscribe(TOPICO_SUBSCRIBE);
      digitalWrite(WiFiBroker, HIGH);
    }
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;

    //obtem a string do payload recebido
    for(int i = 0; i < length; i++){
       char c = (char)payload[i];
       msg += c;
    }
        
    // L1
    if (msg.equals("L1"))
    {
      irsend_Ar1.sendRaw(liga_Ar1, 348, 32);
      digitalWrite(LedAr1, HIGH);
      enviaEstado();
    }
    // L2
    if (msg.equals("L2"))
    {
      irsend_Ar2.sendRaw(liga_Ar2, 200, 32);
      digitalWrite(LedAr2, HIGH);
      enviaEstado();
    }
    // L3
    if (msg.equals("L3"))
    {
      digitalWrite(Relay2, LOW);
      enviaEstado();
    }
    // L4
    if (msg.equals("L4"))
    {
      irsend_Ar1.sendRaw(liga_Ar1, 348, 32);
      irsend_Ar2.sendRaw(liga_Ar2, 200, 32);
      digitalWrite(Relay2, LOW);
      digitalWrite(LedAr1, HIGH);
      digitalWrite(LedAr2, HIGH);
      enviaEstado();
    }
     
    // D1
    if (msg.equals("D1"))
    {
      irsend_Ar1.sendRaw(desliga_Ar1, 348, 32);
      digitalWrite(LedAr1, LOW);
      enviaEstado();
    }

    // D2
    if (msg.equals("D2"))
    {
      irsend_Ar2.sendRaw(desliga_Ar2, 200, 32);
      digitalWrite(LedAr2, LOW);
      enviaEstado();
    }
    // D3
    if (msg.equals("D3"))
    {
      digitalWrite(Relay2, HIGH);
      enviaEstado();
    }
    // D4
    if (msg.equals("D4"))
    {
      irsend_Ar1.sendRaw(desliga_Ar1, 348, 32);
      irsend_Ar2.sendRaw(desliga_Ar2, 200, 32);
      digitalWrite(Relay2, HIGH);
      digitalWrite(LedAr1, LOW);
      digitalWrite(LedAr2, LOW);
      enviaEstado();
    }
    if (msg.equals("x"))
    {
      enviaEstado();
    }
}

void enviaEstado(){
  if (MQTT.connected()){
    if (digitalRead(LedAr1) == LOW){
      if(digitalRead(LedAr2) == LOW){
        if(digitalRead(Relay2) == HIGH){
          MQTT.publish(TOPICO_PUBLISH, "1off2off3off");
        }
        else{
          MQTT.publish(TOPICO_PUBLISH, "1off2off3on");
        }
      }
      else{
        if(digitalRead(Relay2) == HIGH){
          MQTT.publish(TOPICO_PUBLISH, "1off2on3off");
        }
        else{
          MQTT.publish(TOPICO_PUBLISH, "1off2on3on");
        }
      }
    }
    else{
      if(digitalRead(LedAr2) == HIGH){
        if (digitalRead(Relay2) == LOW){
          MQTT.publish(TOPICO_PUBLISH, "1on2on3on");
        }
        else{
          MQTT.publish(TOPICO_PUBLISH, "1on2on3off");
        }
      }
      else{
        if(digitalRead(Relay2) == LOW){
          MQTT.publish(TOPICO_PUBLISH, "1on2off3on");
        }
        else{
          MQTT.publish(TOPICO_PUBLISH, "1on2off3off");
        }
      }
    }
  }
}

void controleManual()
{
  estadoRelay2 = digitalRead(Relay2);

  if(estadoRelay2 == LOW)
  {
    estadoBotao1 = digitalRead(Botao1);

    if(estadoBotao1 == HIGH)
    {
      estado = !estado;
      delay(1000);
    }

    if(estado == HIGH and estadoAnteriorB1 == LOW)
    {
      estadoBotao2 = digitalRead(Botao2);

      if(estadoBotao2 == HIGH and estadoAnteriorB2 == LOW)
      {
        irsend_Ar1.sendRaw(liga_Ar1, 348, 32);
        digitalWrite(LedAr1, HIGH);
        enviaEstado();
        estadoAnteriorB2 = HIGH;
        delay(100);
      }
      else if(estadoBotao2 == HIGH and estadoAnteriorB2 == HIGH)
      {
        irsend_Ar1.sendRaw(desliga_Ar1, 348, 32);
        digitalWrite(LedAr1, LOW);
        enviaEstado();
        estadoAnteriorB2 = LOW;
        delay(100);
      }
      
      estadoAnteriorB1 = LOW;
      delay(100);
    }
  }
}

void loop(){
  conectaWiFi();
  conectaMQTT();
  controleManual();
  MQTT.loop();
}
