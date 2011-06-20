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

#ifndef CAEN_V1290_H
#define CAEN_V1290_H

#define CAEN1290_MEB        0x0000 //D32
#define CAEN1290_CONTROL    0x1000
#define CAEN1290_STATUS     0x1002
#define CAEN1290_IRQ_LVL    0x100A
#define CAEN1290_IRQ_VEC    0x100C
#define CAEN1290_GEO_ADDR   0x100E
#define CAEN1290_CBLT_ADDR  0x1010
#define CAEN1290_CBLT_CTRL  0x1012
#define CAEN1290_RESET      0x1014
#define CAEN1290_SCLR       0x1016
#define CAEN1290_SEVRESET   0x1018
#define CAEN1290_STRIG      0x101A
#define CAEN1290_EVCNT      0x101C //D32
#define CAEN1290_EVSTOR     0x1020
#define CAEN1290_AMFULL_LVL 0x1022
#define CAEN1290_BLT_EVNR   0x1024
#define CAEN1290_FIRM_REV   0x1026
#define CAEN1290_TESTREG    0x1028 //D32
#define CAEN1290_OUTP_CTRL  0x102C
#define CAEN1290_MICRO      0x102E
#define CAEN1290_MICRO_HS   0x1030
#define CAEN1290_SEL_FLASH  0x1032
#define CAEN1290_FLASH_MEM  0x1034
#define CAEN1290_SRAM_PAGE  0x1036
#define CAEN1290_EV_FIFO    0x1038 //D32
#define CAEN1290_EVFIFO_STR 0x103C
#define CAEN1290_EVFIFO_STA 0x103E
#define CAEN1290_DUMMY32    0x1200 //D32
#define CAEN1290_DUMMY16    0x1204
#define CAEN1290_CROM_BASE  0x4000
#define CAEN1290_COMP_BASE  0x8000
#define CAEN1290_ADER32     0x1004
#define CAEN1290_ADER24     0x1006
#define CAEN1290_ADEREN     0x1008

#define CAEN1290_CTL_BERREN         0
#define CAEN1290_CTL_TERM           1
#define CAEN1290_CTL_TERM_SW        2
#define CAEN1290_CTL_EMPTYEV        3
#define CAEN1290_CTL_ALIGN64        4
#define CAEN1290_CTL_COMPEN         5
#define CAEN1290_CTL_TESTFIFOEN     6
#define CAEN1290_CTL_READCOMPEN     7
#define CAEN1290_CTL_EVFIFOEN       8
#define CAEN1290_CTL_EXTRGTIMETAG   9

#define CAEN1290_STA_DREADY         0
#define CAEN1290_STA_ALM_FULL       1
#define CAEN1290_STA_FULL           2
#define CAEN1290_STA_TRG_MATCH      3
#define CAEN1290_STA_HEADER_EN      4
#define CAEN1290_STA_TERM_ON        5
#define CAEN1290_STA_ERROR0         6
#define CAEN1290_STA_ERROR1         7
#define CAEN1290_STA_ERROR2         8
#define CAEN1290_STA_ERROR3         9
#define CAEN1290_STA_BERRF          10
#define CAEN1290_STA_PURG           11
#define CAEN1290_STA_RES_0          12
#define CAEN1290_STA_RES_1          13
#define CAEN1290_STA_PAIR           14
#define CAEN1290_STA_TRIGGER_LOST   15

#define CAEN1290_MCH_WRITE_OK       0
#define CAEN1290_MCH_READ_OK        1

#define MC1290_TRG_MATCH            0x0000
#define MC1290_CONT_STOR            0x0100
#define MC1290_READ_ACQ_MOD         0x0200
#define MC1290_SET_KEEP_TOKEN       0x0300
#define MC1290_CLR_KEEP_TOKEN       0x0400
#define MC1290_LOAD_DEF_CONFIG      0x0500
#define MC1290_SAVE_USER_CONFIG     0x0600
#define MC1290_LOAD_USER_CONFIG     0x0700
#define MC1290_AUTOLOAD_USER_CONFIG 0x0800
#define MC1290_AUTOLOAD_DEF_CONFIG  0x0900

