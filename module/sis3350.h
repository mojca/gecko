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

/***************************************************************************/
/*                                                                         */
/*  Filename: sis3350.h                                                    */
/*                                                                         */
/*  Funktion: headerfile for SIS3350		                           */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 21.06.2007                                       */
/*  last modification:    28.08.2008                                       */
/*  last modification:    29.08.2008 MKI JTAG regs                         */
/*                        01.09.2008 MKI TN ch1/2 setup regs               */
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
/*  � 2008                                                                 */
/*                                                                         */
/***************************************************************************/

#ifndef SIS3350_H
#define SIS3350_H


struct SIS3350_Device_Config_Struct {
    unsigned int 	uintControlStatus;
    unsigned int 	uintInterruptConfiguration;
    unsigned int 	uintInterruptControl;
    unsigned int 	uintAcquisitionControl;
    unsigned int 	uintTriggerDelay	;
    unsigned int 	uintDirectMemorySampleLength	;
    unsigned int 	uintFrequencySynthesizer	;

    unsigned int 	uintMultiEventMaxNofEvents	;
    unsigned int 	uintGateSynchLimitLength	;
    unsigned int 	uintGateSynchExtendLength	;

    unsigned int 	uintRingbufferPreDelay	;
    unsigned int 	uintRingbufferSampleLength	;
    unsigned int 	uintEndAddressThreshold	;

    unsigned int 	uintDirectMemorySampleWrapLength	;


    unsigned int 	uintAdcGain[4]	;
    unsigned int 	uintAdcOffset[4]	;
    unsigned int 	uintExternalInputClockOffset	;
    unsigned int 	uintExternalInputTriggerOffset	;
    unsigned int 	uintTriggerSetup[4]	;
    unsigned int 	uintTriggerThreshold[4]	;
    unsigned int 	uintLemoOutSelect;
} ;

					 


#define SIS3350_CONTROL_STATUS                       		0x0	  /* read/write; D32 */
#define SIS3350_MODID                                		0x4	  /* read only; D32 */
#define SIS3350_IRQ_CONFIG                           		0x8      /* read/write; D32 */
#define SIS3350_IRQ_CONTROL                          		0xC      /* read/write; D32 */

#define SIS3350_ACQUISTION_CONTROL             			0x10      /* read/write; D32 */
#define SIS3350_ACQUISITION_CONTROL                             0x10      /* read/write; D32 */
													
#define SIS3350_TRIGGER_DELAY		             		0x14      /* read/write; D32 */
#define SIS3350_DIRECT_MEMORY_SAMPLE_LENGTH	       		0x18      /* read/write; D32 */
#define SIS3350_FREQUENCE_SYNTHESIZER               		0x1C      /* read/write; D32 */


#define SIS3350_MULTIEVENT_MAX_NOF_EVENTS	     		0x20      /* read/write; D32 */
#define SIS3350_MULTIEVENT_EVENT_COUNTER           		0x24      /* read ; D32 */

#define SIS3350_GATE_SYNCH_LIMIT_LENGTH	           		0x28      /* read/write; D32 */
#define SIS3350_GATE_SYNCH_EXTEND_LENGTH          		0x2C      /* read/write; D32 */


#define SIS3350_ADC_MEMORY_PAGE_REGISTER            		0x34      /* read/write; D32 */
#define SIS3350_LEMO_OUTPUT_SELECT_REGISTER         		0x38      /* read/write; D32 */

#define SIS3350_EXT_CLOCK_TRIGGER_DAC_CONTROL_STATUS 		0x50      /* read/write; D32 */
#define SIS3350_EXT_CLOCK_TRIGGER_DAC_DATA          		0x54      /* read/write; D32 */

#define SIS3350_XILINX_JTAG_TEST            		        0x60      /* write only; D32 */
#define SIS3350_XILINX_JTAG_DATA_IN            		        0x60      /* read  only; D32 */
#define SIS3350_XILINX_JTAG_CONTROL            		        0x64      /* write only; D32 */

#define SIS3350_INTERNAL_TEMPERATURE_REG            		0x70      /* read; D32 */
#define SIS3350_ADC_SERIAL_INTERFACE_REG            		0x74      /* read/write; D32 */



