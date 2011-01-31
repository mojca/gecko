#ifndef CAEN820SCALER_H
#define CAEN820SCALER_H

#include "basedaqmodule.h"

struct Caen820Config {
    uint32_t baddr;
    uint32_t channel_enable;
    uint16_t acq_mode;
    uint32_t dwell_time;
    bool short_data_format;
    bool berr_enable;
    bool hdr_enable;
    bool clear_meb;
    bool auto_reset;

    Caen820Config ()
    : baddr (0)
    , channel_enable (0xFFFFFFFF)
    , acq_mode (1)
    , dwell_time (3)
    , short_data_format (false)
    , berr_enable (true)
    , hdr_enable (false)
    , clear_meb (false)
    , auto_reset (false)
    {}
};

class DemuxCaen820Plugin;
class Caen820UI;

class Caen820Module : public BaseDAqModule {
    Q_OBJECT
public:
    static BaseModule *create (int id, const QString& name);

    ThreadBuffer<uint32_t>* getBuffer () { return NULL; }
    void setChannels ();
    int acquire ();
    bool dataReady ();
    int reset ();
    int configure ();
    void createOutputPlugin ();
    void setBaseAddress (uint32_t baddr);
    uint32_t getBaseAddress () const;

    void applySettings (QSettings *);
    void saveSettings (QSettings *);

public slots:
    void prepareForNextAcquisition () {};

private:
    Caen820Module (int id, const QString &name);

    int getNofActiveChannels ();
    int dataClear ();

private:
    Caen820Config conf_;
    DemuxCaen820Plugin *out_;

    friend class Caen820UI;
};

#endif // CAEN820SCALER_H
