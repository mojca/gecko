/***************************************************************************/
/*                                                                         */
/*  Filename: SIS3302.h                                                    */
/*                                                                         */
/*  Funktion: headerfile for SIS3302					                   */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 10.01.2006                                       */
/*  last modification:    07.06.2006                                       */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  SIS  Struck Innovative Systeme GmbH                                    */
/*                                                                         */
/*  Harksheider Str. 102A                                                  */
/*  22399 Hamburg                                                          */
/*                                                                         */
/*  Tel. +49 (0)40 60 87 305 0                                             */
/*  Fax  +49 (0)40 60 87 305 20                                            */
/*                                                                         */
/*  http://www.struck.de                                                   */
/*                                                                         */
/*  � 2006                                                                 */
/*                                                                         */
/***************************************************************************/

#define SIS3302_CONTROL_STATUS                       0x0	  /* read/write; D32 */
#define SIS3302_MODID                                0x4	  /* read only; D32 */
#define SIS3302_IRQ_CONFIG                           0x8      /* read/write; D32 */
#define SIS3302_IRQ_CONTROL                          0xC      /* read/write; D32 */
#define SIS3302_ACQUISITION_CONTROL                  0x10      /* read/write; D32 */
#define SIS3302_START_DELAY                         0x14      /* read/write; D32 */
#define SIS3302_STOP_DELAY                          0x18      /* read/write; D32 */

#define SIS3302_MAX_NOF_EVENT                       0x20      /* read/write; D32 */
#define SIS3302_ACTUAL_EVENT_COUNTER                0x24      /* read; D32 */


#define SIS3302_CBLT_BROADCAST_SETUP                0x30      /* read/write; D32 */
#define SIS3302_ADC_MEMORY_PAGE_REGISTER            0x34      /* read/write; D32 */

#define SIS3302_DAC_CONTROL_STATUS                  0x50      /* read/write; D32 */
#define SIS3302_DAC_DATA                            0x54      /* read/write; D32 */

#define SIS3302_KEY_RESET                           0x400	  /* write only; D32 */
#define SIS3302_KEY_ARM                             0x410	  /* write only; D32 */
#define SIS3302_KEY_DISARM                          0x414	  /* write only; D32 */
#define SIS3302_KEY_START                           0x418	  /* write only; D32 */
#define SIS3302_KEY_STOP                            0x41C	  /* write only; D32 */
#define SIS3302_KEY_RESET_DDR2_LOGIC                0x428	  /* write only; D32 */
#define SIS3302_KEY_TIMESTAMP_CLEAR                 0x42C	  /* write only; D32 */



#define SIS3302_EVENT_CONFIG_ALL_ADC                0x01000000
#define SIS3302_SAMPLE_LENGTH_ALL_ADC               0x01000004
#define SIS3302_SAMPLE_START_ADDRESS_ALL_ADC        0x01000008
#define SIS3302_ADC_INPUT_MODE_ALL_ADC              0x0100000C

#define SIS3302_TRIGGER_FLAG_CLR_CNT_ALL_ADC		0x0100002C



#define SIS3302_ADC_INPUT_MODE_ADC12                0x0200000C

#define SIS3302_ACTUAL_SAMPLE_ADDRESS_ADC1          0x02000010
#define SIS3302_ACTUAL_SAMPLE_ADDRESS_ADC2          0x02000014

#define SIS3302_ACTUAL_SAMPLE_VALUE_ADC12           0x02000020

#define SIS3302_ACTUAL_SAMPLE_VALUE_ADC12           0x02000020
#define SIS3302_DDR2_TEST_REGISTER_ADC12			0x02000028
#define SIS3302_TRIGGER_FLAG_CLR_CNT_ADC12			0x0200002C

#define SIS3302_TRIGGER_SETUP_ADC1                  0x02000030
#define SIS3302_TRIGGER_THRESHOLD_ADC1              0x02000034
#define SIS3302_TRIGGER_SETUP_ADC2                  0x02000038
#define SIS3302_TRIGGER_THRESHOLD_ADC2              0x0200003C

