#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <Update.h>
#include "namedMesh.h"
#include <UnicViewAD.h>
#include "objSensor.h"

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

objSensor objSensores;

void distCheck();

LCM Lcm(Serial2);

LcmVar display_cmds(9);
LcmVar max_ERRO(4);
LcmVar max_STEP(7);
LcmVar max_DIST(1);
LcmVar iconUp(2);
LcmVar iconDown(3);
LcmVar vpLoad(55);

//uint16_t sens[40];
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
  distCheck();
}

void initSens() {
  uint8_t c = 40;
  //Retangulos
  Lcm.changePicId(0);
  Serial.println();
  while(c--) {
    if(c<40) {
      LcmVar retang(objSensores.sensores[c].ret_VP);
      retang.write(1);

      LcmVar PP_pos_x(objSensores.sensores[c].VP_Pos_x);
      LcmVar PP_pos_y(objSensores.sensores[c].VP_Pos_y);
      PP_pos_x.write(objSensores.sensores[c].pos_x);
      PP_pos_y.write(objSensores.sensores[c].pos_y);

      LcmVar PP_ID_pos_x(objSensores.sensores[c].ID_VP_Pos_x);
      LcmVar PP_ID_pos_y(objSensores.sensores[c].ID_VP_Pos_y);
      PP_ID_pos_x.write(objSensores.sensores[c].ID_pos_x);
      PP_ID_pos_y.write(objSensores.sensores[c].ID_pos_y);

      LcmVar PP_RET_pos_x(objSensores.sensores[c].RET_VP_Pos_x);
      LcmVar PP_RET_pos_y(objSensores.sensores[c].RET_VP_Pos_y);
      PP_RET_pos_x.write(objSensores.sensores[c].RET_pos_x);
      PP_RET_pos_y.write(objSensores.sensores[c].RET_pos_y);
    }
  }
  max_ERRO.write(maxError);
}

void distCheck() {
  uint8_t cont;
  uint16_t maior = 0;
  uint16_t menor = 0xAFFF;
  uint16_t maiorID,menorID;
  uint16_t readAct = 0;
  for (cont=0;cont<40;cont++) {
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
    LcmVar retang_M((maiorID + 1) * 10 + 3);
    LcmVar retang_E((menorID + 1) * 10 + 3);
    retang_M.write(2);
    retang_E.write(2);
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
  Serial2.begin(9600,SERIAL_8N1, RXD2, TXD2);

  pinMode(PIN_EMERG,OUTPUT);
  pinMode(PIN_MOT_UP,OUTPUT);
  pinMode(PIN_MOT_DOWN,OUTPUT);

  ST_stop = ST_up = ST_down = 1;
  digitalWrite(PIN_EMERG,ST_stop);
  digitalWrite(PIN_MOT_UP,ST_up);
  digitalWrite(PIN_MOT_DOWN,ST_down);
  delay(3000);
  Lcm.begin();
  /*mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.setName(nodeName); // This needs to be an unique name!

  mesh.onReceive([](String &from, String &msg) {
    displayWrite(from,msg);
  });

  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
  });*/

  initSens();
  //Lcm.changePicId(3);
  //userScheduler.addTask(taskSendMessage);
  //taskSendMessage.enable();
}

uint8_t contTest = 1;
bool ocult = 1;

void loop() {
  // it will run the user scheduler as well
  //mesh.update();
  if (max_ERRO.available()){
    maxError = max_ERRO.getData();
    //Serial.print("mE: ");
    //Serial.println(maxError);
  }

  if(contTest>40) {
    contTest = 1;
    ocult = !ocult;
  }
  objSensores.dist_sens(contTest,0,1);
  contTest++;

  if (display_cmds.available()) {
    switch (display_cmds.getData()) {
        case 0: //STOP
          ST_stop = 1;
          digitalWrite(PIN_EMERG,ST_stop);
          //Serial.println("STOP");
          break;
        case 1: //PLAY
          if (!Alerta) {
            ST_stop = 0;
            digitalWrite(PIN_EMERG,ST_stop);
          }
          //Serial.println("PLAY");
          break;
        case 2: //UP
          ST_up = !ST_up;
          //delay(2);
          iconUp.write(!ST_up);
          //delay(2);
          digitalWrite(PIN_MOT_UP,ST_up);
          //Serial.println("UP");
          break;
        case 3: //DOWN  RESET PROVISORIO
          ST_down = !ST_down;
          //delay(2);
          iconDown.write(!ST_down);
          //delay(2);
          digitalWrite(PIN_MOT_DOWN,ST_down);
          //Serial.println("DOWN");
          break;
        case 4: //RESET
          //Serial.println("RESET");
          reset();
          break;
        default:
          break;
    }
  }
  delay(5000);
}
