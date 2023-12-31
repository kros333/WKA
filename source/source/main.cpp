#include <QCommandLineParser>
#include <QApplication >
#include <QDebug>
#include "kostcpserver.h"

#define DEFAULT_PORT 55555
#define DAYS_TO_LEFT 20
#define DAYS_TO_RIGHT 5

int main(int argc, char *argv[])
{
    QApplication  a(argc, argv);
    QApplication ::setApplicationName("Web Kos Api Server Test Version");
    QApplication ::setApplicationVersion("0.4");

    QCommandLineParser parser;
    parser.setApplicationDescription("Web kos api test");
    parser.addHelpOption();
    parser.addVersionOption();

    //port option
    QCommandLineOption portOption(QStringList() << "p" << "port",
                 "Sets TCP server port.", "port", QString::number(DEFAULT_PORT));
    parser.addOption(portOption);

    // days to left option
    QCommandLineOption daysLeftOption(QStringList() << "l" << "daysleft",
                 "How many days days to generate to the left (past days)", "daysleft", QString::number(DAYS_TO_LEFT));
    parser.addOption(daysLeftOption);

    // days to right option
    QCommandLineOption daysRightOption(QStringList() << "r" << "daysright",
                 "How many days days to generate to the right (future days)", "daysright", QString::number(DAYS_TO_RIGHT));
    parser.addOption(daysRightOption);

    parser.process(a);

    quint16 servPort = parser.value(portOption).toUShort();
    quint16 daysToLeft = parser.value(daysLeftOption).toUShort();
    quint16 daysToRight = parser.value(daysRightOption).toUShort();
    KosTcpServer server(servPort, daysToLeft, daysToRight);

    return a.exec();
}
