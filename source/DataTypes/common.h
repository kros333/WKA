#ifndef COMMON_H
#define COMMON_H

#include <QDateTime>
#include <QMap>
#include <QString>
//#include "satelliteplanningrule.h"
//#include "satellite.h"

int getRandomInt(int top);
int getRandomInt(int bot, int top);

enum Weather
{
    Clear,
    Cloudy,
    Rain
};
QString getWeatherString(Weather weatherVal);

enum MachineState
{
    NotAvailible,
    Availible,
    Selected,
    IsCloudy
};
QString getMachineStateString(MachineState machineStateVal);

enum CompletionState
{
    PlannedFixed,
    PlannedRange,
    InProgress,
    Done,
    FailedWeather,
    FailedTech,
    FailedUnknown
};
QString getCompletionStateString(CompletionState completionStateVal);
QString getCompletionStateDisplayString(CompletionState completionStateVal);
CompletionState getCompletionStateVal(QString completionStateString, bool *ok);

struct SessionsTableFilterOptions
{
    QList<int> selectedSatelliteIds;
    QDateTime sessionFrom;
    QDateTime sessionTo;
    QDateTime visibilityFrom;
    QDateTime visibilityTo;
    QList<CompletionState> selectedCStates;

};

struct SessionsDoneOutOfTotal
{
    int sessionsDone = 0;
    int sessionsTotal = 0;
    bool colored = true;
};
typedef QMap<int, SessionsDoneOutOfTotal> CompletedSessionsBySatId;



struct TimeSpan
{
    TimeSpan() {}
    TimeSpan(QDateTime from, QDateTime to) { start = from; end = to;}
    QDateTime start;
    QDateTime end;
};
struct VisibilityWindowSummary
{
    struct WeatherSpan {
        WeatherSpan() {}
        WeatherSpan(Weather weather, TimeSpan span) {
            this->span = span;
            this->weather = weather;
        }
        Weather weather;
        TimeSpan span;
    };

    TimeSpan startEnd;
//    QList<TimeSpan> clear;
//    QList<TimeSpan> cloudy;
//    QList<TimeSpan> rainy;
    QList<WeatherSpan> weathers;
    QList<QPair<TimeSpan, QPair<int, CompletionState>>> sessions;
    int kaId;
};

//typedef QMap<Satellite, SatellitePlanningRule> SatellitePlanningSettings;

struct ManualModeRecord
{
    //int satId;
    TimeSpan visibility;
    Weather weatherVal;
    QString infoText;
    int sessionsThisWindow = 0;
};

struct CurrentError
{
    int errorId;
    int count;
    bool isAppendInJournal;
};

#endif // COMMON_H
