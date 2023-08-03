#include "session.h"

QJsonObject Session::toJson()
{
    QJsonObject sessionJson;

    sessionJson["sessionId"] = QString::number(id);
    sessionJson["satId"] = ka->id;

    QJsonObject jResult;
    QJsonObject jTime;
    jTime["from"] =  QString::number(factSessionTimeStart.toSecsSinceEpoch());
    jTime["to"] =  QString::number(factSessionTimeEnd.toSecsSinceEpoch());
    jResult["time"] = jTime;
    jResult["duration"] = QString::number(factSessionTimeStart.secsTo(factSessionTimeEnd));
    if(cState == PlannedFixed || cState == PlannedRange)
        jResult["answers"] = QJsonValue();
    else
        jResult["answers"] = answers;

    QJsonObject jVisibility;
    jVisibility["from"] = QString::number(visibilityStart.toSecsSinceEpoch());
    jVisibility["to"] =  QString::number(visibilityEnd.toSecsSinceEpoch());

    sessionJson["visibility"] = jVisibility;

    sessionJson["state"] = getCompletionStateString(cState);

    sessionJson["result"] = jResult;
    return sessionJson;
}

Session::Session(QJsonObject sessionJson)
{
    //QJsonDocument jsonDoc = QJsonDocument(userJson);
//    login = userJson["login"].toString();
//    passwordHash = userJson["passwordHash"].toString();
//    isAdmin = userJson["isAdmin"].toBool();
//    canPlan = userJson["canPlan"].toBool();
}

Session::Session()
{
    answers = 0;
}
