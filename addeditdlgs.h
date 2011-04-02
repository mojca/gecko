#ifndef ADDEDITDLGS_H
#define ADDEDITDLGS_H

#include "basemodule.h"
#include "basedaqmodule.h"
#include "baseinterfacemodule.h"

#include "modulemanager.h"
#include "pluginmanager.h"
#include "hexspinbox.h"

#include <QComboBox>
#include <QMessageBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>

class AddEditModuleDlg : public QDialog {
    Q_OBJECT
public:
    AddEditModuleDlg (QWidget *parent, BaseModule *mod = NULL)
    : QDialog (parent)
    , module_ (mod)
    {
        setModal (true);
        createUI ();
    }

    void createUI () {
        typeselector_ = new QComboBox (this);
        typeselector_->setEditable (false);
        typeselector_->setInsertPolicy (QComboBox::InsertAlphabetically);
        typeselector_->addItems (ModuleManager::ref().getAvailableTypes());

        name_ = new QLineEdit (this);

        ifaceselector_ = new QComboBox (this);
        typeselector_->setInsertPolicy (QComboBox::InsertAlphabetically);
        ifaceselector_->setEditable (false);
        QStringList ifacenames;
        foreach (BaseInterfaceModule *iface, (*ModuleManager::ref().listInterfaces ())) {
            ifacenames.append (iface->getName ());
        }
        ifaceselector_->addItems (ifacenames);

        baddr_ = new HexSpinBox (this);
        baddr_->setPrefix ("0x");
        baddr_->setSuffix ("0000");
        baddr_->setMinimum (0);
        baddr_->setMaximum (0xFFFF);

        if (module_) {
            setWindowTitle (tr("Edit Module"));
            name_->setText (module_->getName ());
            typeselector_->setCurrentIndex (typeselector_->findText (module_->getTypeName ()));
            typeselector_->setEnabled (false);
            if (module_->getModuleType() == AbstractModule::TypeDAq) {
                BaseDAqModule *daq =dynamic_cast<BaseDAqModule*> (module_);
                if (daq->getInterface ())
                    ifaceselector_->setCurrentIndex (ifaceselector_->findText (
                            daq->getInterface ()->getName ()
                            ));
                baddr_->setValue (daq->getBaseAddress() >> 16);
            } else {
                ifaceselector_->setEnabled (false);
                baddr_->setEnabled (false);
            }
        } else {
            setWindowTitle (tr("Add Module"));
        }

        QGridLayout *l = new QGridLayout (this);
        QLabel *lbl = new QLabel (tr ("Module name:"));
        l->addWidget (lbl, 0, 0, 1, 1, Qt::AlignRight);
        l->addWidget (name_, 0, 1, 1, 1);
        lbl = new QLabel (tr ("Module type:"));
        l->addWidget (lbl, 1, 0, 1, 1, Qt::AlignRight);
        l->addWidget (typeselector_, 1, 1, 1, 1);
        lbl = new QLabel (tr ("Interface:"));
        l->addWidget (lbl, 2, 0, 1, 1, Qt::AlignRight);
        l->addWidget (ifaceselector_, 2, 1, 1, 1);
        lbl = new QLabel (tr ("Base Address:"));
        l->addWidget (lbl, 3, 0, 1, 1, Qt::AlignRight);
        l->addWidget (baddr_, 3, 1, 1, 1);

        QDialogButtonBox *bbox = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        l->addWidget (bbox, 4, 0, 1, 2);
        setLayout (l);

        setTabOrder (name_, typeselector_);
        setTabOrder (typeselector_, ifaceselector_);
        setTabOrder (ifaceselector_, baddr_);
        setTabOrder (baddr_, bbox);

        name_->setFocus ();

        connect (bbox, SIGNAL (accepted()), SLOT (accept()));
        connect (bbox, SIGNAL (rejected()), SLOT (reject()));
        connect (typeselector_, SIGNAL (currentIndexChanged(QString)), SLOT (typeChanged (QString)));

        typeChanged (typeselector_->currentText ());
    }


public slots:
    virtual void accept () {
        if (name_->text().trimmed().isEmpty()) {
            QMessageBox::warning (this, tr ("Scope"), tr ("Please enter a name!"));
            return;
        }

        if (module_ != ModuleManager::ref().get (name_->text()) || PluginManager::ref().get (name_->text ())) {
            QMessageBox::warning (this, tr ("Scope"), tr ("A component with the same name already exists!"));
            return;
        }

        if (module_) {
            if (module_->getModuleType () == AbstractModule::TypeDAq) {
                BaseDAqModule *daq = static_cast<BaseDAqModule *> (module_);
                if (ifaceselector_->currentText().isEmpty()) {
                    QMessageBox::warning (this, tr("Scope"), tr("Please select an interface module!"));
                    return;
                }
                daq->setInterface (
                        ModuleManager::ref ().getIface (ifaceselector_->currentText()));
                daq->setBaseAddress (baddr_->value() << 16);
            }

            ModuleManager::ref ().setModuleName (module_, name_->text ());
        } else {
            BaseModule *m = ModuleManager::ref ().create (typeselector_->currentText (), name_->text ());
            if (m->getModuleType () == AbstractModule::TypeDAq) {
                BaseDAqModule *daq = static_cast<BaseDAqModule *> (m);
                daq->setInterface (
                        ModuleManager::ref ().getIface (ifaceselector_->currentText()));
                daq->setBaseAddress (baddr_->value() << 16);
            }
        }
        QDialog::accept ();
    }

private slots:
    void typeChanged (QString newType) {
        bool isdaq = ModuleManager::ref().getModuleTypeClass (newType) == AbstractModule::TypeDAq;
        ifaceselector_->setEnabled (isdaq);
        baddr_->setEnabled (isdaq);
    }

private:
    BaseModule *module_;

