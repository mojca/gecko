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

#ifndef PLOT2D_H
#define PLOT2D_H

#include <QAction>
#include <QWidget>
#include <QPoint>
#include <QFileDialog>
#include <QList>
#include <QPolygon>
#include <QPainter>
#include <QMenu>
#include <QMouseEvent>
#include <QToolTip>
#include <QReadWriteLock>
#include <QPrinter>
#include <QPainter>
#include <vector>
#include <samdsp.h>

//#define MAX_NOF_LWORDS 0x4000000 // 128 MByte

class QTimer;
class QPixmap;

/*! An annotation belonging to a channel. */
class Annotation
{
public:
    enum annoType{note,vline,hline};

    Annotation(QPoint p, annoType type, QString text = "unset");
    ~Annotation();

    QPoint getPoint() {return this->p;}
    QString getText() {return this->text;}
    annoType getType() {return this->type;}

private:
    QPoint p;
    QString text;
    annoType type;
};

/*! A channel that is displayed in a plot2d */
class Channel : public QObject
{
    Q_OBJECT
public:
    Channel(QVector<double> data);
    ~Channel();

    enum plotType{line,steps};

    void setColor(QColor color);
    void setName(QString name);
    void setId(unsigned int id);
    void setData(QVector<double> data);
    void setType(plotType type);
    void setEnabled(bool enabled);
    void setStepSize(double stepSize);
    void addAnnotation(QPoint p, Annotation::annoType type, QString text);
    void clearAnnotations();

    QColor getColor() {return this->color; }
    QString getname()  {return this->name; }
    unsigned int getId() {return this->id; }
    bool isEnabled() {return this->enabled; }
    double getStepSize() {return this->stepSize; }
    plotType getType() {return this->type; }
    QVector<double> getData() {return this->data; }
    QList<Annotation*> *getAnnotations() {return &annotations; }

    double xmin, xmax, ymin, ymax;
    bool ylog, xlog;

signals:
    void changed ();

private:
    QColor color;
    QString name;
    plotType type;
    unsigned int id;
    bool enabled;
    double stepSize;

    QVector<double> data;
    QList<Annotation *> annotations;
};

/*! A widget for showing two-dimensional plots.
 *  Using addChannel, you can add an arbitrary number of channels to the plot. The
 *  tick marks refer to only one channel which is user-selectable via a context menu.
 *  Displayed data may also be saved to a file.
 *  \todo More Doc!
 */
class plot2d : public QWidget
{
    Q_OBJECT

public:
    plot2d(QWidget *parent, QSize size, unsigned int id);
    ~plot2d();

    void addChannel(unsigned int id, QString name, QVector<double> data,
                    QColor color, Channel::plotType type, double stepSize);
    void removeChannel(unsigned int id);
    void redraw();
    void resetBoundaries(int ch);

    unsigned int getNofChannels() {return this->channels->size();}
    Channel* getChannelById(unsigned int id) {return this->channels->at(id);}
    QReadWriteLock* getChanLock () { return &lock; }

public slots:
    void clearHistogram();
    void saveChannel();
    void savePDF();
    void setMaximumExtends(int,int,int,int);
    void toggleExternalBoundaries(bool);
    void zoomExtends(bool);
    void selectCurTickCh(int _curTickCh);

signals:
    void histogramCleared(unsigned int, unsigned int);

protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *ev);
    void contextMenuEvent(QContextMenuEvent *event);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);

private slots:
    void channelUpdate ();

private:
    SamDSP* dsp;
    unsigned int id;
    double curxmin, curxmax, curymin, curymax;
    double ext_xmin, ext_xmax, ext_ymin, ext_ymax;

    int curTickCh;

    void setBoundaries();
    void drawTicks(QPainter &);
    void drawChannels(QPainter &);
    void drawChannel(QPainter &, unsigned int id);

    void createActions();

    double plotHeight;
    double plotWidth;

    bool useExternalBoundaries;
    bool zoomExtendsTrue;

    QList<Channel*>* channels;
    QAction* clearHistogramAction;
    QAction* saveChannelAction;
    QAction* printAction;
    QList<QAction*> setCurTickChActions;

    enum {ScaleX, ScaleY, ScaleOff} scalemode;
    double scalestart;
    double scaleend;

    QRectF viewport;

    QPixmap *backbuffer;
    bool backbuffervalid;
    QReadWriteLock lock;
};

#endif // PLOT2D_H
