// DESENVOLVIMENTO E IMPLANTAÇÃO DA DOMÓTICA NO IFRN CAMPUS MOSSORÓ
// Autores: Ailson Ferreira, Clayton Maciel, Fábio Lucas, Lariza Maria, Marcos Vinícius, Michel Santana e Vitor Ropke.
// 31 de Março de 2019 --- IFRN - Campus Mossoró

// Incluindo bibliotecas:
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <time.h>
#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <iostream>
#include <string>
#include <sstream>

// Definindo os tópicos MQTT:
#define TOPICO_SUBSCRIBE "domotica237a"
#define TOPICO_PUBLISH "domotica237b"

// Definindo o ID deste NodeMcu:
#define ID_MQTT "NodeMcu-3"

// Definindo a pinagem:
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

// Definindo o SSID e a senha da rede Wi-Fi:
const char* SSID = "Domotica-IF";
const char* PASSWORD = "iotsenha123";

// Definindo o Broker MQTT e sua porta:
const char* BROKER_MQTT = "iot.eclipse.org";
int BROKER_PORT = 1883;

// Definindo as variáveis de tempo:
int fusoHorario = -3 * 3600;
int horarioDeVerao = 0;

// Definindo as variáveis do acionamento programado:
String m_on_h = "0";
String m_on_m = "0";
String m_off_h = "0";
String m_off_m = "0";
String a_on_h = "0";
String a_on_m = "0";
String a_off_h = "0";
String a_off_m = "0";
String n_on_h = "0";
String n_on_m = "0";
String n_off_h = "0";
String n_off_m = "0";
int cont = 1;

// Instanciando um objeto da classe WiFiClient:
WiFiClient espClient;
PubSubClient MQTT(espClient);

// Definindo as funções:
void conectaWiFi();
void conectaMQTT();
void enviaEstado();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void verificaHorario();

// Definindo os pinos que enviarão os sinais infra-vermelho:
IRsend irsend_Ar1(Ar1);
IRsend irsend_Ar2(Ar2);

