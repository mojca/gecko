#ifndef CONFMAP_H
#define CONFMAP_H

#include <stdint.h>
#include <cstdio>
#include <QString>
#include <QSettings>

namespace ConfMap {
/*! Convenience mechanism for loading and saving configuration.
 *  Most modules and plugins use a struct to hold their configuration. When
 *  loading or saving settings, the configuration has to be retrieved from or stored in a QSettings object.
 *
 *  The confmap_t struct simplifies this process by creating a option name to option variable storage location
 *  that can be automatically read and updated. All the module writer has to do is providing an array of mappings
 *  and call ConfMap::apply and ConfMap::save respectively during applySettings and saveSettings.
 *
 *  \code
 *  typedef ConfMap::confmap_t<MyModulesConfig> confmap_t;
 *  static const confmap_t confmap [] = {
 *      confmap_t ("myfirstattr", &MyModulesConfig::firstattr),
 *      confmap_t ("mysecondattr", &MyModulesConfig::secondattr)
 *  };
 *
 *  //...
 *
 *  void MyModule::applySettings (QSettings *s) {
 *      using namespace ConfMap;
 *      s->beginGroup (getName ());
 *      apply (s, &conf, confmap);
 *      s->endGroup ();
 *  }
 *  \endcode
 */
template <typename T>
struct confmap_t {
    QString name; /*!< name of the configuration option. */
    enum {uint8, uint16, uint32, boolean, integer, dble, string} type; /*!< type of the configuration option. */
    union {
        uint8_t T::* uint8_val;
        uint16_t T::* uint16_val;
        uint32_t T::* uint32_val;
        double T::* double_val;
        bool T::* boolean_val;
        int T::* integer_val;
        QString T::* string_val;
    };

    /*! Creates a configuration mapping for a uint8 member. */
    confmap_t (const QString &n, uint8_t T::* pm)
        : name (n), type (uint8), uint8_val (pm) {}
    /*! Creates a configuration mapping for a uint16 member. */
    confmap_t (const QString &n, uint16_t T::* pm)
        : name (n), type (uint16), uint16_val (pm) {}
    /*! Creates a configuration mapping for a uint32 member. */
    confmap_t (const QString &n, uint32_t T::* pm)
        : name (n), type (uint32), uint32_val (pm) {}
    /*! Creates a configuration mapping for a bool member. */
    confmap_t (const QString &n, bool T::* pm)
        : name (n), type (boolean), boolean_val (pm) {}
    /*! Creates a configuration mapping for an int member. */
    confmap_t (const QString &n, int T::* pm)
        : name (n), type (integer), integer_val (pm) {}
    /*! Creates a configuration mapping for a double member. */
    confmap_t (const QString &n, double T::* pm)
        : name (n), type (dble), double_val (pm) {}
    /*! Creates a configuration mapping for a QString member. */
    confmap_t (const QString &n, QString T::* pm)
        : name (n), type (string), string_val (pm) {}
};

/*! Loads settings into the configuration structure.
 *  This function uses the mapping given in \c confmap to load the members of \c conf_ with the values stored in
 *  the QSettings object \c settings.
 *  \relates ConfMap::confmap_t
 *  \sa save
 */
template <typename T, size_t confmap_len>
void apply (QSettings *settings, T *conf_, const confmap_t<T> (&confmap) [confmap_len]) {
    for (size_t i = 0; i < confmap_len; ++i) {
        if (settings->contains (confmap [i].name)) {
            switch (confmap [i].type) {
            case confmap_t<T>::uint8:
                conf_->*(confmap [i].uint8_val) = settings->value (confmap [i].name).toUInt ();
                break;
            case confmap_t<T>::uint16:
                conf_->*(confmap [i].uint16_val) = settings->value (confmap [i].name).toUInt ();
                break;
            case confmap_t<T>::uint32:
                conf_->*(confmap [i].uint32_val) = settings->value (confmap [i].name).toUInt ();
                break;
            case confmap_t<T>::boolean:
                conf_->*(confmap [i].boolean_val) = settings->value (confmap [i].name).toBool ();
                break;
            case confmap_t<T>::integer:
                conf_->*(confmap [i].integer_val) = settings->value (confmap [i].name).toInt ();
                break;
            case confmap_t<T>::dble:
                conf_->*(confmap [i].double_val) = settings->value (confmap [i].name).toDouble ();
                break;
            case confmap_t<T>::string:
                conf_->*(confmap [i].string_val) = settings->value (confmap [i].name).toString ();
                break;
            }
        }
    }
}

/*! Saves the settings in the configuration structure.
 *  This function uses the mapping given in \c confmap to save the members of \c conf_ to the QSettings object \c settings.
 *  \relates ConfMap::confmap_t
 *  \sa apply
 */
template <typename T, size_t confmap_len>
void save (QSettings *settings, T* conf_, const confmap_t<T> (&confmap) [confmap_len]) {
    for (size_t i = 0; i < confmap_len; ++i) {
        switch (confmap [i].type) {
        case confmap_t<T>::uint8:
            settings->setValue (confmap [i].name, conf_->*(confmap [i].uint8_val));
            break;
        case confmap_t<T>::uint16:
            settings->setValue (confmap [i].name, conf_->*(confmap [i].uint16_val));
            break;
        case confmap_t<T>::uint32:
            settings->setValue (confmap [i].name, conf_->*(confmap [i].uint32_val));
            break;
        case confmap_t<T>::boolean:
            settings->setValue (confmap [i].name, conf_->*(confmap [i].boolean_val));
            break;
        case confmap_t<T>::integer:
            settings->setValue (confmap [i].name, conf_->*(confmap [i].integer_val));
            break;
        case confmap_t<T>::dble:
            settings->setValue (confmap [i].name, conf_->*(confmap [i].double_val));
            break;
        case confmap_t<T>::string:
            settings->setValue (confmap [i].name, conf_->*(confmap [i].string_val));
            break;
        }
    }
}

}

#endif // CONFMAP_H
