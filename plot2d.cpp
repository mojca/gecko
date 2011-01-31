#include "plot2d.h"

#include <QTimer>
#include <QPixmap>


Annotation::Annotation(QPoint _p, annoType _type, QString _text)
        : p(_p),type(_type)
{
    this->text = _text;
}

Channel::Channel(vector<double> const& _data)
{
    data = _data;
    this->enabled = true;
    this->color = QColor(Qt::black);
    this->id = 0;
    this->name = QString("unset");
    annotations = new QList<Annotation*>;
    xmin = 0;
    xmax = 0;
    ymin = 0;
    ymax = 0;
}

Channel::~Channel()
{
    clearAnnotations();
    delete this->annotations;
    name.clear();
}

void Channel::clearAnnotations(){ this->annotations->clear();}
void Channel::setColor(QColor color){ this->color = color;}
void Channel::setData(vector<double> const& _data)
{
    this->data = _data;
    emit changed ();
}

void Channel::setEnabled(bool enabled){ this->enabled = enabled;}
void Channel::setId(unsigned int id){ this->id = id;}
void Channel::setName(QString name){ this->name = name;}
void Channel::setType(plotType type){ this->type = type;}
void Channel::setStepSize(double stepSize){ this->stepSize = stepSize;}

void Channel::addAnnotation(QPoint p, Annotation::annoType type, QString text)
{
    Annotation* a = new Annotation(p,type,text);
    this->annotations->push_back(a);
}



// Plot2D
plot2d::plot2d(QWidget *parent, QSize size, unsigned int id)
        : QWidget(parent)
        , curTickCh(0)
        , scalemode (ScaleOff)
        , viewport (0, 0, 1, 1)
        , backbuffer (NULL)
        , backbuffervalid (false)

{
    this->id = id;
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(false);

    this->setGeometry(QRect(QPoint(0,0),size));
    this->channels = new QList<Channel*>;
//    xmin = 0; ymin = 0;
//    xmax = this->width(); ymax = this->height();

    useExternalBoundaries = false;
    zoomExtendsTrue = false;
//    ext_xmin = xmin;
//    ext_xmax = xmax;
//    ext_ymin = ymin;
//    ext_ymax = ymax;

    createActions();
    setMouseTracking(true);
}

plot2d::~plot2d()
{
    channels->clear();
    delete channels;
    delete backbuffer;
}

void plot2d::resizeEvent(QResizeEvent *event)
{
    delete backbuffer;
    backbuffer = NULL;
    update();
    QWidget::resizeEvent(event);
}

void plot2d::addChannel(unsigned int id, QString name, vector<double> const& data,
                        QColor color, Channel::plotType type = Channel::line, double stepSize = 1)
{
    Channel *newChannel = new Channel(data);
    newChannel->setColor(color);
    newChannel->setName(name);
    newChannel->setId(id);
    newChannel->setStepSize(stepSize);
    newChannel->setType(type);
    this->channels->append(newChannel);
    connect (newChannel, SIGNAL(changed()), SLOT(channelUpdate()));

    QAction* a = new QAction(tr("Ch %1").arg(id,1,10),this);
    a->setData(QVariant::fromValue(id));
    setCurTickChActions.push_back(a);
}

void plot2d::removeChannel(unsigned int id)
{
    for(unsigned int i = 0; i < this->getNofChannels(); i++)
    {
        if(this->channels->at(i)->getId() == id)
        {
            Channel * ch = channels->at(i);
            ch->disconnect(SIGNAL(changed()), this, SLOT(channelUpdate()));
            channels->removeAt(i);
            delete ch;
        }
    }
}

void plot2d::mousePressEvent (QMouseEvent *ev) {
    if (ev->y () <= 5 || ev->y () >= height () - 5) {
        scalemode = ScaleX;
        scalestart = viewport.x () + ev->x () / (1.0 * width ()) * viewport.width ();
        scaleend = scalestart;
    } else if (ev->x () <= 5 || ev->x () >= width () - 5) {
        scalemode = ScaleY;
        scalestart = viewport.y () + ev->y () / (1.0 * height ()) * viewport.height ();
        scaleend = scalestart;
    }
}

