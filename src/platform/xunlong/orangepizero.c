/*
	Copyright (c) 2016 CurlyMo <curlymoo1@gmail.com>

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <signal.h>

#include "../../soc/soc.h"
#include "../../wiringx.h"
#include "../platform.h"
#include "orangepizero.h"

struct platform_t *orangepizero = NULL;

/*
	 3v|5v
PA12  8|5v
PA11  9|0v
PA6	  7|15 PG6
	 0v|16 PG7
PA1   0| 1 PA7
PA0	  2|0v
PA3	  3| 4 PA19
	 3v| 5 PA18
PA15 12|0v
PA16 13| 6 PA2
PA14 14|10 PA13
	 0v|11 PA10


LED RED = ??
LED GREEN = ??
*/

static int irq[] = {
 /*  0,   1,   2,   3 */
     1,   7,   0,   3,
 /*  4,   5,   6,   7 */
    19,  18,   2,   6,
 /*  8,   9,  10,  11 */
	12,  11,  13,  10,
 /* 12,  13,  14   15 */
    15,  16,  14,  88,
/*  16 */
    89
};

static int map[] = {
 /*  PA1,  PA7,  PA0,  PA3 */
       1,    7,    0,    3,
 /* PA19, PA18,  PA2,  PA6 */
      19,   18,    2,    6,
 /* PA12, PA11, PA13, PA10 */
      12,   11,   13,   10,
 /* PA15,  PA16,PA14  PG6 */
      15,    16,  14,   88,
/*   PG7 */
      89	  
};

static int orangepizeroValidGPIO(int pin) {
	if(pin >= 0 && pin < (sizeof(map)/sizeof(map[0]))) {
		if(map[pin] == -1) {
			return -1;
		}
		return 0;
	} else {
		return -1;
	}
}

static int orangepizeroPinMode(int i, enum pinmode_t mode) {
	if(map[i] == -1) {
		return -1;
	}
	return orangepizero->soc->pinMode(i, mode);
}

static int orangepizeroDigitalWrite(int i, enum digital_value_t value) {
	if(map[i] == -1) {
		return -1;
	}
	return orangepizero->soc->digitalWrite(i, value);
}

static int orangepizeroDigitalRead(int i) {
	/* Red LED - Green LED *
	if(i == 19 || i == 20) {
		return -1;
	}*/
	return orangepizero->soc->digitalRead(i);
}

static int orangepizeroSetup(void) {
	const size_t msize = sizeof(map) / sizeof(map[0]);
	const size_t qsize = sizeof(irq) / sizeof(irq[0]);
	orangepizero->soc->setup();
	orangepizero->soc->setMap(map, msize);
	orangepizero->soc->setIRQ(irq, qsize);
	return 0;
}

static int orangepizeroISR(int i, enum isr_mode_t mode) {
	if(irq[i] == -1) {
		return -1;
	}
	orangepizero->soc->isr(i, mode);
	return 0;
}

void orangepizeroInit(void) {
	platform_register(&orangepizero, "orangepizero");

	orangepizero->soc = soc_get("Allwinner", "H3");

	orangepizero->digitalRead = &orangepizeroDigitalRead;
	orangepizero->digitalWrite = &orangepizeroDigitalWrite;
	orangepizero->pinMode = &orangepizeroPinMode;
	orangepizero->setup = &orangepizeroSetup;

	orangepizero->isr = &orangepizeroISR;
	orangepizero->waitForInterrupt = orangepizero->soc->waitForInterrupt;

	orangepizero->selectableFd = orangepizero->soc->selectableFd;
	orangepizero->gc = orangepizero->soc->gc;

	orangepizero->validGPIO = &orangepizeroValidGPIO;
}
