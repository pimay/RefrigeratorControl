/*  Control Board - Temperature / Relay
    Objective: publish information in the web
               send and receive information from the mqtt
               read 4 temperature sensors,
               control 4 relays
*/

/*
   req-001: power on the refrigerator if temp read > temp set + deltapos;
   req-002: power off the refrigerator if temp read < temp set - deltaneg;
   req-003: the command to power on/off the refrigerator is at 1 second;
   req-004: read the DS18B20 every 1 second;
   req-005: read the temp and set the temperature through the html site
   req-007: update the software through the OTA;
   req-008.1: mqtt: send the temp read, set temperature and relay status
   req-008.2:  mqtt:  set the temperatures through the mqtt, fix the node-red to retain the publish
   req-009.0: EEPROM save the set temp in the memory in case of power cycle (https://randomnerdtutorials.com/esp32-flash-memory/ or https://www.filipeflop.com/blog/esp8266-gravando-dados-memoria-flash/);
   req-009.0: the delta negative and delta positive is always 1 (one) for this application, only the set temperature is save
   req-010: receive set temperature through mqtt and save in the memory - ready but mqtt has priority, once that it returns, it will use the mqtt stored, better to disable the mqtt only save in the html
   req-009.1: figure out how to record negative set condition, as -10, the set goes from -30 to 30
   req-012: add a delay power up of the refrigerator during power on, > 30 seconds;
   req-014.0: to monitor more than one temperature sensor and select where to use, it was used eppron, it was select according the Number of the sensor, it should be according the address

   To Do
   req-009.1; FS Record the save thing of req-009.0 + temp sensor address, control parameters, mqtt server, etc
   req-014.1: select the temperature sensor and record and select by the address
   req-011: control the fan command according the refrigerator power on;
   req-013: it should be designe to work in the sonoff dual, sonoff normal and nodeMCU;
   
   
*/

/*
  Equivalencia das saidas Digitais entre NodeMCU e ESP8266 (na IDE do Arduino)
  NodeMCU - ESP8266
  D0 = GPIO16;
  D1 = GPIO5;
  D2 = GPIO4;
  D3 = GPIO0;
  D4 = GPIO2;
  D5 = GPIO14;
  D6 = GPIO12;
  D7 = GPIO13;
  D8 = GPIO15;
  RX = GPIO3;
  TX = GPIO1;
  S3 = GPIO10;
  S2 = GPIO9;
*/

/*
  Update the node mcu by FDTI
  NodeMCU pin - FDTI (pin out)
  Vin         - 3.3V
  En          - 3.3V
  GND         - GND
  D3 (GPIO0)  - GND
  TX          - RX
  RX          - TX
*/

/*
   ESP8266 (NODEMCU)
   Button 0 - GPIO0  - D3
   Button 1 - GPIO5  - D2
   Relay 1  - GPIO12 - D6
   Relay 2  - GPIO5  - D1


   Sonoff Dual R2
   Button 0 - GPIO0  - D3
   Button 1 - GPIO9  - S2
   Relay 1  - GPIO12 - D6
   Relay 2  - GPIO5  - D1

   Sonoff Single
   Button 0 - GPIO14 - D5
   Relay    - GPIO12 - D6

   Placa de Cerveja
   D1 = GPIO5 - sensores de temperatura
   D5 = GPIO14;
   D6 = GPIO12;
   D7 = GPIO13;
   D8 = GPIO15;



   Configuration
   board: Generic ESP8266 Module (sonoff)
   flash mode: DOUT
   size: 1M 64K SPIFFS
   Debug Port: Disable
   IwIP: 1.4 higher bandwidth
   Reset method: nodemcu
   cristal 26Mhz
   flash   40 Mhz
   cpu     80Mhz
   Upload speed 115200
   Erase Flash: only sketch

   NodeMCU
   board: NodeMCU 1.0 (ESP-12E Module)
   Upload Speed: 115200
   CPU frequency: 80 Mhz
   Flash Size 4M (NO spiffs)
   Debug port: disable
   debug level: none
   IwIP variant v1.4 Higher Bandwidth
   VTables: Flash
   Ecveptions: Disable
   Erase Flash: only sketch

*/
///////////////////////////////////////////////
///////////////////////////////////////////////
//Libraries:

// WiFi
//#include <FS.h>                   // It is the first library to access the esp memory
//#include <ESP.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>               //Biblioteca que permite chamar o seu modulo ESP8266 na sua rede pelo nome ao inves do IP.
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h> //Biblioteca que cria o servico de atualizacão via wifi (ou Over The Air - OTA)
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>              //MQTT
#include <ArduinoJson.h>                 //https://github.com/bblanchon/ArduinoJson
#include <EEPROM.h>
//Temperature control
#include <OneWire.h>              // biblioteca para acessar o sensor de temperatura (DS28B20)
#include <DallasTemperature.h>    // biblioteca para acessar o sensor de temperatura (DS28B20)

