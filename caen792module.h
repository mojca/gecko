#ifndef CAEN792MODULE_H
#define CAEN792MODULE_H

#include "basedaqmodule.h"
#include "baseplugin.h"
#include "demuxcaenadcplugin.h"
#include "pluginmanager.h"

struct Caen792ModuleConfig {
    uint32_t base_addr;

    uint8_t irq_level;
    uint8_t irq_vector;
    uint8_t ev_trg;

    uint8_t thresholds[32];
    bool killChannel[32];

    uint8_t cratenumber;
    uint16_t fastclear;
    uint8_t i_ped;
    uint8_t slideconst;

    // tdc registers
    uint8_t fsr;
    bool stop_mode;

    // control register 1
    bool block_end;
    bool berr_enable;
    bool program_reset;
    bool align64;

    // bit set 2
    bool memTestModeEnabled;
    bool offline;
    bool overRangeSuppressionEnabled;
    bool zeroSuppressionEnabled;
    bool slidingScaleEnabled;
    bool zeroSuppressionThr;
    bool autoIncrementEnabled;
    bool emptyEventWriteEnabled;
    bool slideSubEnabled;
    bool alwaysIncrementEventCounter;

    unsigned int pollcount;

    Caen792ModuleConfig ()
    : irq_level (0), irq_vector (0), ev_trg (0)
    , cratenumber (0), fastclear (0), i_ped (180), slideconst (0)
    , fsr (0x18), stop_mode (false)
    , block_end (false), berr_enable (true), program_reset (false), align64 (false)
    , memTestModeEnabled (false), offline (false), overRangeSuppressionEnabled (true)
    , zeroSuppressionEnabled (true), slidingScaleEnabled (false), zeroSuppressionThr (false)
    , autoIncrementEnabled (true), emptyEventWriteEnabled (false), slideSubEnabled (false)
    , alwaysIncrementEventCounter (false)
    , pollcount (10000)
    {
        for (int i = 0; i < 32; ++i) {
            killChannel [i] = false;
            thresholds [i] = 0;
        }
    }
};

class Caen792Module : public virtual BaseDAqModule {
	Q_OBJECT
public:
    // Factory method
    static BaseModule *createQdc (int id, const QString &name) {
        return new Caen792Module (id, name, true);
    }
    static BaseModule *createTdc (int id, const QString &name) {
        return new Caen792Module (id, name, false);
    }

	virtual void saveSettings (QSettings*);
	virtual void applySettings (QSettings*);

    ThreadBuffer<uint32_t> *getBuffer () { return NULL; }

	int counterReset ();
	int dataReset ();
	int softReset ();

	virtual void setChannels ();
	virtual int acquire ();
	virtual bool dataReady ();
	virtual int reset ();
	virtual int configure ();

    virtual uint32_t getBaseAddress () const;
    virtual void setBaseAddress (uint32_t baddr);

    uint16_t getInfo () const;
	int readStatus ();

	uint16_t getStatus1 () const { return status1; }
	uint16_t getStatus2 () const { return status2; }
	uint16_t getBitset1 () const { return bitset1; }
	uint16_t getBitset2 () const { return bitset2; }
	uint32_t getEventCount () const { return evcnt; }

	Caen792ModuleConfig *getConfig () { return &conf_; }

	int acquireSingle (uint32_t *data, uint32_t *rd);

private:
    Caen792Module (int _id, const QString &, bool _isqdc);
    void singleShot (uint32_t *data, uint32_t *rd);
    void writeToBuffer();
    void createBuffer ();
    void createOutputPlugin();

public slots:
    virtual void prepareForNextAcquisition () {}

private:
    Caen792ModuleConfig conf_;
    bool isqdc;

    mutable uint16_t info_;
    uint16_t bitset1;
    uint16_t bitset2;
    uint16_t status1;
    uint16_t status2;
    uint32_t evcnt;
    uint32_t data [34];
    uint32_t rd;
};

#endif // CAEN792MODULE_H
