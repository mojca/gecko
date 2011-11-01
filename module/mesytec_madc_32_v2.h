#ifndef MESYTEC_MADC_32_V2_H
#define MESYTEC_MADC_32_V2_H

#include <stdint.h>

// Global definitions
#define MADC32V2_NUM_CHANNELS 32
#define MADC32V2_NUM_BITS 14

#define MADC32V2_SIZE_MEMORY_DATA_WORDS 1026
#define MADC32V2_SIZE_MEMORY_THRESHOLDS_WORDS MADC32V2_NUM_CHANNELS

#define MADC32V2_VAL_THRESHOLD_SWITCH_OFF 0x1fff
#define MADC32V2_VAL_THRESHOLD_UNUSED     0x0000
#define MADC32V2_VAL_IRQ_THRESHOLD_MAX    8120

#define MADC32V2_LEN_EVENT_MAX 34

// Firmware
#define MADC32V2_2_EXPECTED_FIRMWARE 0x0202

// Data structures

typedef union {
    struct {
        uint32_t data_length    :12;
        uint32_t adc_resolution :3;
        uint32_t output_format  :1;
        uint32_t module_id      :8;
        uint32_t zero           :6;
        uint32_t signature      :2;
    }bits;
    uint32_t data;
} madc32_header_t;

//typedef union {
//    struct {
//        uint32_t signature      :2;
//        uint32_t zero           :6;
//        uint32_t module_id      :8;
//        uint32_t output_format  :1;
//        uint32_t adc_resolution :3;
//        uint32_t data_length    :12;
//    }bits;
//    uint32_t data;
//} madc32_header_t;


typedef union {
    struct {
        uint32_t value          :14;
        uint32_t out_of_range   :1;
        uint32_t zero           :1;
        uint32_t channel        :5;
        uint32_t sub_signature  :9;
        uint32_t signature      :2;
    }bits;
    uint32_t data;
} madc32_data_t;

//typedef union {
//    struct {
//        uint32_t signature      :2;
//        uint32_t sub_signature  :9;
//        uint32_t channel        :5;
//        uint32_t zero           :1;
//        uint32_t out_of_range   :1;
//        uint32_t value          :14;
//    }bits;
//    uint32_t data;
//} madc32_data_t;


typedef union {
    struct {
        uint32_t timestamp      :16;
        uint32_t zero           :5;
        uint32_t sub_signature  :9;
        uint32_t signature      :2;
    } bits;
    uint32_t data;
} madc32_extended_timestamp_t;

//typedef union {
//    struct {
//        uint32_t signature      :2;
//        uint32_t sub_signature  :9;
//        uint32_t zero           :5;
//        uint32_t timestamp      :16;
//    } bits;
//    uint32_t data;
//} madc32_extended_timestamp_t;


typedef union {
    uint32_t zero;
} madc32_dummy_t;


typedef union {
    struct {
        uint32_t trigger_counter:30;
        uint32_t signature      :2;
    }bits;
    uint32_t data;
} madc32_end_of_event_t;

//typedef union {
//    struct {
//        uint32_t signature      :2;
//        uint32_t trigger_counter:30;
//    }bits;
//    uint32_t data;
//} madc32_end_of_event_t;


typedef union {
    struct {
        uint32_t threshold      :13;
        uint32_t zero           :3;
    }bits;
    uint32_t data;
} madc32_threshold_t;

//typedef union {
//    struct {
//        uint32_t zero           :3;
//        uint32_t threshold      :13;
//    }bits;
//    uint32_t data;
//} madc32_threshold_t;



// Readout reset
// Single  -> Allow new trigger
// Mode: 1 -> check thr, set irq
// Mode: 3 -> clear BERR, allow next readout

// Multievent Mode 2 and 3:
// IRQ is set, when fill level > threshold
// IRQ is unset, when IRQ ACK or fill level < threshold


// Registers
#define MADC32V2_DATA_FIFO          0x0000
#define MADC32V2_THRESHOLD_MEM      0x4000
#define MADC32V2_ADDR_SOURCE        0x6000 // 0=board, 1=addr_reg
#define MADC32V2_ADDR_REGISTER      0x6002 // 16 bit
#define MADC32V2_MODULE_ID          0x6004 // 8 bit, written to header
#define MADC32V2_SOFT_RESET         0x6008 // 1 bit, reset
#define MADC32V2_FIRMWARE_REVISION  0x600E // e.g. 0x0110 = 1.10