//time couting
#include <Chrono.h>

/* Class da funcao relay
*/
/*
  class Relay {
  private:
    int pino;
    String control;
    String estado;
    String mqtt_sub;
  public:
    Relay(int pino, String control, String estado, String mqtt_sub);
    void atua();
  };
*/


/* Constantes
*/

// HTML update
String Revisao = "0.1";
String Placa = "ESP_B3A978";
String ControlName = "NodeMcu";
String MAC_Address = "84 F3 EB B3 A9 78";
char* Notice = "New";
String Notice_msg;
///////////////////////////////////////////////////////
// to Serial Print
///////////////////////////////////////////////////////
//configuration
boolean serialPrint = false;
boolean serialPrintTemp = false;
boolean serialPrintThermo = false;
//set the parameters through mqtt
boolean mqttSetParameter = false;
//memory
boolean serialPrintMem = false;
boolean recordMemory = false;
///////////////////////////////////////////////////////
//WiFi
#ifndef APSSID
#define APSSID "winterfell"
#endif
#ifndef AAPSK
#define APPSK  "BrieneTarth"
#endif
//DNS - nome do esp na rede
const char* wifi_host      = "Cerveja_ESP"; //Nome que seu ESP8266 (ou NodeMCU) tera na rede
//const char* ssid = "CasaFeliz"; //"dd-wrt_Ext";
//const char* passwd = "12345678"; //"casafeliz2018";
const char* ssid = APSSID;
const char* passwd = APPSK;


//Time
unsigned long second;
unsigned long minute;
unsigned long hours;
unsigned long days;
unsigned long lasttime;
unsigned long fulltime;
String timemeasure;
/*
  unsigned long time1sec = 1000;
  unsigned long time_timeout = millis();
  unsigned long wifireconnect_timeout = millis();

*/

//MQTT
String mqtt_topico;         //Define o topico do mqtt
String mqtt_mensagem;       //Define a mensagem mqtt
boolean mqttButton = true;  // liga/desliga as logicas do mqtt

char* servidor_mqtt = "192.168.0.125";  //URL do servidor MQTT - Raspberry Pi
char* servidor_mqtt_porta = "1883";  //Porta do servidor (a mesma deve ser informada na variável abaixo)
char* servidor_mqtt_usuario = "pi1";  //Usuário
char* servidor_mqtt_senha = "12345"; //Senha
String server_mqtt;
String server_mqtt_porta;
String server_mqtt_usuario;
String server_mqtt_senha;

// MQTT - definicao de sub/pub
const char* mqtt_topico_pub_relay = "geladeira/relayStatus";    //Topico para publicar o comando de inverter o pino do outro ESP8266
const char* mqtt_topico_sub_relay = "geladeira/relay";    //Topico para ler o comando do ESP8266
//read
const char* mqtt_topico_pubTemp1 =  "geladeira/temp1"; //topico para publicar a temperatura 1
const char* mqtt_topico_pubTemp2 =  "geladeira/temp2"; //topico para publicar a temperatura 1
const char* mqtt_topico_pubTemp3 =  "geladeira/temp3"; //topico para publicar a temperatura 1
const char* mqtt_topico_pubTemp4 =  "geladeira/temp4"; //topico para publicar a temperatura 1
const char* mqtt_topico_pubTemp5 =  "geladeira/temp5"; //topico para publicar a temperatura 1
const char* mqtt_topico_pubTemp6 =  "geladeira/temp6"; //topico para publicar a temperatura 1

const char* mqtt_topico_pubTempset =  "geladeira/pub_setTemp"; //topico para publicar o set de temperatura
const char* mqtt_topico_pubTempsetNeg =  "geladeira/pub_setTempDeltaNeg"; //topico para publicar o set de tempe delta neg
const char* mqtt_topico_pubTempsetPos =  "geladeira/pub_setTempDeltaPos"; //topico para publicar o set de tempe delta pos
const char* mqtt_topico_pubCalcTempNeg =  "geladeira/pub_calcTempNeg"; //topico para publicar o calculo temperatura neg
const char* mqtt_topico_pubCalcTempPos =  "geladeira/pub_calcTempPos"; //topico para publicar o calculo temperatura pos
const char* mqtt_topico_pubConfSetTemp =  "geladeira/pub_ConfTemp"; //topico para publicar o calculo temperatura 
//set
const char* mqtt_sub_temp    = "geladeira/sub_temp";    //Topico para ler temperatura
const char* mqtt_sub_tempNeg = "geladeira/sub_tempNeg";    //Topico para ler temperatura negativa
const char* mqtt_sub_tempPos = "geladeira/sub_tempPos";    //Topico para ler a temperatura positiva
const char* mqtt_sub_ConfSetTemp = "geladeira/sub_ConfSetTemp";    //Topico para ler a temperatura positiva