// Definindo os vetores que armazenarão os RAWs IR:
uint16_t liga_Ar1[348] = {2954,9052,472,558,442,1534,468,562,442,564,444,560,442,562,442,564,442,564,444,560,442,1536,468,562,442,568,438,1538,466,564,440,564,440,1536,470,1534,466,1536,470,1536,464,1536,466,566,438,568,438,568,386,596,408,594,414,592,410,594,410,596,408,594,414,590,412,596,410,596,408,594,410,596,410,596,410,596,410,594,410,594,410,622,382,622,382,622,382,622,384,622,382,622,384,624,380,622,386,618,386,620,384,622,386,618,384,620,386,622,386,1586,414,1588,414,1590,414,1588,414,3094,2904,9100,420,1590,412,618,444,562,390,616,386,618,446,558,390,616,386,618,386,618,444,1532,414,618,386,618,390,1584,480,554,448,1528,474,1530,470,1532,474,1528,414,1588,476,1526,416,618,448,556,446,560,446,560,442,558,452,552,442,564,448,558,446,558,446,558,446,558,446,560,446,558,446,562,448,554,448,558,448,558,446,558,448,556,450,558,446,556,452,554,448,558,448,554,448,558,450,558,448,556,448,556,450,556,448,556,450,556,448,556,444,562,446,560,446,556,452,552,448,2024,2970,9014,506,1526,474,558,450,554,452,554,448,556,450,556,448,556,450,558,448,556,446,1554,452,558,444,558,450,1528,474,1530,474,1526,476,1552,452,554,448,1528,472,1530,478,1552,452,552,450,1530,472,558,450,1528,474,1554,444,560,448,558,446,556,446,1556,448,1504,500,1558,448,556,446,560,450,558,444,558,446,558,450,556,448,1552,450,1528,476,528,480,1552,448,528,478,526,478,528,476,1526,476,530,482,524,478,530,474,528,480,524,478,530,476,528,480,1520,480,1524,478,1524,482,1520,478};
uint16_t desliga_Ar1[348] = {2956,9048,502,532,468,1530,478,532,472,534,472,534,472,532,472,534,474,532,472,534,472,1528,476,536,470,532,472,1528,474,1528,476,532,466,1532,476,1526,474,1530,476,1526,476,1526,476,536,472,530,412,594,468,540,408,594,472,534,468,538,466,538,410,596,468,540,466,536,466,540,468,536,468,536,468,538,468,534,410,596,468,536,468,538,470,538,406,596,466,538,410,596,410,596,410,596,410,596,410,596,468,536,464,540,470,536,412,592,470,536,412,592,470,536,412,1586,416,1590,416,2588,2938,9046,440,1586,414,598,436,564,410,596,440,564,442,564,440,564,440,564,440,564,440,1558,416,594,440,568,438,1558,444,566,442,1530,468,1558,444,1558,450,1554,446,1554,448,1556,450,562,440,566,438,564,440,564,442,564,442,564,440,566,440,564,442,564,440,568,438,564,444,562,440,564,442,566,440,562,444,560,444,560,446,558,446,562,444,558,444,560,446,562,444,558,446,558,446,558,448,558,444,560,446,560,446,560,444,558,446,560,446,560,450,558,444,560,446,560,444,560,446,2034,2958,9048,470,1532,470,560,446,562,446,556,446,562,444,558,446,560,446,558,448,558,444,1536,468,562,444,558,446,1532,472,560,446,560,448,558,444,1532,474,1530,470,1532,470,1532,470,560,446,1532,474,558,444,1536,468,1554,448,560,446,560,448,558,444,1534,468,1532,470,1532,472,558,446,560,446,560,446,560,446,560,446,558,446,1532,470,1532,470,560,446,1532,470,560,446,560,448,558,444,1536,468,564,444,558,444,560,446,536,468,562,444,558,444,560,444,564,444,558,444,1534,472,1528,470};
uint16_t liga_Ar2[200] = {4368,4424,500,1680,500,596,496,1684,500,1678,502,596,496,594,498,1680,500,598,550,540,498,1678,500,594,496,598,494,1678,502,1678,502,594,498,1682,558,538,498,594,500,1676,500,1678,500,1676,502,1678,502,1676,502,1678,556,1620,502,1678,500,596,498,594,498,596,498,596,496,598,496,598,550,540,496,1682,500,1680,500,1678,502,596,496,596,496,596,496,598,476,1696,500,598,490,600,498,596,496,1682,502,1678,502,1676,500,1678,526,5264,4408,4370,500,1680,500,596,494,1686,490,1686,500,596,496,600,494,1678,500,600,516,576,494,1678,492,606,496,598,494,1678,496,1682,500,596,496,1684,526,568,496,596,496,1678,502,1678,502,1678,502,1678,500,1678,502,1678,524,1652,502,1676,500,596,498,594,498,594,498,596,496,596,498,598,520,572,496,1682,502,1676,502,1678,502,594,498,596,496,596,498,596,522,1652,502,596,498,596,496,596,498,1682,502,1678,500,1678,502,1680,498};
uint16_t desliga_Ar2[200] = {4352,4398,538,1676,548,546,546,1590,592,1632,548,546,544,546,544,1632,548,548,504,586,546,1630,548,546,546,546,546,1590,588,1632,546,556,536,1638,504,588,500,1678,548,1596,584,1586,592,1630,548,536,548,1642,542,1596,546,1634,584,548,546,546,546,546,546,546,546,1586,592,548,544,550,506,1668,548,1630,546,1632,546,546,546,546,546,548,544,548,544,550,508,582,544,548,544,548,544,1632,544,1634,546,1596,582,1632,546,1592,546,5236,4402,4372,550,1636,548,588,504,1650,534,1628,550,556,534,568,526,1622,556,570,522,588,504,1620,558,570,522,566,528,1624,552,1644,536,562,530,1626,556,560,532,1632,552,1618,560,1618,558,1620,560,556,536,1622,560,1622,554,1628,552,544,580,524,530,574,554,526,568,1588,590,516,576,530,532,1622,588,1590,588,1588,590,528,564,528,564,528,566,526,566,530,530,556,568,526,566,526,566,1588,590,1584,558,1620,558,1624,554,1622,556};
uint16_t aumenta[] = {};
uint16_t diminui[] = {};