#define MC1290_SET_WIN_WIDTH        0x1000
#define MC1290_SET_WIN_OFFS         0x1100
#define MC1290_SET_SW_MARGIN        0x1200
#define MC1290_SET_REJ_MARGIN       0x1300
#define MC1290_EN_SUB_TRG           0x1400
#define MC1290_DIS_SUB_TRG          0x1500
#define MC1290_READ_TRG_CONF        0x1600

#define MC1290_SET_DETECTION        0x2200
#define MC1290_READ_DETECTION       0x2300
#define MC1290_SET_TR_LEAD_LSB      0x2400
#define MC1290_SET_PAIR_RES         0x2500
#define MC1290_READ_RES             0x2600
#define MC1290_SET_DEAD_TIME        0x2800
#define MC1290_READ_DEAD_TIME       0x2900

#define MC1290_EN_HEAD_TRAILER      0x3000
#define MC1290_DIS_HEAD_TRAILER     0x3100
#define MC1290_READ_HEAD_TRAILER    0x3200
#define MC1290_SET_EVENT_SIZE       0x3300
#define MC1290_READ_EVENT_SIZE      0x3400
#define MC1290_EN_ERROR_MARK        0x3500
#define MC1290_DIS_ERROR_MARK       0x3600
#define MC1290_EN_ERROR_BYPASS      0x3700
#define MC1290_DIS_ERROR_BYPASS     0x3800
#define MC1290_SET_ERROR_TYPES      0x3900
#define MC1290_READ_ERROR_TYPES     0x3A00
#define MC1290_SET_FIFO_SIZE        0x3B00
#define MC1290_READ_FIFO_SIZE       0x3C00

#define MC1290_EN_CHANNEL           0x4000 //nn: Channel
#define MC1290_DIS_CHANNEL          0x4100 //nn: Channel
#define MC1290_EN_ALL_CH            0x4200
#define MC1290_DIS_ALL_CH           0x4300
#define MC1290_WRITE_EN_PATTERN     0x4400
#define MC1290_READ_EN_PATTERN      0x4500
#define MC1290_WRITE_EN_PATTERN32   0x4600
#define MC1290_READ_EN_PATTERN32    0x4700

#define MC1290_SET_GLOB_OFFS        0x5000
#define MC1290_READ_GLOB_OFFS       0x5100
#define MC1290_SET_ADJUST_CH        0x5200 //nn: Channel
#define MC1290_READ_ADJUST_CH       0x5300 //nn: Channel
#define MC1290_SET_RC_ADJUST        0x5400 //0n: tdc
#define MC1290_READ_RC_ADJ          0x5500 //0n: tdc
#define MC1290_SAVE_RC_ADJUST       0x5600 //0n: tdc

#define MC1290_READ_TDC_ID          0x6000 //0n: tdc
#define MC1290_READ_MICRO_REV       0x6100
#define MC1290_RESET_DLL_PLL        0x6200

#define MC1290_WRITE_SETUP_REG      0x7000 //nn: word num
#define MC1290_READ_SETUP_REG       0x7100 //nn: word num
#define MC1290_UPDATE_SETUP_REG     0x7200
#define MC1290_DEFAULT_SETUP_REG    0x7300
#define MC1290_READ_ERROR_STATUS    0x7400 //0n: tdc
#define MC1290_READ_DLL_LOCK        0x7500 //0n: tdc
#define MC1290_READ_STATUS_STREAM   0x7600 //0n: tdc
#define MC1290_UPDATE_SETUP_TDC     0x7700 //nn: tdc

#define MC1290_WRITE_EEPROM         0xC000
#define MC1290_READ_EEPROM          0xC100
#define MC1290_REV_DATE_MICRO_FW    0xC200
#define MC1290_WRITE_SPARE          0xC300
#define MC1290_READ_SPARE           0xC400
#define MC1290_ENABLE_TEST_MODE     0xC500
#define MC1290_DISABLE_TEST_MODE    0xC600
#define MC1290_SET_TDC_TSET_OUTPUT  0xC700 //0n: tdc
#define MC1290_SET_DLL_CLOCK        0xC800
#define MC1290_READ_SETUP_SCANPATH  0xC900 //0n: tdc

#endif // CAEN_V1290_H
