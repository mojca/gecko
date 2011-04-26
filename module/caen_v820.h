#ifndef CAEN_V820_H
#define CAEN_V820_H

#define CAEN820_MEB         0x0000 // D32
#define CAEN820_CTR00       0x1000 // D32
#define CAEN820_CTR01       0x1004 // D32
#define CAEN820_CTR02       0x1008 // D32
#define CAEN820_CTR03       0x100C // D32
#define CAEN820_CTR04       0x1010 // D32
#define CAEN820_CTR05       0x1014 // D32
#define CAEN820_CTR06       0x1018 // D32
#define CAEN820_CTR07       0x101C // D32
#define CAEN820_CTR08       0x1020 // D32
#define CAEN820_CTR09       0x1024 // D32
#define CAEN820_CTR10       0x1028 // D32
#define CAEN820_CTR11       0x102C // D32
#define CAEN820_CTR12       0x1030 // D32
#define CAEN820_CTR13       0x1034 // D32
#define CAEN820_CTR14       0x1038 // D32
#define CAEN820_CTR15       0x103C // D32
#define CAEN820_CTR16       0x1040 // D32
#define CAEN820_CTR17       0x1044 // D32
#define CAEN820_CTR18       0x1048 // D32
#define CAEN820_CTR19       0x104C // D32
#define CAEN820_CTR20       0x1050 // D32
#define CAEN820_CTR21       0x1054 // D32
#define CAEN820_CTR22       0x1058 // D32
#define CAEN820_CTR23       0x105C // D32
#define CAEN820_CTR24       0x1060 // D32
#define CAEN820_CTR25       0x1064 // D32
#define CAEN820_CTR26       0x1068 // D32
#define CAEN820_CTR27       0x106C // D32
#define CAEN820_CTR28       0x1070 // D32
#define CAEN820_CTR29       0x1074 // D32
#define CAEN820_CTR30       0x1078 // D32
#define CAEN820_CTR31       0x107C // D32
#define CAEN820_TESTREG     0x1080 // D32
#define CAEN820_CH_EN       0x1100 // D32
#define CAEN820_DWELL       0x1104 // D32
#define CAEN820_CONTROL     0x1108 // D16
#define CAEN820_CTL_BS      0x110A // D16
#define CAEN820_CTL_BC      0x110C // D16
#define CAEN820_STATUS      0x110E // D16
#define CAEN820_GEO         0x1110 // D16
#define CAEN820_IRQ_LVL     0x1112 // D16
#define CAEN820_IRQ_VEC     0x1114 // D16
#define CAEN820_ADERH       0x1116 // D16
#define CAEN820_ADERL       0x1118 // D16
#define CAEN820_ADEREN      0x111A // D16
#define CAEN820_MCST_ADDR   0x111C // D16
#define CAEN820_MCST_CTL    0x111E // D16
#define CAEN820_SRESET      0x1120 // D16
#define CAEN820_SCLR        0x1122 // D16
#define CAEN820_STRIG       0x1124 // D16
#define CAEN820_TRIG_CNT    0x1128 // D32
#define CAEN820_ALM_FULL    0x112C // D16
#define CAEN820_FIRMWARE    0x1132 // D16
#define CAEN820_MEB_EV_NUM  0x1134 // D16
#define CAEN820_MBLT_EV_NUM 0x1130 // D16


#define CAEN820_CTL_ACQ0    0
#define CAEN820_CTL_ACQ1    1
#define CAEN820_CTL_DFMT    2
#define CAEN820_CTL_TEST    3
#define CAEN820_CTL_BERREN  4
#define CAEN820_CTL_HDREN   5
#define CAEN820_CTL_CLRMEB  6
#define CAEN820_CTL_AUTORST 7

#define CAEN820_STA_DREADY  0
#define CAEN820_STA_ALMFULL 1
#define CAEN820_STA_FULL    2
#define CAEN820_STA_GDREADY 3
#define CAEN820_STA_GBUSY   4
#define CAEN820_STA_TERMON  5
#define CAEN820_STA_TERMOFF 6
#define CAEN820_STA_BERRF   7

#endif // CAEN_V820_H
