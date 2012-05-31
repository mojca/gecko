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

#ifndef GECKOUI_H
#define GECKOUI_H

#include <iostream>

#include <QWidget>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QMap>
#include <QMessageBox>
#include <QMutex>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalMapper>
#include <QSpinBox>
#include <QStringList>
#include <QTabWidget>
#include <QTimer>
#include "hexspinbox.h"

#include <QDebug>

class GeckoUiFactory : public QObject
{
    Q_OBJECT

public:
    GeckoUiFactory(QWidget* parent, QTabWidget* _tabs) : parent(parent), tabs(_tabs) {}
    ~GeckoUiFactory() {}

    //    void addTab(QString _name);
    //    void addGroupToTab(QString _tname, QString _name, QString _cname = QString(""));
    //    void addUnnamedGroupToTab(QString _tname, QString _name);
    //    void addGroupToGroup(QString _tname, QString _gname, QString _name, QString _cname = QString(""));
    //    void addUnnamedGroupToGroup(QString _tname, QString _gname, QString _name);

    //    void addButtonToGroup (QString _tname, QString _gname, QString _name, QString _cname);
    //    void addSpinnerToGroup (QString _tname, QString _gname, QString _name, QString _cname, int min, int max);
    //    void addDoubleSpinnerToGroup (QString _tname, QString _gname, QString _name, QString _cname, double min, double max);
    //    void addHexSpinnerToGroup (QString _tname, QString _gname, QString _name, QString _cname, int min, int max);
    //    void addCheckBoxToGroup (QString _tname, QString _gname, QString _name, QString _cname);
    //    void addPopupToGroup (QString _tname, QString _gname, QString _name, QString _cname, QStringList _itNames);

    //    QWidget* attachLabel(QWidget* w,QString _label);

    QSignalMapper* getSignalMapper() {return &sm;}
    QMap<QString,QWidget*>* getWidgets() {return &widgets;}

protected:
    QMap<QString,QWidget*> tabsMap;
    QMap<QString,QWidget*> groups;
    QMap<QString,QWidget*> widgets;
    QSignalMapper sm;

private:
    QWidget* parent;
    QTabWidget* tabs;

    // Generic UI constructor methods

public:

    void addTab(QString _name)
    {
        QWidget* c = new QWidget();
        QGridLayout* l = new QGridLayout;
        l->setMargin(0);
        l->setVerticalSpacing(0);
        c->setLayout(l);
        int idx = tabs->addTab(c,_name);
        tabsMap.insert(_name,tabs->widget(idx));
    }

    //! Adds a named group to a tab
    //! \param _tname: name of the tab to add the group to
    //! \param _name: caption name of the new group
    //! \param _cname: if not empty, then the group will also be checkable, with _cname as config item name
    //! \param _align: alignment of the group contents (either "h" or "v", "h" by default)
    void addGroupToTab(QString _tname, QString _name, QString _cname = QString(""), QString _align = QString("h"))
    {
        if (tabsMap.contains(_tname)) {
            QWidget* c = tabsMap.value(_tname);
            QGroupBox* b = new QGroupBox(_name,c);
            QString identifier = _tname+_name;
            QLayout* l;
            if(_align == QString("h"))
            {
                l = new QHBoxLayout;
            }
            else
            {
                l = new QVBoxLayout;
            }
            //QGridLayout* l = new QGridLayout;
            l->setMargin(0);
            l->setSpacing(0);
            b->setLayout(l);
            c->layout()->addWidget(b);
            groups.insert(identifier,b);
            //cout << "Adding " << identifier.toStdString() << " to groups." << endl;
            if(!_cname.isEmpty())
            {
                b->setCheckable(true);
                b->setObjectName(_cname);
                widgets.insert(_cname,b);
                sm.setMapping(b,_cname);
                connect(b,SIGNAL(toggled(bool)),&sm,SLOT(map()));
            }
        }
    }

