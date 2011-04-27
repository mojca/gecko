#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include <QObject>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QStringList>

/*! The SystemInfo class provides information about the current state of the operating system.
 *  Its attributes are updated in regular intervals. Currently, it provides the current CPU
 *  usage and the number of cores available.
 */
class SystemInfo : public QObject
{
    Q_OBJECT

public:
    SystemInfo() : cpuLoad(0),netLoad(0),lastCpuLoad(0),lastNetLoad(0),nofCores(QThread::idealThreadCount()) {
        if(nofCores == -1) nofCores = 1;
        //std::cout << "Detected " << nofCores << " CPUs in this system" << std::endl;
        oneSecondTimer = new QTimer(this);
        oneSecondTimer->start(1000);
        connect(oneSecondTimer, SIGNAL(timeout()), this, SLOT(oneSecondTimerTimeout()));
    }
    ~SystemInfo() { oneSecondTimer->stop(); }

    int getNofCores() const { return nofCores; }
    double getCpuLoad() const { return cpuLoad-lastCpuLoad; }

private:
    void parseProcStat() {
        QFile file("/proc/stat");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
        QTextStream in(&file);
        QString line = in.readLine();
        bool stop = false;
        while (!line.isNull() && stop == false) {
            if(line.startsWith("cpu  ")) {
                QStringList parts = line.split(QChar(' '));
                // Order is: 1 user, 2 system, 3 nice, 4 idle (1/100 seconds)
                double user   = parts.at(2).toDouble();
                double system = parts.at(3).toDouble();
                double nice   = parts.at(4).toDouble();
                //double idle   = parts.at(5).toDouble();
                lastCpuLoad = cpuLoad;
                cpuLoad = (user+system+nice)/100./nofCores;
                //std::cout << "user: " << std::dec << user << " sys: " << system << " nice: " << nice << std::endl;
                //std::cout << "Current CPU load: " << std::dec << cpuLoad-lastCpuLoad << std::endl;
                stop = true;
            }

            line = in.readLine();
        }
    }

    void parseProcNetDev() {
        // TODO
    }

public slots:
    void oneSecondTimerTimeout() {
        parseProcStat();
        parseProcNetDev();
    }

private:
    QTimer* oneSecondTimer;

    double cpuLoad;
    double netLoad;
    double lastCpuLoad;
    double lastNetLoad;
    int nofCores;
};

#endif // SYSTEMINFO_H
