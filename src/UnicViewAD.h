/*
# Arquivo:

	UnicViewAD.h

# Projeto:

	UnicView AD.ino - Biblioteca para Arduino

# Descrição:

	Esta biblioteca permite que um Arduino se comunique facilmente com o Displays
	Inteligentes Proculus, oferecendo um controle mais intuitivo e prático de todas
	as suas funções.

	Ela elimina a necessidade de um completo entendimento sobre todos os protocolos de
	comunicação e funcionamento interno do Display, abstraindo esses protocolos para
	pequenas funções que garantem facilidade e rapidez de uso.

# Documentação do projeto:

	http://www.victorvision.com.br/projects/arduino-system/library/site/index.html

# Autores:

	* Andreo de Freitas
	* Rafael Moura (revisão)
	* Renan Candido Vieira (revisão)

# Versão:

	1.1

# Licença:

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

#ifndef _LIB_Proculus_h
#define _LIB_Proculus_h
                 //

#include <stdint.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#elif defined(ARDUINO)

#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

//#include <SoftwareSerial.h>
#endif



#if defined(__cplusplus)                //
extern "C" {                            // Diretrizes de compila��o para compilar em C dentro do Arduino - N�o deve ser utilizado em programas C
#endif                                  //

#include "unicviewad_c.h"

#if defined(__cplusplus)                //
}                            // Diretrizes de compila��o para compilar em C dentro do Arduino - N�o deve ser utilizado em programas C
#endif                 

const int rtcReadIntervalInMs = 500;
const uint8_t returnNull = 0xFF;

class LCM
{
public:

#if defined(ARDUINO)

	LCM(Stream& serial);

#else
	LCM();
#endif //defined(ARDUINO)

	void begin(uint16_t header = 0x5AA5);

	void enableCRC();

	uint8_t transferFileBegin(const char* name, uint32_t nameSize, uint32_t fileSize) { return transfer_file_begin(name, nameSize, fileSize); };

	void disableGarbageCollector();
	void enableGarbageCollector();

	void setReadTimeout(uint32_t timeoutInTicks);

	uint8_t writeVP(uint16_t vp, uint8_t* data, uint8_t length);
	uint8_t writeVP(uint16_t vp, uint16_t* data, uint8_t length);
	uint8_t writeVP(uint16_t vp, uint16_t data);

	uint8_t writeRegister(uint8_t reg, uint8_t data);

	void changeBacklight(uint8_t value);
	void beepBuzzer(uint8_t value);

	void changePicId(uint16_t value);

	uint8_t rtcWriteYear(uint16_t value);
	uint8_t rtcWriteMonth(uint8_t value);
	uint8_t rtcWriteDay(uint8_t value);
	uint8_t rtcWriteHour(uint8_t value);
	uint8_t rtcWriteMinute(uint8_t value);
	uint8_t rtcWriteSecond(uint8_t value);

	uint8_t rtcWriteDate(uint16_t day, uint8_t month, uint8_t year);
	uint8_t rtcWriteClock(uint16_t hour, uint8_t minute, uint8_t second = 0);

	void writeTrendCurve0(uint16_t value) { lcm_write_trend_curve0(value); }
	void writeTrendCurve1(uint16_t value) { lcm_write_trend_curve1(value); }
	void writeTrendCurve2(uint16_t value) { lcm_write_trend_curve2(value); }
	void writeTrendCurve3(uint16_t value) { lcm_write_trend_curve3(value); }
	void writeTrendCurve4(uint16_t value) { lcm_write_trend_curve4(value); }
	void writeTrendCurve5(uint16_t value) { lcm_write_trend_curve5(value); }
	void writeTrendCurve6(uint16_t value) { lcm_write_trend_curve6(value); }
	void writeTrendCurve7(uint16_t value) { lcm_write_trend_curve7(value); }
	
	uint8_t clearTrendCurve0() { return lcm_clear_trend_curve0(); }
	uint8_t clearTrendCurve1() { return lcm_clear_trend_curve1(); }
	uint8_t clearTrendCurve2() { return lcm_clear_trend_curve2(); }
	uint8_t clearTrendCurve3() { return lcm_clear_trend_curve3(); }
	uint8_t clearTrendCurve4() { return lcm_clear_trend_curve4(); }
	uint8_t clearTrendCurve5() { return lcm_clear_trend_curve5(); }
	uint8_t clearTrendCurve6() { return lcm_clear_trend_curve6(); }
	uint8_t clearTrendCurve7() { return lcm_clear_trend_curve7(); }

	void activateSoftwareControl(uint8_t value);
	
	uint8_t readVPs(uint16_t vp, uint16_t data[], uint8_t length);
	uint16_t readVP(uint16_t vp);

	uint8_t readRegister(uint8_t reg);

	uint8_t requestVP(uint16_t vp, uint8_t length = 1u);

	uint16_t rtcReadYear	() { if (readRTC() == 1) { return lcm_rtc_get_year();	}	else { return LCM_EMPTY_RESPONSE;	} }
	uint8_t rtcReadMonth	() { if (readRTC() == 1) { return lcm_rtc_get_month();	}	else { return returnNull;		} }
	uint8_t rtcReadDay		() { if (readRTC() == 1) { return lcm_rtc_get_day();	}	else { return returnNull;		} }
	uint8_t rtcReadWeek		() { if (readRTC() == 1) { return lcm_rtc_get_day_of_the_week();	}	else { return returnNull;} }
	uint8_t rtcReadHour		() { if (readRTC() == 1) { return lcm_rtc_get_hours();	}	else { return returnNull;		} }
	uint8_t rtcReadMinute	() { if (readRTC() == 1) { return lcm_rtc_get_minutes();}	else { return returnNull;		} }
	uint8_t rtcReadSecond	() { if (readRTC() == 1) { return lcm_rtc_get_seconds();}	else { return returnNull;		} }

	uint8_t readFirmwareVersion();
	uint16_t readPicId();

	uint8_t readAudioStatus( );
	uint8_t readVideoStatus( );
	uint8_t readKeyboardStatus( );

	void playAudio(uint16_t id, uint8_t volume = 100);
	void stopAudio(uint16_t id = 0xFFFF);

	void writeVideoSettings(uint16_t id, lcm_video_mode_t mode = PLAY_SINGLE_FROM_LCM, uint8_t volume = 100, uint16_t x = 0x00, uint16_t y = 0x00);

	void playPauseVideo();
	void stopVideo();
	void nextVideo();
	void previousVideo();

	void calibrate();
	void resetLCM();

	uint8_t available();

	uint16_t getRxAddress(uint8_t command, uint16_t address);
	uint8_t getRxLength(uint8_t command, uint16_t address);
	uint8_t getRxType(uint8_t command, uint16_t address);

	void getData(uint16_t* data, uint8_t length);

	uint16_t getData();
			
	virtual ~LCM();

	uint8_t writeBytes(uint8_t* data, uint8_t length);

#if defined(ARDUINO)

	uint16_t readBytes();

#endif	

	uint8_t writeRegister(uint8_t reg, uint8_t* data, uint8_t length);
	uint8_t readRegisters(uint8_t reg);
	uint8_t readRegisters(uint8_t reg, uint8_t* value, uint8_t length);
	uint8_t clearTrendCurve(lcm_trend_curve_channel_t channel);
	uint8_t writeTrendCurve(uint16_t* value, lcm_trend_curve_channel_t channel, uint8_t length);
	uint8_t writeRTC(lcm_rtc_datetime_t date);
	uint8_t readRTC();

private:

#if defined(ARDUINO)

	Stream* _StreamSerial;

#endif	

	uint16_t _audioID = 0;
	uint32_t _lastRtcResponseTime = 0;
};

class LcmVar //: public Print
{


public:
	LcmVar();
	LcmVar(uint16_t vp, lcm_var_vp_mode_t variableType = TWO_BYTES);

	uint8_t available();

#if (defined(UINT64_MAX))
	uint8_t write(uint64_t value);
#else
	uint8_t write(uint32_t value);
#endif

	uint8_t write(float value, uint8_t decimalDigits);

	uint8_t write(double value, uint8_t decimalDigits);



#if (defined(UINT64_MAX))
	uint64_t getData();
	uint64_t read();
#else
	uint32_t getData();
	uint32_t read();
#endif

	uint8_t request();

	virtual ~LcmVar();

protected:

	lcm_var_parameters_t _parameters;
};

class LcmString
{
public:
	LcmString();
	LcmString(uint16_t vp, uint8_t length = 1u);

	uint8_t available();
	uint8_t write(char* chars, uint16_t length);
	char getData();
	uint8_t request();

#if defined(ARDUINO)
	uint8_t write(String& value);
#endif
	virtual ~LcmString();
private:
	lcm_string_parameters_t _parameters;
};

#endif
