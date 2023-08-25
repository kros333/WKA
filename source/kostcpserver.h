#ifndef KOSTCPSERVER_H
#define KOSTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonArray>
#include <QByteArray>
#include <datamanager.h>

class KosTcpServer : public QTcpServer
{
public:
    KosTcpServer(quint16 port, quint16 daysToLeft, quint16 daysToRight, int weatherUpdFreq);
    ~KosTcpServer();
    DataManager* dm = nullptr;

private slots:
    void newClient();

    void sockDisconnect();
    void sockReady();
private:
    QMap<int,QTcpSocket *> clients;
    void sendSessionsTableRows(const QJsonObject &jObj, QTcpSocket* socket);
    void sendSessionsGrid(const QJsonObject &jObj, QTcpSocket* socket);
    void sendSessionsRibbons(const QJsonObject &jObj, QTcpSocket* socket);
    void sendSatellitePlanningRules(const QJsonObject &jObj, QTcpSocket* socket);
    QList<QByteArray> splitJsons(const QByteArray &input, bool &ok);
    void mapJsonRequest(const QJsonObject &jObj, QTcpSocket *socket);
    void sendSatelliteDictionary(const QJsonObject &jObj, QTcpSocket *socket);
    void sendSatelliteName(const QJsonObject &jObj, QTcpSocket *socket);
    void sendSatelliteWindows(const QJsonObject &jObj, QTcpSocket *socket);
    void sendSatelliteUserPlannedSessions(const QJsonObject &jObj, QTcpSocket *socket);
    void writeBytes(const QByteArray &bytes, const QList<QTcpSocket *> &socks);
    void writeBytes(const QByteArray &bytes, QTcpSocket *socket);
    void writeSatellitePlanningRule(const QJsonObject &jObj, QTcpSocket *socket);
    void writeSatelliteUserPlannedSession(const QJsonObject &jObj, QTcpSocket *socket);
    void deleteSatelliteUserPlannedSession(const QJsonObject &jObj, QTcpSocket *socket);
    void sendTechLogTabs(const QJsonObject &jObj, QTcpSocket *socket);
    void sendTechLogTableRows(const QJsonObject &jObj, QTcpSocket* socket);
    void sendCompletionStateDictionary(const QJsonObject &jObj, QTcpSocket *socket);
    void sendKosState(const QJsonObject &jObj, QTcpSocket *socket);
    void sendManualModeTable(const QJsonObject &jObj, QTcpSocket *socket);
    void sendManualModeState(const QJsonObject &jObj, QTcpSocket *socket);
    void writeManualModeState(const QJsonObject &jObj, QTcpSocket *socket);
    void sendClosestSatelliteVisibility(const QJsonObject &jObj, QTcpSocket *socket);
    void sendManualModeCurrentInfo(const QJsonObject &jObj, QTcpSocket *socket);
    void startManualSession(const QJsonObject &jObj, QTcpSocket *socket);
    void stopManualSession(const QJsonObject &jObj, QTcpSocket *socket);
};

#endif // KOSTCPSERVER_H
