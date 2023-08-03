#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include "DataTypes/session.h"
#include "DataTypes/common.h"
#include "DataTypes/satelliteplanningrule.h"
#include "weatherwidget.h"

class DataManager : public QObject
{
   Q_OBJECT
public:
    DataManager(quint16 daysToLeft, quint16 daysToRight);
    //virtual ~DataManager();
    const QVector<Session> getSessions(QDateTime targetTime, int limitBack, int limitForward, SessionsTableFilterOptions filter);
    int getSessionsCount();
    const QVector<Satellite *> getSatelliteDictionary();
    const Satellite *getSatelliteName(int satId);
    QMap<QDate /*day*/, CompletedSessionsBySatId> getSessionsGrid(QDate targetDate, int limitBackInDays, int limitForwardInDays);
    QMap<int, QList<VisibilityWindowSummary> > getSessionsRibbon(QDateTime targetDt, int limitBackInHours, int limitForwardInHours);
    const QMap<Satellite*, SatellitePlanningRule> getSatellitePlanningRules();
    bool updateSatellitePlanningRule(int satId, QString newPriority, int newPointsAmount, int newMinSessions, int newMaxSessions, int newMinSessionInterval);
    const QVector<TimeSpan> getSatelliteWindows(int satId);
    bool writeNewSatelliteUserPlannedSession(int satId, QDateTime start, QDateTime end, bool isApproximateTime, int durationInSeconds, qint64 editSessionId);
    const QVector<Session> getUserPlannedSessions(QDateTime targetTime, int limitForward);
    bool deleteSatelliteUserPlannedSession(qint64 deleteSessionId);
    const QVector<QVector<QString> > getTechLogTableRows(QDateTime targetTime, int limitBack, int limitForward, QString tableType);
    bool setManualMode(bool mode) { isManualMode = mode; return true; }
    bool getManualMode() { return isManualMode; }
    const QMap<int, ManualModeRecord *> getManualModeTableInfo();
    QPair<bool, QString> startManualSession(int satId);
    QPair<bool, QString> stopManualSession();
    int getCurManualSessionSatId() { return curManualSessionSatId;}
    bool getCurManualSessionIsRunning() { return manualSessionIsRunning;}
    QString getCurManualSessionInfoText();

    WeatherWidget weatherWidget;

private slots:
    void oneHzTimerElapsed();
private:
    QVector<Session> sessions;
    QVector<Session> userPlannedSessions;
    QVector<QPair<int /*satId*/, VisibilityWindowSummary>> windows;
    QVector<Satellite*> satellites;
    QMap<Satellite*, SatellitePlanningRule> satellitePlanningSettings;
    quint64 globalSessionLastUsedId = 0;
    int globalCurrentSessionArrayPos = 0;
    QTimer oneHzTimer;
    Satellite* getSatellite(int satId);
    QVector<QVector<QString>> journal;
    QVector<QVector<QString>> tests;
    QVector<QVector<QString>> fnks;
    QVector<QVector<QString>> meteos;
    bool isManualMode = false;
    QMap<int /*satId*/, ManualModeRecord*> manualModeInfos;     //to do remake to Qmap with satId as key (and as template?)
    QDateTime curManualSessionStart;
    int curManualSessionPoints = 0;
    int curManualSessionSatId = -1;
    int manualSessionIsRunning = false;


    const static int rows = 20;
    const static int cols = 20;
    bool matrix[rows][cols];
    bool new_matrix[rows][cols];
    int cloud_count = 0;
    int ratio = 1;
    int rain_duration = 0;
    int wind = rand() % 4;
};

#endif // DATAMANAGER_H
