#ifndef CAEN1290MODULE_H
#define CAEN1290MODULE_H

#include "basemodule.h"
#include "caen1290dmx.h"

struct Caen1290Config;
class Caen1290UI;

class Caen1290Module : public BaseModule {
    Q_OBJECT
private:
    class ModuleType {
    public:
        static const ModuleType V1190A, V1190B, V1290A, V1290N;

        enum Value {V1190A_v, V1190B_v, V1290A_v, V1290N_v};

        Value operator ()() const { return value_; }
        uint64_t boardId () const { return boardId_; }
        int channels () const {return channels_; }
        int tdcs () const { return tdcs_; }
        bool hires () const { return value_ == V1290A_v || value_ == V1290N_v; }

    private:
        ModuleType (Value v, uint64_t boardId, int channels, int tdcs)
        : value_ (v)
        , boardId_ (boardId)
        , channels_ (channels)
        , tdcs_ (tdcs)
        {}

        Value value_;
        uint64_t boardId_;
        int channels_;
        int tdcs_;
    };
    Caen1290Module(int id, QString name, ModuleType type);
public:
    ~Caen1290Module();

    static AbstractModule *create1290a (int id, const QString &name) {
        return new Caen1290Module (id, name, ModuleType::V1290A);
    }
    static AbstractModule *create1290n (int id, const QString &name) {
        return new Caen1290Module (id, name, ModuleType::V1290N);
    }
    static AbstractModule *create1190a (int id, const QString &name) {
        return new Caen1290Module (id, name, ModuleType::V1190A);
    }
    static AbstractModule *create1190b (int id, const QString &name) {
        return new Caen1290Module (id, name, ModuleType::V1190B);
    }

    virtual void setChannels();
    virtual int acquire(Event *ev);
    virtual bool dataReady();
    virtual int reset();
    virtual int configure();
    virtual void setBaseAddress (uint32_t baddr);
    virtual uint32_t getBaseAddress () const;

    virtual void saveSettings (QSettings *);
    virtual void applySettings (QSettings *);

private:
    int writeToMC (uint16_t data);
    bool isCorrectModule ();
    int softClear ();

private:
    Caen1290Config *conf_;
    Caen1290Demux dmx_;

    ModuleType type_;
    QVector<EventSlot*> evslots_;

    friend class Caen1290UI;
};

struct Caen1290Config {
    uint32_t base_addr;
    bool berr_enable;
    bool term;
    bool term_sw;
    bool emptyev;
    bool align64;
    bool comp_enable;
    bool test_fifo_enable;
    bool read_comp_sram;
    bool ev_fifo_enable;
    bool extrigtimetag_enable;
    uint16_t almost_full_lvl;
    uint16_t out_prog;
    uint16_t nof_aln_events;

    bool trig_match_mode;

    uint16_t win_width;
    uint16_t win_offset;
    uint16_t sw_margin;
    uint16_t rej_margin;
    bool sub_trg;

    uint16_t edge_detect_mode;
    uint16_t edge_lsb_val;
    uint16_t pair_lead_time;
    uint16_t pair_width_res;
    uint16_t hit_dead_time;

    uint16_t enable_chan_bitmap [8];
    uint16_t global_offset_coarse;
    uint16_t global_offset_fine;

    uint8_t channel_adj [128];
    uint16_t rc_adj [4];

    Caen1290Config ()
    : base_addr (0)
    , berr_enable (true), term (false), term_sw (false), emptyev (false), align64 (false)
    , comp_enable (true), test_fifo_enable (false), read_comp_sram (false), ev_fifo_enable (false)
    , extrigtimetag_enable (false), almost_full_lvl (0), out_prog (0), nof_aln_events (0)
    , trig_match_mode (false), win_width (0x14), win_offset (0xFFD8), sw_margin (0x08)
    , rej_margin (0x04), sub_trg (false), edge_detect_mode (2), edge_lsb_val (2), pair_lead_time (0)
    , pair_width_res (0), hit_dead_time (0), global_offset_coarse (0)
    , global_offset_fine (0)
    {
        for (int i = 0; i < 128; ++i) channel_adj [i] = 0;
        for (int i = 0; i < 4 ; ++i) rc_adj [i] = 0;
        for (int i = 0; i < 8 ; ++i) enable_chan_bitmap [i] = 0xFFFF;
    }
};

#endif // CAEN1290MODULE_H
