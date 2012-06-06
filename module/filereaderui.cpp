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

#include "filereaderui.h"
#include "filereadermodule.h"
#include <iostream>

FileReaderUI::FileReaderUI(FileReaderModule* _module)
    : module(_module), uif(this,&tabs), applyingSettings(false),
      previewRunning(false)
{
    createUI();
    createPreviewUI();

    previewTimer = new QTimer();
    previewTimer->setInterval(50);
    previewTimer->setSingleShot(true);

    std::cout << "Instantiated" << _module->getName().toStdString() << "UI" << std::endl;
}

FileReaderUI::~FileReaderUI(){}

void FileReaderUI::createUI()
{
    QGridLayout* l = new QGridLayout;
    l->setMargin(0);
    l->setVerticalSpacing(0);

    int nt = 0; // current tab number
    int ng = 0; // current group number

    // TAB ACQUISITION
    tn.append("Acq"); uif.addTab(tn[nt]);

    gn.append("Event Setup"); uif.addGroupToTab(tn[nt],gn[ng],"","v");
    uif.addPopupToGroup(tn[nt],gn[ng],"Mode","multi_event_mode",
                        (QStringList()
                         << "Single long trace"
                         << "Multiple short events"));
    uif.addPopupToGroup(tn[nt],gn[ng],"Data Format","data_length_format",
                        (QStringList()
                         << "8 bit"
                         << "16 bit"
                         << "32 bit"
                         << "64 bit"));
    // uif.addPopupToGroup(tn[nt],gn[ng],"ADC resolution","adc_resolution",
    //                     (QStringList()
    //                      << "2k"
    //                      << "4k"
    //                      << "4k HiRes"
    //                      << "8k"
    //                      << "8k HiRes"));

    gn.append("File"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    uif.addFileBrowserToGroup(tn[nt],gn[ng],"File name:","input_file_name","input_file_browse_button", "Browse ...");

    // // TAB Addressing
    // tn.append("Addr"); nt++; uif.addTab(tn[nt]);
    // 
    // gn.append("ID"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    // uif.addLineEditReadOnlyToGroup(tn[nt],gn[ng],"Base Address","base_addr","");
    // uif.addLineEditReadOnlyToGroup(tn[nt],gn[ng],"Firmware","firmware","unknown");
    // uif.addLineEditReadOnlyToGroup(tn[nt],gn[ng],"Firmware expected","firmware_expected","");
    // uif.addButtonToGroup(tn[nt],gn[ng],"Update","update_firmware_button");
    // uif.addLineEditToGroup(tn[nt],gn[ng],"Module ID","module_id","");
    // 
    // gn.append("Basic"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    // uif.addPopupToGroup(tn[nt],gn[ng],"Address Source","address_source",
    //                     (QStringList()
    //                      << "Board"
    //                      << "Register"));
    // uif.addHexSpinnerToGroup(tn[nt],gn[ng],"Address Register","base_addr_register",0,0xffff); // 16 bits
    // 
    // gn.append("MCST/CBLT"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    // uif.addRadioGroupToGroup(tn[nt],gn[ng],"Mode",
    //                          (QStringList() << "None" << "CBLT" << "MCST"),
    //                          (QStringList() << "mcst_cblt_none"
    //                                         << "enable_cblt_mode"
    //                                         << "enable_mcst_mode"));
    // uif.addRadioGroupToGroup(tn[nt],gn[ng],"CBLT Placement",
    //                          (QStringList() << "First" << "Middle" << "Last"),
    //                          (QStringList() << "enable_cblt_first"
    //                                         << "enable_cblt_middle"
    //                                         << "enable_cblt_last"));
    // 
    // gn.append("Readout"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    // uif.addPopupToGroup(tn[nt],gn[ng],"Vme Access Mode","vme_mode",
    //                     (QStringList()
    //                      << "Single Reads"
    //                      << "DMA 32bit"
    //                      << "FIFO Reads"
    //                      << "Block Transfer 32bit"
    //                      << "Block Transfer 64bit"
    //                      << "VME2E accelerated mode"));
    // 
    // // TAB Control
    // tn.append("Ctrl"); nt++; uif.addTab(tn[nt]);
    // gn.append("Control"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    // uif.addUnnamedGroupToGroup(tn[nt],gn[ng],"b0_");
    // uif.addButtonToGroup(tn[nt],gn[ng]+"b0_","Start","start_button");
    // uif.addButtonToGroup(tn[nt],gn[ng]+"b0_","Stop","stop_button");
    // uif.addUnnamedGroupToGroup(tn[nt],gn[ng],"b1_");
    // uif.addButtonToGroup(tn[nt],gn[ng]+"b1_","**Reserved**","reserved_button");
    // uif.addButtonToGroup(tn[nt],gn[ng]+"b1_","Readout Reset","readout_reset_button");
    // uif.addUnnamedGroupToGroup(tn[nt],gn[ng],"b2_");
    // uif.addButtonToGroup(tn[nt],gn[ng]+"b2_","Reset","reset_button");
    // uif.addButtonToGroup(tn[nt],gn[ng]+"b2_","FIFO Reset","fifo_reset_button");
    // uif.addUnnamedGroupToGroup(tn[nt],gn[ng],"b3_");
    // uif.addButtonToGroup(tn[nt],gn[ng]+"b3_","Configure","configure_button");
    // uif.addButtonToGroup(tn[nt],gn[ng]+"b3_","Single Shot","singleshot_button");
    // 
    // // TAB INPUT/OUTPUT Config
    // tn.append("I/O"); nt++; uif.addTab(tn[nt]);
    // gn.append("Input"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    // uif.addPopupToGroup(tn[nt],gn[ng],"Input Range","input_range",(QStringList()
    //                            << "4 V" << "8 V" << "10 V" ));
    // gn.append("ECL Outputs"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    // uif.addPopupToGroup(tn[nt],gn[ng],"Gate1 Mode","ecl_gate1_mode",(QStringList()
    //                            << "Gate"
    //                            << "Oscillator"));
    // uif.addPopupToGroup(tn[nt],gn[ng],"Fast Clear Mode","ecl_fclear_mode",(QStringList()
    //                            << "Fast Clear"
    //                            << "Timestamp Reset"));
    // uif.addPopupToGroup(tn[nt],gn[ng],"Busy Mode","ecl_busy_mode",(QStringList()
    //                            << "Busy"
    //                            << "Reserved"));
    // uif.addCheckBoxToGroup(tn[nt],gn[ng],"Terminate Gate 0 In","enable_termination_input_gate0");
    // uif.addCheckBoxToGroup(tn[nt],gn[ng],"Terminate Fast Clear","enable_termination_input_fast_clear");
    // gn.append("NIM Outputs"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    // uif.addPopupToGroup(tn[nt],gn[ng],"Gate1 Mode","nim_gate1_mode",(QStringList()
    //                            << "Gate"
    //                            << "Oscillator"));
    // uif.addPopupToGroup(tn[nt],gn[ng],"Fast Clear Mode","nim_fclear_mode",(QStringList()
    //                            << "Fast Clear"
    //                            << "Timestamp Reset"));
    // uif.addPopupToGroup(tn[nt],gn[ng],"Busy Mode","nim_busy_mode",(QStringList()
    //                            << "Busy"
    //                            << "Gate 0 Out"
    //                            << "Gate 1 Out"
    //                            << "Control Bus Out"
    //                            << "Buffer Full"
    //                            << "Buffer Over Threshold"));
    // gn.append("Gate Generators"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    // uif.addPopupToGroup(tn[nt],gn[ng],"Mode","gate_generator_mode",(QStringList()
    //                            << "Off" << "Gate 0" << "Gate 1" << "Both"));
    // uif.addSpinnerToGroup(tn[nt],gn[ng],"Delay 0","hold_delay_0",0,255); // 8 bits
    // uif.addSpinnerToGroup(tn[nt],gn[ng],"Delay 1","hold_delay_1",0,255); // 8 bits
    // uif.addSpinnerToGroup(tn[nt],gn[ng],"Width 0","hold_width_0",0,255); // 8 bits
    // uif.addSpinnerToGroup(tn[nt],gn[ng],"Width 1","hold_width_1",0,255); // 8 bits
    // 
    // // TABs THRESHOLD
    // int ch = 0;
    // int nofChPerTab = 4;
    // int nofRows = 4;
    // int nofTabs = FILEREADER_NUM_CHANNELS/nofChPerTab/nofRows;
    // 
    // for(int t=0; t<nofTabs; t++) {
    //     tn.append(tr("T%1-%2").arg(ch).arg(ch+(nofChPerTab*nofRows))); nt++; uif.addTab(tn[nt]);
    // 
    //     for(int i=0; i<nofRows; i++)
    //     {
    //         QString un = tr("noname_%1").arg(i);
    //         gn.append(un); ng++; uif.addUnnamedGroupToTab(tn[nt],gn[ng]);
    //         for(int j=0; j<nofChPerTab; j++)
    //         {
    //             gn.append(tr("Channel %1").arg(ch)); ng++; uif.addGroupToGroup(tn[nt],un,gn[ng],tr("enable_channel%1").arg(ch));
    //             uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Threshold",tr("thresholds%1").arg(ch),0,8191); // 13 bits
    //             ch++;
    //         }
    //     }
    // }
    // 
    // // TAB Clock and IRQ
    // tn.append("Clock/IRQ"); nt++; uif.addTab(tn[nt]);
    // 
    // gn.append("Clock"); ng++; uif.addGroupToTab(tn[nt],gn[ng]);
    // uif.addPopupToGroup(tn[nt],gn[ng],"Timestamp Source","time_stamp_source",(QStringList()
    //          << "From VME"
    //          << "External LEMO"));
    // uif.addSpinnerToGroup(tn[nt],gn[ng],"Timestamp Divisor","time_stamp_divisor",0,65535); // 16 bits
    // 
    // gn.append("Interrupt Setup"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    // uif.addCheckBoxToGroup(tn[nt],gn[ng],"VME enable IRQ","enable_irq");
    // uif.addSpinnerToGroup(tn[nt],gn[ng],"IRQ level","irq_level",0,7);
    // uif.addHexSpinnerToGroup(tn[nt],gn[ng],"IRQ vector","irq_vector",0,0xff);
    // uif.addSpinnerToGroup(tn[nt],gn[ng],"IRQ threshold","irq_threshold",0,0x1fff); // bit
    // uif.addSpinnerToGroup(tn[nt],gn[ng],"Maximum amount of transfer data","max_transfer_data",0,0x3fff); // 14 bit
    // 
    // // TAB Counters
    // tn.append("Counters"); nt++; uif.addTab(tn[nt]);
    // 
    // gn.append("Counters"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    // uif.addLineEditReadOnlyToGroup(tn[nt],gn[ng],"Event counter","event_counter","0");
    // uif.addLineEditReadOnlyToGroup(tn[nt],gn[ng],"Timestamp Counter","timestamp_counter","0");
    // uif.addLineEditReadOnlyToGroup(tn[nt],gn[ng],"ADC busy time","adc_busy_time","0");
    // uif.addLineEditReadOnlyToGroup(tn[nt],gn[ng],"Gate 1 time","gate1_time","0");
    // uif.addLineEditReadOnlyToGroup(tn[nt],gn[ng],"Time","time","0");
    // uif.addButtonToGroup(tn[nt],gn[ng],"Update","counter_update_button");
    // 
    // // TAB RCBus
    // tn.append("RCBus"); nt++; uif.addTab(tn[nt]);
    // gn.append("Read"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    // uif.addSpinnerToGroup(tn[nt],gn[ng],"Module ID","rc_module_id_read",0,15);
    // uif.addLineEditToGroup(tn[nt],gn[ng],"Address","rc_addr_read","");
    // uif.addLineEditReadOnlyToGroup(tn[nt],gn[ng],"Data","rc_data_read","");
    // uif.addLineEditReadOnlyToGroup(tn[nt],gn[ng],"Return Status","rc_status_read","");
    // uif.addButtonToGroup(tn[nt],gn[ng],"Read","rc_read_button");
    // gn.append("Write"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    // uif.addSpinnerToGroup(tn[nt],gn[ng],"Module ID","rc_module_id_write",0,15);
    // uif.addLineEditToGroup(tn[nt],gn[ng],"Address","rc_addr_write","");
    // uif.addLineEditToGroup(tn[nt],gn[ng],"Data","rc_data_write","");
    // uif.addLineEditReadOnlyToGroup(tn[nt],gn[ng],"Return Status","rc_status_write","");
    // uif.addButtonToGroup(tn[nt],gn[ng],"Write","rc_write_button");
    // 
    // //###
    // 
    l->addWidget(dynamic_cast<QWidget*>(&tabs));
    
    QWidget* bottomButtons = new QWidget(this);
    {
    QHBoxLayout* l = new QHBoxLayout();
    QPushButton* previewButton = new QPushButton("Preview");
    // connect(previewButton,SIGNAL(clicked()),this,SLOT(clicked_previewButton()));
    l->addWidget(previewButton);
    singleShotPreviewButton = new QPushButton("Singleshot");
    startStopPreviewButton = new QPushButton("Start");
    startStopPreviewButton->setCheckable(true);
    // connect(singleShotPreviewButton,SIGNAL(clicked()),this,SLOT(clicked_singleshot_button()));
    connect(startStopPreviewButton,SIGNAL(clicked()),this,SLOT(clicked_startStopPreviewButton()));
    l->addWidget(singleShotPreviewButton);
    l->addWidget(startStopPreviewButton);
    bottomButtons->setLayout(l);
    }
    l->addWidget(bottomButtons);
    
    this->setLayout(l);
    connect(uif.getSignalMapper(),SIGNAL(mapped(QString)),this,SLOT(uiInput(QString)));

   QList<QWidget*> li = this->findChildren<QWidget*>();
   foreach(QWidget* w, li)
   {
       printf("%s\n",w->objectName().toStdString().c_str());
   }
}

void FileReaderUI::createPreviewUI()
{
    int nofCh = FILEREADER_NUM_CHANNELS;
    int nofCols = nofCh/sqrt(nofCh);
    int nofRows = nofCh/nofCols;

    if(nofCols*nofRows < nofCh) ++nofCols;


    QTabWidget* t = new QTabWidget();
    QGridLayout* l = new QGridLayout();

    // Histograms
    int ch = 0;
    QWidget* rw = new QWidget();
    {
        QGridLayout* li = new QGridLayout();
        for(int r = 0; r < nofRows; ++r) {
            for(int c = 0; c < nofCols; ++c) {
                if(ch < nofCh) {
                    previewCh[ch] = new plot2d(this,QSize(160,120),ch);
                    previewCh[ch]->addChannel(0,"raw",previewData[ch],QColor(Qt::blue),Channel::line,1);
                    li->addWidget(previewCh[ch],r,c,1,1);
                }
                ++ch;
            }
        }
        rw->setLayout(li);
    }
    // Value displays
    ch = 0;
    QWidget* vw = new QWidget();
    {
        QGridLayout* li = new QGridLayout();
        for(int r = 0; r < nofRows; ++r) {
            for(int c = 0; c < nofCols; ++c) {
                if(ch < nofCh) {
                    QGroupBox* b = new QGroupBox(tr("Ch: %1").arg(ch));
                    {
                        QGridLayout* lb = new QGridLayout();
                        lb->setSpacing(0);
                        lb->setMargin(0);
                        energyValueDisplay[ch] = new QLabel();
                        timestampDisplay[ch] = new QLabel();
                        resolutionDisplay[ch] = new QLabel();
                        flagDisplay[ch] = new QLabel();
                        moduleIdDisplay[ch] = new QLabel();
    
                        lb->addWidget(new QLabel("Module ID:"),0,0,1,1);
                        lb->addWidget(moduleIdDisplay[ch],0,1,1,1);
                        lb->addWidget(new QLabel("Flags:"),1,0,1,1);
                        lb->addWidget(flagDisplay[ch],1,1,1,1);
                        lb->addWidget(new QLabel("ADC Resolution:"),2,0,1,1);
                        lb->addWidget(resolutionDisplay[ch],2,1,1,1);
                        lb->addWidget(new QLabel("Timestamp:"),3,0,1,1);
                        lb->addWidget(timestampDisplay[ch],3,1,1,1);
                        lb->addWidget(new QLabel("Energy:"),4,0,1,1);
                        lb->addWidget(energyValueDisplay[ch],4,1,1,1);
    
                        b->setLayout(lb);
                    }
                    li->addWidget(b,r,c,1,1);
                }
                ++ch;
            }
        }
        vw->setLayout(li);
    }

    t->addTab(rw,"Histograms");
    t->addTab(vw,"Values");
    l->addWidget(t);

    previewWindow.setLayout(l);
    previewWindow.setWindowTitle("File Preview");
    previewWindow.resize(640,480);
}

// Slot handling

void FileReaderUI::uiInput(QString _name)
{
    if(applyingSettings == true) return;

    QGroupBox* gb = findChild<QGroupBox*>(_name);
    if(gb != 0)
    {
        if(_name.startsWith("enable_channel")) {
            QRegExp reg("[0-9]{1,2}");
            reg.indexIn(_name);
            int ch = reg.cap().toInt();
            if(gb->isChecked()) module->conf_.enable_channel[ch] = true;
            else module->conf_.enable_channel[ch] = false;
            printf("Changed enable_channel %d\n",ch); fflush(stdout);
        }
    }

    QCheckBox* cb = findChild<QCheckBox*>(_name);
    if(cb != 0)
    {
        if(_name == "enable_multi_event_send_different_eob_marker") {
            module->conf_.enable_multi_event_send_different_eob_marker = cb->isChecked();
        }
        if(_name == "enable_multi_event_compare_with_max_transfer_data") {
            module->conf_.enable_multi_event_compare_with_max_transfer_data = cb->isChecked();
        }
        if(_name == "enable_adc_override") {
            module->conf_.enable_adc_override = cb->isChecked();
        }
        if(_name == "enable_switch_off_sliding_scale") {
            module->conf_.enable_switch_off_sliding_scale = cb->isChecked();
        }
        if(_name == "enable_skip_out_of_range") {
            module->conf_.enable_skip_out_of_range = cb->isChecked();
        }
        if(_name == "enable_ignore_thresholds") {
            module->conf_.enable_ignore_thresholds = cb->isChecked();
        }
        if(_name == "enable_termination_input_gate0") {
            module->conf_.enable_termination_input_gate0 = cb->isChecked();
        }
        if(_name == "enable_termination_input_fast_clear") {
            module->conf_.enable_termination_input_fast_clear = cb->isChecked();
        }
        if(_name == "enable_external_time_stamp_reset") {
            module->conf_.enable_external_time_stamp_reset = cb->isChecked();
        }
        //QMessageBox::information(this,"uiInput","You changed the checkbox "+_name);
    }

    QComboBox* cbb = findChild<QComboBox*>(_name);
    if(cbb != 0)
    {
        if(_name == "addr_source") {
            module->conf_.addr_source = static_cast<FileReaderModuleConfig::AddressSource>(cbb->currentIndex());
        }
        if(_name == "multi_event_mode") {
            module->conf_.multi_event_mode = static_cast<FileReaderModuleConfig::MultiEventMode>(cbb->currentIndex());
        }
        if(_name == "data_length_format") {
            module->conf_.data_length_format = static_cast<FileReaderModuleConfig::DataLengthFormat>(cbb->currentIndex());
        }
        if(_name == "vme_mode") {
            module->conf_.vme_mode = static_cast<FileReaderModuleConfig::VmeMode>(cbb->currentIndex());
            std::cout << "Changed vme_mode to" << module->conf_.vme_mode << std::endl;
        }
        if(_name == "time_stamp_source") {
            module->conf_.time_stamp_source = static_cast<FileReaderModuleConfig::TimeStampSource>(cbb->currentIndex());
        }
        if(_name == "adc_resolution") {
            module->conf_.adc_resolution = static_cast<FileReaderModuleConfig::AdcResolution>(cbb->currentIndex());
        }
        if(_name == "output_format") {
            module->conf_.output_format = static_cast<FileReaderModuleConfig::OutputFormat>(cbb->currentIndex());
        }
        if(_name == "gate_generator_mode") {
            module->conf_.gate_generator_mode = static_cast<FileReaderModuleConfig::GateGeneratorMode>(cbb->currentIndex());
        }
        if(_name == "ecl_gate1_mode") {
            module->conf_.ecl_gate1_mode =
                    static_cast<FileReaderModuleConfig::EclGate1Mode>(cbb->currentIndex());
        }
        if(_name == "ecl_fclear_mode") {
            module->conf_.ecl_fclear_mode =
                    static_cast<FileReaderModuleConfig::EclFClearMode>(cbb->currentIndex());
        }
        if(_name == "ecl_busy_mode") {
            module->conf_.ecl_busy_mode =
                    static_cast<FileReaderModuleConfig::EclBusyMode>(cbb->currentIndex());
        }
        if(_name == "nim_gate1_mode") {
            module->conf_.nim_gate1_mode =
                    static_cast<FileReaderModuleConfig::NimGate1Mode>(cbb->currentIndex());
        }
        if(_name == "nim_fclear_mode") {
            module->conf_.nim_fclear_mode =
                    static_cast<FileReaderModuleConfig::NimFClearMode>(cbb->currentIndex());
        }
        if(_name == "nim_busy_mode") {
            module->conf_.nim_busy_mode =
                    static_cast<FileReaderModuleConfig::NimBusyMode>(cbb->currentIndex());
            if(module->conf_.nim_busy_mode == FileReaderModuleConfig::nbRes5)
                module->conf_.nim_busy_mode = FileReaderModuleConfig::nbBufOverThr;
        }
        if(_name == "input_range") {
            switch(cbb->currentIndex()) {
            case 0:
                module->conf_.input_range = FileReaderModuleConfig::ir4V;
                break;
            case 1:
                module->conf_.input_range = FileReaderModuleConfig::ir8V;
                break;
            case 2:
            default:
                module->conf_.input_range = FileReaderModuleConfig::ir10V;
                break;
            }
        }
        if(_name == "marking_type") {
            switch(cbb->currentIndex()) {
            case 0:
                module->conf_.marking_type = FileReaderModuleConfig::mtEventCounter;
                break;
            case 1:
                module->conf_.marking_type = FileReaderModuleConfig::mtTimestamp;
                break;
            case 2:
                module->conf_.marking_type = FileReaderModuleConfig::mtExtendedTs;
                break;
            default:
                module->conf_.marking_type = FileReaderModuleConfig::mtEventCounter;
                break;
            }
        }
        if(_name == "bank_operation") {
            switch(cbb->currentIndex()) {
            case 0:
                module->conf_.bank_operation = FileReaderModuleConfig::boConnected;
                break;
            case 1:
                module->conf_.bank_operation = FileReaderModuleConfig::boIndependent;
                break;
            case 2:
                module->conf_.bank_operation = FileReaderModuleConfig::boToggle;
                break;
            default:
                module->conf_.bank_operation = FileReaderModuleConfig::boConnected;
                break;
            }
        }
        if(_name == "test_pulser_mode") {
            switch(cbb->currentIndex()) {
            case 0:
                module->conf_.test_pulser_mode = FileReaderModuleConfig::tpOff;
                break;
            case 1:
                module->conf_.test_pulser_mode = FileReaderModuleConfig::tpAmp0;
                break;
            case 2:
                module->conf_.test_pulser_mode = FileReaderModuleConfig::tpAmpLow;
                break;
            case 3:
                module->conf_.test_pulser_mode = FileReaderModuleConfig::tpAmpHigh;
                break;
            case 4:
                module->conf_.test_pulser_mode = FileReaderModuleConfig::tpToggle;
                break;
            default:
                module->conf_.test_pulser_mode = FileReaderModuleConfig::tpOff;
                break;
            }
        }
        //QMessageBox::information(this,"uiInput","You changed the combobox "+_name);
    }
    QSpinBox* sb = findChild<QSpinBox*>(_name);
    if(sb != 0)
    {
        if(_name == "irq_level") module->conf_.irq_level = sb->value();
        if(_name == "irq_vector"){
            module->conf_.irq_vector = sb->value();
        }
        if(_name == "irq_threshold"){
            module->conf_.irq_threshold = sb->value();
        }
        if(_name == "base_addr_register"){
            module->conf_.base_addr_register = sb->value();
        }
        if(_name == "time_stamp_divisor"){
            module->conf_.time_stamp_divisor = sb->value();
        }
        if(_name == "max_transfer_data"){
            module->conf_.max_transfer_data= sb->value();
        }
        if(_name == "rc_module_id_read"){
            module->conf_.rc_module_id_read = sb->value();
        }
        if(_name == "rc_module_id_write"){
            module->conf_.rc_module_id_write = sb->value();
        }
        if(_name.startsWith("hold_delay_")) {
            int ch = _name.right(1).toInt();
            module->conf_.hold_delay[ch] = sb->value();
        }
        if(_name.startsWith("hold_width_")) {
            int ch = _name.right(1).toInt();
            module->conf_.hold_width[ch] = sb->value();
        }
        if(_name.startsWith("thresholds")) {
            QRegExp reg("[0-9]{1,2}");
            reg.indexIn(_name);
            int ch = reg.cap().toInt();
            module->conf_.thresholds[ch] = sb->value();
        }

    }
    QRadioButton* rb = findChild<QRadioButton*>(_name);
    if(rb != 0)
    {
        if(_name == "mcst_cblt_none" && rb->isChecked()) {
            module->conf_.cblt_mcst_ctrl = 0;
            module->conf_.mcst_cblt_none = true;
        }
        if(_name == "enable_cblt_mode" && rb->isChecked()) {
            module->conf_.cblt_mcst_ctrl |=
                    (1 << MADC32V2_OFF_CBLT_MCST_CTRL_ENABLE_CBLT);
            module->conf_.enable_cblt_mode = true;
        }
        if(_name == "enable_mcst_mode" && rb->isChecked()) {
            module->conf_.cblt_mcst_ctrl |=
                    (1 << MADC32V2_OFF_CBLT_MCST_CTRL_ENABLE_MCST);
            module->conf_.enable_mcst_mode = true;
        }
        if(_name == "enable_cblt_first" && rb->isChecked()) {
            module->conf_.cblt_mcst_ctrl |=
                    (1 << MADC32V2_OFF_CBLT_MCST_CTRL_ENABLE_FIRST_MODULE);
            module->conf_.enable_cblt_first = true;
        }
        if(_name == "enable_cblt_last" && rb->isChecked()) {
            module->conf_.cblt_mcst_ctrl |=
                    (1 << MADC32V2_OFF_CBLT_MCST_CTRL_ENABLE_LAST_MODULE);
            module->conf_.enable_cblt_last = true;
        }
        if(_name == "enable_cblt_middle" && rb->isChecked()) {
            module->conf_.cblt_mcst_ctrl &=
                    ~((1 << MADC32V2_OFF_CBLT_MCST_CTRL_ENABLE_FIRST_MODULE)
                      |(1 << MADC32V2_OFF_CBLT_MCST_CTRL_ENABLE_LAST_MODULE));
            module->conf_.enable_cblt_middle = true;
        }
    }
    QLabel* lb = findChild<QLabel*>(_name);
    if(lb != 0) {
        if(_name == "input_file_name") {
            // not sure if I would have to do something here or not
        }
    }
    QPushButton* pb = findChild<QPushButton*>(_name);
    if(pb != 0)
    {
        qDebug() << "findChild<QPushButton> " << _name;
        if(_name == "trigger_button") clicked_start_button();
        if(_name == "stop_button") clicked_start_button();
        if(_name == "stop_button") clicked_stop_button();
        if(_name == "reset_button") clicked_reset_button();
        if(_name == "fifo_reset_button") clicked_fifo_reset_button();
        if(_name == "readout_reset_button") clicked_readout_reset_button();
        if(_name == "configure_button") clicked_configure_button();
        if(_name == "counter_update_button") clicked_counter_update_button();
        if(_name == "singleshot_button") clicked_singleshot_button();
        if(_name == "update_firmware_button") clicked_update_firmware_button();

        if(_name == "input_file_browse_button") clicked_input_file_browse_button();
    }

}

void FileReaderUI::clicked_start_button()
{
    module->startAcquisition();
}

void FileReaderUI::clicked_stop_button()
{
    module->stopAcquisition();
}

void FileReaderUI::clicked_reset_button()
{
    module->reset();
}

void FileReaderUI::clicked_fifo_reset_button()
{
    module->fifoReset();
}

void FileReaderUI::clicked_readout_reset_button()
{
    module->readoutReset();
}

void FileReaderUI::clicked_configure_button()
{
    module->configure();
}

void FileReaderUI::clicked_counter_update_button()
{
    std::cout << "Clicked counter_update_button" << std::endl;
    uint32_t ev_cnt = module->getEventCounter();
    QLabel* ec = (QLabel*) uif.getWidgets()->find("event_counter").value();
    ec->setText(tr("%1").arg(ev_cnt,2,10));
    std::cout << "Event counter value: " << ev_cnt << std::endl;
}

void FileReaderUI::clicked_singleshot_button()
{
    if(!module->getInterface()) {
        return;
    }
    if(!module->getInterface()->isOpen()){
        if(0 != module->getInterface()->open()) {
            QMessageBox::warning (this, tr ("<%1> SIS3302 ADC").arg (module->getName()), tr ("Could not open interface"), QMessageBox::Ok);
            return;
        }
    }
    uint32_t data[MADC32V2_LEN_EVENT_MAX];
    uint32_t rd = 0;
    module->singleShot(data,&rd);
    //printf("FileReaderUI: singleShot: Read %d words:\n",rd);

    updatePreview();

    if(previewRunning) {
        previewTimer->start();
    }
}

// remove this function
void FileReaderUI::clicked_update_firmware_button()
{/*
    module->updateModuleInfo();
    QLineEdit* m = (QLineEdit*) uif.getWidgets()->find("module_id").value();
    QLabel* f = (QLabel*) uif.getWidgets()->find("firmware").value();
    f->setText(tr("%1.%2").arg(module->conf_.firmware_revision_major,2,16,QChar('0')).arg(module->conf_.firmware_revision_minor,2,16,QChar('0')));
    m->setText(tr("0x%1").arg(module->conf_.module_id,4,16,QChar('0')));
*/
}

void FileReaderUI::clicked_input_file_browse_button()
{
    // qDebug() << "Clicked browse button.";
    QDir oldFilePath(QFileInfo(module->conf_.input_file_name).absoluteDir());
    if (!oldFilePath.exists() || module->conf_.input_file_name.isEmpty()) {
        oldFilePath = QDir::home();
    }
    // qDebug() << "FileReaderUI::clicked_input_file_browse_button " << module->conf_.input_file_name << " " << oldFilePath.absolutePath();
    setFileName(QFileDialog::getOpenFileName(this,tr("Choose data file"),oldFilePath.absolutePath(), tr("Data files (*.*)"), 0, QFileDialog::DontResolveSymlinks));
    // settings_changed = true;
}

void FileReaderUI::setFileName(QString _filename)
{
    qDebug() << "FileReaderUI::setFileName " << _filename;

    // TODO: a few sanity checks about filename; we might want to write tr("no filename selected")
    QLabel* fileNameLabel = (QLabel*) uif.getWidgets()->find("input_file_name").value();

    if (!_filename.isEmpty()) {
        module->conf_.input_file_name = _filename;
    }
    //qDebug() << "FileReaderUI::setFileName(null=" << _filename.isNull() << ",empty=" << _filename.isEmpty() << ")";

    if (module->conf_.input_file_name.isEmpty()) {
        fileNameLabel->setText(tr("(no file selected)"));
    } else {
        fileNameLabel->setText(module->conf_.input_file_name);
    }
}

void FileReaderUI::clicked_previewButton()
{
    if(previewWindow.isHidden())
    {
        previewWindow.show();
    }
    else
    {
        previewWindow.hide();
    }
}

void FileReaderUI::clicked_startStopPreviewButton()
{
    if(previewRunning) {
        startStopPreviewButton->setText("Start");
//        previewTimer->stop();
        previewRunning = false;
    } else {
    // do a few redouts from the file
        if(!module->getInterface()) {
            // TODO: what exactly is this?
            qDebug() << "FileReaderUI::clicked_startStopPreviewButtor: no interface";
            return;
        }
        if(!module->getInterface()->isOpen()) {
            // TODO: what is this?
            qDebug() << "FileReaderUI::clicked_startStopPreviewButtor: not open";
        }
        if(previewWindow.isHidden()) {
            previewWindow.show();
        }
        previewRunning = true;
    }
/*
    if(previewRunning) {
        // Stopping
        startStopPreviewButton->setText("Start");
        previewTimer->stop();
        previewRunning = false;
    } else {
        // Starting
        // precondition check
        if(!module->getInterface()) {
            return;
        }
        if(!module->getInterface()->isOpen()){
            if(0 != module->getInterface()->open()) {
                QMessageBox::warning (this, tr ("<%1> SIS3302 ADC").arg (module->getName()), tr ("Could not open interface"), QMessageBox::Ok);
                return;
            }
        }

        startStopPreviewButton->setText("Stop");
        if(previewWindow.isHidden()) {
            previewWindow.show();
        }
        previewRunning = true;
        previewTimer->start();
        connect(previewTimer,SIGNAL(timeout()),this,SLOT(timeout_previewTimer()));
    }
*/
}

void FileReaderUI::updatePreview()
{
    //std::cout << "FileReaderUI::updatePreview" << std::endl << std::flush;
    for(int ch = 0; ch < FILEREADER_NUM_CHANNELS; ++ch) {
        if(module->conf_.enable_channel[ch]) {
            // Values
            moduleIdDisplay[ch]->setText(tr("0x%1").arg(module->getModuleIdConfigured(),4,16));
            QString flagString = "";
            flagDisplay[ch]->setText(flagString);
            timestampDisplay[ch]->setText(tr("0x%1").arg(module->current_time_stamp,16,16));
            energyValueDisplay[ch]->setText(tr("%1").arg(module->current_energy[ch]));
            resolutionDisplay[ch]->setText(tr("%1").arg(module->current_resolution));

            // HIST data
            int nof_bins = 1024;
            previewData[ch].resize(nof_bins);
            //printf("Channel size: %d\n",previewData[ch].size());
            int current_bin = ((double)(nof_bins)/8192.)*((double)(module->current_energy[ch]) );
            // printf("Current bin: %d \n",current_bin);
            if(current_bin > 0 && current_bin < nof_bins) {
                previewData[ch][current_bin]++;
            }

            {
                QWriteLocker lck(previewCh[ch]->getChanLock());
                previewCh[ch]->getChannelById(0)->setData(previewData[ch]);
            }
            previewCh[ch]->update();
        }
    }
}

void FileReaderUI::timeout_previewTimer() {
    if(previewRunning) {
        clicked_singleshot_button();
    }
}

// Settings handling

void FileReaderUI::applySettings()
{
    applyingSettings = true;

    QList<QGroupBox*> gbs = findChildren<QGroupBox*>();
    if(!gbs.empty())
    {
        QList<QGroupBox*>::const_iterator it = gbs.begin();
        while(it != gbs.end())
        {
            QGroupBox* w = (*it);
            for(int ch=0; ch < FILEREADER_NUM_CHANNELS; ch++) {
                if(w->objectName() == tr("enable_channel%1").arg(ch)) w->setChecked(module->conf_.enable_channel[ch]);
            }
            it++;
        }
    }
    QList<QCheckBox*> cbs = findChildren<QCheckBox*>();
    if(!cbs.empty())
    {
        QList<QCheckBox*>::const_iterator it = cbs.begin();
        while(it != cbs.end())
        {
            QCheckBox* w = (*it);

            if(w->objectName() == "enable_multi_event_send_different_eob_marker") w->setChecked(module->conf_.enable_multi_event_send_different_eob_marker);
            if(w->objectName() == "enable_multi_event_compare_with_max_transfer_data") w->setChecked(module->conf_.enable_multi_event_compare_with_max_transfer_data);
            if(w->objectName() == "enable_adc_override") w->setChecked(module->conf_.enable_adc_override);
            if(w->objectName() == "enable_switch_off_sliding_scale") w->setChecked(module->conf_.enable_switch_off_sliding_scale);
            if(w->objectName() == "enable_skip_out_of_range") w->setChecked(module->conf_.enable_skip_out_of_range);
            if(w->objectName() == "enable_ignore_thresholds") w->setChecked(module->conf_.enable_ignore_thresholds);
            if(w->objectName() == "enable_termination_input_gate0") w->setChecked(module->conf_.enable_termination_input_gate0);
            if(w->objectName() == "enable_termination_input_fast_clear") w->setChecked(module->conf_.enable_termination_input_fast_clear);
            if(w->objectName() == "enable_external_time_stamp_reset") w->setChecked(module->conf_.enable_external_time_stamp_reset);

            it++;
        }
    }
    QList<QComboBox*> cbbs = findChildren<QComboBox*>();
    if(!cbbs.empty())
    {
        QList<QComboBox*>::const_iterator it = cbbs.begin();
        while(it != cbbs.end())
        {
            QComboBox* w = (*it);
            //printf("Found combobox with the name %s\n",w->objectName().toStdString().c_str());
            if(w->objectName() == "addr_source") w->setCurrentIndex(module->conf_.addr_source);
            if(w->objectName() == "multi_event_mode") w->setCurrentIndex(module->conf_.multi_event_mode);
            if(w->objectName() == "vme_mode") w->setCurrentIndex(module->conf_.vme_mode);
            if(w->objectName() == "data_length_format") w->setCurrentIndex(module->conf_.data_length_format);
            if(w->objectName() == "time_stamp_source") w->setCurrentIndex(module->conf_.time_stamp_source);
            if(w->objectName() == "adc_resolution") w->setCurrentIndex(module->conf_.adc_resolution);
            if(w->objectName() == "output_format") w->setCurrentIndex(module->conf_.output_format);
            if(w->objectName() == "gate_generator_mode") w->setCurrentIndex(module->conf_.gate_generator_mode);
            if(w->objectName() == "ecl_gate1_mode") w->setCurrentIndex(module->conf_.ecl_gate1_mode);
            if(w->objectName() == "ecl_fclear_mode") w->setCurrentIndex(module->conf_.ecl_fclear_mode);
            if(w->objectName() == "ecl_busy_mode") w->setCurrentIndex(module->conf_.ecl_busy_mode);
            if(w->objectName() == "nim_gate1_mode") w->setCurrentIndex(module->conf_.nim_gate1_mode);
            if(w->objectName() == "nim_fclear_mode") w->setCurrentIndex(module->conf_.nim_fclear_mode);
            if(w->objectName() == "input_range") {
                switch (module->conf_.input_range){
                case FileReaderModuleConfig::ir4V: w->setCurrentIndex(0); break;
                case FileReaderModuleConfig::ir8V: w->setCurrentIndex(1); break;
                case FileReaderModuleConfig::ir10V: w->setCurrentIndex(2); break;
                default: w->setCurrentIndex(2); break;
                }
            }
            if(w->objectName() == "marking_type") w->setCurrentIndex(module->conf_.marking_type);
            if(w->objectName() == "bank_operation") w->setCurrentIndex(module->conf_.bank_operation);
            if(w->objectName() == "test_pulser_mode") w->setCurrentIndex(module->conf_.test_pulser_mode);
            it++;
        }
    }
    QList<QSpinBox*> csb = findChildren<QSpinBox*>();
    if(!csb.empty())
    {
        QList<QSpinBox*>::const_iterator it = csb.begin();
        while(it != csb.end())
        {
            QSpinBox* w = (*it);
            //printf("Found spinbox with the name %s\n",w->objectName().toStdString().c_str());
            if(w->objectName() == "irq_level") w->setValue(module->conf_.irq_level);
            if(w->objectName() == "irq_vector") w->setValue(module->conf_.irq_vector);
            if(w->objectName() == "irq_threshold") w->setValue(module->conf_.irq_threshold);
            if(w->objectName() == "base_addr_register") w->setValue(module->conf_.base_addr_register);
            if(w->objectName() == "time_stamp_divisor") w->setValue(module->conf_.time_stamp_divisor);
            if(w->objectName() == "max_transfer_data") w->setValue(module->conf_.max_transfer_data);
            if(w->objectName() == "rc_module_id_read") w->setValue(module->conf_.rc_module_id_read);
            if(w->objectName() == "rc_module_id_write") w->setValue(module->conf_.rc_module_id_write);

            for(int ch=0; ch<2; ch++)
            {
                if(w->objectName() == tr("hold_delay_%1").arg(ch)) w->setValue(module->conf_.hold_delay[ch]);
                if(w->objectName() == tr("hold_width_%1").arg(ch)) w->setValue(module->conf_.hold_width[ch]);
            }
            for(int ch=0; ch<FILEREADER_NUM_CHANNELS; ch++)
            {
                if(w->objectName() == tr("thresholds%1").arg(ch)) w->setValue(module->conf_.thresholds[ch]);
            }
            it++;
        }
    }
    QList<QRadioButton*> crb = findChildren<QRadioButton*>();
    if(!crb.empty())
    {
        QList<QRadioButton*>::const_iterator it = crb.begin();
        while(it != crb.end())
        {
            QRadioButton* w = (*it);
            if(w->objectName() == "mcst_cblt_none") w->setChecked(module->conf_.mcst_cblt_none);
            if(w->objectName() == "enable_cblt_mode") w->setChecked(module->conf_.enable_cblt_mode);
            if(w->objectName() == "enable_mcst_mode") w->setChecked(module->conf_.enable_mcst_mode);
            if(w->objectName() == "enable_cblt_first") w->setChecked(module->conf_.enable_cblt_first);
            if(w->objectName() == "enable_cblt_last") w->setChecked(module->conf_.enable_cblt_last);
            if(w->objectName() == "enable_cblt_middle") w->setChecked(module->conf_.enable_cblt_middle);
            it++;
        }
    }
    QList<QLabel*> clb = findChildren<QLabel*>();
    if(!clb.empty())
    {
        qDebug() << "some QLabel here";
        QList<QLabel*>::const_iterator it = clb.begin();
        while(it != clb.end())
        {
            qDebug() << "    iterator step";
            QLabel *w = (*it);

            if(w->objectName() == "input_file_name") {
                QString fileName = module->conf_.input_file_name;
                qDebug() << "input file name found inside apply settings.";
                w->setText(fileName.isEmpty() ? tr("no file selected") : fileName);
            }
            it++;
        }
    }

    //QLabel* b_addr = (QLabel*) uif.getWidgets()->find("base_addr").value();
    //b_addr->setText(tr("0x%1").arg(module->conf_.base_addr,2,16,QChar('0')));

    applyingSettings = false;
}


