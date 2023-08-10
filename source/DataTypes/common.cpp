#include "common.h"


QString getWeatherString(Weather weatherVal)
{
    switch (weatherVal) {
    case Clear:
        return "WEATHER.SUNNY";
    case Cloudy:
        return "WEATHER.CLOUDY";
    case Rain:
        return "WEATHER.RAINY";
    }
}

QString getMachineStateString(MachineState machineStateVal)
{
    switch (machineStateVal) {
    case NotAvailible:
        return "MACHINE_STATE.NOT_AVAILIBLE";
    case Availible:
        return "MACHINE_STATE.AVAILIBLE";
    case Selected:
        return "MACHINE_STATE.SELECTED";
    case IsCloudy:
        return "MACHINE_STATE.CLOUDY";
    }
}

QString getCompletionStateString(CompletionState completionStateVal)
{
    switch (completionStateVal) {
    case PlannedFixed:
        return "CS.PLANNEDFIXED";
    case PlannedRange:
        return "CS.PLANNEDRANGE";
    case InProgress:
        return "CS.INPROGRESS";
    case Done:
        return "CS.DONE";
    case FailedWeather:
        return "CS.FAILEDWEATHER";
    case FailedTech:
        return "CS.FAILEDTECH";
    case FailedUnknown:
        return "CS.FAILEDUNKNOWN";
    }
}

QString getCompletionStateDisplayString(CompletionState completionStateVal)
{
    switch (completionStateVal) {
    case PlannedFixed:
        return "Запланирован";
    case PlannedRange:
        return "Запланирован (в промежутке)";
    case InProgress:
        return "Выполняется";
    case Done:
        return "Выполнен";
    case FailedWeather:
        return "Не выполнен (погода)";
    case FailedTech:
        return "Не выполнен (тех.)";
    case FailedUnknown:
        return "Не выполнен (неизв.)";
    }
}

CompletionState getCompletionStateVal(QString completionStateString, bool* ok)
{
   *ok = true;
    if(completionStateString == "CS.PLANNEDFIXED")
        return PlannedFixed;
    if(completionStateString == "CS.PLANNEDRANGE")
        return PlannedRange;
    if(completionStateString == "CS.INPROGRESS")
        return InProgress;
    if(completionStateString == "CS.DONE")
        return Done;
    if(completionStateString == "CS.FAILEDWEATHER")
        return FailedWeather;
    if(completionStateString == "CS.FAILEDTECH")
        return FailedTech;
    if(completionStateString == "CS.FAILEDUNKNOWN")
        return FailedUnknown;

    *ok = false;
    return FailedUnknown; //:C
}

int getRandomInt(int bot, int top)
{
    return qrand() % ((top + 1) - bot) + bot;
}

int getRandomInt(int top)
{
    return getRandomInt(0,top);
}
