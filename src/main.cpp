/*********************************************************************
 atmega328p based clock with SSD1306 oled display and DS3231 rtc.

 TODO: Set alarm
 TODO: Attach speaker
 *********************************************************************/

#include "main.h"

char disp_hr[3];
char disp_min[3];
char disp_sec[3];
char disp_day[4];
char disp_year[5];
char disp_mon[3];
char disp_date[3];
char disp_temp[5];

volatile unsigned long btnmillis = 0;
volatile unsigned long btnmillis_lp = 0;
volatile unsigned int button = BUTTON_NONE;
volatile unsigned int button_lp = BUTTON_NONE;

volatile unsigned int status = STATUS_NONE;
volatile unsigned int adj_slot = ADJ_SLOT_NONE;

void setup() {

	ssd1306_begin();
	ssd1306_cleardisplay();

	DDRD &= ~BUTTON_SET & ~BUTTON_ADJ & ~ALARM_INPUT; // configure as inputs
	PORTD |= BUTTON_SET | BUTTON_ADJ | ALARM_INPUT; // pull input pins up

	PCICR |= (1 << PCIE2); //Interrupt on pin change FROM PCINT16 to PCINT23 Digital
	PCMSK2 = (1 << PCINT19) | (1 << PCINT20);

	RTC.alarm(ALARM_1); //clear the alarm flag
	RTC.squareWave(SQWAVE_NONE); //configure the INT/SQW pin for "interrupt" operation (disable square wave output and save power)
	RTC.alarmInterrupt(ALARM_1, true); //enable interrupt output for Alarm 1

	char buf[2];
	strncpy(buf, HOUR_MIN_SEP, 1);
	printStr(&buf[0], 1, 4, 6, 2);

}

ISR(PCINT2_vect) {
	if (!(PIND & BUTTON_SET)) {
		button = BUTTON_SET;
		btnmillis = millis();
		if (++ adj_slot > ADJ_SLOT_DAY) {
			adj_slot = ADJ_SLOT_SEC;
		}
		button_lp = BUTTON_NONE;
	} else if (!(PIND & BUTTON_ADJ)) {
		button = BUTTON_ADJ;
		btnmillis = millis();
		button_lp = BUTTON_NONE;
	} else {
		button = BUTTON_NONE;
		button_lp = BUTTON_NONE;
	}
}

void loop() {
	tmElements_t tm;
	if (!RTC.read(tm)) {
		time_t t = makeTime(tm);
		printDateTime(tm, t);
		printTemp(RTC.temperature());
		if (status == STATUS_NONE && button == BUTTON_SET
				&& millis() - btnmillis > LONG_PRESS_DELAY) {
			status = STATUS_SET;
			button = BUTTON_NONE;
			adj_slot = ADJ_SLOT_SEC;
		} else if (status == STATUS_SET && button == BUTTON_SET
				&& millis() - btnmillis > LONG_PRESS_DELAY) {
			status = STATUS_NONE;
			button = BUTTON_NONE;
		} else if (status == STATUS_SET && button_lp == BUTTON_ADJ
				&& millis() - btnmillis > LONG_PRESS_DELAY) {
			if (millis() - btnmillis_lp > CONT_ADJ_LP_DELAY) {
				adj(tm);
				btnmillis_lp = millis();
			}
		} else if (status == STATUS_SET && button == BUTTON_ADJ) {
			adj(tm);
			button = BUTTON_NONE;
			button_lp = BUTTON_ADJ;
		} else if (status == STATUS_SET && millis() - btnmillis > 60000
				&& second(t) == 0) {
			status = STATUS_NONE;
		}
	}

}

void printTemp(int tempCX4) {
	char buf[4];
	sprintf(buf, FMT_TEMP, tempCX4 / 4, 0xF8);
	if (strcmp(disp_temp, buf) != 0) {
		printStr(&buf[0], 4, 0, 0, 2);
		strncpy(disp_temp, buf, 4);
	}
}

void printDateTime(const tmElements_t &tm, time_t t) {
	printDay(tm);
	printYear(tm);
	printMon(tm);
	printDate(tm);
	printHour(tm);
	printMin(tm);
	printSec(tm);
}

