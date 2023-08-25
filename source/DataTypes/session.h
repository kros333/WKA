#ifndef SESSION_H
#define SESSION_H

#include <QJsonObject>
#include "common.h"
#include "satellite.h"
#include <QDateTime>
#include "satelliteplanningrule.h"

/// Сеанс
struct Session
{
    Session(QJsonObject sessionJson);
    Session();

    QJsonObject toJson();

    quint64 id;
    Satellite* ka;
    QDateTime visibilityStart;
    QDateTime visibilityEnd;
    QDateTime factSessionTimeStart;
    QDateTime factSessionTimeEnd;
    CompletionState cState;
    int answers;
    SatellitePlanningRule currentPlanningRule;
    int WindowId;
};

#endif // SESSION_H
