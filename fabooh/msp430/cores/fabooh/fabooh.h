/*
 * fabooh.h - like Energia.h but not ...
 *
 * Created: Nov-12-2012
 *  Author: rick@kimballsoftware.com
 *    Date: 03-02-2013
 * Version: 1.0.2
 *
 * =========================================================================
 *  Copyright © 2013 Rick Kimball
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FABOOH_H_
#define FABOOH_H_

#define FABOOH_VER 0x0100
#define FABOOH_BLD 0x0002

#ifdef __GNUC__
#define ALWAYS_INLINE inline __attribute__((always_inline))
#define NEVER_INLINE __attribute__((noinline))
#else
#define ALWAYS_INLINE inline
#define NEVER_INLINE
#warning This code takes advantage of features only found in msp430-gcc!!!
#endif

#include <msp430.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "logic.h"
#include "ringbuffer.h"
#include "cpu.h"
#include "gpio.h"
#include "print.h"

#include <pins.h> // driven by the selected board -I ${FABOOH_CORE}/variants/${BOARD}

ALWAYS_INLINE
void setup(void);
ALWAYS_INLINE
void loop(void);

#endif /* FABOOH_H_ */
