#include "EMIC2.h"

// Creates an instance of the EMIC2 class and initializes variables
EMIC2::EMIC2()
{
  _paused = 0;
  _voice = 0;
  _volume = 0;
}

// Upon destruction of an instance,
// it frees dynamically allocated data members
EMIC2::~EMIC2()
{
  delete _emic2_io;
}

// Initializes serial port and checks for availability of the Emic 2 module
void EMIC2::begin(HardwareSerial *hs)
{
  _emic2_io = hs;
//  _emic2_io->begin(9600);

  #ifdef VERBOSE
  Serial.println("Serial Port is set");
  #endif

  // When the Emic 2 powers on, it takes about 3 seconds for it to successfully initialize
  // It then sends a ":" character to indicate it's ready to accept commands
  // If the Emic 2 is already initialized, a CR will also causes it to send a ":"
  _emic2_io->print('\n');  // Sends a CR in case the system is already up
  while ( _emic2_io->read() != ':' ) ;  // When the Emic 2 has initialized and is ready,
  // it will send a single ':' character...
  // so it waits here until it receives the ':'
  delay(10);  // Short delay
  _emic2_io->flush();  // Flushs receive buffer

  #ifdef VERBOSE
  Serial.println("Emic 2 Module is ready");
  #endif

}

// Waits (Blocking) until Emic 2 is ready to receive a command
void EMIC2::ready()
{
  _emic2_io->flush();  // Flushes receive buffer
  _emic2_io->print('\n');  // Triggers response
  while (_emic2_io->read() != ':') ;  // Awaits for ':' indicator
}

// Sends a message to the Emic 2 module to speak
void EMIC2::speak(char *msg)
{
  ready();

  #ifdef VERBOSE
  Serial.print("Playing message: ");
  Serial.println(msg);
  #endif

  // Sends the desired string to convert to speech
  _emic2_io->print('S');
  _emic2_io->print(msg);
  _emic2_io->print('\n');
}

// Sends a message to the Emic 2 module to speak
void EMIC2::speak(String msg)
{
  ready();

  #ifdef VERBOSE
  Serial.print("Playing message: ");
  Serial.println(msg);
  #endif

  // Sends the desired string to convert to speech
  _emic2_io->print('S');
  _emic2_io->print(msg);
  _emic2_io->print('\n');
}

// Sends a message to the Emic 2 module to speak
void EMIC2::speak(char num)
{
  ready();

  #ifdef VERBOSE
  Serial.print("Playing message: ");
  Serial.println(num);
  #endif

  // Sends the desired string to convert to speech
  _emic2_io->print('S');
  _emic2_io->print(num);
  _emic2_io->print('\n');
}

// Sends a message to the Emic 2 module to speak
void EMIC2::speak(unsigned char num)
{
  ready();

  #ifdef VERBOSE
  Serial.print("Playing message: ");
  Serial.println(num);
  #endif

  // Sends the desired string to convert to speech
  _emic2_io->print('S');
  _emic2_io->print(num);
  _emic2_io->print('\n');
}

// Sends a message to the Emic 2 module to speak
void EMIC2::speak(int num)
{
  ready();

  #ifdef VERBOSE
  Serial.print("Playing message: ");
  Serial.println(num);
  #endif

  // Sends the desired string to convert to speech
  _emic2_io->print('S');
  _emic2_io->print(num);
  _emic2_io->print('\n');
}

// Sends a message to the Emic 2 module to speak
void EMIC2::speak(unsigned int num)
{
  ready();

  #ifdef VERBOSE
  Serial.print("Playing message: ");
  Serial.println(num);
  #endif

  // Sends the desired string to convert to speech
  _emic2_io->print('S');
  _emic2_io->print(num);
  _emic2_io->print('\n');
}

// Sends a message to the Emic 2 module to speak
void EMIC2::speak(long num)
{
  ready();

  #ifdef VERBOSE
  Serial.print("Playing message: ");
  Serial.println(num);
  #endif

  // Sends the desired string to convert to speech
  _emic2_io->print('S');
  _emic2_io->print(num);
  _emic2_io->print('\n');
}

// Sends a message to the Emic 2 module to speak
void EMIC2::speak(unsigned long num)
{
  ready();

  #ifdef VERBOSE
  Serial.print("Playing message: ");
  Serial.println(num);
  #endif

  // Sends the desired string to convert to speech
  _emic2_io->print('S');
  _emic2_io->print(num);
  _emic2_io->print('\n');
}

// Sends a message to the Emic 2 module to speak
void EMIC2::speak(double num)
{
  ready();

  #ifdef VERBOSE
  Serial.print("Playing message: ");
  Serial.println(num);
  #endif

  // Sends the desired string to convert to speech
  _emic2_io->print('S');
  _emic2_io->print(num);
  _emic2_io->print('\n');
}

// Plays demonstration messages
void EMIC2::speakDemo(uint8_t num)
{
  if ( num >= 0 && num < 3 )
  {
    ready();

    // Sends demo command
    _emic2_io->print('D');
    _emic2_io->print(num);
    _emic2_io->print('\n');

    #ifdef VERBOSE
    Serial.print("Plays demo ");
    Serial.println(num);
    #endif
  }
}

// Sends a command specified entirely on the input argument
void EMIC2::sendCmd(char *cmd)
{
  ready();

  #ifdef VERBOSE
  Serial.print("Sending command: ");
  Serial.println(cmd);
  #endif

  _emic2_io->print(cmd);  // Sends command
}

// Sets voice
void EMIC2::setVoice(uint8_t voice)
{
  if ( voice >= 0 && voice < 9 )
  {
    ready();

    _voice = voice;  // Updates state

    // Sends voice command
    _emic2_io->print('N');
    _emic2_io->print(_voice);
    _emic2_io->print('\n');
    while (_emic2_io->read() != ':') ;

    #ifdef VERBOSE
    Serial.print("Voice set to ");
    Serial.println(_voice);
    #endif
  }
}

// Sets volume level (in dB)
void EMIC2::setVolume(int8_t volume)
{
  ready();

  // Enforces bounds and updates state
  if ( volume < -48 ) _volume = -48;
  else if ( volume > 18 ) _volume = 18;
  else _volume = volume;

  // Sends volume command
  _emic2_io->print('V');
  _emic2_io->print(_volume);
  _emic2_io->print('\n');
  while (_emic2_io->read() != ':') ;

  #ifdef VERBOSE
  Serial.print("Volume set to ");
  Serial.println(_volume);
  #endif
}
