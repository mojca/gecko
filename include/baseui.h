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

#ifndef BASEUI_H
#define BASEUI_H

#include <QWidget>

/*! Base class for module UIs.
 *  Derive from this class to create a user interface for your modules.
 */
class BaseUI : public QWidget
{
public:
    BaseUI (QWidget *parent = NULL)
        : QWidget (parent) {}

    virtual ~BaseUI () {}
    /*! Called by the module whenever the configuration changes. Implementors should update the UI to reflect the new configuration. */
    virtual void applySettings() = 0;
};

#endif // BASEUI_H