#define SIS3350_KEY_RESET                           		0x400	  /* write only; D32 */
#define SIS3350_KEY_ARM                             		0x410	  /* write only; D32 */
#define SIS3350_KEY_DISARM                          		0x414	  /* write only; D32 */
#define SIS3350_KEY_TRIGGER                         		0x418	  /* write only; D32 */
#define SIS3350_KEY_TIMESTAMP_CLR                   		0x41C	  /* write only; D32 */


		  

#define SIS3350_EVENT_CONFIG_ALL_ADC			  	0x01000000
#define SIS3350_DIRECT_MEMORY_SAMPLE_WRAP_LENGTH_ALL_ADC  	0x01000004	  
#define SIS3350_SAMPLE_START_ADDRESS_ALL_ADC     		0x01000008


#define SIS3350_RINGBUFFER_SAMPLE_LENGTH_ALL_ADC    		0x01000020	  
#define SIS3350_RINGBUFFER_PRE_DELAY_ALL_ADC        		0x01000024	  
#define SIS3350_END_ADDRESS_THRESHOLD_ALL_ADC       		0x01000028	  





#define SIS3350_EVENT_CONFIG_ADC12				0x02000000
#define SIS3350_DIRECT_MEMORY_SAMPLE_WRAP_LENGTH_ADC12  	0x02000004	  
#define SIS3350_SAMPLE_START_ADDRESS_ADC12     			0x02000008

#define SIS3350_ACTUAL_SAMPLE_ADDRESS_ADC1          		0x02000010	  
#define SIS3350_ACTUAL_SAMPLE_ADDRESS_ADC2          		0x02000014	  
#define SIS3350_PREVIOUS_BANK_SAMPLE_ADDRESS_ADC1   		0x02000018	  
#define SIS3350_PREVIOUS_BANK_SAMPLE_ADDRESS_ADC2   		0x0200001C	  


#define SIS3350_RINGBUFFER_SAMPLE_LENGTH_ADC12      		0x02000020	  
#define SIS3350_RINGBUFFER_PRE_DELAY_ADC12          		0x02000024	  
#define SIS3350_END_ADDRESS_THRESHOLD_ADC12       		0x02000028


#define SIS3350_TRIGGER_SETUP_ADC1                  		0x02000030	  
#define SIS3350_TRIGGER_THRESHOLD_ADC1              		0x02000034	  
#define SIS3350_TRIGGER_SETUP_ADC2                  		0x02000038	  
#define SIS3350_TRIGGER_THRESHOLD_ADC2              		0x0200003C	  

#define SIS3350_ADC_INPUT_TAP_DELAY_ADC1         		0x02000040
#define SIS3350_ADC_INPUT_TAP_DELAY_ADC2        		0x02000044
#define SIS3350_ADC_VGA_ADC1 			    		0x02000048
#define SIS3350_ADC_VGA_ADC2 			    		0x0200004C
														
#define SIS3350_ADC12_DAC_CONTROL_STATUS            		0x02000050      /* read/write; D32 */
#define SIS3350_ADC12_DAC_DATA                      		0x02000054      /* read/write; D32 */

#define SIS3350_SAMPLE_COUNTER_THRESHOLD_T2T1_ADC1     		0x02000070	  
#define SIS3350_SAMPLE_COUNTER_THRESHOLD_T4T3_ADC1     		0x02000074	  
#define SIS3350_SAMPLE_COUNTER_THRESHOLD_T2T1_ADC2     		0x02000078	  
#define SIS3350_SAMPLE_COUNTER_THRESHOLD_T4T3_ADC2     		0x0200007C	  

#define SIS3350_EVENT_CONFIG_ADC34                              0x03000000
#define SIS3350_DIRECT_MEMORY_SAMPLE_WRAP_LENGTH_ADC34  	0x03000004	  
#define SIS3350_SAMPLE_START_ADDRESS_ADC34     			0x03000008

#define SIS3350_ACTUAL_SAMPLE_ADDRESS_ADC3          		0x03000010	  
#define SIS3350_ACTUAL_SAMPLE_ADDRESS_ADC4          		0x03000014	  
#define SIS3350_PREVIOUS_BANK_SAMPLE_ADDRESS_ADC3   		0x03000018	  
#define SIS3350_PREVIOUS_BANK_SAMPLE_ADDRESS_ADC4   		0x0300001C	  

