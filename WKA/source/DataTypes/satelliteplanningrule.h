#ifndef SATELLITEPLANNINGRULE_H
#define SATELLITEPLANNINGRULE_H

#include <QJsonObject>
#include "common.h"
enum SatellitePlanningPriority
{
    DoNotTrack,
    Low,
    Mid,
    High
};
QString getSatellitePlanningPriorityString(SatellitePlanningPriority priority);
SatellitePlanningPriority getSatellitePlanningPriorityVal(QString satellitePlanningPriorityString, bool *ok);

struct  SatellitePlanningRule
{
    SatellitePlanningPriority priority = SatellitePlanningPriority(getRandomInt(3));
    int pointsAmount = getRandomInt(1, 5);
    int minSessions = getRandomInt(1, 3);
    int maxSessions = getRandomInt(minSessions, 5);
    int minSessionInterval = getRandomInt(0, 12) * 5;

public:
    QJsonObject toJson();
};

#endif // SATELLITEPLANNINGRULE_H
