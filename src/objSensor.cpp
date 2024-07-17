#include "objSensor.h"
#include <UnicViewAD.h>
#include <math.h>

#define PRETO 0x0000
#define BRANCO 0xFFFF

enum tip
{
    NumDISP_DIST,
    RETANGULO,
    NumDISP_ID
};

objSensor::objSensor()
{
    dist_sens_init(40);
}

objSensor::~objSensor()
{
}

void objSensor::dist_sens(uint8_t qtSens, uint8_t inici, bool apagar)
{
    if(qtSens == 0) {
        qtSens = 1;
    }
    float distriF = (40.00 / qtSens)+0.6;
    uint8_t cont = inici;
    uint8_t nextToCh = inici;
    uint8_t colocado = 0;
    uint8_t vFal_Col, falt_colocado = 0;
    uint16_t distri = distriF;
    vFal_Col = distri * qtSens - 40;
    if (vFal_Col>40) {
        vFal_Col = 0;
    }
    while (cont < 40)
    {
        //Serial.print(cont);
        LcmVar ID_PP2(sensores[cont].ID_value_VP); // PARA IDENTIFICAR A REFERENCIA DO SENSOR NO DISPLAY
        LcmVar DISTANCIA_VP(sensores[cont].value_VP);
        if(apagar) {
            if ((cont == 0 || cont == nextToCh) && colocado<qtSens)
            {
                ocult(cont, 0);
                nextToCh += distri;
                colocado++;
                sensores[cont].ID_value_num = colocado;
                ID_PP2.write(sensores[cont].ID_value_num);
            }
            else
            {
                ocult(cont, 1);
                DISTANCIA_VP.write(0); //ZERA o valor da distancia quando ocultar
            }
        } else {
            if ((cont == 0 || cont == nextToCh) && colocado < qtSens)
            {
                while (sensores[cont].ativo == true)
                {
                    cont++;
                }
                ocult(cont, 0);
                nextToCh += distri;
                colocado++;
                sensores[cont].ID_value_num = colocado;
                ID_PP2.write(sensores[cont].ID_value_num);
                sensores[cont].ID_put = false;
            }
        }
        cont++;
    }
    vFal_Col = qtSens - colocado;
    if (apagar && vFal_Col != 0 && vFal_Col < 40)
    {
        dist_sens(vFal_Col, 1, 0);
    }
}

void objSensor::dist_sens_init(uint8_t qtSens)
{
    uint8_t c = 0;
    uint16_t distri = 9;
    while (c < 40)
    {
        sensores[c].value_PP = c * 100 + 100; //PP do numeric displa que marca distancia
        sensores[c].angle = c * distri;

        /*DIST*/
        sensores[c].value_VP = sensores[c].value_PP - 0x02;
        sensores[c].VP_Pos_x = sensores[c].value_PP + 0x01;
        sensores[c].VP_Pos_y = sensores[c].value_PP + 0x02;
        sensores[c].VP_Color = sensores[c].value_PP + 0x03;
        if (c == 0 || c == 20)
        {
            sensores[c].raio_ND_Dist += 5;
        }
        convertPol_Rad(sensores[c].raio_ND_Dist, sensores[c].angle, NumDISP_DIST);
        sensores[c].pos_x = sensores[c].raio_ND_Dist + 20;
        sensores[c].pos_y = sensores[c].angle + 8;

        /*ID*/
        sensores[c].ID_angle = c * distri;
        sensores[c].ID_value_PP = sensores[c].value_PP + 0x1E;
        sensores[c].ID_value_VP = sensores[c].value_PP - 0x04;
        sensores[c].ID_VP_Pos_x = sensores[c].value_PP + 0x1F;
        sensores[c].ID_VP_Pos_y = sensores[c].value_PP + 0x20;
        sensores[c].ID_VP_Color = sensores[c].value_PP + 0x21;

        convertPol_Rad(sensores[c].ID_raio_ND_Dist, sensores[c].ID_angle, NumDISP_ID);
        sensores[c].ID_pos_x = sensores[c].ID_raio_ND_Dist + 4;
        sensores[c].ID_pos_y = sensores[c].ID_angle + 8;

        /*RET*/
        sensores[c].RET_angle = c * distri;
        sensores[c].ret_VP = 11 + c;
        sensores[c].RET_VP_Pos_x = sensores[c].value_PP + 0x3D;
        sensores[c].RET_VP_Pos_y = sensores[c].value_PP + 0x3E;

        convertPol_Rad(sensores[c].RET_raio_ND_Dist, sensores[c].RET_angle, RETANGULO);
        sensores[c].RET_pos_x = sensores[c].RET_raio_ND_Dist;
        sensores[c].RET_pos_y = sensores[c].RET_angle;
        c++;
    }
}

