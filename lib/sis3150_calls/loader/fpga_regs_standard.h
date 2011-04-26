/* -*- c++ -*- */
/*
 * Copyright 2003,2004,2006 Free Software Foundation, Inc.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.	If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef INCLUDED_FPGA_REGS_STANDARD_H
#define INCLUDED_FPGA_REGS_STANDARD_H

// Register numbers 0 to 31 are reserved for use in fpga_regs_common.h.
// Registers 64 to 79 are available for custom FPGA builds.


// DDC / DUC

#define	FR_INTERP_RATE		32	// [1,1024]
#define	FR_DECIM_RATE		33	// [1,256]

// DDC center freq

#define FR_RX_FREQ_0		34
#define FR_RX_FREQ_1		35
#define FR_RX_FREQ_2		36
#define FR_RX_FREQ_3		37

// See below for DDC Starting Phase

// ------------------------------------------------------------------------
//  configure FPGA Rx mux
//
//    3                   2                   1                       
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
// +-----------------------+-------+-------+-------+-------+-+-----+
// |      must be zero     | Q3| I3| Q2| I2| Q1| I1| Q0| I0|Z| NCH |
// +-----------------------+-------+-------+-------+-------+-+-----+
//
// There are a maximum of 4 digital downconverters in the the FPGA.
// Each DDC has two 16-bit inputs, I and Q, and two 16-bit outputs, I & Q.
//
// DDC I inputs are specified by the two bit fields I3, I2, I1 & I0
//
//   0 = DDC input is from ADC 0
//   1 = DDC input is from ADC 1
//   2 = DDC input is from ADC 2
//   3 = DDC input is from ADC 3
//
// If Z == 1, all DDC Q inputs are set to zero
// If Z == 0, DDC Q inputs are specified by the two bit fields Q3, Q2, Q1 & Q0
//
// NCH specifies the number of complex channels that are sent across
// the USB.  The legal values are 1, 2 or 4, corresponding to 2, 4 or
// 8 16-bit values.

#define	FR_RX_MUX		38

// ------------------------------------------------------------------------
//  configure FPGA Tx Mux.
//
//    3                   2                   1                       
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
// +-----------------------+-------+-------+-------+-------+-+-----+
// |                       | DAC3  | DAC2  | DAC1  |  DAC0 |0| NCH |
// +-----------------------------------------------+-------+-+-----+
//
// NCH specifies the number of complex channels that are sent across
// the USB.  The legal values are 1 or 2, corresponding to 2 or 4
// 16-bit values.
//
// There are two interpolators with complex inputs and outputs.
// There are four DACs.  (We use the DUC in each AD9862.)
//
// Each 4-bit DACx field specifies the source for the DAC and
// whether or not that DAC is enabled.  Each subfield is coded
// like this: 
//
//    3 2 1 0
//   +-+-----+
//   |E|  N  |
//   +-+-----+
//
// Where E is set if the DAC is enabled, and N specifies which
// interpolator output is connected to this DAC.
//
//  N   which interp output
// ---  -------------------
//  0   chan 0 I
//  1   chan 0 Q
//  2   chan 1 I
//  3   chan 1 Q

#define	FR_TX_MUX		39

// ------------------------------------------------------------------------
// REFCLK control
//
// Control whether a reference clock is sent to the daughterboards,
// and what frequency.  The refclk is sent on d'board i/o pin 0.
//
//    3                   2                   1                       
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
// +-----------------------------------------------+-+------------+
// |             Reserved (Must be zero)           |E|   DIVISOR  |
// +-----------------------------------------------+-+------------+

//
// Bit 7  -- 1 turns on refclk, 0 allows IO use
// Bits 6:0 Divider value

#define FR_TX_A_REFCLK          40
#define FR_RX_A_REFCLK          41
#define FR_TX_B_REFCLK          42
#define FR_RX_B_REFCLK          43

#  define bmFR_REFCLK_EN	   0x80
#  define bmFR_REFCLK_DIVISOR_MASK 0x7f

// ------------------------------------------------------------------------
// DDC Starting Phase

#define FR_RX_PHASE_0		44
#define FR_RX_PHASE_1		45
#define FR_RX_PHASE_2		46
#define FR_RX_PHASE_3		47

// ------------------------------------------------------------------------
// Tx data format control register
//
//    3                   2                   1                       
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
// +-------------------------------------------------------+-------+
// |                    Reserved (Must be zero)            |  FMT  |
// +-------------------------------------------------------+-------+
//
//  FMT values:

#define FR_TX_FORMAT		48
#  define bmFR_TX_FORMAT_16_IQ		0	// 16-bit I, 16-bit Q

// ------------------------------------------------------------------------
// Rx data format control register
//
//    3                   2                   1                       
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
// +-----------------------------------------+-+-+---------+-------+
// |          Reserved (Must be zero)        |B|Q|  WIDTH  | SHIFT |
// +-----------------------------------------+-+-+---------+-------+
//
//  FMT values:

#define FR_RX_FORMAT		49

#  define bmFR_RX_FORMAT_SHIFT_MASK	(0x0f <<  0)	// arithmetic right shift [0, 15]
#  define bmFR_RX_FORMAT_SHIFT_SHIFT	0
#  define bmFR_RX_FORMAT_WIDTH_MASK	(0x1f <<  4)    // data width in bits [1, 16] (not all valid)
#  define bmFR_RX_FORMAT_WIDTH_SHIFT    4
#  define bmFR_RX_FORMAT_WANT_Q		(0x1  <<  9)    // deliver both I & Q, else just I
#  define bmFR_RX_FORMAT_BYPASS_HB	(0x1  << 10)    // bypass half-band filter

// Soon the valid combinations will be:
//
//   B  Q  WIDTH  SHIFT
//   x  0    16     0
//   x  1    16     0
//   x  0     8     8
//   x  1     8     8

// Possible future values of WIDTH = {4, 2, 1}
// 12 takes a bit more work, since we need to know packet alignment.

// ------------------------------------------------------------------------
// FIXME register numbers 50 to 63 are available

// ------------------------------------------------------------------------
// Registers 64 to 79 are reserved for user custom FPGA builds.
// The standard USRP software will not touch these.

#define FR_USER_0	64
#define FR_USER_1	65
#define FR_USER_2	66
#define FR_USER_3	67
#define FR_USER_4	68
#define FR_USER_5	69
#define FR_USER_6	70
#define FR_USER_7	71
#define FR_USER_8	72
#define FR_USER_9	73
#define FR_USER_10	74
#define FR_USER_11	75
#define FR_USER_12	76
#define FR_USER_13	77
#define FR_USER_14	78
#define FR_USER_15	79


// =======================================================================
// READBACK Registers
// =======================================================================

#define FR_RB_IO_RX_A_IO_TX_A	1	// read back a-side i/o pins
#define	FR_RB_IO_RX_B_IO_TX_B	2	// read back b-side i/o pins

// ------------------------------------------------------------------------
// FPGA Capability register
//
//    3                   2                   1                       
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
// +-----------------------------------------------+-+-----+-+-----+
// |                    Reserved (Must be zero)    |T|NDUC |R|NDDC |
// +-----------------------------------------------+-+-----+-+-----+
//
// Bottom 4-bits are Rx capabilities
// Next   4-bits are Tx capabilities

#define	FR_RB_CAPS	3
#  define bmFR_RB_CAPS_NDDC_MASK	(0x7 << 0)   // # of digital down converters 0,1,2,4
#  define bmFR_RB_CAPS_NDDC_SHIFT	0
#  define bmFR_RB_CAPS_RX_HAS_HALFBAND	(0x1 << 3)
#  define bmFR_RB_CAPS_NDUC_MASK        (0x7 << 4)   // # of digital up converters 0,1,2
#  define bmFR_RB_CAPS_NDUC_SHIFT	4
#  define bmFR_RB_CAPS_TX_HAS_HALFBAND	(0x1 << 7)


#endif /* INCLUDED_FPGA_REGS_STANDARD_H */
