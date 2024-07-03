/*
# Arquivo:

   UnicViewAD.h

# Projeto:

   UnicViewAD.lcm - Biblioteca Proculus em C

# DescriÃ§Ã£o:

   Esta biblioteca permite que um microcontrolador se comunique facilmente com o Displays
   Inteligentes Proculus, oferecendo um controle mais intuitivo e prÃ¡tico de todas
   as suas funÃ§Ãµes.

   Ela elimina a necessidade de um completo entendimento sobre todos os protocolos de
   comunicaÃ§Ã£o e funcionamento interno do Display, abstraindo esses protocolos para
   pequenas funÃ§Ãµes que garantem facilidade e rapidez de uso.

# Autores:

   * Andreo de Freitas
   * Rafael Moura (revisÃ£o)

# VersÃ£o:

   1.0

# LicenÃ§a:

   Copyright (c) 2020 Proculus Technologies Limited and Victor Vision LTDA

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.

 */

#ifndef UNIC_VIEW_AD_C_H
#define UNIC_VIEW_AD_C_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#if (defined(__PCB__) || defined(__PCM__) || defined(__PCH__) || defined(__PCD__))
#include <STDLIBM.H>
#endif

 /****************************************************************************************************************************************************/
 /****************************************************************************************************************************************************
 * Protocolo
 */

void lcm_init();

void lcm_set_read_timeout(uint32_t timeout_in_ticks);

void lcm_enable_garbage_collector();
void lcm_disable_garbage_collector();

void lcm_crc_enable();

void lcm_set_header(uint16_t header);

uint8_t transfer_file_begin(const char* name, uint32_t name_size, uint32_t file_size);

/****************************************************************************************************************************************************/
/****************************************************************************************************************************************************
 * Dados
 */

#define LCM_EMPTY_RESPONSE 0xFFFF
#define LCM_STRING_TERMINATOR 0xFF

#define LCM_WRITE_REGISTER_COMMAND 0x80
#define LCM_READ_REGISTER_COMMAND 0x81
#define LCM_WRITE_VP_COMMAND 0x82
#define LCM_READ_VP_COMMAND 0x83
#define LCM_WRITE_TREND_CURVE_COMMAND 0x84

typedef struct
{
   uint8_t* p_data_buffer;
   uint8_t data_buffer_index;
   uint8_t rx_type;
   uint16_t rx_address;
   uint8_t rx_length;
   uint32_t garbage_collector_counter;
}lcm_rx_package_t;

/**********************************************************************************
 * VP
 */

uint8_t lcm_write_vp(uint16_t vp, uint16_t value);
uint8_t lcm_write_multiple_vps(uint16_t vp, uint16_t* value, uint8_t length);

uint16_t lcm_read_vp(uint16_t vp);
uint8_t lcm_read_multiple_vps(uint16_t vp, uint16_t* value, uint8_t length);

uint8_t lcm_request_vp(uint16_t vp, uint8_t length);

/**********************************************************************************
 * Register
 */

uint8_t lcm_write_register(uint8_t reg, uint8_t value);
uint8_t lcm_write_multiple_registers(uint8_t reg, uint8_t* value, uint8_t length);

uint8_t lcm_read_register(uint8_t reg);
uint8_t lcm_read_multiple_registers(uint8_t reg, uint8_t* value, uint8_t length);

uint8_t lcm_request_register(uint8_t reg, uint8_t length);

/***********************************************************************************
 * Data Receive
 */

uint8_t lcm_available();

uint8_t lcm_get_rx_type();
uint16_t lcm_get_rx_address();
uint8_t lcm_get_rx_length();

uint16_t lcm_get_data();
uint8_t lcm_get_data_array(uint16_t* buffer, uint8_t length);


/****************************************************************************************************************************************************/
/****************************************************************************************************************************************************
 * Entidades
 */

 /*****************************************************************************************************************************************************
  * Registradores
  */

  /***********************************************************************************
   * Registradores comuns
   */

void lcm_change_pic_id(uint16_t value);
void lcm_change_brightness(uint8_t value);
void lcm_beep_buzzer(uint8_t value);
void lcm_activate_software_control(uint8_t value);
uint8_t lcm_read_firmware_version();
uint16_t lcm_read_pic_id();
void lcm_start_calibration();
void lcm_reset_lcm();
uint8_t lcm_read_keyboard_status();