// Definindo as variáveis dos botões:
int estadoRelay2 = HIGH;
int estadoBotao1 = LOW;
int estadoAnteriorB1 = LOW;
int estadoBotao2 = LOW;
int estadoAnteriorB2 = LOW;
int estado = LOW;

// Criando função para conexão Wi-Fi:
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
}

// Criando função para conexão ao broker MQTT:
void conectaMQTT()
{
  while (!MQTT.connected())
  {
    digitalWrite(WiFiBroker, HIGH);
    delay(500);
    digitalWrite(WiFiBroker, LOW);
    delay(500);
    if (MQTT.connect(ID_MQTT))
    {
      MQTT.subscribe(TOPICO_SUBSCRIBE);
      enviaEstado();
    }
  }
}

// Criando função para conexão com o servidor do horário:
void conectaServidorHorario()
{  
  configTime(fusoHorario, horarioDeVerao, "pool.ntp.org", "time.nist.gov", "a.st1.ntp.br");
  
  while(!time(nullptr))
  {
    digitalWrite(WiFiBroker, HIGH);
    delay(1000);
    digitalWrite(WiFiBroker, LOW);
    delay(1000);
  }
  digitalWrite(WiFiBroker, HIGH);
}

// Criando função para o recebimento de dados e posterior acionamento dos dispositivos:
void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
    String msg;
    String first;
    String schedule_a;
    String schedule_b;
    String c_string_a;
    String c_string_b;

    first = (char)payload[0];

    if(first == "t")
    {
      for(int i = 1; i < 3; i++){
        c_string_a = "";
        char c = (char)payload[i];
        c_string_a += c;
        if(i == 1)
        {
          if(c_string_a != "0")
          {
            schedule_a += c_string_a;
          }
        }
        if(i == 2)
        {
            schedule_a += c_string_a;
        }
      }

      for(int i = 3; i < 5; i++){
        c_string_b = "";
        char c = (char)payload[i];
        c_string_b += c;
        if(i == 3)
        {
          if(c_string_b != "0")
          {
            schedule_b += c_string_b;
          }
        }
        if(i == 4)
        {
            schedule_b += c_string_b;
        }
      }

      if(cont == 1)
      {
        m_on_h = schedule_a;
        m_on_m = schedule_b;
      }

      else if(cont == 2)
      {
        m_off_h = schedule_a;
        m_off_m = schedule_b;
      }

      else if(cont == 3)
      {
        a_on_h = schedule_a;
        a_on_m = schedule_b;
      }

      else if(cont == 4)
      {
        a_off_h = schedule_a;
        a_off_m = schedule_b;
      }

      else if(cont == 5)
      {
        n_on_h = schedule_a;
        n_on_m = schedule_b;
      }

      else if(cont == 6)
      {
        n_off_h = schedule_a;
        n_off_m = schedule_b;
        cont = 0;
      }
    
      cont += 1;
    }
    

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
      digitalWrite(Relay2, LOW);
      irsend_Ar1.sendRaw(liga_Ar1, 348, 32);
      irsend_Ar2.sendRaw(liga_Ar2, 200, 32);
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
      digitalWrite(Relay2, HIGH);
      irsend_Ar1.sendRaw(desliga_Ar1, 348, 32);
      irsend_Ar2.sendRaw(desliga_Ar2, 200, 32);
      digitalWrite(LedAr1, LOW);
      digitalWrite(LedAr2, LOW);
      enviaEstado();
    }
    // Um sistema web acabou de se conectar:
    if (msg.equals("x"))
    {
      enviaEstado();
    }
}

