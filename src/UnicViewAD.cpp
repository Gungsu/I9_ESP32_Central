/*
# Arquivo:

	UnicViewAD.cpp

# Projeto:

	UnicView AD.ino - Biblioteca para Arduino

# Descri��o:

	Esta biblioteca permite que um Arduino se comunique facilmente com o Displays 
	Inteligentes Proculus, oferecendo um controle mais intuitivo e pr�tico de todas 
	as suas fun��es.

	Ela elimina a necessidade de um completo entendimento sobre todos os protocolos de 
	comunica��o e funcionamento interno do Display, abstraindo esses protocolos para 
	pequenas fun��es que garantem facilidade e rapidez de uso.

# Documenta��o do projeto:

	http://www.victorvision.com.br/projects/arduino-system/library/site/index.html

# Autores:

	* Andreo de Freitas
	* Rafael Moura (revis�o)
	* Renan Candido Vieira (revis�o)

# Vers�o:

	1.2

# Licen�a:

	Copyright (c) 2019 Proculus Technologies Limited and Victor Vision LTDA

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

#include "UnicViewAD.h"


extern LCM Lcm;

#if defined(ARDUINO)


#if defined(__cplusplus)                //
extern "C" {                            // Diretrizes de compila��o para compilar em C dentro do Arduino - N�o deve ser utilizado em programas C
#endif                                  //

	uint8_t lcm_write_bytes(uint8_t* data, uint8_t length)
	{
		return Lcm.writeBytes(data, length);
	}

	uint16_t lcm_read_byte()
	{
		return Lcm.readBytes();
	}

#if defined(__cplusplus)                //
}                            // Diretrizes de compila��o para compilar em C dentro do Arduino - N�o deve ser utilizado em programas C
#endif    //defined(__cplusplus)

LCM::LCM(Stream& serial) : _StreamSerial(&serial)
{
}

#else

#include <math.h>

LCM::LCM()
{
}

extern uint8_t lcmWriteBytes(uint8_t* data, uint8_t length);
extern uint16_t lcmReadBytes();

#if defined(__cplusplus)                //
extern "C" {                            // Diretrizes de compila��o para compilar em C dentro do Arduino - N�o deve ser utilizado em programas C
#endif                                  //

	uint8_t lcm_write_bytes(uint8_t* data, uint8_t length)
	{
		return lcmWriteBytes(data, length);
	}

	uint16_t lcm_read_byte()
	{
		return lcmReadBytes();
	}

#if defined(__cplusplus)                //
}                            // Diretrizes de compila��o para compilar em C dentro do Arduino - N�o deve ser utilizado em programas C
#endif    //defined(__cplusplus)



#endif	//defined(ARDUINO) || defined(ESP8266) || defined(ESP32)

void LCM::begin(uint16_t header)
{
	lcm_set_header(header);
	lcm_init();
}

void LCM::enableCRC()
{
	lcm_crc_enable();
}

void LCM::setReadTimeout(uint32_t timeoutInTicks)
{
	lcm_set_read_timeout(timeoutInTicks);
}

void LCM::disableGarbageCollector()
{
	lcm_disable_garbage_collector();
}

void LCM::enableGarbageCollector()
{
	lcm_enable_garbage_collector();
}

#if defined(ARDUINO)

uint8_t LCM::writeBytes(uint8_t* data, uint8_t length)
{
	return _StreamSerial->write(data, length);
}

uint16_t LCM::readBytes()
{

	if (_StreamSerial->available())
	{
		return _StreamSerial->read();
	}
	else
	{
		return LCM_EMPTY_RESPONSE;
	}
}

#endif	//defined(ARDUINO) || defined(ESP8266) || defined(ESP32)


uint8_t LCM::writeVP(uint16_t vp, uint16_t data)
{
    return lcm_write_multiple_vps(vp, &data, 1);
}

uint8_t LCM::writeVP(uint16_t vp, uint8_t *data, uint8_t length)
{
    uint16_t data16[length];
    for (uint8_t i = 0; i < length; i++)
    {
        data16[i] = data[i];
    }
    return lcm_write_multiple_vps(vp, data16, length);
}

uint8_t LCM::writeVP(uint16_t vp, uint16_t *data, uint8_t length)
{
	return lcm_write_multiple_vps(vp, data, length);
}

void LCM::changeBacklight(uint8_t value)
{
	lcm_change_brightness(value);
}

void LCM::beepBuzzer(uint8_t value)
{
	lcm_beep_buzzer(value);
}

void LCM::changePicId(uint16_t value)
{
	lcm_change_pic_id(value);
}

uint8_t LCM::writeRTC(lcm_rtc_datetime_t date)
{
	lcm_write_rtc(date);
	return 1;
}

uint8_t LCM::rtcWriteYear(uint16_t value)
{
	if (readRTC())
	{
		lcm_rtc_write_year(value);
		return 1;
	}
	return 0;
}

uint8_t LCM::rtcWriteMonth(uint8_t value)
{
	if (readRTC())
	{
		lcm_rtc_write_month(value);
		return 1;
	}
	return 0;
}

uint8_t LCM::rtcWriteDay(uint8_t value)
{
	if (readRTC())
	{
		lcm_rtc_write_day(value);
		return 1;
	}
	return 0;
}

uint8_t LCM::rtcWriteHour(uint8_t value)
{
	if (readRTC())
	{
		lcm_rtc_write_hours(value);
		return 1;
	}
	return 0;
}

uint8_t LCM::rtcWriteMinute(uint8_t value)
{
	if (readRTC())
	{
		 lcm_rtc_write_minutes(value);
		return 1;
	}
	return 0;
}

uint8_t LCM::rtcWriteSecond(uint8_t value)
{
	if (readRTC())
	{
		 lcm_rtc_write_seconds(value);
		return 1;
	}
	return 0;
}

uint8_t LCM::rtcWriteDate(uint16_t day, uint8_t month, uint8_t year)
{
	if (readRTC())
	{
		 lcm_rtc_write_date(day, month, year);
		return 1;
	}
	return 0;
}

uint8_t LCM::rtcWriteClock(uint16_t hour, uint8_t minute, uint8_t second)
{
	if (readRTC())
	{
		 lcm_rtc_write_time(hour, minute, second);
		return 1;
	}
	return 0;
}

void LCM::activateSoftwareControl(uint8_t softwareControlCode)
{
	lcm_activate_software_control(softwareControlCode);
}

void LCM::playAudio(uint16_t id, uint8_t volume)
{
    _audioID = id;
	lcm_play_audio(id, volume);
}

void LCM::stopAudio(uint16_t id)
{
	if (id == 0xFFFF)
	{
		id = _audioID;
	}
	lcm_stop_audio(id);
}

void LCM::writeVideoSettings(uint16_t id, lcm_video_mode_t mode, uint8_t volume, uint16_t x, uint16_t y)
{
	lcm_write_video_settings(id, mode, volume, x, y);
}

void LCM::playPauseVideo()
{
	lcm_play_pause_video();
}

void LCM::stopVideo()
{
	lcm_stop_video();
}

void LCM::nextVideo()
{
	lcm_next_video();
}

void LCM::previousVideo()
{
	lcm_previous_video();
}

void LCM::calibrate()
{
	lcm_start_calibration();
}

uint8_t LCM::clearTrendCurve(lcm_trend_curve_channel_t channel)
{
    return lcm_clear_trend_curve(channel);
}

uint8_t LCM::writeTrendCurve(uint16_t* value, lcm_trend_curve_channel_t channel, uint8_t length)
{
	lcm_write_trend_curve(value, channel, length);
	return 1;
}

void LCM::resetLCM()
{
	lcm_reset_lcm();
}

uint8_t LCM::readFirmwareVersion()
{
	return lcm_read_firmware_version();
}

uint16_t LCM::readPicId()
{
    return lcm_read_pic_id();
}

uint8_t LCM::readRTC()
{
	return lcm_read_rtc();
}

uint8_t LCM::readAudioStatus()
{
    return lcm_read_audio_status();
}

uint8_t LCM::readVideoStatus()
{
    return lcm_read_video_status();
}

uint8_t LCM::readKeyboardStatus()
{
    return lcm_read_keyboard_status();
}

uint8_t LCM::writeRegister(uint8_t reg, uint8_t data)
{
    return lcm_write_multiple_registers(reg, &data, 1);
}

uint8_t LCM::writeRegister(uint8_t reg, uint8_t *data, uint8_t length)
{
	return lcm_write_multiple_registers(reg, data, length);
}

uint8_t LCM::readRegisters(uint8_t reg)
{
	return lcm_read_register(reg);
}

uint8_t LCM::readRegisters(uint8_t reg, uint8_t* value, uint8_t length)
{
	return lcm_read_multiple_registers(reg, value, length);
}

uint16_t LCM::readVP(uint16_t vp)
{
	return lcm_read_vp(vp);
}

uint8_t LCM::readVPs(uint16_t vp, uint16_t data[], uint8_t length)
{
	return lcm_read_multiple_vps(vp, data, length);
}

uint8_t LCM::requestVP(uint16_t vp, uint8_t length)
{
	return lcm_request_vp(vp, length);
}

uint8_t LCM::available()
{
	return lcm_available();
}

uint8_t LCM::getRxType(uint8_t command, uint16_t address)
{
	return lcm_get_rx_type();
}

uint16_t LCM::getRxAddress(uint8_t command, uint16_t address)
{
	return lcm_get_rx_address();
}

uint8_t LCM::getRxLength(uint8_t command, uint16_t address)
{
	return lcm_get_rx_length();
}

void LCM::getData(uint16_t* buffer, uint8_t length)
{
	lcm_get_data_array(buffer, length);
}

uint16_t LCM::getData()
{
    return lcm_get_data();
}

LCM::~LCM()
{
}

LcmVar::LcmVar()
{
}

LcmVar::LcmVar(uint16_t vp, lcm_var_vp_mode_t variableType)
{
	_parameters.vp = vp;
	_parameters.vp_mode = variableType;
}

#if (defined(UINT64_MAX))
uint8_t LcmVar::write(uint64_t value)
{
	if (_parameters.vp_mode == EIGHT_BYTES)
	{
		return lcm_var_write64(&_parameters, value);		
	}
	else
	{
		return lcm_var_write(&_parameters, value);		
	}
}
#else

uint8_t LcmVar::write(uint32_t value)
{
	return lcm_var_write(&_parameters, value);
}
#endif

uint8_t LcmVar::write(float value, uint8_t decimalDigits)
{
#if (defined(UINT64_MAX))
	uint64_t valueInteger;
#else
	uint32_t valueInteger;
#endif

	uint32_t powerOf = pow(10, decimalDigits);

	valueInteger = value * powerOf;

	return write(valueInteger);
}

uint8_t LcmVar::write(double value, uint8_t decimalDigits)
{
#if (defined(UINT64_MAX))
	uint64_t valueInteger;
#else
	uint32_t valueInteger;
#endif
	
	uint32_t powerOf = pow(10, decimalDigits);

	valueInteger = value * powerOf;
	
	return write(valueInteger);
}

uint8_t LcmVar::available()
{
    return lcm_var_available(&_parameters);
}

#if (defined(UINT64_MAX))

uint64_t LcmVar::getData()
{
	return lcm_var_get_data(&_parameters);
}

uint64_t LcmVar::read()
{
	return lcm_var_read(&_parameters);
}

#else
uint32_t LcmVar::getData()
{
	return lcm_var_get_data(&_parameters);
}

uint32_t LcmVar::read()
{
	return lcm_var_read(&_parameters);
}

#endif


uint8_t LcmVar::request()
{
	return lcm_var_request(&_parameters);
}

LcmVar::~LcmVar()
{
}

LcmString::LcmString()
{
}

LcmString::LcmString(uint16_t vp, uint8_t length)
{
	_parameters.vp = vp;
	_parameters.length_in_bytes = length;
}

uint8_t LcmString::available()
{
	return lcm_string_available(&_parameters);
}
uint8_t LcmString::write(char* chars, uint16_t length)
{
	return lcm_string_write(&_parameters, (uint8_t*)chars, length);
}

#if defined(ARDUINO)
uint8_t LcmString::write(String& value)
{
	return lcm_string_write(&_parameters, (uint8_t*)value.c_str(), value.length());
}
#endif
char LcmString::getData()
{
	return lcm_string_get_data(&_parameters);
}

uint8_t LcmString::request()
{
	return lcm_string_request(&_parameters);
}

LcmString::~LcmString()
{
}