/***********************************************************************************
 * RTC
 */

typedef struct
{
   uint16_t year; /*!< Range from 1970 to 2099.*/
   uint8_t month; /*!< Range from 1 to 12.*/
   uint8_t day; /*!< Range from 1 to 31 (depending on month).*/
   uint8_t week; /*!< Range from 0 to 7 (Sunday - Saturday).*/
   uint8_t hour; /*!< Range from 0 to 23.*/
   uint8_t minute; /*!< Range from 0 to 59.*/
   uint8_t second; /*!< Range from 0 to 59.*/

} lcm_rtc_datetime_t;

void lcm_write_rtc(lcm_rtc_datetime_t lcm_rtc_datetime);
uint8_t lcm_read_rtc();

void lcm_rtc_write_year(uint16_t value);
void lcm_rtc_write_month(uint8_t value);
void lcm_rtc_write_day(uint8_t value);
void lcm_rtc_write_hours(uint8_t value);
void lcm_rtc_write_minutes(uint8_t value);
void lcm_rtc_write_seconds(uint8_t value);
void lcm_rtc_write_date(uint16_t day, uint8_t month, uint8_t year);
void lcm_rtc_write_time(uint16_t hours, uint8_t minutes, uint8_t seconds);

uint16_t lcm_rtc_get_year();
uint8_t lcm_rtc_get_month();
uint8_t lcm_rtc_get_day();
uint8_t lcm_rtc_get_day_of_the_week();
uint8_t lcm_rtc_get_hours();
uint8_t lcm_rtc_get_minutes();
uint8_t lcm_rtc_get_seconds();

/***********************************************************************************
 * Media
 */

typedef enum
{
   PLAY = 0x5B,         /*Inicia a reproduÃ§Ã£o do Ã¡udio*/
   PAUSE = 0x5C         /*Pausa a reproduÃ§Ã£o do Ã¡udio*/

} lcm_audio_controls_t;

typedef enum
{
   PLAY_SINGLE_FROM_LCM = 0x00,      /*Inicia a reproduÃ§Ã£o de um vÃ­deo contido no LCM, a reproduÃ§Ã£o Ã© encerrada quando o vÃ­deo terminar*/
   LOOP_SINGLE_FROM_LCM,            /*Inicia a reproduÃ§Ã£o de um vÃ­deo contido no LCM, a reproduÃ§Ã£o Ã© reiniciada quando o vÃ­deo terminar*/
   REPEAT_ALL_FROM_LCM,            /*Inicia a reproduÃ§Ã£o ed um vÃ­deo contido no LCM, e reproduz todos os outros vÃ­does em seguÃªncia, repetindo a reproduÃ§Ã£o*/
   PLAY_SINGLE_FROM_USB,            /*Inicia a reproduÃ§Ã£o de um vÃ­deo contido no USB, a reproduÃ§Ã£o Ã© encerrada quando o vÃ­deo terminar*/
   LOOP_SINGLE_FROM_USB,            /*Inicia a reproduÃ§Ã£o de um vÃ­deo contido no USB, a reproduÃ§Ã£o Ã© reiniciada quando o vÃ­deo terminar*/
   REPEAT_ALL_FROM_USB            /*Inicia a reproduÃ§Ã£o ed um vÃ­deo contido no USB, e reproduz todos os outros vÃ­does em seguÃªncia, repetindo a reproduÃ§Ã£o*/

} lcm_video_mode_t;

uint8_t lcm_read_audio_status();
uint8_t lcm_read_video_status();
void lcm_play_audio(uint16_t id, uint8_t volume);
void lcm_stop_audio(uint16_t id);

void lcm_write_video_settings(uint16_t id, lcm_video_mode_t lcm_video_mode, uint8_t volume, uint16_t x, uint16_t y);
void lcm_play_pause_video();
void lcm_stop_video();
void lcm_next_video();
void lcm_previous_video();

/***********************************************************************************************************************************************
 * Trend Curve
 */