    void addUnnamedGroupToTab(QString _tname, QString _name, QString _align = QString("h"))
    {
        if (tabsMap.contains(_tname)) {
            QWidget* c = tabsMap.value(_tname);
            QWidget* g = new QWidget(c);
            QString identifier = _tname+_name;
            QLayout* l;
            if(_align == QString("h"))
            {
                l = new QHBoxLayout;
            }
            else
            {
                l = new QVBoxLayout;
            }
            //QGridLayout* l = new QGridLayout;
            l->setMargin(0);
            l->setSpacing(0);
            g->setLayout(l);
            g->setObjectName(identifier);
            c->layout()->addWidget(g);
            groups.insert(identifier,g);
            //cout << "Adding " << identifier.toStdString() << " to groups." << endl;
        }
    }

    void addGroupToGroup(QString _tname, QString _gname, QString _name, QString _cname = QString(""))
    {
        QString identifier = _tname+_gname;
        if (groups.contains(identifier)) {
            QWidget* g = groups.value(identifier);
            QGroupBox* b = new QGroupBox(_name,g);
            QGridLayout* l = new QGridLayout;
            l->setMargin(0);
            l->setVerticalSpacing(0);
            b->setLayout(l);
            g->layout()->addWidget(b);
            identifier += _name;
            groups.insert(identifier,b);
            //cout << "Adding " << identifier.toStdString() << " to groups." << endl;
            b->setObjectName(identifier);
            if(!_cname.isEmpty())
            {
                b->setCheckable(true);
                b->setObjectName(_cname);
                widgets.insert(_cname,b);
                sm.setMapping(b,_cname);
                connect(b,SIGNAL(toggled(bool)),&sm,SLOT(map()));
            }
        }
    }

    void addRadioGroupToGroup(QString _tname, QString _gname,
                              QString _name, QStringList _rnames, QStringList _cnames, QString _align = QString("h"))
    {
        QString identifier = _tname+_gname;
        if (groups.contains(identifier)) {
            QWidget* g = groups.value(identifier);
            QGroupBox* b = new QGroupBox(_name,g);
            QLayout* l;
            if(_align == QString("h")) {
                l = new QHBoxLayout;
            } else {
                l = new QVBoxLayout;
            }
            l->setMargin(0);
            l->setSpacing(0);
            b->setLayout(l);
            g->layout()->addWidget(b);
            identifier += _name;
            groups.insert(identifier,b);
            //cout << "Adding " << identifier.toStdString() << " to groups." << endl;
            b->setObjectName(identifier);
            if(!_cnames.isEmpty() && !_rnames.isEmpty())
            {
                int cnt = 0;
                foreach(QString rname, _rnames) {
                    if(cnt < _cnames.size()) {
                        QRadioButton* rb = new QRadioButton(rname,b);
                        QString _cname = _cnames.at(cnt);
                        rb->setObjectName(_cname);
                        widgets.insert(_cname,rb);
                        sm.setMapping(rb,_cname);
                        connect(rb,SIGNAL(toggled(bool)),&sm,SLOT(map()));
                        l->addWidget(rb);
                    } else {
                        std::cout << "ERROR: No config name for radio button: "
                                  << rname.toStdString() << std::endl;
                    }
                    ++cnt;
                }
            }
        }
    }

    void addUnnamedGroupToGroup(QString _tname, QString _gname, QString _name)
    {
        QString identifier = _tname+_gname;
        if (groups.contains(identifier)) {
            QWidget* g = groups.value(identifier);
            QWidget* b = new QWidget(g);
            QHBoxLayout* l = new QHBoxLayout;
            l->setMargin(0);
            l->setSpacing(0);
            b->setLayout(l);
            g->layout()->addWidget(b);
            identifier += _name;
            groups.insert(identifier,b);
            //cout << "Adding " << identifier.toStdString() << " to groups." << endl;
            b->setObjectName(identifier);
        }
    }
    void addButtonToGroup(QString _tname, QString _gname, QString _name, QString _cname)
    {
        QString identifier = _tname+_gname;
        if (groups.contains(identifier)) {
            QWidget* g = groups.value(identifier);
            QPushButton* b = new QPushButton(_name,g);
            g->layout()->addWidget(b);
            widgets.insert(_cname,b);
            b->setObjectName(_cname);
            sm.setMapping(b,_cname);
            connect(b,SIGNAL(clicked()),&sm,SLOT(map()));
        }
    }

