#ifndef SIS3350CONTROL_H
#define SIS3350CONTROL_H

#include <QObject>
#include <QMap>
#include <cstdio>
#include <ctime>
#include <sis3150usb_vme.h>
#include "sys/time.h"
#include <cstring>
#include <cerrno>
#include <vector>
#include <inttypes.h>
//#include "sis3150usb_vme_calls.h"
#include "../local_3350readout/sis3350.h"
//#include "../local_3350readout/pc_to_vme_routines.h"
#include "../local_3350readout/pc_vme_interface_defines.h"
#include "../../samdsp/samdsp.h"
#include "configmanager.h"

//#define MAXNUMDEV 10
#define MAX_NOF_LWORDS 0x4000000 // 128 MByte

using namespace std;

class Sis3350config
{
public:
    unsigned int base_addr;
    unsigned int sample_length;
    unsigned int direct_mem_wrap_length;
    unsigned int direct_mem_sample_length;
    unsigned int direct_mem_pretrigger_length;
    unsigned int pre_delay;
    unsigned int clock1;
    unsigned int clock2;
    unsigned int nof_reads;
    unsigned int multievent_max_nof_events;
    unsigned int acq_mode;
    bool acq_enable_lemo;
    bool acq_multi_event;
    bool ctrl_lemo_invert;

    int trigger_pulse_length[4];
    int trigger_gap_length[4];
    int trigger_peak_length[4];
    unsigned short trigger_threshold[4];
    unsigned short trigger_offset[4];
    bool trigger_enable[4];
    bool trigger_fir[4];
    bool trigger_gt[4];

    unsigned int ext_clock_trigger_dac_control;
    unsigned int ext_clock_trigger_daq_data;

    unsigned short variable_gain[4];
    unsigned int adc_offset[4];

    char filePath[200];
    int filePointSkip;
    bool writeEnabled[4];
    unsigned int pollcount;


    // PostProcessing
    unsigned int ftw;
    int threshold;
};

class Sis3350control : public QObject
{

public:
    Sis3350control();

    // Basic functions
    //int open();
    int close();
    void setDevice(HANDLE m_device);
    int reset();
    int getConfigFromCode();
    int getConfigFromFile();
    unsigned int configure();
    int acquisition();
    int postProcess();
    int fileWrite(int i);
    int setupQDCspectrum();
    int addToQDCspectrum();

    // Internal functions
    unsigned int configureControlStatus();
    unsigned int configureAcquisitionMode();
    unsigned int configureDirectMemory();
    unsigned int configureMultiEventRegister();
    unsigned int configureTriggers();
    unsigned int configureExternalDAC();
    unsigned int configureInputGains();
    unsigned int configureClock();
    unsigned int configureRingBuffer();

    vector<vector<vector<double> > > byteArrayToVector();

    // Struck functions
    int sis3350_DMA_Read_MBLT64_ADC_DataBuffer(unsigned int module_address, unsigned int adc_channel /* 0 to 3 */,
                 unsigned int adc_buffer_sample_start_addr, unsigned int adc_buffer_sample_length,   // 16-bit words
                 unsigned int*  dma_got_no_of_words, unsigned int* uint_adc_buffer);
    int sis3350_write_dac_offset(unsigned int module_dac_control_status_addr, unsigned int dac_select_no, unsigned int dac_value );

    int sub_vme_A32D32_read (unsigned int vme_adr, unsigned int* vme_data) ;
    int sub_vme_A32D32_write (unsigned int vme_adr, unsigned int vme_data) ;
    int sub_vme_A32BLT32_read (unsigned int vme_adr, unsigned int* dma_buffer, unsigned int request_nof_words, unsigned int* got_nof_words);
    int sub_vme_A32MBLT64_read (unsigned int vme_adr, unsigned int* dma_buffer, unsigned int request_nof_words, unsigned int* got_nof_words);

    // Configuration class
    Sis3350config conf;
    ConfigManager *c;

    // Device variables
//    struct SIS3150USB_Device_Struct info[MAXNUMDEV];
    HANDLE m_device;
    FILE *fHandle;

    unsigned int addr,data;

    uint32_t wblt_data[4][MAX_NOF_LWORDS];
    uint32_t rblt_data[4][MAX_NOF_LWORDS];

    QMap<double, unsigned int> *qdcSpectrum;

};

#endif // SIS3350CONTROL_H
