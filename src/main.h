// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _ATMEGA328P_ALARM_CLOCK_MAIN_H_
#define _ATMEGA328P_ALARM_CLOCK_MAIN_H_
//add your includes for the project _ATMEGA328P_ALARM_CLOCK_MAIN_H_ here
#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <stdint.h>
#include "DS3232RTC.h"
#include "ssd1306_spi.h"

//end of add your includes here

#define ALARM_INPUT (1 << PD2)
#define BUTTON_SET (1 << PD3)
#define BUTTON_ADJ (1 << PD4)
#define FMT_TWO_DIGITS "%02d"
#define FMT_DIGITS "%d"
#define FMT_TEMP "%d%cC"
#define HOUR_MIN_SEP ":"
#define CHAR_EMPTY_2 "   "
#define CHAR_EMPTY_3 "    "
#define CHAR_EMPTY_4 "     "

#define STATUS_NONE 0
#define STATUS_SET 1
#define ADJ_SLOT_NONE 0
#define ADJ_SLOT_SEC 1
#define ADJ_SLOT_HR 2
#define ADJ_SLOT_MIN 3
#define ADJ_SLOT_YEAR 4
#define ADJ_SLOT_MONTH 5
#define ADJ_SLOT_DATE 6
#define ADJ_SLOT_DAY 7

#define BUTTON_NONE 0

#define LONG_PRESS_DELAY 1000
#define CONT_ADJ_LP_DELAY 128

#ifdef __cplusplus
extern "C" {
#endif
static char days_in_week[7][4] = { { "SUN" }, { "MON" }, { "TUE" }, { "WED" }, {
		"THU" }, { "FRI" }, { "SAT" } };
void printTemp(int tempCX4);
void printDateTime(const tmElements_t &tm, time_t t);
void printDay(const tmElements_t &tm);
void printYear(const tmElements_t &tm);
void printMon(const tmElements_t &tm);
void printDate(const tmElements_t &tm);
void printHour(const tmElements_t &tm);
void printMin(const tmElements_t &tm);
void printSec(const tmElements_t &tm);
void printStr(char *buf, unsigned int len, unsigned int col, unsigned int page,
		unsigned int size);
void printDigits(unsigned int _adj_slot, unsigned int val, char* disp,
		unsigned int len, unsigned int col, unsigned int page,
		unsigned int size);
void adj(tmElements_t &tm);
int strcmpl(const char* s0, const char* s1, unsigned int len);

#ifdef __cplusplus
} // extern "C"
#endif

//Do not add code below this line
#endif /* _ATMEGA328P_ALARM_CLOCK_MAIN_H_ */

