/*
Copyright 2011 Bastian Loeher, Roland Wirth

This file is part of GECKO.

GECKO is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

GECKO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CAEN_V965_H
#define CAEN_V965_H
// $Id$
// definitions and data structures for CAEN v965 Dual Range QDCs

#include <stdint.h>

typedef struct {
	uint8_t unused1;
	uint8_t cnt		:6;
        uint8_t unused2		:2;
	uint8_t crate;
	uint8_t id		:3; // should be 0b010
	uint8_t geo		:5;
} t_v965_hdr;

typedef struct {
	uint16_t adc		:12;
	uint8_t  ov		: 1;
	uint8_t  un		: 1;
	uint8_t  dummy1		: 2;
	uint8_t  ch		: 4;
	uint8_t  dummy2		: 3;
	uint8_t  id		: 3; // should be 0b000
        uint8_t  geo		: 5;
} t_v965_data;

typedef struct {
        uint32_t ev_cnt		:24;
	uint8_t  id		: 3; // should be0b100
        uint8_t  geo		: 5;
} t_v965_eob;

// v965 register macros
#define CAEN965_MEB		0x0000
#define CAEN965_FIRMWARE	0x1000
#define CAEN965_GEO_ADDR	0x1002
#define CAEN965_CBLT_ADDR	0x1004
#define CAEN965_BIT_SET1	0x1006
#define CAEN965_BIT_CLR1	0x1008
#define CAEN965_IRQ_LVL		0x100A
#define CAEN965_IRQ_VEC		0x100C
#define CAEN965_STAT1		0x100E
#define CAEN965_CONTROL1	0x1010
#define CAEN965_ADER_HIGH	0x1012
#define CAEN965_ADER_LOW	0x1014
#define CAEN965_SINGLE_RST	0x1016
#define CAEN965_CBLT_CTRL	0x101A
#define CAEN965_EV_TRG		0x1020
#define CAEN965_STAT2		0x1022
#define CAEN965_EVCNT_L		0x1024
#define CAEN965_EVCNT_H		0x1026
#define CAEN965_INCR_EV		0x1028
#define CAEN965_INCR_OFFSET	0x102A
#define CAEN965_LD_TESTREG	0x102C
#define CAEN965_FCLR_WND	0x102E
#define CAEN965_BIT_SET2	0x1032
#define CAEN965_BIT_CLR2	0x1034
#define CAEN965_W_TESTADDR	0x1036
#define CAEN965_MEMTEST_H	0x1038
#define CAEN965_MEMTEST_L	0x103A
#define CAEN965_CRATE_SEL	0x103C
#define CAEN965_TEST_EV_W	0x103E
#define CAEN965_EVCNT_RST	0x1040
#define CAEN965_IPED		0x1060
#define CAEN965_R_TESTADDR	0x1064
#define CAEN965_SW_COMM		0x1068
#define CAEN965_SLD_CONSTANT	0x106A
#define CAEN965_AAD		0x1070
#define CAEN965_BAD		0x1072
#define CAEN965_THRESHOLDS	0x1080

// ROM
#define CAEN965_ROM_OUIMSB	0x8026
#define CAEN965_ROM_OUI		0x802a
#define CAEN965_ROM_OUILSB	0x802e
#define CAEN965_ROM_VERSION	0x8032
#define CAEN965_ROM_BOARDIDMSB	0x8036
#define CAEN965_ROM_BOARDID	0x803a
#define CAEN965_ROM_BOARDIDLSB	0x803e
#define CAEN965_ROM_REVISION	0x804e
#define CAEN965_ROM_SerialMSB	0x8F02
#define CAEN965_ROM_SerialLSB	0x8F06

// bits
#define CAEN965_B1_BERRF	3
#define CAEN965_B1_SELADDR	4
#define CAEN965_B1_SOFTRST	7

#define CAEN965_S1_DREADY	0
#define CAEN965_S1_GDREADY	1
#define CAEN965_S1_BUSY		2
#define CAEN965_S1_GBUSY	3
//#define CAEN965_S1_AMNESIA	4 // NOT IMPLEMENTED
#define CAEN965_S1_PURGED	5
//#define CAEN965_S1_TERMON	6 // NOT IMPLEMENTED
//#define CAEN965_S1_TERMOFF	7 // NOT IMPLEMENTED
#define CAEN965_S1_EVRDY	8

#define CAEN965_C1_BLKEND	2
#define CAEN965_C1_PROGRST	4
#define CAEN965_C1_BERREN	5
#define CAEN965_C1_ALIGN64	6

#define CAEN965_S2_BUFEMPTY	1
#define CAEN965_S2_BUFFULL	2
#define CAEN965_S2_DSEL0	4
#define CAEN965_S2_DSEL1	5
#define CAEN965_S2_CSEL0	6
#define CAEN965_S2_CSEL1	7

#define CAEN965_B2_TESTMEM	0
#define CAEN965_B2_OFFLINE	1
#define CAEN965_B2_CLRDATA	2
#define CAEN965_B2_OVDIS	3
#define CAEN965_B2_UNDIS	4
#define CAEN965_B2_TESTACQ	6
#define CAEN965_B2_SLIDEN	7
#define CAEN965_B2_STEPTH	8
#define CAEN965_B2_AUTOINC	11
#define CAEN965_B2_EMPTYEN	12
#define CAEN965_B2_SLSUBEN	13
#define CAEN965_B2_ALLTRG	14
#define CAEN965_B2_MASK		(~((1<<15)|(1<<10)|(1<<9)|(1<<5)))

#define CAEN965_THRESH_KILL	8
#endif // CAEN_V965_H
