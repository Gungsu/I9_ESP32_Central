#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <Update.h>
#include "namedMesh.h"
#include <UnicViewAD.h>
#include "objSensor.h"
#include <list>

#define   MESH_SSID       "rede_mesh_i9"
#define   MESH_PASSWORD   "deumaoito"
#define   MESH_PORT       5555
#define   RXD2            16
#define   TXD2            17
#define   PIN_EMERG       23
#define   PIN_MOT_UP      19
#define   PIN_MOT_DOWN    18
#define   DISPLAYVERSION  1

Scheduler userScheduler;
namedMesh mesh;

objSensor objSensores;

void distCheck(uint16_t nValue);
void but_stop();

LCM Lcm(Serial2);

LcmVar version(5);
uint64_t ver64;
LcmVar display_cmds(9);
LcmVar max_ERRO(4);
LcmVar max_STEP(7);
LcmVar max_DIST(1);
LcmVar iconUp(2);
LcmVar iconDown(3);
LcmVar vpLoad(55);
LcmVar iconPlay(6);

LcmVar vpvalue0(98);
LcmVar vpvalue90(1098);

uint8_t numTotalSensorAtivo;
uint16_t maxError = 500;
uint16_t maxStep = 2000;
unsigned long timeCont,timeOld;
uint16_t maior = 0;
uint16_t menor = 0xAFFF;
bool ST_stop,ST_up,ST_down,Alerta;
bool waitAlerta;

String nodeName = "central_i9"; // Name needs to be unique

void displayWrite(String from, String Value) {
  String c_from = from.substring(1);
  uint8_t sensID = atoi(c_from.c_str());
  uint16_t sensVAL = atoi(Value.c_str());
  objSensores.write(sensID,sensVAL);
  distCheck(sensVAL);
}

void initSens() {
  uint8_t c = 40;
  //Retangulos
  Lcm.changePicId(0);
  objSensores.posi_sens();
  max_ERRO.write(maxError);
  max_STEP.write(maxStep);
  objSensores.dist_sens(1, 0, 1);
}

void distCheck(uint16_t nValue) {
  if (Alerta) 
    return;
  /***VVV  CHECK IF DIFERENCE ON NODES VVV****/
  std::list<uint32_t> numBerNodes = mesh.getNodeList(); 
  uint8_t nodesSize = numBerNodes.size();
  if (nodesSize != numTotalSensorAtivo) {
    numTotalSensorAtivo = nodesSize;
    objSensores.dist_sens(numTotalSensorAtivo, 0, 1);
  }
  /***^^^  CHECK IF DIFERENCE ON NODES ^^^****/
  uint8_t cont,cont2;
  maior = 0;
  menor = 0xAFFF;
  while (cont<40 && cont2<nodesSize) {
    if (objSensores.sensores[cont].ativo != 0) {
      cont2++;
      if (objSensores.sensores[cont].value_num > maior)
      {
        maior = objSensores.sensores[cont].value_num;
        objSensores.sensInAlert1 = objSensores.sensores[cont].ID_value_num;
      }
      if (objSensores.sensores[cont].value_num < menor)
      {
        menor = objSensores.sensores[cont].value_num;
        objSensores.sensInAlert2 = objSensores.sensores[cont].ID_value_num;
      }
    }
    cont++;
  }
  max_DIST.write(maior-menor);
  if ((maior - menor > maxError || maior >= maxStep) && waitAlerta) // maior que Distancia Step e Erro
  {
    objSensores.act_alerta(1);
    Alerta = true;
    digitalWrite(PIN_EMERG, 0);
    but_stop();
  }
}

void reset() {
  but_stop();
  Alerta = false;
  ST_stop = 1;
  digitalWrite(PIN_EMERG, ST_stop);
  objSensores.act_alerta(0);
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600,SERIAL_8N1, RXD2, TXD2);

  pinMode(PIN_EMERG,OUTPUT);
  pinMode(PIN_MOT_UP,OUTPUT);
  pinMode(PIN_MOT_DOWN,OUTPUT);

  ST_stop = ST_up = ST_down = 1;
  digitalWrite(PIN_EMERG,ST_stop);
  digitalWrite(PIN_MOT_UP,ST_up);
  digitalWrite(PIN_MOT_DOWN,ST_down);
  uint8_t x = 30;
  while(x--) {
    delay(100);
    Serial.print("T");
  }
  Serial.println();
  Lcm.begin();

  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.setName(nodeName); // This needs to be an unique name!

  mesh.onReceive([](String &from, String &msg) {
    displayWrite(from,msg);
  });

  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
  });

  initSens();
  Serial.println(" INICIANDO ");
}

void but_stop() {
  waitAlerta = false;
  iconPlay.write(0);

  ST_up = 1; // turnOff UP
  iconUp.write(!ST_up); 
  digitalWrite(PIN_MOT_UP, ST_up);

  ST_down = 1;
  iconDown.write(!ST_down); // turnOff down
  digitalWrite(PIN_MOT_DOWN, ST_down);
}

void but_up() {
  if (!ST_down)
  {
    ST_down = !ST_down;
    iconDown.write(!ST_down);
    digitalWrite(PIN_MOT_DOWN, ST_down);
    delay(600);
  }
  ST_up = !ST_up;
  iconUp.write(!ST_up);
  digitalWrite(PIN_MOT_UP, ST_up);
}

void loop() {
  // it will run the user scheduler as well
  mesh.update();

  if (max_ERRO.available()){
    maxError = max_ERRO.getData();
  }

  if (max_STEP.available()) {
    maxStep = max_STEP.getData();
  }

  if (Serial.available()) {
    Serial.print(Serial.readString());
  }

  if (Alerta) {
    if (millis() >= timeOld + 1000)
    { // if timeCont<timeOld
      timeOld = millis();
      objSensores.act_alerta(1);
    }
  }
  
  if (display_cmds.available()) {
    switch (display_cmds.getData()) {
        case 0: //STOP
          but_stop();
          //Serial.println("STOP");
          break;
        case 1: //PLAY
          if (Alerta) break;
          if(waitAlerta) {
            but_stop();
          } else {
            waitAlerta = true;
            iconPlay.write(1);
            delay(600);
            if(ST_up) {
              but_up();
            }
          }
          //Serial.println("PLAY");
          break;
        case 2: //UP
            if (Alerta) break;
            but_up();
            // Serial.println("UP");
            break;
          case 3: // DOWN  RESET PROVISORIO
            if (Alerta)
              break;
            if (!ST_up)
            {
              ST_up = !ST_up;
              iconUp.write(!ST_up);
              digitalWrite(PIN_MOT_UP, ST_up);
              delay(600);
            }
            ST_down = !ST_down;
            iconDown.write(!ST_down);
            digitalWrite(PIN_MOT_DOWN, ST_down);
            // Serial.println("DOWN");
            break;
          case 4: // RESET
            // Serial.println("RESET");
            reset();
            break;
          default:
            break;
          }
  }
}