    QComboBox *typeselector_;
    QLineEdit *name_;
    QComboBox *ifaceselector_;
    HexSpinBox *baddr_;
};

class AddEditPluginDlg : public QDialog {
    Q_OBJECT
public:
    AddEditPluginDlg (QWidget *parent = NULL, BasePlugin *p = NULL)
    : QDialog (parent)
    , plugin_ (p)
    {
        setModal (true);
        createUI ();
    }

    void createUI () {
        name_ = new QLineEdit (this);

        typeselector_ = new QComboBox (this);
        typeselector_->setEditable (false);
        typeselector_->setInsertPolicy (QComboBox::InsertAlphabetically);
        typeselector_->addItems (PluginManager::ref ().getAvailableTypes ());

        groupselector_ = new QComboBox (this);
        groupselector_->setEditable (false);
        groupselector_->setInsertPolicy (QComboBox::InsertAlphabetically);
        groupselector_->addItem("Select Type");
        groupselector_->addItems (PluginManager::ref ().getAvailableGroups());

        attributes_ = new QTableWidget (this);
        attributes_->setColumnCount (2);
        attributes_->setHorizontalHeaderLabels (QStringList () << tr("Name") << tr("Value"));
        attributes_->verticalHeader ()->hide ();

        QGroupBox *attribgrp = new QGroupBox (tr ("Attributes"), this);
        QVBoxLayout *vbox = new QVBoxLayout ();
        vbox->addWidget (attributes_);
        attribgrp->setLayout (vbox);

        QDialogButtonBox *bbox = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

        QLabel *lbl;
        QGridLayout *l = new QGridLayout (this);
        lbl = new QLabel (tr("Plugin Name:"));
        l->addWidget (lbl, 0, 0, 1, 1, Qt::AlignRight);
        l->addWidget (name_, 0, 1, 1, 1);
        lbl = new QLabel (tr("Plugin Type:"));
        l->addWidget (lbl, 1, 0, 1, 1, Qt::AlignRight);
        l->addWidget (groupselector_, 1, 1, 1, 1);
        l->addWidget (typeselector_, 2, 1, 1, 1);
        l->addWidget (attribgrp, 3, 0, 1, 2);
        l->addWidget (bbox, 4, 0, 1, 2);
        setLayout (l);

        if (plugin_) {
            typeselector_->setEnabled (false);
            groupselector_->setEnabled (false);
            attribgrp->setEnabled (false);
            name_->setText (plugin_->getName ());
            typeselector_->setCurrentIndex (typeselector_->findText (plugin_->getTypeName ()));
            groupselector_->setCurrentIndex(groupselector_->findText(PluginManager::ref ().getGroupName(plugin_->getPluginGroup())));
            AbstractPlugin::AttributeMap amap = plugin_->getAttributeMap ();
            AbstractPlugin::Attributes attrs = plugin_->getAttributes ();
            attributes_->setRowCount (amap.size ());

            int i = 0;
            foreach (QString aname, amap.keys ()) {
                QTableWidgetItem *n = new QTableWidgetItem (aname);
                QTableWidgetItem *v = new QTableWidgetItem ((attrs.contains (aname) ? attrs.value (aname) : QVariant (amap.value (aname))).toString ());
                n->setFlags (Qt::ItemIsEnabled);
                v->setFlags (Qt::ItemIsEditable | Qt::ItemIsEnabled);
                attributes_->setItem (i, 0, n);
                attributes_->setItem (i, 1, v);
                ++i;
            }
        } else {
            connect (typeselector_, SIGNAL(currentIndexChanged(QString)), SLOT(typeChanged(QString)));
            connect (groupselector_, SIGNAL(currentIndexChanged(QString)), SLOT(groupChanged(QString)));
            groupChanged(groupselector_->currentText());
            typeChanged (typeselector_->currentText ());
        }

        connect (bbox, SIGNAL (accepted()), SLOT (accept()));
        connect (bbox, SIGNAL (rejected()), SLOT (reject()));
    }

private slots:
    void groupChanged(QString newgroupstring) {
        AbstractPlugin::Group newgroup = PluginManager::ref().getGroupFromString(newgroupstring);
        typeselector_->clear();
        typeselector_->addItems(PluginManager::ref().getAvailableTypesOfGroup(newgroup));
        if(typeselector_->count() > 0) {
            typeselector_->setEnabled(true);
            typeChanged(typeselector_->currentText());
        }
        else {
            typeselector_->setDisabled(true);
        }
    }