//Relay Pin
int PIN1 = 14;   // D5 (nodeMCU) / GPIO14 (ESP8266);
int relay = 12;   // D6 (nodeMCU) / GPIO12 (ESP8266);
boolean relaystatus=true;

//power up delay
int delayPowerUp=20;
String str_delayPwrUp;
String str_dlyPwrUp;

//Temperature Pin
#define ONE_WIRE_BUS PIN1  // D5 (nodeMCU) // GPIO14 (ESP9266)
//#define ONE_WIRE_BUS D2  // SONOFF DUAL
//for more sensors
DeviceAddress thermo1, thermo2, thermo3, thermo4, thermo5, thermo6;


//get the set temperature from memory during power up
float selectTemp = 18;


//sensor read
float temp1;
float temp2;
float temp3;
float temp4;
float temp5;
float temp6;
String temp1Add;
String temp2Add;
String temp3Add;
String temp4Add;
String temp5Add;
String temp6Add;

float selectDeltaPos = 1;
float selectDeltaNeg = 1;
float selectTempPos=selectTemp+selectDeltaPos;
float selectTempNeg=selectTemp-selectDeltaNeg;
float selSensorTemp; // temperature of select sensor
String str_selectTemp;
String str_selectDeltaPos;
String str_selectDeltaNeg;
String str_setsensor;
String setTemp;
String setTempPos;
String setTempNeg;
String setSensor;
int tempDeviceCount;      //Number of devices
int selectSensor; //number of select sensor
//unsigned long tempoTempSensor = 0;
unsigned long delayTempSensor = 1000;


Chrono requestTempSensor;
Chrono readTempSensor;
Chrono cmdrelay;

/* DB18S20
 *  Resolution /Resolution in degC / time to process
 *  9  - 0.5degC - 93.75ms
 *  10 - 0.25degC - 187.5ms
 *  11 - 0.125degC - 375.0ms
 *  12 - 0.0625degC - 750.0ms
 */

//memory 
#define EEPROM_SIZE 4
String saveMemMQTT = "false";


///////////////////////////////////////////////
///////////////////////////////////////////////
/* General configuration
*/

///WiFi/WiFi Server
ESP8266HTTPUpdateServer atualizadorOTA; //Este e o objeto que permite atualizacao do programa via wifi (OTA)
ESP8266WebServer servidorWeb(80);       //Servidor Web na porta 80
WiFiClient espClient;                   //serve para o MQTT, create the WiFiClient
PubSubClient client(espClient);         //Serve para o MQTT publish/subscriber

//Temperature Sensor
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature temp_sensors(&oneWire);

///////////////////////////////////////////////
///////////////////////////////////////////////
/* OTA Configuration
*/
//Servico OTA
void InicializaServicoAtualizacao() {
  atualizadorOTA.setup(&servidorWeb);
  servidorWeb.begin();
  if (serialPrint){
    Serial.print("O servico de atualizacao remota (OTA) Foi iniciado com sucesso! Abra http://");
    Serial.print(wifi_host);
    Serial.print(".local/update no seu browser para iniciar a atualizacao\n");
  }
}

///////////////////////////////////////////////
///////////////////////////////////////////////
/* DNS
*/
//Inicializa o servico de nome no DNS
void InicializaMDNS() {
  for (int imDNS = 0; imDNS <= 3; imDNS++) {
    if (!MDNS.begin(wifi_host)) {
      // se nao esta conectado tenta conectar, espera 1 segundo entre loops
      if (serialPrint){
        Serial.print("Erro ao iniciar o servico mDNS!");
        Serial.print("O servico mDNS foi iniciado com sucesso!");
        Serial.print("\n");
      }
      MDNS.addService("http", "tcp", 80);
      client.publish("cerveja/supervisorio", "delay mdns");
      delay(1000);
    }  else {
      //leave loop
      break;
    }
  }
}

///////////////////////////////////////////////
///////////////////////////////////////////////
/* MQTT - Sub - Indica os subscribes para monitorar
   adicionar todos os subscribes
*/
void mqttSubscribe() {
  if (mqttSetParameter){
    client.subscribe(mqtt_topico_sub_relay, 1); //QoS 1
    client.subscribe(mqtt_sub_temp, 1); //QoS 1
    client.subscribe(mqtt_sub_tempNeg, 1); //QoS 1
    client.subscribe(mqtt_sub_tempPos, 1); //QoS 1
    client.subscribe(mqtt_sub_ConfSetTemp, 1);
  }
}


