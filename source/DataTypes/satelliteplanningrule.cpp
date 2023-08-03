#include "satelliteplanningrule.h"

QString getSatellitePlanningPriorityString(SatellitePlanningPriority priority)
{
    switch (priority) {
    case DoNotTrack:
        return "SATPRIORITY.DONOTTRACK";
    case Low:
        return "SATPRIORITY.LOW";
    case Mid:
        return "SATPRIORITY.MID";
    case High:
        return "SATPRIORITY.HIGH";

    }
}

SatellitePlanningPriority getSatellitePlanningPriorityVal(QString satellitePlanningPriorityString, bool *ok)
{
*ok = true;
 if(satellitePlanningPriorityString == "SATPRIORITY.DONOTTRACK")
     return DoNotTrack;
 if(satellitePlanningPriorityString == "SATPRIORITY.LOW")
     return Low;
 if(satellitePlanningPriorityString == "SATPRIORITY.MID")
     return Mid;
 if(satellitePlanningPriorityString == "SATPRIORITY.HIGH")
     return High;

 *ok = false;
 return DoNotTrack; //:C
}


QJsonObject SatellitePlanningRule::toJson()
{
    QJsonObject satellitePlanningRuleJson;
    satellitePlanningRuleJson["priority"] = getSatellitePlanningPriorityString(priority);
    satellitePlanningRuleJson["pointsAmount"] = pointsAmount;
    satellitePlanningRuleJson["minSessions"] = minSessions;
    satellitePlanningRuleJson["maxSessions"] = maxSessions;
    satellitePlanningRuleJson["minSessionInterval"] = minSessionInterval;

    return satellitePlanningRuleJson;
}