void objSensor::ocult(uint8_t sens, bool value)
{
    LcmVar PP_pos_x(sensores[sens].VP_Pos_x);
    LcmVar PP_pos_y(sensores[sens].VP_Pos_y);
    LcmVar VP_Color(sensores[sens].VP_Color);
    LcmVar PP_ID_pos_x(sensores[sens].ID_VP_Pos_x);
    LcmVar PP_ID_pos_y(sensores[sens].ID_VP_Pos_y);
    LcmVar VP_ID_Color(sensores[sens].ID_VP_Color);
    LcmVar retang(sensores[sens].ret_VP);
    if(value) {
    //764,464
        PP_pos_x.write(760);
        PP_pos_y.write(464);
        VP_Color.write(BRANCO);
        PP_ID_pos_x.write(760);
        PP_ID_pos_y.write(464);
        VP_ID_Color.write(BRANCO);
        retang.write(0);
        sensores[sens].ativo = false;
    } else {    
        PP_pos_x.write(sensores[sens].pos_x);
        PP_pos_y.write(sensores[sens].pos_y);
        VP_Color.write(PRETO);
        PP_ID_pos_x.write(sensores[sens].ID_pos_x);
        PP_ID_pos_y.write(sensores[sens].ID_pos_y);
        VP_ID_Color.write(PRETO);
        retang.write(1);
        sensores[sens].ativo = true;
    }
}

void objSensor::posi_sens() {
    uint8_t c = 40;
    while (c--)
    {
        if (c < 40)
        {
            LcmVar retang(sensores[c].ret_VP);
            retang.write(1);

            LcmVar PP_pos_x(sensores[c].VP_Pos_x);
            LcmVar PP_pos_y(sensores[c].VP_Pos_y);
            LcmVar PP_set_vp(sensores[c].value_PP);
            PP_pos_x.write(sensores[c].pos_x);
            PP_pos_y.write(sensores[c].pos_y);
            PP_set_vp.write(sensores[c].value_VP);

            LcmVar PP_ID_pos_x(sensores[c].ID_VP_Pos_x);
            LcmVar PP_ID_pos_y(sensores[c].ID_VP_Pos_y);
            LcmVar PP_ID_set_vp(sensores[c].ID_value_PP);
            PP_ID_pos_x.write(sensores[c].ID_pos_x);
            PP_ID_pos_y.write(sensores[c].ID_pos_y);
            PP_ID_set_vp.write(sensores[c].ID_value_VP);

            LcmVar PP_RET_pos_x(sensores[c].RET_VP_Pos_x);
            LcmVar PP_RET_pos_y(sensores[c].RET_VP_Pos_y);
            PP_RET_pos_x.write(sensores[c].RET_pos_x);
            PP_RET_pos_y.write(sensores[c].RET_pos_y);
        }
    }
}

void objSensor::write(uint8_t sens, uint16_t valor) {
    uint8_t local = 40;
    uint8_t idSens;
    while(local--) {
        if (sensores[local].ID_value_num == sens) {
            idSens = local;
            /*Serial.print(idSens);
            Serial.print(" ");
            Serial.print(sens);
            Serial.print(" ");
            Serial.println(valor);*/
            break;
        }
    }
    LcmVar vpSensor(sensores[idSens].value_VP);
    sensores[idSens].value_num = valor;
    vpSensor.write(sensores[idSens].value_num);
    if (!sensores[idSens].ID_put) { //ADICIONA IDENTIFICADOR CONFORME NUMERO FISICO DO SENSOR
        LcmVar vpIdSensor(sensores[idSens].ID_value_VP);
        vpIdSensor.write(sensores[idSens].ID_value_num); // ID_value_num
        sensores[idSens].ID_put = true;
    }
}

void objSensor::act_alerta(bool active)
{
    uint8_t local = 0;
    bool next = false;
    if(active) {
        while(local <40 && !changeAlert) {
            if(sensores[local].ativo == true) {
                if ((sensores[local].ID_value_num == sensInAlert1 || sensores[local].ID_value_num == sensInAlert2) && !next)
                {
                    idSens1 = local;
                    next = true;
                }
                else if ((sensores[local].ID_value_num == sensInAlert1 || sensores[local].ID_value_num == sensInAlert2) && next)
                {
                    idSens2 = local;
                }
                if(idSens1 != 0 && idSens2 != 0 && idSens1 != idSens2) {
                    changeAlert = true;
                    break;
                }
            }
            local++;
        }
        // Serial.print("##MENOR ");
        // Serial.println(idSens1);
        // Serial.print("##MAIOR ");
        // Serial.println(idSens2);
        LcmVar retang1(sensores[idSens1].ret_VP);
        LcmVar retang2(sensores[idSens2].ret_VP);
        /*Serial.print("S1: ");
        Serial.print(sensores[idSens1].ret_VP);
        Serial.print(" S2: ");
        Serial.println(sensores[idSens2].ret_VP);*/
        if (toggleAlert) {
            retang1.write(2);
            retang2.write(2);
            toggleAlert = !toggleAlert;
        } else {
            retang1.write(3);
            retang2.write(3);
            toggleAlert = !toggleAlert;
        }
    } else {
        LcmVar retang1(sensores[idSens1].ret_VP);
        LcmVar retang2(sensores[idSens2].ret_VP);
        retang1.write(1);
        retang2.write(1);
        changeAlert = false;
    }
}

void convertPol_Rad(uint16_t &R_X, uint16_t &DG_Y, uint8_t tipo)
{
    int C_X = 527;
    int C_Y = 226;
    if (tipo == RETANGULO) {
        C_X = 527 + 36;
    } else if (tipo == NumDISP_ID){
        C_X = 527 + 36;
    }
    double NG = DG_Y * 3.14159 / 180;
    int R_H = R_X;
    R_X = C_X + R_H * sin(NG);
    DG_Y = C_Y - R_H * cos(NG);
}