    void addSpinnerToGroup(QString _tname, QString _gname, QString _name, QString _cname, int min, int max)
    {
        QString identifier = _tname+_gname;
        if (groups.contains(identifier)) {
            QWidget* g = groups.value(identifier);
            QSpinBox* b = new QSpinBox(g);
            b->setRange(min,max);
            //b->setMinimum(min);
            //b->setMaximum(max);
            QWidget* w = attachLabel(b,_name);
            g->layout()->addWidget(w);
            sm.setMapping(b,_cname);
            widgets.insert(_cname,b);
            b->setObjectName(_cname);
            connect(b,SIGNAL(valueChanged(int)),&sm,SLOT(map()));
        }
    }

    void addDoubleSpinnerToGroup(QString _tname, QString _gname, QString _name, QString _cname, double min, double max)
    {
        QString identifier = _tname+_gname;
        if (groups.contains(identifier)) {
            QWidget* g = groups.value(identifier);
            QDoubleSpinBox* b = new QDoubleSpinBox(g);
            b->setRange(min,max);
            //b->setMinimum(min);
            //b->setMaximum(max);
            QWidget* w = attachLabel(b,_name);
            g->layout()->addWidget(w);
            sm.setMapping(b,_cname);
            widgets.insert(_cname,b);
            b->setObjectName(_cname);
            connect(b,SIGNAL(valueChanged(int)),&sm,SLOT(map()));
        }
    }

    void addHexSpinnerToGroup(QString _tname, QString _gname, QString _name, QString _cname, int min, int max)
    {
        QString identifier = _tname+_gname;
        if (groups.contains(identifier)) {
            QWidget* g = groups.value(identifier);
            HexSpinBox* b = new HexSpinBox(g);
            b->setPrefix ("0x");
            b->setRange(min,max);
            //b->setMinimum(min);
            //b->setMaximum(max);
            QWidget* w = attachLabel(b,_name);
            g->layout()->addWidget(w);
            sm.setMapping(b,_cname);
            widgets.insert(_cname,b);
            b->setObjectName(_cname);
            connect(b,SIGNAL(valueChanged(int)),&sm,SLOT(map()));
        }
    }

    void addCheckBoxToGroup(QString _tname, QString _gname, QString _name, QString _cname)
    {
        QString identifier = _tname+_gname;
        if (groups.contains(identifier)) {
            QWidget* g = groups.value(identifier);
            QCheckBox* b = new QCheckBox(g);
            QWidget* w = attachLabel(b,_name);
            g->layout()->addWidget(w);
            sm.setMapping(b,_cname);
            widgets.insert(_cname,b);
            b->setObjectName(_cname);
            connect(b,SIGNAL(stateChanged(int)),&sm,SLOT(map()));
        }
    }

    void addPopupToGroup(QString _tname, QString _gname, QString _name, QString _cname, QStringList _itNames)
    {
        QString identifier = _tname+_gname;
        if (groups.contains(identifier)) {
            QWidget* g = groups.value(identifier);
            QComboBox* b = new QComboBox(g);
            QWidget* w = attachLabel(b,_name);
            g->layout()->addWidget(w);
            for(int i = 0; i<_itNames.size();i++)
            {
                QString _it = _itNames.at(i);
                b->addItem(_it,QVariant(i));
            }
            sm.setMapping(b,_cname);
            widgets.insert(_cname,b);
            b->setObjectName(_cname);
            connect(b,SIGNAL(currentIndexChanged(int)),&sm,SLOT(map()));
        }
    }