void plot2d::mouseReleaseEvent(QMouseEvent *) {
    switch (scalemode) {
    case ScaleX:
        if (fabs (scalestart - scaleend) / viewport.width () * width () >= 5) {
            viewport.setLeft (std::min (scalestart, scaleend));
            viewport.setRight (std::max (scalestart, scaleend));
        }
        break;
    case ScaleY:
        if (fabs (scalestart - scaleend) / viewport.height () * height ()>= 5) {
            viewport.setTop (std::min (scalestart, scaleend));
            viewport.setBottom (std::max (scalestart, scaleend));
        }
        break;
    default:
        break;
    }

    if (scalemode != ScaleOff) {
        scalemode = ScaleOff;
        unsetCursor ();
        backbuffervalid = false;
        update ();
    }
}

void plot2d::mouseMoveEvent(QMouseEvent *ev)
{
    curxmin = channels->at(curTickCh)->xmin;
    curxmax = channels->at(curTickCh)->xmax;
    curymin = channels->at(curTickCh)->ymin;
    curymax = channels->at(curTickCh)->ymax;

    QPoint p = (ev->pos());
    QToolTip::showText(ev->globalPos(),
                       tr("%1,%2")
                       .arg((int)((viewport.x()+viewport.width()*p.x()/width())*(curxmax-curxmin)),3,10)
                       .arg((int)(((viewport.y()+viewport.height()*(1-1.0*p.y()/height()))*(curymax-curymin))+curymin),3,10),this);

    if (ev->y () <= 5 || ev->y () >= height () - 5) {
        setCursor (Qt::SizeHorCursor);
    } else if (ev->x () <= 5 || ev->x () >= width () - 5) {
        setCursor (Qt::SizeVerCursor);
    } else if (scalemode == ScaleOff) {
        unsetCursor ();
    }

    if (scalemode == ScaleX || scalemode == ScaleY) {
        switch (scalemode) {
        case ScaleX: scaleend = viewport.x () + ev->x () / (1.0 * width ()) * viewport.width (); break;
        case ScaleY: scaleend = viewport.y () + ev->y () / (1.0 * height ()) * viewport.height (); break;
        default: break;
        }
        update ();
    }
}

void plot2d::mouseDoubleClickEvent(QMouseEvent *) {
    viewport.setCoords (0, 0, 1, 1);
    backbuffervalid = false;
    update ();
}

void plot2d::channelUpdate () {
    backbuffervalid = false;
}

void plot2d::paintEvent(QPaintEvent *)
{
    if (!backbuffer || !backbuffervalid) {
        if (!backbuffer) {
            backbuffer = new QPixmap (size ());
        }

        backbuffer->fill (Qt::white);
        QPainter pixmappainter (backbuffer);
        setBoundaries();
        drawChannels(pixmappainter);
        drawTicks(pixmappainter);
        backbuffervalid = true;
    }

    QPainter painter (this);
    painter.drawPixmap(0, 0, *backbuffer);

    if (scalemode == ScaleX) {
        double start = (scalestart - viewport.x ()) / viewport.width();
        double end = (scaleend - viewport.x()) / viewport.width();
        QLine line1 (start * width (), 0.005 * height (), end * width (), 0.005 * height ());
        QLine line2 (start * width (), 0.995 * height (), end * width (), 0.995 * height ());

        painter.save ();
        QPen mypen = painter.pen();
        mypen.setColor(QColor(0,0,0,150));
        mypen.setWidth(5);
        painter.setPen (mypen);
        painter.drawLine (line1);
        painter.drawLine (line2);

        painter.restore ();
    }

    if (scalemode == ScaleY) {
        double start = (scalestart - viewport.y ()) / viewport.height();
        double end = (scaleend - viewport.y()) / viewport.height();
        QLine line1 (0.005 * width (), start * height (), 0.005 * width (), end * height ());
        QLine line2 (0.995 * width (), start * height (), 0.995 * width (), end * height ());

        painter.save ();
        QPen mypen = painter.pen();
        mypen.setColor(QColor(0,0,0,150));
        mypen.setWidth(5);
        painter.setPen (mypen);
        painter.drawLine (line1);
        painter.drawLine (line2);
        painter.restore ();
    }
}

void plot2d::redraw()
{
    this->update();
}