// IRQ (only ROAK)
#define MADC32V2_IRQ_LEVEL          0x6010 // 3 bit
#define MADC32V2_IRQ_VECTOR         0x6012 // 8 bit
#define MADC32V2_IRQ_TEST           0x6014 // init IRQ test
#define MADC32V2_IRQ_RESET          0x6016 // reset IRQ test
#define MADC32V2_IRQ_THRESHOLD      0x6018 // 13 bit, max = 8120
#define MADC32V2_MAX_TRANSFER_DATA  0x601A // 14 bit, multivent == 3
#define MADC32V2_IRQ_WITHDRAW       0x601C // 1 bit

// CBLT, MCST
#define MADC32V2_CBLT_MCST_CTRL     0x6020 // 8 bit
#define MADC32V2_CBLT_ADDRESS       0x6022 // 8 bit
#define MADC32V2_MCST_ADDRESS       0x6024 // 8 bit

// FIFO handling
#define MADC32V2_BUFFER_DATA_LENGTH 0x6030 // 16 bit
#define MADC32V2_DATA_LENGTH_FORMAT 0x6032 // 2 bit
#define MADC32V2_READOUT_RESET      0x6034 // only write
#define MADC32V2_MULTIEVENT_MODE    0x6036 // 4 bit, 0 == single
#define MADC32V2_MARKING_TYPE       0x6038 // 2 bit
#define MADC32V2_START_ACQUISITION  0x600A // 1 bit
#define MADC32V2_FIFO_RESET         0x603C // only write, init FIFO
#define MADC32V2_DATA_READY         0x603E // DRDY == 1

// Operation mode
#define MADC32V2_BANK_MODE          0x6040 // 2 bit
#define MADC32V2_ADC_RESOLUTION     0x6042 // 3 bit
#define MADC32V2_OUTPUT_FORMAT      0x6044 // 1 bit, 0 == mesytec
#define MADC32V2_ADC_OVERRIDE       0x6046 // 2 bit (overrides the output width)
#define MADC32V2_SLIDING_SCALE_OFF  0x6048 // 1 bit, 1 == off
#define MADC32V2_SKIP_OUT_OF_RANGE  0x604A // 1 bit, 1 == skip
#define MADC32V2_IGNORE_THRESHOLDS  0x604C // 1 bit, 1 == thresholds are ignored

// Gate generator
#define MADC32V2_HOLD_DELAY_0       0x6050 // 8 bit, 0 == 25 ns, 1 == 150 ns
#define MADC32V2_HOLD_DELAY_1       0x6052 // 8 bit, for bank 1
#define MADC32V2_HOLD_WIDTH_0       0x6054 // 8 bit, multiple of 50 ns
#define MADC32V2_HOLD_WIDTH_1       0x6056 // 8 bit, for bank 1
#define MADC32V2_USE_GATE_GENERATOR 0x6058 // 2 bit

// Inputs, outputs
#define MADC32V2_INPUT_RANGE        0x6060 // 2 bit
#define MADC32V2_ECL_TERMINATED     0x6062 // 3 bit, switch off, when inputs unused
#define MADC32V2_ECL_GATE1_OSC      0x6064 // 1 bit, 0 == gate, 1 == osc (0x6096!)
#define MADC32V2_ECL_FAST_CLEAR_RST 0x6066 // 1 bit, 0 == fclr, 1 == reset timestamp
#define MADC32V2_ECL_BUSY           0x6068 // 1 bit, 0 == busy, 1 unused
#define MADC32V2_NIM_GATE1_OSC      0x606A // 1 bit, 0 == gate, 1 == osc (0x6096!)
#define MADC32V2_NIM_FAST_CLEAR_RST 0x606C // 1 bit, 0 == fclr, 1 == reset timestamp
#define MADC32V2_NIM_BUSY           0x606E // 4 bit

// Test pulser
#define MADC32V2_PULSER_STATUS      0x6070 // 4 bit

// Mesytec control bus
#define MADC32V2_RC_BUSNO           0x6080 // 2 bit, 0 == external
#define MADC32V2_RC_MODNUM          0x6082 // 4 bit, module id [0..15]
#define MADC32V2_RC_OPCODE          0x6084 // 7 bit
#define MADC32V2_RC_ADDR            0x6086 // 8 bit, internal address
#define MADC32V2_RC_DATA            0x6088 // 16 bit
#define MADC32V2_RC_SEND_RET_STATUS 0x608A // 4 bit

// Counters:
// =========
// Read the counters in the order of low, high
// Latched at low word read
#define MADC32V2_RESET_COUNTER_AB   0x6090 // 4 bit
#define MADC32V2_EVENT_COUNTER_LOW  0x6092 // 16 bit
#define MADC32V2_EVENT_COUNTER_HIGH 0x6094 // 16 bit
#define MADC32V2_TIMESTAMP_SOURCE   0x6096 // 2 bit
#define MADC32V2_TIMESTAMP_DIVISOR  0x6098 // 16 bit, ts = t / div, 0 == 65536
#define MADC32V2_TIMESTAMP_CNT_L    0x609C // 16 bit
#define MADC32V2_TIMESTAMP_CNT_H    0x609E // 16 bit

