#include <Arduino.h>
#include <UnicViewAD.h>

typedef struct
{
    //geral
    uint16_t value_PP;
    bool visible;
    //numDisplay Dist
    uint16_t pos_x;
    uint16_t pos_y;
    uint16_t value_VP;
    uint16_t value_num;
    uint16_t VP_Pos_x;
    uint16_t VP_Pos_y;
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
    uint16_t ID_value_VP;
    uint16_t ID_value_num;
    uint16_t ID_VP_Pos_x;
    uint16_t ID_VP_Pos_y;
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
    st_sensor sensores[40];
    void ocult(uint8_t sens, bool value);
    void dist_sens(uint8_t qtSens, uint8_t inici, bool apagar);
    objSensor();
    ~objSensor();
};

void convertPol_Rad(uint16_t &R_X, uint16_t &DG_Y, uint8_t tipo);
