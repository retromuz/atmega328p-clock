/*********************************************************************
 This is an example for our Monochrome OLEDs based on SSD1306 drivers
 Pick one up today in the adafruit shop!
 ------> http://www.adafruit.com/category/63_98
 This example is for a 128x64 size display using SPI to communicate
 4 or 5 pins are required to interface
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!
 Written by Limor Fried/Ladyada  for Adafruit Industries.
 BSD license, check license.txt for more information
 All text above, and the splash screen must be included in any redistribution
 *********************************************************************/

#include "ssd1306_spi.h"

PortReg *mosiport, *clkport;
PortMask mosipinmask, clkpinmask;

int8_t _vccstate;

void ssd1306_begin() {
    _vccstate = SSD1306_SWITCHCAPVCC;
    // set pins for software-SPI
    pinMode(SSD_1306_SPI_SID, OUTPUT);
    pinMode(SSD_1306_SPI_SCK, OUTPUT);
    clkport = portOutputRegister(digitalPinToPort(SSD_1306_SPI_SCK));
    clkpinmask = digitalPinToBitMask(SSD_1306_SPI_SCK);
    mosiport = portOutputRegister(digitalPinToPort(SSD_1306_SPI_SID));
    mosipinmask = digitalPinToBitMask(SSD_1306_SPI_SID);

    // Setup reset pin direction (used by both SPI and I2C)
    pinMode(SSD_1306_SPI_RST, OUTPUT);
    digitalWrite(SSD_1306_SPI_RST, HIGH);
    // VDD (3.3V) goes high at start, lets just chill for a ms
    delay(1);
    // bring reset low
    digitalWrite(SSD_1306_SPI_RST, LOW);
    // wait 10ms
    delay(10);
    // bring out of reset
    digitalWrite(SSD_1306_SPI_RST, HIGH);

    // Init sequence for 128x64 OLED module
    ssd1306_command(SSD1306_DISPLAYOFF);                    // 0xAE
    ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
    ssd1306_command(0x80);                           // the suggested ratio 0x80
    ssd1306_command(SSD1306_SETMULTIPLEX);                  // 0xA8
    ssd1306_command(0x3F);
    ssd1306_command(SSD1306_SETDISPLAYOFFSET);              // 0xD3
    ssd1306_command(0x0);                                   // no offset
    ssd1306_command(SSD1306_SETSTARTLINE | 0x0);            // line #0
    ssd1306_command(SSD1306_CHARGEPUMP);                    // 0x8D
    if (_vccstate == SSD1306_EXTERNALVCC) {
        ssd1306_command(0x10);
    } else {
        ssd1306_command(0x14);
    }
    ssd1306_command(SSD1306_MEMORYMODE);                    // 0x20
    ssd1306_command(0x00);                                // 0x0 act like ks0108
    ssd1306_command(SSD1306_SEGREMAP | 0x1);
    ssd1306_command(SSD1306_COMSCANDEC);
    ssd1306_command(SSD1306_SETCOMPINS);                    // 0xDA
    ssd1306_command(0x12);
    ssd1306_command(SSD1306_SETCONTRAST);                   // 0x81
    if (_vccstate == SSD1306_EXTERNALVCC) {
        ssd1306_command(0x9F);
    } else {
        ssd1306_command(0x01);
    }
    ssd1306_command(SSD1306_SETPRECHARGE);                  // 0xd9
    if (_vccstate == SSD1306_EXTERNALVCC) {
        ssd1306_command(0x22);
    } else {
        ssd1306_command(0xF1);
    }
    ssd1306_command(SSD1306_SETVCOMDETECT);                 // 0xDB
    ssd1306_command(0x40);
    ssd1306_command(SSD1306_DISPLAYALLON_RESUME);           // 0xA4
    ssd1306_command(SSD1306_NORMALDISPLAY);                 // 0xA6

    ssd1306_command(SSD1306_DISPLAYON);                 //--turn on oled panel
}

void ssd1306_cleardisplay() {
    ssd1306_command(SSD1306_COLUMNADDR);
    ssd1306_command(0);   // Column start address (0 = reset)
    ssd1306_command(SSD1306_LCDWIDTH - 1); // Column end address (127 = reset)

    ssd1306_command(SSD1306_PAGEADDR);
    ssd1306_command(0); // Page start address (0 = reset)
    ssd1306_command(7); // Page end address

    // SPI

    for (unsigned int i = 0; i < (SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8);
            i++) {
        *clkport &= ~clkpinmask;
        *mosiport |= mosipinmask;	//d_c: 0=command
        *clkport |= clkpinmask;
        fastSPIwrite(0x00);
    }
}

// Draw a character
void ssd1306_drawchar(unsigned int column, unsigned int page, unsigned char c,
        unsigned int size) {

#ifndef __EXTENDED_CHARSET
    c = c - 32;
#endif

    if (size == 1) {
        for (unsigned int i = 0; i < 6; i++) {
            unsigned int line;
            if (i == 5)
                line = 0x0;
            else
                line = font[(c * 5) + i];
            ssd1306_draw(column + i, page, line);
        }
    } else {
        for (unsigned int i = 0; i < 6; i++) {
            unsigned int line;
            if (i == 5)
                line = 0x0;
            else
                line = font[(c * 5) + i];
            unsigned int line0, line1;
            line0 = 0;
            line1 = 0;
            for (unsigned int bit = 0; bit < 4; bit++) {
                line0 |= ((line & (1 << bit)) ? 1 : 0) << (bit * 2);
                line0 |= ((line & (1 << bit)) ? 1 : 0) << ((bit * 2) + 1);
                line1 |= ((line & (1 << (bit + 4))) ? 1 : 0) << (bit * 2);
                line1 |= ((line & (1 << (bit + 4))) ? 1 : 0) << ((bit * 2) + 1);
                ssd1306_draw(column + i, page, line0);
                ssd1306_draw(column + i + 1, page, line0);
                ssd1306_draw(column + i, page + 1, line1);
                ssd1306_draw(column + i + 1, page + 1, line1);
            }
            ++column;
        }
    }
}

void ssd1306_draw(unsigned int column, unsigned int page, unsigned int val) {
    ssd1306_command(SSD1306_COLUMNADDR);
    ssd1306_command(column);   // Column start address (0 = reset)
    ssd1306_command(column);   // Column end address (127 = reset)

    ssd1306_command(SSD1306_PAGEADDR);
    ssd1306_command(page);   // Page start address (0 = reset)
    ssd1306_command(page);   // Page end address

// SPI
    *clkport &= ~clkpinmask;
    *mosiport |= mosipinmask;   //d_c: 0=command
    *clkport |= clkpinmask;

    fastSPIwrite(val);
}

void ssd1306_command(unsigned int c) {
// SPI
    *clkport &= ~clkpinmask;
    *mosiport &= ~mosipinmask;   //d_c: 0=command
    *clkport |= clkpinmask;

    fastSPIwrite(c);
}

inline void fastSPIwrite(unsigned int d) {
    for (unsigned int bit = 0x80; bit; bit >>= 1) {
        *clkport &= ~clkpinmask;
        if (d & bit)
            *mosiport |= mosipinmask;
        else
            *mosiport &= ~mosipinmask;
        *clkport |= clkpinmask;
    }
}