#define MADC32V2_ADC_BUSY_TIME_LOW  0x60A0 // 16 bit, [1 us]
#define MADC32V2_ADC_BUSY_TIME_HIGH 0x60A2 // 16 bit,
#define MADC32V2_GATE_1_TIME_LOW    0x60A4 // 16 bit, when NIM is active, [1 us]
#define MADC32V2_GATE_1_TIME_HIGH   0x60A6 // 16 bit
#define MADC32V2_TIME_0             0x60A8 // 16 bit, [1 us], 48 bit total
#define MADC32V2_TIME_1             0x60AA // 16 bit
#define MADC32V2_TIME_2             0x60AC // 16 bit
#define MADC32V2_STOP_COUNTER       0x60AE // 2 bit

// Signatures
#define MADC32V2_SIG_DATA       0x0
#define MADC32V2_SIG_HEADER     0x1
#define MADC32V2_SIG_END_BERR   0x2
#define MADC32V2_SIG_END        0x3

// Sub-signatures
#define MADC32V2_SIG_DATA_EVENT 0x20
#define MADC32V2_SIG_DATA_TIME  0x24
#define MADC32V2_SIG_DATA_DUMMY 0x00

// Acquisition modes:
// ==================
// Mode: 0 -> Single event
// Mode: 1 -> Unlimited transfer, no readout reset needed (no CBLT)
// Mode: 3 -> Limited transfer until threshold, then BERR
//
// Bit 2 set: Sends EOB with SIGNATURE == 0x2, otherwise 0x3 and BERR
// Bit 3 set: Compares number of transmitted events with max_transfer_data
//            for BERR condition
#define MADC32V2_VAL_MULTIEVENT_MODE_OFF        0x0
#define MADC32V2_VAL_MULTIEVENT_MODE_1          0x1
#define MADC32V2_VAL_MULTIEVENT_MODE_2          0x2
#define MADC32V2_VAL_MULTIEVENT_MODE_3          0x3
#define MADC32V2_VAL_MULTIEVENT_MODE_EOB_BERR   0x4
#define MADC32V2_VAL_MULTIEVENT_MODE_MAX_DATA   0x8

// Bank operation modes:
// =====================
// Mode: 0 -> banks are connected
// Mode: 1 -> banks are independent
// Mode: 3 -> toggle between banks for zero deadtime
#define MADC32V2_VAL_BANK_MODE_CONNECTED        0x0
#define MADC32V2_VAL_BANK_MODE_INDEPENDENT      0x1
#define MADC32V2_VAL_BANK_MODE_TOGGLE           0x3

// Gate generator modes:
// =====================
// Gate generator 1 can only be active, when banks are not connected
#define MADC32V2_VAL_GG_MODE_OFF                0x0
#define MADC32V2_VAL_GG_MODE_ONLY_0             0x1
#define MADC32V2_VAL_GG_MODE_ONLY_1             0x2
#define MADC32V2_VAL_GG_MODE_BOTH               0x3

// Adc resolution
#define MADC32V2_VAL_ADC_RESOLUTION_2K          0x0 // 800 ns
#define MADC32V2_VAL_ADC_RESOLUTION_4K          0x1 // 1.6 us
#define MADC32V2_VAL_ADC_RESOLUTION_4K_HIRES    0x2 // 3.2 us
#define MADC32V2_VAL_ADC_RESOLUTION_8K          0x3 // 6.4 us
#define MADC32V2_VAL_ADC_RESOLUTION_8K_HIRES    0x4 // 12.5 us

// Input ranges
#define MADC32V2_VAL_INPUT_RANGE_4V             0x0
#define MADC32V2_VAL_INPUT_RANGE_10V            0x1
#define MADC32V2_VAL_INPUT_RANGE_8V             0x2

// Marking types
#define MADC32V2_VAL_MARKING_TYPE_EVENT_COUNTER 0x0
#define MADC32V2_VAL_MARKING_TYPE_TIMESTAMP     0x1
#define MADC32V2_VAL_MARKING_TYPE_EXTENDED_TS   0x3

// NIM Busy output modes
#define MADC32V2_VAL_NIM_BUSY_AS_BUSY           0x0
#define MADC32V2_VAL_NIM_BUSY_AS_GATE_0_OUT     0x1
#define MADC32V2_VAL_NIM_BUSY_AS_GATE_1_OUT     0x2
#define MADC32V2_VAL_NIM_BUSY_AS_CBUS_OUT       0x3
#define MADC32V2_VAL_NIM_BUSY_AS_BUF_FULL       0x4
#define MADC32V2_VAL_NIM_BUSY_AS_BUF_OVER_THR   0x8