void printDay(const tmElements_t &tm) {
	char *dt = days_in_week[tm.Wday - 1];
	char buf[4];

	if (status == STATUS_SET && adj_slot == ADJ_SLOT_DAY && button != BUTTON_ADJ
			&& button_lp != BUTTON_ADJ && millis() % 500 > 300) {
		strncpy(buf, CHAR_EMPTY_3, 3);
	} else {
		strncpy(buf, dt, 3);
	}
	if (strcmpl(disp_day, buf, 3) != 0) {
		printStr(&buf[0], 3, 5, 0, 2);
		strncpy(disp_day, buf, 3);
	}
}

void printYear(const tmElements_t &tm) {
	printDigits(ADJ_SLOT_YEAR, (unsigned int) tm.Year + 1970, &disp_year[0],
			4, 6, 3, 1);
}

void printMon(const tmElements_t &tm) {
	printDigits(ADJ_SLOT_MONTH, (unsigned int) tm.Month, &disp_mon[0], 2, 11,
			3, 1);
}

void printDate(const tmElements_t &tm) {
	printDigits(ADJ_SLOT_DATE, (unsigned int) tm.Day, &disp_date[0], 2, 14,
			3, 1);
}

void printHour(const tmElements_t &tm) {
	printDigits(ADJ_SLOT_HR, (unsigned int) tm.Hour, &disp_hr[0], 2, 2, 6,
			2);
}

void printMin(const tmElements_t &tm) {
	printDigits(ADJ_SLOT_MIN, (unsigned int) tm.Minute, &disp_min[0], 2, 5,
			6, 2);
}

void printSec(const tmElements_t &tm) {
	printDigits(ADJ_SLOT_SEC, (unsigned int) tm.Second, &disp_sec[0], 2, 14,
			7, 1);
}

void printDigits(unsigned int _adj_slot, unsigned int val, char* disp,
		unsigned int len, unsigned int col, unsigned int page,
		unsigned int size) {
	char buf[len + 1];
	if (status == STATUS_SET && adj_slot == _adj_slot && button != BUTTON_ADJ
			&& button_lp != BUTTON_ADJ && millis() % 500 > 300) {
		strncpy(buf, (len == 2 ?
		CHAR_EMPTY_2 :
									(len == 3 ? CHAR_EMPTY_3 : CHAR_EMPTY_4)),
				len);
	} else {
		sprintf(buf, len == 2 ? FMT_TWO_DIGITS : FMT_DIGITS, val);
	}
	if (strcmp(disp, buf) != 0) {
		printStr(&buf[0], len, col, page, size);
		strncpy(disp, buf, len);
	}
}

void printStr(char* buf, unsigned int len, unsigned int col, unsigned int page,
		unsigned int size) {
	unsigned int i = 0;
	do {
		ssd1306_drawchar((8 * size) * (col + i), page, buf[i], size);
	} while (++i < len);
}

void adj(tmElements_t &tm) {
	if (adj_slot == ADJ_SLOT_SEC) {
		tm.Second = 0;
	} else if (adj_slot == ADJ_SLOT_HR) {
		tm.Hour = tm.Hour < 23 ? tm.Hour + 1 : 0;
	} else if (adj_slot == ADJ_SLOT_MIN) {
		tm.Minute = tm.Minute < 59 ? tm.Minute + 1 : 0;
	} else if (adj_slot == ADJ_SLOT_YEAR) {
		tm.Year = tm.Year < 130 ? tm.Year + 1 : 30;
	} else if (adj_slot == ADJ_SLOT_MONTH) {
		tm.Month = tm.Month < 12 ? tm.Month + 1 : 1;
	} else if (adj_slot == ADJ_SLOT_DATE) {
		tm.Day = tm.Day + 1;
		tmElements_t tm2;
		breakTime(makeTime(tm), tm2);
		tm.Day = tm2.Day;
	} else if (adj_slot == ADJ_SLOT_DAY) {
		tm.Wday = tm.Wday < 7 ? tm.Wday + 1 : 1;
	}
	RTC.write(tm);
}

int strcmpl(const char* s0, const char* s1, unsigned int len) {
	for (int x = 0; x < len; x++) {
		if (s0[x] != s1[x]) {
			return 1;
		}
	}
	return 0;
}
