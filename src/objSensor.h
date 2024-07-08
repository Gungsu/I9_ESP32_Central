#include <Arduino.h>
#include <UnicViewAD.h>

typedef struct
{
    //geral
    uint16_t value_PP;
    bool ativo;
    //numDisplay Dist
    uint16_t pos_x;
    uint16_t pos_y;
    uint16_t value_VP;
    uint16_t value_num;
    uint16_t VP_Pos_x;
    uint16_t VP_Pos_y;
    uint16_t VP_Color;
    uint16_t raio_ND_Dist = 210;
    uint16_t angle;

    //RETANGULO
    uint16_t ret_VP;
    uint16_t RET_pos_x;
    uint16_t RET_pos_y;
    uint16_t RET_value_VP;
    uint16_t RET_value_num;
    uint16_t RET_VP_Pos_x;
    uint16_t RET_VP_Pos_y;
    uint16_t RET_raio_ND_Dist = 171;
    uint16_t RET_angle;

    //numDisplay ID
    uint16_t ID_pos_x;
    uint16_t ID_pos_y;
    uint16_t ID_value_PP;
    uint16_t ID_value_VP;
    uint16_t ID_value_num; //REFERENCIA DO SENSOR ###### ID
    uint16_t ID_VP_Pos_x;
    uint16_t ID_VP_Pos_y;
    uint16_t ID_VP_Color;
    uint16_t ID_raio_ND_Dist = 145;
    uint16_t ID_angle;

} st_sensor;

class objSensor
{
private:
    uint16_t raioP = 200;
    uint16_t raioI = 140;
    void dist_sens_init(uint8_t qtSens);

public:
    uint8_t sensInAlert1, sensInAlert2;
    uint8_t idSens1 = 0, idSens2 = 0;
    bool toggleAlert;
    bool changeAlert = false;
    st_sensor sensores[40];
    void ocult(uint8_t sens, bool value);
    void dist_sens(uint8_t qtSens, uint8_t inici, bool apagar);
    void posi_sens();
    void write(uint8_t sens, uint16_t valor);
    void act_alerta(bool active);
    objSensor();
    ~objSensor();
};

void convertPol_Rad(uint16_t &R_X, uint16_t &DG_Y, uint8_t tipo);
