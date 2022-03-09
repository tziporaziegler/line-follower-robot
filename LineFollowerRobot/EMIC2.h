#ifndef EMIC2_h
#define EMIC2_h

#include <Arduino.h>
#include <SD.h>
#include <string.h>

// #define VERBOSE true // Use this if you want verbose output of the operations executed

#define SD_C 1

// A class for interfacing the Emic 2 module
class EMIC2
{
public:
	EMIC2();
	~EMIC2();
	void begin(HardwareSerial *hs);
	void speak(char *msg);
	void speak(String msg);
	void speak(char num);
	void speak(unsigned char num);
	void speak(int num);
	void speak(unsigned int num);
	void speak(long num);
	void speak(unsigned long num);
	void speak(double num);
	void speakDemo(uint8_t num);
	void sendCmd(char *cmd);
	void ready();
	void setVoice(uint8_t voice);
	void setVolume(int8_t volume);

private:
	HardwareSerial *_emic2_io;
	uint8_t _paused;
	uint8_t _voice;
	int8_t _volume;
};

#endif  // EMIC2