    void typeChanged (QString newType) {
        AbstractPlugin::AttributeMap amap = PluginManager::ref ().getAttributeMap (newType);
        int i = 0;
        attributes_->clearContents ();
        attributes_->setRowCount (amap.size ());
        foreach (QString aname, amap.keys ()) {
            QTableWidgetItem *n = new QTableWidgetItem (aname);
            QTableWidgetItem *v = new QTableWidgetItem (QVariant (amap.value (aname)).toString ());
            n->setFlags (Qt::ItemIsEnabled);
            v->setFlags (Qt::ItemIsEditable | Qt::ItemIsEnabled);
            attributes_->setItem (i, 0, n);
            attributes_->setItem (i, 1, v);
            ++i;
        }
    }

    void accept () {
        if (name_->text().trimmed().isEmpty()) {
            QMessageBox::warning (this, tr ("Scope"), tr ("Please enter a name!"));
            return;
        }

        if (plugin_ != PluginManager::ref().get (name_->text()) || ModuleManager::ref().get (name_->text ())) {
            QMessageBox::warning (this, tr ("Scope"), tr ("A component with the same name already exists!"));
            return;
        }

        if (plugin_) {
            PluginManager::ref ().setPluginName (plugin_, name_->text ());
        } else {
            AbstractPlugin::AttributeMap amap = PluginManager::ref ().getAttributeMap (typeselector_->currentText ());
            AbstractPlugin::Attributes attrs;
            for (int i = 0; i < attributes_->rowCount (); ++i) {
                QString aname = attributes_->item(i, 0)->text ();
                if (!amap.contains (aname)) continue;
                QVariant val = QVariant::fromValue (attributes_->item (i, 1)->text());
                if (val.convert (amap.value (aname))) {
                    attrs.insert (aname, val);
                } else {
                    QMessageBox::warning (this, tr("Scope"), tr("Value for attribute \"%1\" is invalid").arg(aname));
                    return;
                }
            }
            PluginManager::ref ().create (typeselector_->currentText (), name_->text (), attrs);
        }

        QDialog::accept ();
    }

private:
    BasePlugin *plugin_;
    QLineEdit *name_;
    QComboBox *typeselector_;
    QComboBox *groupselector_;
    QTableWidget *attributes_;
};

#endif // ADDEDITDLGS_H