// Pulser status
#define MADC32V2_VAL_PULSER_STATUS_OFF          0x0
#define MADC32V2_VAL_PULSER_STATUS_AMP_0        0x4
#define MADC32V2_VAL_PULSER_STATUS_AMP_LOW      0x5
#define MADC32V2_VAL_PULSER_STATUS_AMP_HIGH     0x6
#define MADC32V2_VAL_PULSER_STATUS_AMP_TOGGLE   0x7

// Counter reset mode
#define MADC32V2_VAL_RST_COUNTER_AB_ALL_A       0x1
#define MADC32V2_VAL_RST_COUNTER_AB_ALL_B       0x2
#define MADC32V2_VAL_RST_COUNTER_AB_ALL         0x3
#define MADC32V2_VAL_RST_COUNTER_AB_A_EXT       0xb

// Timestamp source:
// =================
// Bit 1 set: enable external reset
#define MADC32V2_VAL_TIMESTAMP_SOURCE_VME       0x0
#define MADC32V2_VAL_TIMESTAMP_SOURCE_EXTERNAL  0x1
#define MADC32V2_VAL_TIMESTAMP_SOURCE_EXT_RST   0x2

// Stop/Run Counters
#define MADC32V2_VAL_STOP_COUNTER_B_RUN         0x0
#define MADC32V2_VAL_STOP_COUNTER_B_STOP        0x1
#define MADC32V2_VAL_STOP_COUNTER_TS_RUN        0x0
#define MADC32V2_VAL_STOP_COUNTER_TS_STOP       0x2
#define MADC32V2_VAL_STOP_COUNTER_BOTH          0x3

// Mesytec control bus:
// ====================
// - Set MADC32V2_NIM_BUSY to MADC32V2_VAL_NIM_BUSY_AS_CBUS_OUT
// - Wait 100 us until output is configured (or poll register)
//
// Send time on the bus is 400 us
// Wait at least this amount before reading response

// Control bus address
#define MADC32V2_VAL_RC_ADDR_XXXXXX

// Control bus return status:
// ==========================
// Gate 0 led: bus traffic
// Gate 1 led: bus error
#define MADC32V2_VAL_RC_RET_STATUS_ACTIVE       0x1
#define MADC32V2_VAL_RC_RET_STATUS_COLLISION    0x2
#define MADC32V2_VAL_RC_RET_STATUS_NO_RESPONSE  0x4

// Control bus opcodes
#define MADC32V2_VAL_RC_OPCODE_ON               0x3
#define MADC32V2_VAL_RC_OPCODE_OFF              0x4
#define MADC32V2_VAL_RC_OPCODE_READ_ID          0x6
#define MADC32V2_VAL_RC_OPCODE_WRITE_DATA       0x16
#define MADC32V2_VAL_RC_OPCODE_READ_DATA        0x18

// Offsets
#define MADC32V2_OFF_DATA_SIG   30

#define MADC32V2_MSK_DATA_SIG   0x3

#define MADC32V2_OFF_CBLT_MCST_CTRL_DISABLE_CBLT         0
#define MADC32V2_OFF_CBLT_MCST_CTRL_ENABLE_CBLT          1
#define MADC32V2_OFF_CBLT_MCST_CTRL_DISABLE_LAST_MODULE  2
#define MADC32V2_OFF_CBLT_MCST_CTRL_ENABLE_LAST_MODULE   3
#define MADC32V2_OFF_CBLT_MCST_CTRL_DISABLE_FIRST_MODULE 4
#define MADC32V2_OFF_CBLT_MCST_CTRL_ENABLE_FIRST_MODULE  5
#define MADC32V2_OFF_CBLT_MCST_CTRL_DISABLE_MCST         6
#define MADC32V2_OFF_CBLT_MCST_CTRL_ENABLE_MCST          7

#define MADC32V2_OFF_CBLT_ADDRESS 24
#define MADC32V2_OFF_MCST_ADDRESS 24

#define MADC32V2_OFF_ECL_TERMINATION_GATE_0 0
#define MADC32V2_OFF_ECL_TERMINATION_GATE_1 1
#define MADC32V2_OFF_ECL_TERMINATION_FCLEAR 2

// Lengths
#define MADC32V2_LEN_DATA_SIG   2

// Data handling:
// ==============
// FIFO has 8k x 32 bit
// Event structure, maximum size of event is 34 words
//
// Event structure:
// 0 : header
// 1 : data
// ...
// n-1 : data
// n : end of event



#endif // MESYTEC_MADC_32_V2_H