void plot2d::setBoundaries()
{
    // Get extents in data
    foreach(Channel* ch, (*channels))
    {
        if(useExternalBoundaries)
        {
            if(ch->ymax > ext_ymax) ch->ymax = ext_ymax;
            if(ch->ymin < ext_ymin) ch->ymin = ext_ymin;
            if(ch->xmax > ext_xmax) ch->xmax = ext_xmax;
            if(ch->xmin < ext_xmin) ch->xmin = ext_xmin;
        }
        else if(ch->isEnabled())
        {
            int newymin;
            int newymax;

            vector<double> data = ch->getData();
            if(data.size() > 0)
            {
                ch->xmax = data.size();
                if(ch->getType() == Channel::steps)
                {
                    ch->ymin = 0;
                }
                else
                {
                    newymin = dsp->min(data)[AMP];
                    if(newymin < ch->ymin) ch->ymin = newymin;
                }
                newymax = dsp->max(data)[AMP];
                if(newymax > ch->ymax) ch->ymax = newymax;
                if(zoomExtendsTrue)
                {
                    ch->ymax = newymax;
                    ch->ymin = newymin;
                }
            }

            //cout << "Bounds: (" << ch->xmin << "," << ch->xmax << ") (" << ch->ymin << "," << ch->ymax << ") " << endl;

            data.clear();
        }
    }
}

void plot2d::drawChannels(QPainter &painter)
{
    for(unsigned int i = 0; i < this->getNofChannels(); i++)
    {
        if(channels->at(i)->isEnabled())
        {
            drawChannel(painter, i);
        }
    }
}

void plot2d::drawChannel(QPainter &painter, unsigned int id)
{
    Channel *curChan = channels->at(id);
    const vector<double> &data = curChan->getData();
    Channel::plotType curType = curChan->getType();
    double nofPoints = data.size();

    //cout << "Drawing ch " << id << " with size " << data.size() << endl;

    if(nofPoints > 0)
    {
        painter.save ();
        painter.setWindow(QRectF (viewport.x () * width (), viewport.y () * height (),
                                  viewport.width () * width (), viewport.height () * height ()).toRect ());
        double max = curChan->ymax;
        double min = curChan->ymin;

        // Move 0,0 to lower left corner
        painter.translate(0,this->height());
        if(curType == Channel::steps)
        {
            min = 0;
        }

        //cout << "Bounds: (" << curChan->xmin << "," << curChan->xmax << ") (" << curChan->ymin << "," << curChan->ymax << ") " << endl;

        QPolygon poly;
        int lastX = 0;
        double stepX = (curChan->xmax*1.)/(double)(width()/viewport.width());
        if(stepX < 1) stepX = 1;
        int delta = 0;
        int deltaX = 0;
        int lastData = 0;
        for(unsigned int i = curChan->xmin; (i < nofPoints && i < curChan->xmax); i++)
        {
            // Only append point, if it would actually be displayed
            if(abs(data[i]-data[lastX]) > abs(delta))
            {
                delta = data[i]-data[lastX];
                deltaX = i;
                //std::cout << "Delta: " << delta << " , i: " << i << std::endl;
            }
            if((int)(i) >= lastX+stepX || i == (curChan->xmax-1))
            {
                 // y-values increase downwards
                 //poly.push_back(QPoint(i,-data[i]));
                 //poly.push_back(QPoint(i+1,-data[i]));
                 lastData += delta;
                 poly.push_back(QPoint(i,-data[deltaX]));
                 poly.push_back(QPoint(i+1,-data[deltaX]));
                 lastX = i;
                 delta=0;
            }
        }
        painter.setPen(QPen(curChan->getColor()));
        painter.drawText(QPoint(0,id*20),tr("%1").arg(id,1,10));

        // Scale and move to display complete signals
        if(max-min < 0.00000001) max++;
        if(curChan->xmax-curChan->xmin < 0.00000001) curChan->xmax++;
        //cout << "Scaling: " << width()/nofPoints << " " << height()/(max-min) << endl;
        painter.scale(width()/(curChan->xmax-curChan->xmin),height()/(max-min));
        painter.translate(0,min);

        painter.drawPolyline(poly);
        //std::cout << "Drew " << std::dec << poly.size() << " points" << std::endl;

        painter.restore ();
    }
}