#define SIS3302_EVENT_DIRECTORY_ADC1			    0x02010000	  /* read only; D32, BLT32, MBLT64; size: 512Lwords 0x800 Bytes */
#define SIS3302_EVENT_DIRECTORY_ADC2			    0x02018000	  /* read only; D32, BLT32, MBLT64; size: 512Lwords 0x800 Bytes */




#define SIS3302_ADC_INPUT_MODE_ADC34                0x0280000C

#define SIS3302_ACTUAL_SAMPLE_ADDRESS_ADC3          0x02800010
#define SIS3302_ACTUAL_SAMPLE_ADDRESS_ADC4          0x02800014

#define SIS3302_ACTUAL_SAMPLE_VALUE_ADC34           0x02800020
#define SIS3302_DDR2_TEST_REGISTER_ADC34			0x02800028
#define SIS3302_TRIGGER_FLAG_CLR_CNT_ADC34			0x0280002C

#define SIS3302_TRIGGER_SETUP_ADC3                  0x02800030
#define SIS3302_TRIGGER_THRESHOLD_ADC3              0x02800034
#define SIS3302_TRIGGER_SETUP_ADC4                  0x02800038
#define SIS3302_TRIGGER_THRESHOLD_ADC4              0x0280003C

#define SIS3302_EVENT_DIRECTORY_ADC3			    0x02810000	  /* read only; D32, BLT32, MBLT64; size: 512Lwords 0x800 Bytes */
#define SIS3302_EVENT_DIRECTORY_ADC4			    0x02818000	  /* read only; D32, BLT32, MBLT64; size: 512Lwords 0x800 Bytes */


#define SIS3302_ADC_INPUT_MODE_ADC56                0x0300000C

#define SIS3302_ACTUAL_SAMPLE_ADDRESS_ADC5          0x03000010
#define SIS3302_ACTUAL_SAMPLE_ADDRESS_ADC6          0x03000014

#define SIS3302_ACTUAL_SAMPLE_VALUE_ADC56           0x03000020
#define SIS3302_DDR2_TEST_REGISTER_ADC56			0x03000028
#define SIS3302_TRIGGER_FLAG_CLR_CNT_ADC56			0x0300002C

#define SIS3302_TRIGGER_SETUP_ADC5                  0x03000030
#define SIS3302_TRIGGER_THRESHOLD_ADC5              0x03000034
#define SIS3302_TRIGGER_SETUP_ADC6                  0x03000038
#define SIS3302_TRIGGER_THRESHOLD_ADC6              0x0300003C

#define SIS3302_EVENT_DIRECTORY_ADC5			    0x03010000	  /* read only; D32, BLT32, MBLT64; size: 512Lwords 0x800 Bytes */
#define SIS3302_EVENT_DIRECTORY_ADC6			    0x03018000	  /* read only; D32, BLT32, MBLT64; size: 512Lwords 0x800 Bytes */


#define SIS3302_ADC_INPUT_MODE_ADC78                0x0380000C

#define SIS3302_ACTUAL_SAMPLE_ADDRESS_ADC7          0x03800010
#define SIS3302_ACTUAL_SAMPLE_ADDRESS_ADC8          0x03800014

#define SIS3302_ACTUAL_SAMPLE_VALUE_ADC78           0x03800020
#define SIS3302_DDR2_TEST_REGISTER_ADC78			0x03800028
#define SIS3302_TRIGGER_FLAG_CLR_CNT_ADC78			0x0380002C

#define SIS3302_TRIGGER_SETUP_ADC7                  0x03800030
#define SIS3302_TRIGGER_THRESHOLD_ADC7              0x03800034
#define SIS3302_TRIGGER_SETUP_ADC8                  0x03800038
#define SIS3302_TRIGGER_THRESHOLD_ADC8              0x0380003C