#define SIS3350_RINGBUFFER_SAMPLE_LENGTH_ADC34      		0x03000030	  
#define SIS3350_RINGBUFFER_PRE_DELAY_ADC34          		0x03000034
#define SIS3350_END_ADDRESS_THRESHOLD_ADC34                     0x03000028


#define SIS3350_TRIGGER_SETUP_ADC3                  		0x03000030	  
#define SIS3350_TRIGGER_THRESHOLD_ADC3              		0x03000034	  
#define SIS3350_TRIGGER_SETUP_ADC4                  		0x03000038	  
#define SIS3350_TRIGGER_THRESHOLD_ADC4              		0x0300003C	  

#define SIS3350_ADC_INPUT_TAP_DELAY_ADC3         		0x03000040
#define SIS3350_ADC_INPUT_TAP_DELAY_ADC4        		0x03000044
#define SIS3350_ADC_VGA_ADC3 			    		0x03000048
#define SIS3350_ADC_VGA_ADC4 			    		0x0300004C

#define SIS3350_ADC34_DAC_CONTROL_STATUS            		0x03000050      /* read/write; D32 */
#define SIS3350_ADC34_DAC_DATA                      		0x03000054      /* read/write; D32 */


#define SIS3350_SAMPLE_COUNTER_THRESHOLD_T2T1_ADC3     		0x03000070	  
#define SIS3350_SAMPLE_COUNTER_THRESHOLD_T4T3_ADC3     		0x03000074	  
#define SIS3350_SAMPLE_COUNTER_THRESHOLD_T2T1_ADC4     		0x03000078	  
#define SIS3350_SAMPLE_COUNTER_THRESHOLD_T4T3_ADC4     		0x0300007C	  

#define SIS3350_CTRL_DISABLE_LEMO_IN_INVERT              	0x00100000
#define SIS3350_CTRL_ENABLE_LEMO_IN_INVERT               	0x00000010
#define SIS3350_CTRL_DISABLE_USER_LED				0x00010000
#define SIS3350_CTRL_ENABLE_USER_LED				0x00000001

#define SIS3350_IRQ_CTRL_STATUS_FLAG_SOURCE3               	0x08000000
#define SIS3350_IRQ_CTRL_STATUS_FLAG_SOURCE2               	0x04000000
#define SIS3350_IRQ_CTRL_STATUS_FLAG_SOURCE1               	0x02000000
#define SIS3350_IRQ_CTRL_STATUS_FLAG_SOURCE0               	0x01000000


#define SIS3350_IRQ_CTRL_DISABLE_CLR_SOURCE3               	0x00080000
#define SIS3350_IRQ_CTRL_DISABLE_CLR_SOURCE2               	0x00040000
#define SIS3350_IRQ_CTRL_DISABLE_CLR_SOURCE1               	0x00020000
#define SIS3350_IRQ_CTRL_DISABLE_CLR_SOURCE0               	0x00010000

#define SIS3350_IRQ_CTRL_STATUS_FLAG_VME_IRQ               	0x00008000
#define SIS3350_IRQ_CTRL_STATUS_FLAG_INTERNAL_IRQ          	0x00004000

#define SIS3350_IRQ_CTRL_ENABLE_SOURCE3           	    	0x00000008
#define SIS3350_IRQ_CTRL_ENABLE_SOURCE2             	  	0x00000004
#define SIS3350_IRQ_CTRL_ENABLE_SOURCE1               		0x00000002
#define SIS3350_IRQ_CTRL_ENABLE_SOURCE0               		0x00000001

#define SIS3350_IRQ_CONF_RORA_MODE 		       		0x00000000
#define SIS3350_IRQ_CONF_ROAK_MODE 		       	    	0x00001000
#define SIS3350_IRQ_CONF_VME_IRQ_ENABLE        	    		0x00000800