void plot2d::drawTicks(QPainter &painter)
{
    int ch = curTickCh;

    int i=0, value=0;
    int incx=0, incy=0;

    int chxmin = channels->at(ch)->xmin;
    int chxmax = channels->at(ch)->xmax;
    int chymin = channels->at(ch)->ymin;
    int chymax = channels->at(ch)->ymax;

    int xmin = (chxmax - chxmin) * viewport.left ();
    int xmax = (chxmax - chxmin) * viewport.right ();
    int ymin = (chymax - chymin) * (1 - viewport.bottom ());
    int ymax = (chymax - chymin) * (1 - viewport.top ());

    // Draw tickmarks around the border

    // Range chooser
    while(incx<50 && incx<(xmax-xmin)/10)
    {
        incx+=10;
    }
    while(incx<(xmax-xmin)/10)
    {
        incx+=50;
    }
    while(incy<10 && incy<(ymax-ymin)/5)
    {
        incy+=1;
    }
    while(incy<50 && incy<(ymax-ymin)/5)
    {
        incy+=10;
    }
    while(incy<(ymax-ymin)/5)
    {
        incy+=50;
    }

    if(incx == 0) incx = 1;
    if(incy == 0) incy = 1;

    painter.save ();
    painter.setPen(QPen(channels->at(ch)->getColor()));

    // x Ticks
    value=xmin;

    int xtickInc = 0;
    if(xmax-xmin <= 1)
    {
        xtickInc = width();
    }
    else
    {
        xtickInc = (incx*width()/(xmax-xmin));
    }

    for(i=0; i<width(); i+=xtickInc)
    {
        //std::cout << "Drawing x tick " << i << std::endl;
        QLine line1(i,0,i,height()*0.01);
        QLine line2(i,height(),i,height()*0.99);
        painter.drawLine(line1);
        painter.drawLine(line2);
        painter.drawText(i+2,10,tr("%1").arg(value,5,10));
        value+=incx;
    }

    // y Ticks
    value=ymin;

    int ytickInc = 0;
    if(ymax-ymin <= 1)
    {
        ytickInc = height();
    }
    else
    {
        ytickInc = (incy*height()/(ymax-ymin));
    }

    if(ymax-ymin < 0.000001) ymax += 1;
    for(i=height(); i>0; i-=ytickInc)
    {
        //std::cout << "Drawing y tick " << i << std::endl;
        QLine line1(0,i,width()*0.01,i);
        QLine line2(width(),i,width()*0.99,i);
        painter.drawLine(line1);
        painter.drawLine(line2);
        painter.drawText(width()-40,i-6,tr("%1").arg(value,5,10));
        value+=incy;
    }

    painter.restore ();
}

void plot2d::createActions()
{
    clearHistogramAction = new QAction(tr("Clear Histogram"), this);
    connect(clearHistogramAction, SIGNAL(triggered()), this, SLOT(clearHistogram()));
    saveChannelAction = new QAction(tr("Save channel..."), this);
    connect(saveChannelAction, SIGNAL(triggered()), this, SLOT(saveChannel()));
}

void plot2d::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(this->clearHistogramAction);
    menu.addAction(this->saveChannelAction);
    QMenu* sub = menu.addMenu("Axes for");
    sub->addActions(setCurTickChActions);
    QAction* curAct = menu.exec(event->globalPos());
    foreach(QAction* act, setCurTickChActions)
    {
        if(curAct == act)
        {
            selectCurTickCh(act->data().value<int>());
        }
    }
}

void plot2d::clearHistogram()
{
    for(unsigned int i = 0; i < this->getNofChannels(); i++)
    {
        emit this->histogramCleared(channels->at(i)->getId(),this->id);
    }
}

void plot2d::setMaximumExtends(int _xmin, int _xmax, int _ymin, int _ymax)
{
    ext_xmin = _xmin;
    ext_xmax = _xmax;
    ext_ymin = _ymin;
    ext_ymax = _ymax;
}

void plot2d::toggleExternalBoundaries(bool newValue)
{
    useExternalBoundaries = newValue;
}

void plot2d::zoomExtends(bool newValue)
{
    zoomExtendsTrue = newValue;
}

void plot2d::selectCurTickCh(int _curTickCh)
{
    curTickCh = _curTickCh;
}

void plot2d::resetBoundaries(int ch)
{
    channels->at(ch)->xmin = 0;
    channels->at(ch)->xmax = 1;
    channels->at(ch)->ymin = 0;
    channels->at(ch)->ymax = 1;
}

void plot2d::saveChannel()
{
    QString fileName = QFileDialog::getSaveFileName(this,"Save channel as...","","Data files (*.dat)");
    if (fileName.isEmpty())
        return;
    vector<double> data = channels->first()->getData();
    dsp->vectorToFile(data,fileName.toStdString());
}
