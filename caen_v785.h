#ifndef CAEN_785
#define CAEN_785
// 20050512 A. Shevchenko
// control structure for CAEN VME ADC V785

#include <stdint.h>

typedef struct	// header
{
uint8_t dummy;
uint8_t cnt	: 6;	// number of memorized channels
uint8_t	unused	: 2;
uint8_t crate;		// crate number according to Crate Select Register
uint8_t id	: 3;	// Header identifier - must be = 2 = %010
uint8_t geo	: 5;	// GEO address
} t_v785_hdr;

typedef struct	// data
{
uint16_t adc	: 12;	// ADC value 12 bit
uint8_t	ov	: 1;	// Overflow bit. 0 - not in overflow condition
uint8_t	un	: 1;	// Underthreshold bit. 1-value below
uint8_t  vd      : 1;    // dummy? Valid data bit? 1-data valid
uint8_t	dummy	: 1;
uint8_t	ch	: 6;
uint8_t	dummy2	: 2;
uint8_t	id	: 3;	// for data must be %000
uint8_t geo	: 5;	// GEO address
} t_v785_data;

typedef struct	// eob
{
uint32_t	ev_cnt	: 24;	// event counter
uint8_t	id	: 3;	// for eob must be %100
uint8_t	geo	: 5;
} t_v785_eob;	// End of Block

typedef struct
{
// t_v785_ev[16]; //w: 
uint32_t meb[512]; 	// r:	multi event buffer	0x0000
uint32_t dummy[512];	// not implemented
uint16_t firmware; 	// r:	firmware version	0x1000
uint16_t geo;	  	// rw:	GEo Address		0x1002
uint16_t mcst_addr; 	// rw:	MSCT/CBLT Address	0x1004
uint16_t bset1;	// rw:	Bit Set 1		0x1006
uint16_t bclr1;	// rw:	Bit Clear 1		0x1008
uint16_t ilev;		// rw: 	interrupt level		0x100A
uint16_t ivec;	// rw:	interrupt vector	0x100C
uint16_t stat1;	// r:	status register 1	0x100E
uint16_t creg1;		// rw:  control register 1	0x1010
uint16_t ader_h;		// rw:	ADER high		0x1012
uint16_t ader_l;		// rw:	ADER low		0x1014
uint16_t ssrst;	// w:	Single Shot reset	0x1016
uint16_t dumm1;		// not implemented
uint16_t mcst_reg;	// rw:	MSCT/CBLT Register	0x101A
uint16_t dumm2[2];	// not implemented
uint16_t evtreg;		// rw:	event trigger register	0x1020
uint16_t stat2;	// r:	status register 2	0x1022
uint16_t cnt_l;	// r:	event counter low	0x1024
uint16_t cnt_h;	// r:	event counter high	0x1026
uint16_t incev;		// w:	increment event		0x1028
uint16_t incoff;		// w:	increment offset	0x102A
uint16_t ltestreg;	// rw:	load test register	0x102C
uint16_t fclrwin;	// rw:	fclr window		0x102E
uint16_t dumm3;		// not implemented
uint16_t bset2;	// rw:	Bit Set 2		0x1032
uint16_t bclr2;	// w:?	Bit Clear 2		0x1034
uint16_t wmtest;	// w:	W Memory test address	0x1036
uint16_t mtest_h;	// w:	memory test high	0x1038
uint16_t mtest_l;	// w:	memory test low		0x103A
uint16_t cratesel;	// rw:	Crate Select		0x103C
uint16_t tew;	// w:	Test event Write	0x103E
uint16_t cnt_rst;	// w:	Event Counter Reset	0x1040
uint16_t dumm4[17];	// not implemented
uint16_t r_test;		// w:	R Test Address		0x1064
uint16_t dumm6;		// not impl
uint16_t sw_comm;	// w:	SW Comm			0x1068
uint16_t slide;		// rw:	Slide Constant		0x106A
uint16_t dumm7[2];	// not impl
uint16_t aad;		// r:	AAD			0x1070
uint16_t bad;		// r:	BAD			0x1072
uint16_t dumm8[6];	// not impl
uint16_t thr[32];	// rw:	Thresholds		0x1080

// 0x8026 OUI MSB
// 0x802a OUI
// 0x802e OUI LSB
// 0x8032 Version
// 0x8036 BOARD ID MSB
// 0x803a BOARD ID
// 0x803e BOARD ID LSB
// 0x8F02 Serial MSB
// 0x8F06 Serial LSB

} t_v785;	// length of the structure must be 0x10C0

// Registers
#define CAEN785_MEB         0x0000
#define CAEN785_FIRMWARE    0x1000
#define CAEN785_BIT_SET1    0x1006
#define CAEN785_BIT_CLR1    0x1008
#define CAEN785_IRQ_LVL     0x100A
#define CAEN785_IRQ_VEC     0x100C
#define CAEN785_STAT1       0x100E
#define CAEN785_CONTROL1    0x1010
#define CAEN785_ADER_HIGH   0x1012
#define CAEN785_ADER_LOW    0x1014
#define CAEN785_SINGLE_RST  0x1016
#define CAEN785_EV_TRG      0x1020
#define CAEN785_STAT2       0x1022
#define CAEN785_EVCNT_L     0x1024
#define CAEN785_EVCNT_H     0x1026
#define CAEN785_INCR_EV     0x1028
#define CAEN785_INCR_OFFSET 0x102A
#define CAEN785_BIT_SET2    0x1032
#define CAEN785_BIT_CLR2    0x1034
#define CAEN785_CRATE_SEL   0x103C
#define CAEN785_EVCNT_RST   0x103E
#define CAEN785_SW_COMM     0x1068
#define CAEN785_TEST_CONV   0x1068  // alias
#define CAEN785_SLIDE_CONST 0x106A
#define CAEN785_THRESHOLDS  0x1080

// ROM defines
#define CAEN785_ROM_OUIMSB     0x8026
#define CAEN785_ROM_OUI        0x802a
#define CAEN785_ROM_OUILSB     0x802e
#define CAEN785_ROM_Version    0x8032
#define CAEN785_ROM_BOARDIDMSB 0x8036
#define CAEN785_ROM_BOARDID    0x803a
#define CAEN785_ROM_BOARDIDLSB 0x803e
#define CAEN785_ROM_SerialMSB  0x8F02
#define CAEN785_ROM_SerialLSB  0x8F06

#endif
