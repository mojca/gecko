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