///////////////////////////////////////////////
///////////////////////////////////////////////
/* MQTT - Reconectar
    Serve para reconectar o sistema de MQTT, tenta reconectar 3 vez por ciclo
*/
void reconnectMQTT() {
  String textretorno = "reconnect MQTT";
  //client.publish("cerveja/supervisorio", textretorno.c_str());
  //Try to reconect to MQTT without the while loop, try 3 times, before leave
  //for (int iI = 0; iI <= 3; iI++) {
  if (!client.connected()) {
    //Tentativa de conectar. Se o MQTT precisa de autenticação, será chamada a função com autenticação, caso contrário, chama a sem autenticação.
    bool conectado = strlen(servidor_mqtt_usuario) > 0 ?
                     client.connect("ESP8266Client", servidor_mqtt_usuario, servidor_mqtt_senha) :
                     client.connect("ESP8266Client");

    //Conectado com senha
    if (conectado) {
      if (serialPrint){
        Serial.println("connected");
      }
      // Once Connected, read the subscribes
      mqttSubscribe();
    } else {
      if (serialPrint){
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 1 seconds");
      }
      // Wait 1 seconds before retrying
      client.publish("geladeira/supervisorio", "delay reconnect mqtt");
      delay(1000);
    }
  }
  /* else {
    //leave loop
    break;
    }
    } //for loop
  */
}

///////////////////////////////////////////////
///////////////////////////////////////////////
/* MQTT - Sub - monitora as mensagens do servidor
   Função que será chamada ao monitar as mensagens do servidor MQTT
   Retorna todas as mensagens lidas
*/
void retornoMQTTSub(char* topico, byte* mensagem, unsigned int tamanho) {
  //A mensagem vai ser On ou Off
  //Convertendo a mensagem recebida para string
  mensagem[tamanho] = '\0';
  String strMensagem = String((char*)mensagem);
  strMensagem.toLowerCase();
  //float f = s.toFloat();

  //informa o topico e mensagem
  mqtt_topico = String(topico);
  mqtt_mensagem = strMensagem;


  String textretorno = "Retorno topico: ";
  textretorno += String(topico).c_str();
  textretorno += " in mqtt_topico: ";
  textretorno += mqtt_topico.c_str();
  textretorno += " msg: ";
  textretorno += mqtt_mensagem.c_str();
  textretorno += " \n\n";
  client.publish("geladeira/supervisorio", textretorno.c_str());

}
///////////////////////////////////////////////
///////////////////////////////////////////////
/*  Calculo Tempo
*/
//void updateTempo(const char* subMqtt) {
void updateTempo() {
  //Tempo Calculo
  //Zera o tempo quando estiver em zero ou o contador for ativo
  fulltime = millis();
  //if ((fulltime == 0) or ((strcmp(mqtt_topico.c_str(), subMqtt) == 0) and (mqtt_mensagem == false))) {
  if ((fulltime == 0)) {
    lasttime = fulltime;
    second = 0;
    minute = 0;
    hours = 0;
    days = 0;
  }
  //Conta os segundos
  if (fulltime > lasttime + 1000) {
    //unsigned int diffsec = round((millis() - lasttime)/1000);
    //if (diffsec <= 1.5){
    second = ++second;
    //} else {
    //  second =++ diffsec;
    //}
    lasttime = fulltime;
  }
  if (second >= 60) {
    //if((second-60)>0){
    //  second = (second-60);
    //}else{
    second = 0;
    //}
    minute = ++minute;
  }
  if (minute == 60) {
    hours = ++hours;
    minute = 0;
  }
  if (hours == 24) {
    days = ++ days;
    hours = 0;
  }
}

