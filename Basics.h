#pragma once
#include <Arduino.h>
//#include <stdarg.h>

//#define LEAN //uncomment this for a leaner (but less functional) implementation. The following will now work:
//   1. printf(F()) - F() Macro cannot be used to define strings in flash memory from within the printf function. 

#define UINT8 uint8_t
#define UINT16 uint16_t
#define UINT32 uint32_t
#define UINT64 uint64_t

//Overloading MACROs. See here: http://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments
#define GET_MACRO(_1,_2,_3,NAME,...) NAME
//#define FOO(...) GET_MACRO(__VA_ARGS__, FOO3, FOO2)(__VA_ARGS__)

#define GET_MACRO_ZERO_OR_MORE(_0, _1, _2, NAME, ...) NAME
//Usage: 
//#define FOO(...) GET_MACRO(_0, ##__VA_ARGS__, FOO2, FOO1, FOO0)(__VA_ARGS__)
//#define FOO0() printf("0 args\n");
//#define FOO1(a) printf("1 arg\n");
//#define FOO2(a, b) printf("2 args\n");

#define BIT_READ(value, bit) (((value) >> (bit)) & 0x01)
#define BIT_SET(value, bit) ((value) |= (1UL << (bit)))
#define BIT_CLEAR(value, bit) ((value) &= ~(1UL << (bit)))
#define BIT_WRITE(value, bit, bitvalue) (bitvalue ? BIT_SET(value, bit) : BIT_CLEAR(value, bit))

#define BitIsSet(port, bit) (((port) >> (bit)) & 0x01)
#define BitWrite(port, bit, value) ((value)?((port) |= 0x01 << (bit)):((port) &= ~(0x01 << (bit))))

//These are only true for the Arduino UNO
#define LedOut 13
#define LedOutBit 5
#define LedOutPort PORTB

inline boolean CycleCheckMS(UINT32 *lastOccurance, UINT32 cycleDuration) 
{
  UINT32 currentTime = millis();
  if(currentTime - *lastOccurance >= cycleDuration)
  {
    *lastOccurance = currentTime;
    return true;
  }
  else
    return false;
}

inline boolean CycleCheckUS(UINT32 *lastOccurance, UINT32 cycleDuration) 
{
  UINT32 currentTime = micros();
  if(currentTime - *lastOccurance >= cycleDuration)
  {
    *lastOccurance = currentTime;
    return true;
  }
  else
    return false;
}

inline boolean CycleCheck(UINT32 currentTime, UINT32 *lastOccurance, UINT32 cycleDuration) 
{
  if(currentTime - *lastOccurance >= cycleDuration)
  {
    *lastOccurance = currentTime;
    return true;
  }
  else
    return false;
}



//String stuff///////////////////////////////////////////////////////////////////////////
#define STRINGIFY(s) #s
#define PRINT_INT(val) p("%s = %d\n", STRINGIFY(val), val)
//#define PRINT_HEX(val) p("%s = 0x%X\n", STRINGIFY(val), val)
#define PRINT_HEX(val) p("%s = 0x%lx\n", STRINGIFY(val), val)

/*Printf functions!
 Examples:
 p("%s", "Hello world");
 p("%s\n", "Hello world"); // with line break
 unsigned long a=0xFFFFFFFF;
 p("Decimal a: %l\nDecimal unsigned a: %lu\n", a, a); 
 p("Hex a: %x\n", a); 
 p(F("Best to store long strings to flash to save %s"),"memory");*/

#if !defined(LEAN)

//This version supports the F() macro. Built-in.
//For an explanation on why this works check out: https://gcc.gnu.org/onlinedocs/gcc/Variadic-Macros.html
#define p(fmt, ...) ArduinoPrintf(F(fmt), ## __VA_ARGS__)
void ArduinoPrintf(const __FlashStringHelper *fmt, ... )
{
  char buf[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt);
#ifdef __AVR__
  vsnprintf_P(buf, sizeof(buf), (const char *)fmt, args); // progmem for AVR
#else
  vsnprintf(buf, sizeof(buf), (const char *)fmt, args); // for the rest of the world
#endif
  va_end(args);
  Serial.print(buf);
}

#else

//This version won't work with the F() macro
#define p(...) GET_MACRO_ZERO_OR_MORE(_0, ##__VA_ARGS__, p2, p1, p0)(__VA_ARGS__)
#define p0() ArduinoPrintf("\n")
#define p1(fmt) ArduinoPrintf(fmt)
#define p2(fmt, ...) ArduinoPrintf(fmt), __VA_ARGS__)
void ArduinoPrintf(char *fmt, ... )
{
  char buf[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt );
  vsnprintf(buf, 128, fmt, args);
  va_end (args);
  Serial.print(buf);
}

#endif




