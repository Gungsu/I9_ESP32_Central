#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <Update.h>
#include "namedMesh.h"
#include <UnicViewAD.h>

#define   MESH_SSID       "rede_mesh_i9"
#define   MESH_PASSWORD   "deumaoito"
#define   MESH_PORT       5555
#define   RXD2            16
#define   TXD2            17
#define   PIN_EMERG       23
#define   PIN_MOT_UP      19
#define   PIN_MOT_DOWN    18

Scheduler userScheduler;
namedMesh mesh;

void distCheck();

LCM Lcm(Serial2);

LcmVar display_cmds(1);
LcmVar max_ERRO(4);
LcmVar max_STEP(7);
LcmVar max_DIST(1);

uint16_t sens[40];
uint8_t numTotalSensor = 40;
uint16_t maxError = 500;
bool ST_stop,ST_up,ST_down,Alerta;

String nodeName = "central_i9"; // Name needs to be unique

Task taskSendMessage( TASK_SECOND*30, TASK_FOREVER, []() {
    String msg = String("This is a message from: ") + nodeName + String(" for logNode");
    String to = "logNode";
    mesh.sendSingle(to, msg); 
});

void displayWrite(String from, String Value) {
  String c_from = from.substring(1);
  uint8_t sensID = atoi(c_from.c_str());
  uint16_t sensVAL = atoi(Value.c_str());
  LcmVar sens_l(sensID*10);
  sens_l.write(sensVAL);
  sens[sensID-1] = sensVAL;
  distCheck();
}

void initSens() {
  uint8_t c = 40;
  while(c--) {
    sens[c] = 0;
    if(c<4) {
      LcmVar sens_M((c+1)*10+3);
      sens_M.write(0);
    }
  }
  max_ERRO.write(maxError);
}

void distCheck() {
  uint8_t cont;
  uint16_t maior = 0;
  uint16_t menor = 0xAFFF;
  uint16_t maiorID,menorID;
  uint16_t readAct;
  for (cont=0;cont<40;cont++) {
    readAct = sens[cont];
    if (readAct != 0) {
      if (readAct > maior) {
        maior = readAct;
        maiorID = cont;
      }
      if (readAct < menor) {
        menor = readAct;
        menorID = cont;
      }
    }
  }
  max_DIST.write(maior-menor);
  if (maior-menor > maxError) {
    LcmVar sens_M((maiorID+1)*10+3);
    LcmVar sens_E((menorID+1)*10+3);
    sens_M.write(1);
    sens_E.write(1);
    if(ST_stop == 0) {
      ST_stop = 1;
      Alerta = true;
      digitalWrite(PIN_EMERG,ST_stop);
    }
  }
}

void reset() {
  initSens();
  Alerta = false;
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200,SERIAL_8N1, RXD2, TXD2);

  pinMode(PIN_EMERG,OUTPUT);
  pinMode(PIN_MOT_UP,OUTPUT);
  pinMode(PIN_MOT_DOWN,OUTPUT);

  ST_stop = ST_up = ST_down = 1;
  digitalWrite(PIN_EMERG,ST_stop);
  digitalWrite(PIN_MOT_UP,ST_up);
  digitalWrite(PIN_MOT_DOWN,ST_down);

  Lcm.begin();

  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.setName(nodeName); // This needs to be an unique name! 

  /*mesh.onReceive([](uint32_t from, String &msg) {
    Serial.printf("Received message by id from: %u, %s\n", from, msg.c_str());
    
  });*/

  mesh.onReceive([](String &from, String &msg) {
    displayWrite(from,msg);
  });

  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
  });

  initSens();

  //userScheduler.addTask(taskSendMessage);
  //taskSendMessage.enable();
}

void loop() {
  // it will run the user scheduler as well
  mesh.update();
  if (max_ERRO.available()){
    maxError = max_ERRO.getData();
  }

  if (display_cmds.available()) {
    switch (display_cmds.getData()) {
        case 0: //STOP
          ST_stop = 1;
          digitalWrite(PIN_EMERG,ST_stop);
          break;
        case 1: //DOWN  RESET PROVISORIO
          ST_down = !ST_down;
          digitalWrite(PIN_MOT_DOWN,ST_down);
          break;
        case 2: //UP
          ST_up = !ST_up;
          digitalWrite(PIN_MOT_UP,ST_up);
          break;
        case 3: //PLAY
          if (!Alerta) {
            ST_stop = 0;
            digitalWrite(PIN_EMERG,ST_stop);
          }
          break;
        case 4: //RESET
          reset();
          break;
        default:
          break;
    }
  }
  //delay(500);
}