///////////////////////////////////////////////
///////////////////////////////////////////////
//SENSORS COUNT and ADDRESS
void sensoresTempCount(){
  //esta funcao faz a contagem do numero de sensores de temperatura
  temp_sensors.begin();
  //seta a resolucao dos sensores de temperatura
  temp_sensors.setResolution(10);
  //contador de
  tempDeviceCount = temp_sensors.getDeviceCount();
  delay(delayTempSensor);

  //get the sensors address if the temp sensor > 0
  if (tempDeviceCount>0){
    temp1Add = "";
    temp2Add = "";
    temp3Add = "";
    temp4Add = "";
    temp5Add = "";
    temp6Add = "";
    for (int nTemp = 0 ; nTemp < tempDeviceCount; nTemp++){
      String thermX = "thermo" + String(nTemp+1);
      //temp_sensors.getAddress(thermo1,nTemp);
       if (serialPrintThermo){
        Serial.print("print thermox: ");
        Serial.print(thermX);
        Serial.print(" ");
        Serial.print(nTemp);
        Serial.print(" ");
       }
        for (uint8_t iadd = 0; iadd < 8; iadd++)
          {
            // zero pad the address if necessary
            if (serialPrintThermo){
              if (thermX[iadd] < 16) Serial.print("0");
              Serial.print(thermX[iadd], HEX);
              Serial.print(" iadd: ");
              Serial.print(iadd );
              Serial.print(" ");
            }
            if (nTemp == 0){
              temp1Add += String(thermX[iadd],HEX);
              temp1Add += " ";
            }
            if (nTemp == 1){
              temp2Add += String(thermX[iadd],HEX);
              temp2Add += " ";
            }
            if (nTemp == 2){
              temp3Add += (thermX[iadd],HEX);
              temp3Add += " ";
            }
            if (nTemp == 3){
              temp4Add += (thermX[iadd],HEX);
              temp4Add += " ";
            }
            if (nTemp == 4){
              temp5Add += (thermX[iadd],HEX);
              temp5Add += " ";
            }
            if (nTemp == 5){
              temp6Add += (thermX[iadd],HEX);
              temp6Add += " ";
            }
            
          }
          if (serialPrintThermo){
            Serial.print("\n");
            Serial.print("temp1Add: ");
            Serial.print(temp1Add);
            Serial.print("\n");
            Serial.print("temp2Add: ");
            Serial.print(temp2Add);
            Serial.print("\n");
            Serial.print("\n");
          }
      }
      //temp_sensors.getAddress(thermo1,i);
    };
  
};
///////////////////////////////////////////////
///////////////////////////////////////////////
// UPDATE TEMPERATURE
void updateTemperature(float selTemp, float selDeltaPos, float selDeltaNeg){
  selectTempPos = selTemp+selDeltaPos;
  selectTempNeg = selTemp-selDeltaNeg;
}
///////////////////////////////////////////////
///////////////////////////////////////////////
//WRITE MEMORY
void writeMemory(float selTemp, boolean recMemory, boolean serialPrintMem){
  //record the new value
  if (recMemory){
    //compare if the new value is really new and write in the memory
    
    if(selectTemp != (EEPROM.read(0)+(float(EEPROM.read(1))/100))-50){
      if (serialPrintMem){
        Serial.print("EEPROM 1 ");
        Serial.print(EEPROM.read(0));
        Serial.print(".");
        Serial.print(EEPROM.read(1));
        Serial.print(".\n");
      }
      //write the new value into the memory, only Int positive is allowed,
      //the +50 is to allowed to save negative values
      EEPROM.write(0, int(selTemp)+50);
      //the decimal part
      EEPROM.write(1, ((selTemp*100)-int(selTemp)*100));
      EEPROM.commit();
      if (serialPrintMem){
        Serial.print("EEPROM 2 ");
        Serial.print(EEPROM.read(0));
        Serial.print(".");
        Serial.print(EEPROM.read(1));
        Serial.print(".\n");
      }
    }
  }
}

