#include "viewport.h"
#include <iostream>
#include "confmap.h"

Viewport::Viewport(QWidget *parent, u_int32_t (*data)[MAX_NOF_LWORDS], unsigned int nof_channels, unsigned int nof_samples)
        : QWidget(parent)
{
    // Set initial size
    this->x = 480;
    this->y = 320;
    this->maxY = 4096;

    this->data = data;
    this->nof_channels = nof_channels;
    this->nof_samples = nof_samples;

    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    this->setMinimumSize(QSize(x,y));
    this->setMaximumSize(QSize(x,y));

    setMouseTracking(true);

}

typedef ConfMap::confmap_t<Viewport> confmap_t;
const confmap_t Viewport::confmap [] = {
    confmap_t ("x", &Viewport::x),
    confmap_t ("y", &Viewport::y),
    confmap_t ("maxY", &Viewport::maxY)
};

void Viewport::applySettings(QSettings *s) {
    s->beginGroup ("Viewport");
    ConfMap::apply (s, this, confmap);
    s->endGroup ();
}

void Viewport::changeSampleLength(unsigned int newValue)
{
    std::cout << "Changed viewport sample length to " << newValue << std::endl;
    this->nof_samples = newValue;
}

void Viewport::paintEvent(QPaintEvent *)
{
    drawSignals();
    //drawFrame();
    drawTicks();
}

void Viewport::mouseMoveEvent(QMouseEvent *ev)
{
    QPoint p = (ev->pos());
    QToolTip::showText(ev->globalPos(),
                       tr("%1,%2").arg((this->nof_samples*p.x()/this->x),
                                       3,10).arg((int)(this->maxY*(1-1.0*p.y()/this->y)),3,10),this);
}

void Viewport::drawSignals()
{
    unsigned int i=0, j=0;
    QPainter painter(this);

    // Read points from data
    for(i=0;i<this->nof_channels;i++)
    {
        QPolygon poly;
        for(j=4;j<(this->nof_samples/2+4);j++)
        {
            unsigned int h = (data[i][j] >> 16) & 0xFFFF;
            unsigned int l =  data[i][j] & 0xFFFF;
            QPoint pointh((((j-4)*2.)*x)/this->nof_samples,y-(static_cast<int>((h*y)/maxY)));
            QPoint pointl((((j-4)*2.*x)+1)/this->nof_samples,y-(static_cast<int>((l*y)/maxY)));
            //printf("Appending point %d %d\n",pointh->x(),pointh->y());
            poly.append(pointl);
            if(pointh.x() != pointl.x()) poly.append(pointh);
        }


        painter.setPen(QPen(QColor(255-i*40,i*40,i*60)));
        painter.drawPolyline(poly);
        painter.drawText(poly.at(0),tr("%1").arg(i,1,10,QChar()));
        poly.clear();
    }
}

void Viewport::drawTicks()
{
    int i=0, j=0;
    unsigned int incx=0, incy=0;

    // Draw tickmarks around the border
    QPainter painter(this);
    painter.setPen(QPen(QColor(0,0,0)));
    while(incx<50 && incx<nof_samples/10)
    {
        incx+=10;
    }
    while(incx<nof_samples/10)
    {
        incx+=50;
    }
    while(incy<static_cast<unsigned int>(maxY)/5)
    {
        incy+=50;
    }
    for(i=0; i<this->x; i+=(incx*this->x/nof_samples))
    {
        QLine line1(i,0,i,3);
        QLine line2(i,y,i,y-3);

        painter.drawLine(line1);
        painter.drawLine(line2);
        painter.drawText(i+2,10,tr("%1").arg(j,5,10));
        j+=incx;
    }
    j=0;
    for(i=this->y; i>0; i-=(incy*this->y/maxY))
    {
        QLine line1(0,i,3,i);
        painter.drawLine(line1);
        QLine line2(x,i,x-3,i);
        painter.drawLine(line2);
        painter.drawText(x-30,i-6,tr("%1").arg(j,5,10));
        j+= incy;
    }
}