typedef enum
{
   CHANNEL_ALL = 0x00,         /*!<Channels-CHANNEL_ALL>*/
   CHANNEL_0 = 0x01,         /*!<Channels-CHANNEL_0>*/
   CHANNEL_1 = 0x02,         /*!<Channels-CHANNEL_1>*/
   CHANNEL_2 = 0x04,         /*!<Channels-CHANNEL_2>*/
   CHANNEL_3 = 0x08,         /*!<Channels-CHANNEL_3>*/
   CHANNEL_4 = 0x10,         /*!<Channels-CHANNEL_4>*/
   CHANNEL_5 = 0x20,         /*!<Channels-CHANNEL_5>*/
   CHANNEL_6 = 0x40,         /*!<Channels-CHANNEL_6>*/
   CHANNEL_7 = 0x80         /*!<Channels-CHANNEL_7>*/

}lcm_trend_curve_channel_t;

void lcm_write_trend_curve0(uint16_t value);
void lcm_write_trend_curve1(uint16_t value);
void lcm_write_trend_curve2(uint16_t value);
void lcm_write_trend_curve3(uint16_t value);
void lcm_write_trend_curve4(uint16_t value);
void lcm_write_trend_curve5(uint16_t value);
void lcm_write_trend_curve6(uint16_t value);
void lcm_write_trend_curve7(uint16_t value);

uint8_t lcm_clear_trend_curve0();
uint8_t lcm_clear_trend_curve1();
uint8_t lcm_clear_trend_curve2();
uint8_t lcm_clear_trend_curve3();
uint8_t lcm_clear_trend_curve4();
uint8_t lcm_clear_trend_curve5();
uint8_t lcm_clear_trend_curve6();
uint8_t lcm_clear_trend_curve7();

uint8_t lcm_clear_trend_curve(lcm_trend_curve_channel_t lcm_trend_curve_channel);
void lcm_write_trend_curve(uint16_t* value, lcm_trend_curve_channel_t lcm_trend_curve_channel, uint8_t length);

/*****************************************************************************************************************************************************
 * VPs
 */

 /******************************************************
  * LCMVAR
  */

typedef enum
{
   LOW_BYTE,         //NÃ£o implementado
   HIGH_BYTE,         //NÃ£o implementado
   TWO_BYTES,
   FOUR_BYTES,
   EIGHT_BYTES,
   TWO_BYTES_UNSIGNED,
   FOUR_BYTES_UNSIGNED,

} lcm_var_vp_mode_t;

typedef struct
{
   uint16_t vp;
   lcm_var_vp_mode_t vp_mode;
   lcm_rx_package_t* p_rx_package;

}lcm_var_parameters_t;

uint8_t lcm_var_available(lcm_var_parameters_t* lcm_var_parameters);
uint8_t lcm_var_request(lcm_var_parameters_t* lcm_var_parameters);
uint8_t lcm_var_write(lcm_var_parameters_t* lcm_var_parameters, uint32_t value);

#if (defined(UINT64_MAX))
uint8_t lcm_var_write64(lcm_var_parameters_t* lcm_var_parameters, uint64_t value);
uint64_t lcm_var_get_data(lcm_var_parameters_t* lcm_var_parameters);
uint64_t lcm_var_read(lcm_var_parameters_t* lcm_var_parameters);
#else
uint32_t lcm_var_get_data(lcm_var_parameters_t* lcm_var_parameters);
uint32_t lcm_var_read(lcm_var_parameters_t* lcm_var_parameters);
#endif

/******************************************************
 * LCMString
 */

typedef struct
{
   uint16_t vp;
   uint8_t length_in_bytes;
   lcm_rx_package_t* p_rx_package;

}lcm_string_parameters_t;

uint8_t lcm_string_available(lcm_string_parameters_t* lcm_string_parameters);
uint8_t lcm_string_get_data(lcm_string_parameters_t* lcm_string_parameters);
uint8_t lcm_string_request(lcm_string_parameters_t* lcm_string_parameters);
uint8_t lcm_string_write(lcm_string_parameters_t* lcm_string_parameters, uint8_t* chars, uint16_t length);

/***********************************************************************************
 * Utils
 */

uint8_t lcm_int_to_bcd(uint8_t value);
uint8_t lcm_bcd_to_int(uint8_t value);

#endif /*UNIC_VIEW_AD_C_H*/
