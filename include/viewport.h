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

#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QWidget>
#include <QPoint>
#include <QList>
#include <QPolygon>
#include <QPainter>
#include <QMouseEvent>
#include <QToolTip>

#include "confmap.h"

//#define MAX_NOF_LWORDS 0x4000000 // 256 MByte
#define MAX_NOF_LWORDS 0x2000000 // 128 MByte


// THIS CLASS IS DEPRECATED AND ITS USE IS DISCOURAGED


class QSettings;

class Viewport : public QWidget
{
public:
    Viewport(QWidget *parent, u_int32_t (*)[MAX_NOF_LWORDS], unsigned int nof_channels, unsigned int nof_samples);
    void changeSampleLength(unsigned int newValue);
    void applySettings (QSettings *);

protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void drawSignals();
    void drawTicks();

    int x;
    int y;
    int maxY;

    unsigned int nof_channels;
    unsigned int nof_samples;
    u_int32_t (*data)[MAX_NOF_LWORDS];

private:
    static const ConfMap::confmap_t<Viewport> confmap [];
};

#endif // VIEWPORT_H