#define SIS3350_IRQ_CONF_LEVEL7		           	    	0x00000700
#define SIS3350_IRQ_CONF_LEVEL6		           	    	0x00000600
#define SIS3350_IRQ_CONF_LEVEL5		           	    	0x00000500
#define SIS3350_IRQ_CONF_LEVEL4		           	    	0x00000400
#define SIS3350_IRQ_CONF_LEVEL3		           	    	0x00000300
#define SIS3350_IRQ_CONF_LEVEL2		           	    	0x00000200
#define SIS3350_IRQ_CONF_LEVEL1		           	    	0x00000100

#define SIS3350_IRQ_CONF_VECTOR_MASK           	    		0x000000FF


// define Sample Clock source
#define SIS3350_ACQ_SET_CLK_SOURCE_FREQ_SYNTHESIZER     	0x30000000  /* default after Reset  */
#define SIS3350_ACQ_SET_CLK_SOURCE_FPGA				0x20001000  /*    */
#define SIS3350_ACQ_SET_CLK_SOURCE_EXTERNAL_LVDS		0x10002000  /*    */
#define SIS3350_ACQ_SET_CLK_SOURCE_EXTERNAL_BNC 		0x00003000  /*    */

#define SIS3350_ACQ_CLK_SOURCE_MASK_BITS		 	0x30003000  /*    */



#define SIS3350_ACQ_STATUS_END_ADDRESS_FLAG	       		0x00080000
#define SIS3350_ACQ_STATUS_BUSY_FLAG	        		0x00020000
#define SIS3350_ACQ_STATUS_ARMED_FLAG	        		0x00010000


#define SIS3350_ACQ_DISABLE_LVDS_TRIGGER        		0x02000000
#define SIS3350_ACQ_ENABLE_LVDS_TRIGGER         		0x00000200

#define SIS3350_ACQ_DISABLE_LEMO_TRIGGER        		0x01000000
#define SIS3350_ACQ_ENABLE_LEMO_TRIGGER         		0x00000100

#define SIS3350_ACQ_DISABLE_INTERNAL_TRIGGER        		0x00400000
#define SIS3350_ACQ_ENABLE_INTERNAL_TRIGGER         		0x00000040
															
#define SIS3350_ACQ_DISABLE_MULTIEVENT              		0x00200000
#define SIS3350_ACQ_ENABLE_MULTIEVENT               		0x00000020

#define SIS3350_ACQ_OPERATION_RINGBUFFER_ASYNCH         	0x000700000
#define SIS3350_ACQ_OPERATION_RINGBUFFER_SYNCH          	0x000600001
#define SIS3350_ACQ_OPERATION_DIRECT_MEMORY_GATE_ASYNCH 	0x000500002
#define SIS3350_ACQ_OPERATION_DIRECT_MEMORY_GATE_SYNCH  	0x000400003
#define SIS3350_ACQ_OPERATION_DIRECT_MEMORY_STOP        	0x000300004
#define SIS3350_ACQ_OPERATION_DIRECT_MEMORY_START       	0x000200005


#define SIS3350_TRIGGER_SETUP_FIR_TRIGGER_MODE	       		0x01000000
#define SIS3350_TRIGGER_SETUP_TRIGGER_GT_MODE	       		0x02000000
#define SIS3350_TRIGGER_SETUP_TRIGGER_ENABLE	       		0x04000000

#define SIS3350_EVENT_CONFIG_MEMORY_TEST_ENABLE   		0x00008000


#define SIS3350_ADC1_OFFSET					0x04000000
#define SIS3350_ADC2_OFFSET					0x05000000
#define SIS3350_ADC3_OFFSET					0x06000000
#define SIS3350_ADC4_OFFSET					0x07000000

#define SIS3350_NEXT_ADC_OFFSET					0x01000000


#define SIS3350_MAX_PAGE_SAMPLE_LENGTH				0x800000    // 8 MSample / 16 MByte
#define SIS3350_MAX_SAMPLE_LENGTH				0x8000000    // 128 MSample / 256 MByte



#define SIS3350_EXTERNAL_DAC_THRESHOLD_TTL_VALUE 		37500	  /* 1.45V  */
#define SIS3350_EXTERNAL_DAC_THRESHOLD_NIM_VALUE 		31500	  /* -370mV  */

#endif // SIS3350_H