///////////////////////////////////////////////
///////////////////////////////////////////////
/* Setup

*/
void setup() {

  Serial.begin(115200);

  //nodemcu RELAY of sonoff
  pinMode(relay, OUTPUT);
  digitalWrite(relay,HIGH);

  //start the memory
  EEPROM.begin(EEPROM_SIZE);
 
  //Version without WifiManager
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //Default 180 seconds
  wifiManager.setTimeout(180);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP_ESP_D6C81D", "senha1234")) {
    if (serialPrint){
      Serial.println("failed to connect and hit timeout");
    }
    delay(3000);
    client.publish("geladeira/supervisorio", "delay autoconnect");
    //reset and try again, or maybe put it to deep sleep
    //ESP.reset();
    //delay(5000);
    if (serialPrint){
      Serial.print("setup connect \n");
    }
  }

  //verify the temperature in the memory and recovery the memory
  if (recordMemory){
    if (serialPrintMem){
      Serial.print("Select Temp: ");
      Serial.print(selectTemp);
      Serial.print(" ");
      Serial.print(EEPROM.read(0));
      Serial.print(".");
      Serial.print(EEPROM.read(1));
      Serial.print("\n");
    }
    //select memory
    if (selectTemp != (EEPROM.read(0)+(float(EEPROM.read(1))/100))-50){  
      //the "-50", the EEPROM only save from 0-256, therefore it is added an
      //delta of 50degC to save in the memory only positive number.
      selectTemp = EEPROM.read(0)+(float(EEPROM.read(1))/100)-50;

      if (serialPrintMem){
            Serial.print("EEPROM 0: ");
            Serial.print(EEPROM.read(0));
            Serial.print(" 0: ");
            Serial.print((float(EEPROM.read(1))));
            Serial.print(" 10: ");
            Serial.print((float(EEPROM.read(1))/10));
            Serial.print(" 100: ");
            Serial.print((float(EEPROM.read(1))/100));
            Serial.print(" final 0 : ");
            Serial.print(EEPROM.read(0)+float(EEPROM.read(1)));
            Serial.print(" final 10: ");
            Serial.print(EEPROM.read(0)+(float(EEPROM.read(1))/10));
            Serial.print(" final 100: ");
            Serial.print(EEPROM.read(0)+(float(EEPROM.read(1))/100));
            Serial.print(" serial: ");
            Serial.print(selectTemp);
            Serial.print("\n");
          }
    }else{
      //set the default value during power up
      selectTemp = 18;
    }
    
    //delay during power up
    if (delayPowerUp != EEPROM.read(2)){
        delayPowerUp = EEPROM.read(2);
    }

    //sensor selection number
    if (selectSensor != EEPROM.read(3)){
        selectSensor = EEPROM.read(3);
    }
  }
  //update temperature
  updateTemperature(selectTemp,selectDeltaPos,selectDeltaNeg);

  //Inicia o servidor web
  servidorWeb.on("/", handleRootInicial);
  //web simplificado:
  //servidorWeb.on("/", handleControl2);
  
  //Reset Button
  servidorWeb.on("/ESPReset", []() {
    WebReset();
    client.publish("geladeira/supervisorio", "delay reset");
    delay(2000);
    ESP.reset();
  });

  // MQTT Button
  servidorWeb.on("/MQTTButtonWeb", []() {
    if (mqttButton == true) {
      mqttButton = false;
    } else {
      mqttButton = true;
    }
    //carrega a pagina anterior
    handleControl();
  });
  


  //Vai para o controle
  servidorWeb.on("/htmlControl", []() {
    //carrega a pagina de controle
    handleControl();
  });

  //Vai para o controle
  servidorWeb.on("/countTemp", []() {
    //count temp device
    sensoresTempCount();
    //carrega a pagina de controle
    handleRootInicial();
  });

  servidorWeb.on("/tempPost", []() {
    //handlePostForm();
    boolean changeOccur = false;
    
    //Select sensor
    if (servidorWeb.arg("setSensor") != "")
    { changeOccur = true;
      str_selectTemp = servidorWeb.arg("setSensor");
      selectSensor = str_selectTemp.toInt();

      //write the memory the select sensor
      if (selectSensor != EEPROM.read(3)){
        if (selectSensor<11){
          EEPROM.write(3,selectSensor );
          //commit the EEPROM
          EEPROM.commit();
        }
        
      }
    }
    
    //Temperature
    if (servidorWeb.arg("setTemp") != "")
    { changeOccur = true;
      str_selectTemp = servidorWeb.arg("setTemp");
      selectTemp = str_selectTemp.toFloat();
      //write the memory
      writeMemory(selectTemp, recordMemory, serialPrintMem);
    };
    
    //Temperature pos
    if (servidorWeb.arg("setTempPos") != "")
    { changeOccur = true;
      str_selectDeltaPos = servidorWeb.arg("setTempPos");
      selectDeltaPos = str_selectDeltaPos.toFloat();
    }
    
    
    //Temperature neg
    if (servidorWeb.arg("setTempNeg") != "")
    { changeOccur = true;
      str_selectDeltaNeg = servidorWeb.arg("setTempNeg");
      selectDeltaNeg = str_selectDeltaNeg.toFloat();
    }
    
    //update the temperatures
    if (changeOccur){
      updateTemperature(selectTemp,selectDeltaPos,selectDeltaNeg);
      //selectTempPos = selectTemp+selectDeltaPos;
      //selectTempNeg = selectTemp-selectDeltaNeg;
    };
    
    //carrega a pagina de controle
    handleRootInicial();
  });

  servidorWeb.on("/delayPwrUpForm", []() {
    handlePostForm();
    boolean changeOccur = false;
    if (servidorWeb.arg("str_delayPwrUp") != "")
    { changeOccur = true;
      str_dlyPwrUp = servidorWeb.arg("str_delayPwrUp");
      delayPowerUp = str_dlyPwrUp.toInt();

      //write the memory
      if (delayPowerUp != EEPROM.read(2)){
        if (delayPowerUp<249){
          EEPROM.write(2,delayPowerUp );
        }else{
          //full time to power up
          EEPROM.write(2,249);
        }
        //commit the EEPROM
        EEPROM.commit();
      }
    }
     //carrega a pagina de controle
      handleControl();
  });
  
  servidorWeb.on("/postForm", []() {
    handlePostForm();
    boolean changeOccur = false;
    if (servidorWeb.arg("mqttServer") != "")
    { changeOccur = true;
      server_mqtt = servidorWeb.arg("mqttServer");
    }

    server_mqtt.toCharArray(servidor_mqtt, server_mqtt.length() + 1);
    if (servidorWeb.arg("mqttPort") != "")
    { changeOccur = true;
      server_mqtt_porta = servidorWeb.arg("mqttPort");
    }
    server_mqtt_porta.toCharArray(servidor_mqtt_porta, server_mqtt_porta.length() + 1);
    if (servidorWeb.arg("mqttUser") != "")
    { changeOccur = true;
      server_mqtt_usuario = servidorWeb.arg("mqttUser");
    }
    server_mqtt_usuario.toCharArray(servidor_mqtt_usuario, server_mqtt_usuario.length() + 1);
    //Mqtt Password
    if (servidorWeb.arg("mqttPassword") != "")
    { changeOccur = true;
      server_mqtt_senha = servidorWeb.arg("mqttPassword");
    }
    server_mqtt_senha.toCharArray(servidor_mqtt_senha, server_mqtt_senha.length() + 1);
    //Notice
    if (servidorWeb.arg("nOtice") != "")
    { changeOccur = true;
      Notice_msg = servidorWeb.arg("nOtice");
    }
    Notice_msg.toCharArray(Notice, Notice_msg.length() + 1);

    
    //Inicia o MQTT configuracao
    if (changeOccur == true) {
      int portaInt = atoi(servidor_mqtt_porta); //convert string to number
      client.setServer(servidor_mqtt, portaInt);
      client.setCallback(retornoMQTTSub);
      //Inicia os subscribes
      mqttSubscribe();
    }

    //carrega a pagina de controle
    handleControl();
  });

  servidorWeb.on("/htmlRootInicial", []() {
    
    //carrega a pagina de controle
    handleRootInicial();
  });

  //Ativa o webserver
  servidorWeb.begin();

  // Nome MDS
  //InicializaMDNS();

  // OTA
  InicializaServicoAtualizacao();

  // Inicializa biblioteca de temperatura
  sensoresTempCount();
  //start the time
  requestTempSensor.restart();
  
  //request the temp read
  temp_sensors.requestTemperatures();
  
  cmdrelay.restart();
  
  //Inicia o MQTT configuracao
  int portaInt = atoi(servidor_mqtt_porta); //convert string to number
  client.setServer(servidor_mqtt, portaInt);
  client.setCallback(retornoMQTTSub);
  //Inicia os subscribes
  mqttSubscribe();

  //Watchdog

  client.publish("geladeira/supervisorio", "delay setup");
  delay(50);
  //delay of power up to avoid any power up issue
  delay(delayPowerUp*1000);
}
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
/* Loop

*/
void loop() {

  //Try to connect in the Wifi server each 60 seconds
  if (WiFi.status() != WL_CONNECTED) {
    //if (millis() - wifireconnect_timeout > 500) {
    //wifireconnect_timeout = millis();
    //WiFi.persistent(false);
    //WiFi.disconnect();
    //delay(2000);
    //WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, passwd);
    delay(500);
    client.publish("geladeira/supervisorio", "delay wifi connect");
    if (serialPrint){
      Serial.print("Connecting again ");
      Serial.print(WiFi.status());
      Serial.print(".\n");
    }
    //countWire++;

    // Nome MDS
    //InicializaMDNS();
    //}
  }  else {
    if (serialPrint){
      Serial.print("Connected ");
      Serial.print(WiFi.status());
      Serial.print(".\n");
    }
  }

  //Try to connect in the MQTT server each 60 seconds if it has a wireless connection
  //if ((WiFi.status() == WL_CONNECTED) and (!client.connected())) {
  if (!client.connected() and (mqttButton == true)) {
    // reconnect the mqqt
    //troubleshooting the next line
    reconnectMQTT();
    if (serialPrint){
      Serial.println("reconectar mqtt");
    }
    
    //countMQTT++;
    //}
    client.publish("geladeira/supervisorio", "delay - mqtt reconnect");
  }

  //Calculo o tempo transcorrido
  updateTempo();
  timemeasure = String("Tempo (D:H:M:mS): " + String(days) + ":" + String(hours) + ":" + String(minute) + ":" + String(second) + ", total: " + String(fulltime) );

  //MQTT
  client.publish("geladeira/time", timemeasure.c_str());

  /*
   * seta temperatura via mqqt
   */
   if (mqttButton and mqttSetParameter){
     // set temperature
     if (serialPrintTemp){
          Serial.print("mqtt topico: ");
          Serial.print(mqtt_topico);
          Serial.print(" mqtt sub temp:  ");
          Serial.print(mqtt_sub_tempPos);
          Serial.print(" select temp: ");
          Serial.print(selectTemp);
          Serial.print("\n");
          }
          
       if (strcmp(mqtt_topico.c_str(), mqtt_sub_temp) == 0){
        // update the set temperature if necessary
          //if (mqtt_mensagem.toFloat() != selectTemp){
            selectTemp = mqtt_mensagem.toFloat();
            //update temperature
            updateTemperature(selectTemp,selectDeltaPos,selectDeltaNeg);
          //}
       }
       
       // set temperature positive
       if (strcmp(mqtt_topico.c_str(), mqtt_sub_tempPos) == 0){
        // update the set temperature if necessary
          //if (mqtt_mensagem.toFloat() != selectDeltaPos){
            selectDeltaPos = mqtt_mensagem.toFloat();
            //update temperature
            updateTemperature(selectTemp,selectDeltaPos,selectDeltaNeg);
          //}
       }
       // set temperature negative
       if (strcmp(mqtt_topico.c_str(), mqtt_sub_tempNeg) == 0) {
        // update the set temperature if necessary
          //if (mqtt_mensagem.toFloat() != selectDeltaNeg){
            selectDeltaNeg = mqtt_mensagem.toFloat();
            //update temperature
            updateTemperature(selectTemp,selectDeltaPos,selectDeltaNeg);
          //}
       }
       //confirm the set temperature
       if (strcmp(mqtt_topico.c_str(), mqtt_sub_ConfSetTemp) == 0) {
        // update the confirmation temperature if the previous is false
          if ((mqtt_mensagem != saveMemMQTT) and saveMemMQTT == "false"){
            if (serialPrintMem){
            Serial.print("mqtt_mensagem: ");
            Serial.print(mqtt_mensagem);
            Serial.print(" saveMemMQTT " );
            Serial.print(saveMemMQTT);
            Serial.print("\n");
            }
            //save the new state
            saveMemMQTT = "true";
            
            //write the memory
            writeMemory(selectTemp, recordMemory, serialPrintMem);
          }
          if ((mqtt_mensagem != saveMemMQTT) and saveMemMQTT == "true"){
            //return to false, do nothing;
            saveMemMQTT = "false";
            if (serialPrintMem){
            Serial.print("mqtt_mensagem: ");
            Serial.print(mqtt_mensagem);
            Serial.print(" saveMemMQTT " );
            Serial.print(saveMemMQTT);
            Serial.print("\n");
            }
          }
       }
       
   }
  /*
   * Select Sensor number
   */
  //Verifica se ha algum sensor de temperatura
  if (tempDeviceCount > 0) {
    selSensorTemp = temp_sensors.getTempCByIndex(selectSensor-1);    
  }else{
    selSensorTemp = 123; 
  }
  /*
   *  Ativar o rele
   */
   if (cmdrelay.hasPassed(1000)){
      cmdrelay.restart();
     if (selSensorTemp>=selectTempPos)
     {
      //ligado
      relaystatus = true;
      digitalWrite(relay,LOW);
     }
     else if (selSensorTemp<=selectTempNeg)
     {
      //desligado
      relaystatus = false;
      digitalWrite(relay,HIGH);
     }
     client.publish(mqtt_topico_pub_relay, String(relaystatus).c_str());
   }
  
  /*
      Leitura de Temperatura
  */
  //Verifica se ha algum sensor de temperatura
  if (tempDeviceCount > 0) {
    //if (fulltime - tempoTempSensor > delayTempSensor) {
    /*
    if (requestTempSensor.hasPassed(1000)){
      requestTempSensor.restart();
      //request the temp read
      temp_sensors.requestTemperatures();
    }
    */
    if (readTempSensor.hasPassed(1000)){
      readTempSensor.restart();
      
      switch (tempDeviceCount) {
        case 6:
          temp6 = temp_sensors.getTempCByIndex(5);
          client.publish(mqtt_topico_pubTemp6, String(temp6).c_str());
        case 5:
          temp5 = temp_sensors.getTempCByIndex(4);
          client.publish(mqtt_topico_pubTemp5, String(temp5).c_str());
        case 4:
          temp4 = temp_sensors.getTempCByIndex(3);
          client.publish(mqtt_topico_pubTemp4, String(temp4).c_str());
        case 3:
          temp3 = temp_sensors.getTempCByIndex(2);
          client.publish(mqtt_topico_pubTemp3, String(temp3).c_str());
        case 2:
          temp2 = temp_sensors.getTempCByIndex(1);
          client.publish(mqtt_topico_pubTemp2, String(temp2).c_str());
        case 1:
          temp1 = temp_sensors.getTempCByIndex(0);
          client.publish(mqtt_topico_pubTemp1, String(temp1).c_str());
          break;
        default:
          break;
      }

      //public the setting temperature
      client.publish(mqtt_topico_pubTempset, String(selectTemp).c_str());
      client.publish(mqtt_topico_pubTempsetNeg, String(selectDeltaNeg).c_str());
      client.publish(mqtt_topico_pubTempsetPos, String(selectDeltaPos).c_str());
      client.publish(mqtt_topico_pubCalcTempNeg, String(selectTempNeg).c_str());
      client.publish(mqtt_topico_pubCalcTempPos, String(selectTempPos).c_str());
      
      //request the temp read
      temp_sensors.requestTemperatures();
    }
  
    /*
      // faz a loop entre o sensores
      for (int i = 0; i < tempDeviceCount; i++) {
        //temp1
        if (i == 0) {
          temp1 = temp_sensors.getTempCByIndex(i);
          client.publish("geladeira/temp1", String(temp1).c_str());
        }
      }
      */
    
  }

  
  
  if (serialPrintTemp){
   Serial.print(selSensorTemp);
   Serial.print("\n");
  }
  //Servidor Web
  servidorWeb.handleClient();
  //Wifi MQTT;
  if (mqttButton){
    client.loop();
  }


}