#define SIS3302_EVENT_DIRECTORY_ADC7			    0x03810000	  /* read only; D32, BLT32, MBLT64; size: 512Lwords 0x800 Bytes */
#define SIS3302_EVENT_DIRECTORY_ADC8			    0x03818000	  /* read only; D32, BLT32, MBLT64; size: 512Lwords 0x800 Bytes */











#define SIS3302_ADC1_OFFSET                         0x04000000
#define SIS3302_ADC2_OFFSET                         0x04800000
#define SIS3302_ADC3_OFFSET                         0x05000000
#define SIS3302_ADC4_OFFSET                         0x05800000
#define SIS3302_ADC5_OFFSET                         0x06000000
#define SIS3302_ADC6_OFFSET                         0x06800000
#define SIS3302_ADC7_OFFSET                         0x07000000
#define SIS3302_ADC8_OFFSET                         0x07800000

#define SIS3302_NEXT_ADC_OFFSET                     0x00800000

/* define sample clock */
#define SIS3302_ACQ_SET_CLOCK_TO_100MHZ                 0x70000000  /* default after Reset  */
#define SIS3302_ACQ_SET_CLOCK_TO_50MHZ                  0x60001000
#define SIS3302_ACQ_SET_CLOCK_TO_25MHZ                  0x50002000
#define SIS3302_ACQ_SET_CLOCK_TO_10MHZ                  0x40003000
#define SIS3302_ACQ_SET_CLOCK_TO_1MHZ                   0x30004000
#define SIS3302_ACQ_SET_CLOCK_TO_LEMO_RANDOM_CLOCK_IN   0x20005000
#define SIS3302_ACQ_SET_CLOCK_TO_LEMO_CLOCK_IN          0x10006000
#define SIS3302_ACQ_SET_CLOCK_TO_P2_CLOCK_IN            0x00007000


#define SIS3302_ACQ_DISABLE_LEMO_START_STOP         0x01000000
#define SIS3302_ACQ_ENABLE_LEMO_START_STOP          0x00000100

#define SIS3302_ACQ_DISABLE_INTERNAL_TRIGGER        0x00400000
#define SIS3302_ACQ_ENABLE_INTERNAL_TRIGGER         0x00000040

#define SIS3302_ACQ_DISABLE_MULTIEVENT              0x00200000
#define SIS3302_ACQ_ENABLE_MULTIEVENT               0x00000020

#define SIS3302_ACQ_DISABLE_AUTOSTART               0x00100000
#define SIS3302_ACQ_ENABLE_AUTOSTART                0x00000010



/* bits of SIS3302_Event Configuration register ; D-register*/
#define EVENT_CONF_ENABLE_SAMPLE_LENGTH_STOP			0x20
#define EVENT_CONF_ENABLE_WRAP_PAGE_MODE				0x10

#define EVENT_CONF_PAGE_SIZE_16M_WRAP					0x0
#define EVENT_CONF_PAGE_SIZE_4M_WRAP					0x1
#define EVENT_CONF_PAGE_SIZE_1M_WRAP					0x2
#define EVENT_CONF_PAGE_SIZE_256K_WRAP					0x3

#define EVENT_CONF_PAGE_SIZE_64K_WRAP					0x4
#define EVENT_CONF_PAGE_SIZE_16K_WRAP					0x5
#define EVENT_CONF_PAGE_SIZE_4K_WRAP					0x6
#define EVENT_CONF_PAGE_SIZE_1K_WRAP					0x7

#define EVENT_CONF_PAGE_SIZE_512_WRAP					0x8
#define EVENT_CONF_PAGE_SIZE_256_WRAP					0x9
#define EVENT_CONF_PAGE_SIZE_128_WRAP					0xA
#define EVENT_CONF_PAGE_SIZE_64_WRAP					0xB

#define SIS3302_MAX_NOF_LWORDS 0x1000000