// Criando função para o envio do estado atual dos dispositivos:
void enviaEstado()
{
    int cont = 0;
    
    if (digitalRead(LedAr1) == HIGH){
      MQTT.publish(TOPICO_PUBLISH, "1L");
      cont += 1;
    }
    else
    {
      MQTT.publish(TOPICO_PUBLISH, "1D");
    }
      
    if (digitalRead(LedAr2) == HIGH){
      MQTT.publish(TOPICO_PUBLISH, "2L");
      cont += 1;
    }
    else
    {
      MQTT.publish(TOPICO_PUBLISH, "2D");
    }
    
    if (digitalRead(Relay2)== LOW){
      MQTT.publish(TOPICO_PUBLISH, "3L");
      cont += 1;
    }
    else
    {
      MQTT.publish(TOPICO_PUBLISH, "3D");
    }
    
    if (cont == 3){
      MQTT.publish(TOPICO_PUBLISH, "4L");
    }
    else if (cont == 0)
    {
      MQTT.publish(TOPICO_PUBLISH, "4D");
    }
}

void verificaHorario()
{
  Serial.println("...");
  Serial.println(m_on_h);
  Serial.println(m_on_m);
  Serial.println(m_off_h);
  Serial.println(m_off_m);
  Serial.println(a_on_h);
  Serial.println(a_on_m);
  Serial.println(a_off_h);
  Serial.println(a_off_m);
  Serial.println(n_on_h);
  Serial.println(n_on_m);
  Serial.println(n_off_h);
  Serial.println(n_off_m);
  Serial.println("...");

  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  int hora = p_tm->tm_hour;
  int minuto = p_tm->tm_min;
  int segundo = p_tm->tm_sec;

  String hour = String(hora);
  String minute = String(minuto);
  // Serial.println(teste.compareTo(m_on_h));

  // Serial.println(hora);
  // Serial.println(minuto);
  // Serial.println(segundo);
  // Serial.println("...");

  if((hour == m_on_h && minute == m_on_m) || (hour == a_on_h && minute == a_on_m) || (hour == n_on_h && minute == n_on_m))
  {
    if(segundo == 0 || segundo == 1)
    {
      digitalWrite(Relay2, LOW);
      irsend_Ar1.sendRaw(liga_Ar1, 348, 32);
      irsend_Ar2.sendRaw(liga_Ar2, 200, 32);
      digitalWrite(LedAr1, HIGH);
      digitalWrite(LedAr2, HIGH);
      enviaEstado();
      Serial.println("LIGANDO!!!");
    }    
  }

  if((hour == m_off_h && minute == m_off_m) || (hour == a_off_h && minute == a_off_m) || (hour == n_off_h && minute == n_off_m))
  {    
    if(segundo == 0 || segundo == 1)
    {
      digitalWrite(Relay2, HIGH);
      irsend_Ar1.sendRaw(desliga_Ar1, 348, 32);
      irsend_Ar2.sendRaw(desliga_Ar2, 200, 32);
      digitalWrite(LedAr1, LOW);
      digitalWrite(LedAr2, LOW);
      enviaEstado();
      Serial.println("DESLIGANDO!!!");
    }
  } 

  delay(1000);
}

/*void controleManual()
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
}*/

void setup()
{
  pinMode(LedAr1, OUTPUT);
  pinMode(LedAr2, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(WiFiBroker, OUTPUT);
  //pinMode(Botao1, INPUT);
  //pinMode(Botao2, INPUT);
  
  digitalWrite(Relay2, HIGH);

  irsend_Ar1.begin();
  irsend_Ar2.begin();

  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(mqtt_callback);

  Serial.begin(115200);
}

void loop(){
  conectaWiFi();
  conectaMQTT();
  conectaServidorHorario();
  //controleManual();
  verificaHorario();
  MQTT.loop();
}