    void addFileBrowserToGroup(QString _tname, QString _gname, QString _name, QString _cname_filename, QString _cname_button, QString _buttonText)
    {
        QString identifier = _tname+_gname;
        if (groups.contains(identifier)) {
            QWidget* g = groups.value(identifier);
            qDebug() << "creating file group for " << identifier;

            QWidget* container = new QWidget();
            {
                QWidget* w = new QWidget(g); // top widget
                QGridLayout* cl = new QGridLayout(w);

                QLabel* fileLabel = new QLabel(_name); // File Name:
                // why do we need QLabel(g) here?
                QLabel* fileName  = new QLabel(w); // to be set up later: <file_not_found or actual file>; it could/should be QLineEdit

                QPushButton* buttonBrowseFile = new QPushButton(_buttonText,w); // Browse ...

                cl->addWidget(fileLabel,0,0);
                cl->addWidget(fileName,0,2);
                cl->addWidget(buttonBrowseFile,0,4);

                cl->setColumnMinimumWidth(1,10);
                cl->setColumnMinimumWidth(3,10);

                cl->setContentsMargins(0,0,0,0);
                cl->setSpacing(0);
                cl->setColumnStretch(2,1);

                g->layout()->addWidget(w);
                g->layout()->setSpacing(0);

                fileName->setFrameStyle(QFrame::Panel | QFrame::Sunken);
                fileName->setAlignment(Qt::AlignLeft);
                // TODO: change this to setFilename
                fileName->setText(tr("no file selected"));

                sm.setMapping(fileName, _cname_filename);
                sm.setMapping(buttonBrowseFile, _cname_button);
                widgets.insert(_cname_filename, fileName);
                widgets.insert(_cname_button, buttonBrowseFile);
                fileName->setObjectName(_cname_filename);
                buttonBrowseFile->setObjectName(_cname_button);
                connect(buttonBrowseFile,SIGNAL(clicked()),&sm,SLOT(map()));
            }
            /*
            QWidget* g = groups.value(identifier);

            QWidget* w = new QWidget(); // top widget
            QGridLayout* l = new QGridLayout(w); // grid layout for top widget

            QLabel* label = new QLabel(_name); // File Name
            QLabel* filename = new QLabel(g); // <file / file_not_found>
            QPushButton* buttonLoadFile = new QPushButton(_buttonText,w); // Browse ...

            l->addWidget(label,0,0);
            l->addWidget(filename,0,2);
            l->addWidget(buttonLoadFile,0,4);

            l->setColumnMinimumWidth(1,50);
            l->setColumnMinimumWidth(3,50);
            l->setContentsMargins(0,0,0,0);
            l->setSpacing(0);
            l->setColumnStretch(2,1);

            g->layout()->addWidget(w);
            g->layout()->setSpacing(0);

            // file not selected
            filename->setFrameStyle(QFrame::Panel | QFrame::Sunken);
            filename->setAlignment(Qt::AlignLeft);
            filename->setText(_fileNotSelectedText);

            sm.setMapping(filename,_cname); // I don't know what this does
            widgets.insert(_cname,filename);
            // TODO
            //connect(b,SIGNAL());
            connect(buttonLoadFile,SIGNAL(clicked()),this,SLOT(map()));
*/
        }
    }

    void addLineEditReadOnlyToGroup(QString _tname, QString _gname, QString _name, QString _cname, QString _defaultText)
    {
        QString identifier = _tname+_gname;
        if (groups.contains(identifier)) {
            QWidget* g = groups.value(identifier);
            //QLineEdit* b = new QLineEdit(g);
            QLabel* b = new QLabel(g);
            b->setFrameStyle(QFrame::Panel | QFrame::Sunken);
            b->setAlignment(Qt::AlignRight);
            b->setText(_defaultText);
            //b->setReadOnly(true);
            QWidget* w = attachLabel(b,_name);
            g->layout()->addWidget(w);
            sm.setMapping(b,_cname);
            widgets.insert(_cname,b);
            b->setObjectName(_cname);
        }
    }

    void addLineEditToGroup(QString _tname, QString _gname, QString _name, QString _cname, QString _defaultText)
    {
        QString identifier = _tname+_gname;
        if (groups.contains(identifier)) {
            QWidget* g = groups.value(identifier);
            QLineEdit* b = new QLineEdit(g);
            b->setText(_defaultText);
            QWidget* w = attachLabel(b,_name);
            g->layout()->addWidget(w);
            sm.setMapping(b,_cname);
            widgets.insert(_cname,b);
            b->setObjectName(_cname);
        }
    }

    QWidget* attachLabel(QWidget* w,QString _label)
    {
        QLabel* lbl = new QLabel(_label);
        QHBoxLayout* l = new QHBoxLayout();
        QWidget* ret = new QWidget();
        l->setMargin(0);
        l->addWidget(lbl);
        l->addWidget(w);
        l->setSpacing(0);
        ret->setLayout(l);
        return ret;
    }

};

#endif // GECKOUI_H
