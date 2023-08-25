#include "kostcpserver.h"

KosTcpServer::KosTcpServer(quint16 port, quint16 daysToLeft, quint16 daysToRight, int weatherUpdFreq)
{

    if(!this->listen(QHostAddress::AnyIPv4, port))
        qWarning() <<  QString("Failed to open server on port %1, %2").arg(port).arg(this->errorString());


    qDebug() << QString("Server launched on port %1").arg(port);
    connect(this, &QTcpServer::newConnection, this, &KosTcpServer::newClient);


    dm = new DataManager(daysToLeft, daysToRight, weatherUpdFreq);
}

KosTcpServer::~KosTcpServer()
{
    dm->deleteLater();
}

void KosTcpServer::newClient()
{
    QTcpSocket* clientSocket=this->nextPendingConnection();
    int clId=clientSocket->socketDescriptor();
    clients[clId]=clientSocket;
    qDebug() << QString("New client, id= %1").arg(QString::number(clId));
    connect(clientSocket, &QAbstractSocket::readyRead, this, &KosTcpServer::sockReady);
    connect(clientSocket, &QAbstractSocket::disconnected, this, &KosTcpServer::sockDisconnect);
    //writeBytes("{\"type\":\"connect\",\"status\":\"yes\"}", clientSocket);
}

//прерывание соединения
void KosTcpServer::sockDisconnect()
{
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    qDebug() << QString("Client disconnected, id= %1").arg(QString::number(clients.key(clientSocket)));
    clients.remove(clients.key(clientSocket));
    clientSocket->deleteLater();

}

void KosTcpServer::sockReady()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    QList<QByteArray> splitData;
    QByteArray fullData;

    bool ok;
    do {
        fullData.append(socket->readAll());
        splitData = splitJsons(fullData, ok);
    } while (socket->waitForReadyRead(ok ? 10 : 500));

    foreach (QByteArray data, splitData)
    {
        QJsonParseError jsonErr;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &jsonErr);
        qWarning() << "recieved JSON: " << jsonDoc;
        if(jsonErr.error != QJsonParseError::NoError)
        {
            qWarning() << QString("Json parsing error: %1").arg(jsonErr.errorString());
            continue;
        }
        mapJsonRequest(jsonDoc.object(), socket);

    }
}

void KosTcpServer::mapJsonRequest(const QJsonObject &jObj, QTcpSocket* socket)
{
    QString type = jObj["type"].toString();
    if (type == "getSatelliteDictionary") // api #2
    {
        sendSatelliteDictionary(jObj, socket);
    }
    if (type == "getSatelliteName") // api #3
    {
        sendSatelliteName(jObj, socket);
    }
    if (type == "getSessionsTableRows") // api #4
    {
        sendSessionsTableRows(jObj, socket);
    }
    if (type == "getSessionsGrid") // api #5
    {
        sendSessionsGrid(jObj, socket);
    }
    if (type == "getSessionsRibbons") // api #6
    {
        sendSessionsRibbons(jObj, socket);
    }
    if (type == "getSatellitePlanningRules") // api #7
    {
        sendSatellitePlanningRules(jObj, socket);
    }
    if (type == "writeSatellitePlanningRule") // api #8
    {
        writeSatellitePlanningRule(jObj, socket);
    }
    if (type == "getSatelliteWindows") // api #9
    {
        sendSatelliteWindows(jObj, socket);
    }
    if (type == "writeSatelliteUserPlannedSession") // api #10
    {
        writeSatelliteUserPlannedSession(jObj, socket);
    }
    if (type == "getSatelliteUserPlannedSessions") // api #11
    {
        sendSatelliteUserPlannedSessions(jObj, socket);
    }
    if (type == "deleteSatelliteUserPlannedSession") // api #12
    {
        deleteSatelliteUserPlannedSession(jObj, socket);
    }
    if (type == "getTechLogTabs") // api #13
    {
        sendTechLogTabs(jObj, socket);
    }
    if (type == "getTechLogTableRows") // api #14
    {
        sendTechLogTableRows(jObj, socket);
    }
    if (type == "getCompletionStateDictionary") // api #15
    {
        sendCompletionStateDictionary(jObj, socket);
    }
    if (type == "getKosState") // api #16
    {
        sendKosState(jObj, socket);
    }
    if (type == "getManualModeState") // api #17
    {
        sendManualModeState(jObj, socket);
    }
    if (type == "setManualModeState") // api #18
    {
        writeManualModeState(jObj, socket);
    }
    if (type == "getManualModeTable") // api #19
    {
        sendManualModeTable(jObj, socket);
    }
    if (type == "getManualModeCurrentInfo") // api #20
    {
        sendManualModeCurrentInfo(jObj, socket);
    }
    if (type == "getSatelliteClosestVisibility") // api #21
    {
        sendClosestSatelliteVisibility(jObj, socket);
    }
    if (type == "startManualSession") // api #22
    {
        startManualSession(jObj, socket);
    }
    if (type == "stopManualSession") // api #23
    {
        stopManualSession(jObj, socket);
    }
}

void KosTcpServer::sendSessionsTableRows(const QJsonObject &jObj, QTcpSocket* socket)
{
    //time
    QDateTime targetDt = QDateTime::fromSecsSinceEpoch(jObj["targetTime"].toString().toLongLong());
    int limitBack = jObj["limitBack"].toInt();
    int limitForward = jObj["limitForward"].toInt();

    //filters
    SessionsTableFilterOptions filter;
    auto jFilter = jObj["filter"].toObject();
    if(!jFilter.isEmpty())
    {
        auto jSelectedSatelliteIds = jFilter["satIds"].toArray();
        if(!jSelectedSatelliteIds.isEmpty())
        {
            foreach(auto jVal, jSelectedSatelliteIds)
                filter.selectedSatelliteIds.append(jVal.toInt());
        }

        auto jSessionTime = jFilter["time"].toObject();
        if(!jSessionTime.isEmpty())
        {
            auto jSessionFrom = jSessionTime["from"];
            if(!jSessionFrom.isNull())
                filter.sessionFrom = QDateTime::fromSecsSinceEpoch(jSessionFrom.toString().toLongLong());
            auto jSessionTo = jSessionTime["to"];
            if(!jSessionTo.isNull())
                filter.sessionTo = QDateTime::fromSecsSinceEpoch(jSessionTo.toString().toLongLong());
        }

        auto jSessionVisibility = jFilter["visibility"].toObject();
        if(!jSessionVisibility.isEmpty())
        {
            auto jSessionFrom = jSessionVisibility["from"];
            if(!jSessionFrom.isNull())
                filter.visibilityFrom = QDateTime::fromSecsSinceEpoch(jSessionFrom.toString().toLongLong());
            auto jSessionTo = jSessionVisibility["to"];
            if(!jSessionTo.isNull())
                filter.visibilityTo = QDateTime::fromSecsSinceEpoch(jSessionTo.toString().toLongLong());
        }

        auto jSelectedStates = jFilter["states"].toArray();
        if(!jSelectedStates.isEmpty())
        {
            foreach(auto jVal, jSelectedStates)
            {
                bool ok;
                auto cState = getCompletionStateVal(jVal.toString(), &ok);
                if(ok)
                    filter.selectedCStates.append(cState);
            }
        }

    }

    auto recentSessions = dm->getSessions(targetDt, limitBack, limitForward, filter);

    QJsonObject jResponse;
    jResponse["type"] = "sessionsTableRows";
    QJsonArray jSessionPage;
    for(int i = 0; i < recentSessions.length(); i++)
        jSessionPage << recentSessions[i].toJson();
    jResponse["sessionsTable"] = jSessionPage;

    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);

}

void KosTcpServer::sendSessionsGrid(const QJsonObject &jObj, QTcpSocket* socket)
{
    QDateTime targetDt = QDateTime::fromSecsSinceEpoch(jObj["targetTime"].toString().toLongLong()); // TODO local time shenaningans
    int limitBack = jObj["previousDaysAmount"].toInt();
    int limitForward = jObj["futureDaysAmount"].toInt();
    auto foundSessionSummaries = dm->getSessionsGrid(targetDt.date(),  limitBack,  limitForward);

    QJsonObject jResponse;
    jResponse["type"] = "sessionsGrid";

    QJsonArray jDays;
    foreach(auto day, foundSessionSummaries.keys())
    {
        QJsonObject jDaySummary;
        jDaySummary["date"] = QString::number(QDateTime(day,QTime(0,0)).toSecsSinceEpoch());
        QJsonArray jSessionsOfDay;
        foreach(auto satId, foundSessionSummaries[day].keys())
        {
            QJsonObject jSummaryByKa;
            jSummaryByKa["satId"] = satId;
            jSummaryByKa["sessionsDone"] = foundSessionSummaries[day][satId].sessionsDone;
            jSummaryByKa["sessionsTotal"] = foundSessionSummaries[day][satId].sessionsTotal;
            jSummaryByKa["colored"] = foundSessionSummaries[day][satId].colored;
            jSessionsOfDay << jSummaryByKa;
        }
        jDaySummary["sessionsBySatId"] = jSessionsOfDay;
        jDays << jDaySummary;
    }
    jResponse["daysGrid"] = jDays;

    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);

}

void KosTcpServer::sendSessionsRibbons(const QJsonObject &jObj, QTcpSocket* socket)
{
    QDateTime targetDt = QDateTime::fromSecsSinceEpoch(jObj["targetTime"].toString().toLongLong()); // TODO local time shenaningans
    int limitBack = jObj["previousHoursAmount"].toInt();
    int limitForward = jObj["futureHoursAmount"].toInt();
    auto visibilityRibbons = dm->getSessionsRibbon(targetDt,  limitBack,  limitForward);

    QJsonObject jResponse;
    jResponse["type"] = "sessionsRibbons";

    QJsonArray jSatellites;
    foreach(auto satId, visibilityRibbons.keys())
    {
        QJsonObject jSatelliteSummary;
        jSatelliteSummary["satId"] = satId;
        QJsonArray jVisibilitiesOfSatellite;
        foreach(auto summary, visibilityRibbons[satId])
        {
            QJsonObject jSingleVisibility;
            jSingleVisibility["visibilityFrom"] = QString::number(summary.startEnd.start.toSecsSinceEpoch());
            jSingleVisibility["visibilityTo"] = QString::number(summary.startEnd.end.toSecsSinceEpoch());

            QJsonArray jSessions;
            foreach(auto sessionInfo, summary.sessions)
            {
                QJsonObject jOneSession;
                jOneSession["sessionFrom"] = QString::number(sessionInfo.first.start.toSecsSinceEpoch());
                jOneSession["sessionTo"] = QString::number(sessionInfo.first.end.toSecsSinceEpoch());
                jOneSession["state"] = getCompletionStateString(sessionInfo.second.second);
                jSessions << jOneSession;
            }
            jSingleVisibility["sessions"] = jSessions;

            //            QJsonArray jRains;
            //            foreach(auto rain, summary.rainy)
            //            {
            //                QJsonObject jOneRain;
            //                jOneRain["from"] = QString::number(rain.start.toSecsSinceEpoch());
            //                jOneRain["to"] = QString::number(rain.end.toSecsSinceEpoch());
            //                jRains << jOneRain;
            //            }
            //            jSingleVisibility["rainy"] = jRains;

            //            QJsonArray jClouds;
            //            foreach(auto cloud, summary.cloudy)
            //            {
            //                QJsonObject jOneCloud;
            //                jOneCloud["from"] = QString::number(cloud.start.toSecsSinceEpoch());
            //                jOneCloud["to"] = QString::number(cloud.end.toSecsSinceEpoch());
            //                jClouds << jOneCloud;
            //            }
            //            jSingleVisibility["cloudy"] = jClouds;

            //            QJsonArray jSunnies;
            //            foreach(auto sun, summary.clear)
            //            {
            //                QJsonObject jOneSun;
            //                jOneSun["from"] = QString::number(sun.start.toSecsSinceEpoch());
            //                jOneSun["to"] = QString::number(sun.end.toSecsSinceEpoch());
            //                jSunnies << jOneSun;
            //            }
            //            jSingleVisibility["sunny"] = jSunnies;

            QJsonArray jRains;
            QJsonArray jClouds;
            QJsonArray jSunnies;
            foreach(const auto &weather, summary.weathers)
            {
                QJsonObject jOneWeather;
                jOneWeather["from"] = QString::number(weather.span.start.toSecsSinceEpoch());
                jOneWeather["to"] = QString::number(weather.span.end.toSecsSinceEpoch());
                switch (weather.weather) {
                case Weather::Cloudy:
                    jClouds << jOneWeather;
                    break;
                case Weather::Rain:
                    jRains << jOneWeather;
                    break;
                case Weather::Clear:
                    jSunnies << jOneWeather;
                    break;
                }
            }
            jSingleVisibility["rainy"] = jRains;
            jSingleVisibility["cloudy"] = jClouds;
            jSingleVisibility["sunny"] = jSunnies;

            jVisibilitiesOfSatellite << jSingleVisibility;
        }
        jSatelliteSummary["visibilities"] = jVisibilitiesOfSatellite;
        jSatellites << jSatelliteSummary;
    }
    jResponse["visibilityRibbons"] = jSatellites;

    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);

}

void KosTcpServer::sendSatellitePlanningRules(const QJsonObject &jObj, QTcpSocket* socket)
{
    auto satRules = dm->getSatellitePlanningRules();

    QJsonObject jResponse;
    jResponse["type"] = "satellitePlanningRules";

    QJsonArray jSatRules;
    auto satellites = dm->getSatelliteDictionary();
    for(int i = 0; i < satellites.length(); i++)
    {
        auto jRule = satRules[satellites[i]].toJson();
        jRule["satId"] = satellites[i]->id;
        jSatRules << jRule;
    }
    jResponse["rules"] = jSatRules;

    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);

}

void KosTcpServer::writeSatellitePlanningRule(const QJsonObject &jObj, QTcpSocket* socket)
{
    int satId = jObj["satId"].toInt(-1);
    QString newPriority = jObj["priority"].toString("");
    int newPointsAmount = jObj["pointsAmount"].toInt(-1);
    int newMinSessions = jObj["minSessions"].toInt(-1);
    int newMaxSessions = jObj["maxSessions"].toInt(-1);
    int newMinSessionInterval = jObj["minSessionInterval"].toInt(-1);

    bool res = dm->updateSatellitePlanningRule(satId, newPriority, newPointsAmount, newMinSessions, newMaxSessions, newMinSessionInterval);

    QJsonObject jResponse;
    jResponse["type"] = "writeSatellitePlanningRuleResult";
    jResponse["satId"] = satId;
    jResponse["ok"] = res;

    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);

}

void KosTcpServer::sendSatelliteDictionary(const QJsonObject &jObj, QTcpSocket* socket)
{
    auto satDictionary = dm->getSatelliteDictionary();

    QJsonObject jResponse;
    jResponse["type"] = "satelliteDictionary";
    QJsonArray jSatDictionary;
    for(int i = 0; i < satDictionary.length(); i++)
        jSatDictionary << satDictionary[i]->toJson();
    jResponse["dictionary"] = jSatDictionary;

    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);

}

void KosTcpServer::sendSatelliteName(const QJsonObject &jObj, QTcpSocket* socket)
{
    int satId = jObj["satId"].toInt();
    auto foundSatellite = dm->getSatelliteName(satId);

    QJsonObject jResponse;
    jResponse["type"] = "satelliteName";
    if(foundSatellite == nullptr)
    {
        jResponse["satId"] = QJsonValue();
        jResponse["satName"] = QJsonValue();
    }
    else
    {
        jResponse["satId"] = foundSatellite->id;
        jResponse["satName"] = QString(foundSatellite->name);
    }

    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);

}

void KosTcpServer::sendSatelliteWindows(const QJsonObject &jObj, QTcpSocket *socket)
{
    int satId = jObj["satId"].toInt(-1);
    auto satWindows = dm->getSatelliteWindows(satId);

    QJsonObject jResponse;
    jResponse["type"] = "satelliteVisibilityWindows";
    QJsonArray jSatWindows;
    for(int i = 0; i < satWindows.length(); i++)
    {
        QJsonObject jWindow;
        jWindow["from"] = QString::number(satWindows[i].start.toSecsSinceEpoch());
        jWindow["to"] = QString::number(satWindows[i].end.toSecsSinceEpoch());
        jSatWindows << jWindow;
    }
    jResponse["satId"] = satId;
    jResponse["windows"] = jSatWindows;

    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);
}

void KosTcpServer::writeSatelliteUserPlannedSession(const QJsonObject &jObj, QTcpSocket *socket)
{
    int satId = jObj["satId"].toInt(-1);
    bool isApproximateTime = jObj["isApproximateTime"].toBool(false);
    int durationInSeconds = jObj["duration"].toInt(-1);
    QDateTime sessionStart = QDateTime::fromSecsSinceEpoch(jObj["from"].toString().toLongLong()); // TODO local time shenaningans
    QDateTime sessionEnd = QDateTime::fromSecsSinceEpoch(jObj["to"].toString().toLongLong()); // TODO local time shenaningans
    bool ok = false;
    qint64 editSessionId = jObj["editUserPlannedSessionId"].toString("").toLongLong(&ok);
    if(!ok) editSessionId = -1;

    bool res = dm->writeNewSatelliteUserPlannedSession(satId, sessionStart, sessionEnd, isApproximateTime, durationInSeconds, editSessionId);

    QJsonObject jResponse;
    jResponse["type"] = "writeSatelliteUserPlannedSessionResult";
    jResponse["satId"] = satId;
    jResponse["ok"] = res;

    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);
}

void KosTcpServer::deleteSatelliteUserPlannedSession(const QJsonObject &jObj, QTcpSocket *socket)
{
    bool ok = false;
    qint64 editSessionId = jObj["userPlannedSessionId"].toString("").toLongLong(&ok);
    if(!ok) editSessionId = -1;

    if(ok)
        ok = dm->deleteSatelliteUserPlannedSession(editSessionId);

    QJsonObject jResponse;
    jResponse["type"] = "deleteSatelliteUserPlannedSessionResult";
    jResponse["userPlannedSessionId"] = editSessionId == -1 ? QJsonValue() : QString::number(editSessionId);
    jResponse["ok"] = ok;

    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);
}

void KosTcpServer::sendTechLogTabs(const QJsonObject &jObj, QTcpSocket *socket)
{
    QJsonObject jResponse;
    jResponse["type"] = "techLogTabsDictionary";
    QJsonArray jDict;
    //general
    QJsonObject jJournalEntry;
    jJournalEntry["name"] = "journal";
    jJournalEntry["displayName"] = "Журнал";
    QJsonArray jJournalEntryHeaders;
    jJournalEntryHeaders << "ID" << "Время" << "Информация";
    jJournalEntry["headers"] = jJournalEntryHeaders;
    jDict << jJournalEntry;
    //tsd
    QJsonObject jTsdEntry;
    jTsdEntry["name"] = "tsd";
    jTsdEntry["displayName"] = "Результаты тестирования";
    QJsonArray jTsdEntryHeaders;
    jTsdEntryHeaders << "ID" << "Время" << "Наличие плана работ" << "Наличие ЦУ" << "№ смены";
    jTsdEntry["headers"] = jTsdEntryHeaders;
    jDict << jTsdEntry;
    //fnk
    QJsonObject jFnkEntry;
    jFnkEntry["name"] = "fnk";
    jFnkEntry["displayName"] = "Результаты контроля";
    QJsonArray jFnkEntryHeaders;
    jFnkEntryHeaders << "ID" << "Время" << "Работоспособность БИСКОС" << "Работоспособность МВИПИ" << "Работоспособность Опт. каналов";
    jFnkEntry["headers"] = jFnkEntryHeaders;
    jDict << jFnkEntry;
    //meteo
    QJsonObject jMeteoEntry;
    jMeteoEntry["name"] = "meteo";
    jMeteoEntry["displayName"] = "Метео";
    QJsonArray jMeteoEntryHeaders;
    jMeteoEntryHeaders << "ID" << "Время" << "Температура" << "Скорость ветра";
    jMeteoEntry["headers"] = jMeteoEntryHeaders;
    jDict << jMeteoEntry;

    jResponse["tabsDictionary"] = jDict;

    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);
}

void KosTcpServer::sendSatelliteUserPlannedSessions(const QJsonObject &jObj, QTcpSocket *socket)
{
    QDateTime targetDt = QDateTime::fromSecsSinceEpoch(jObj["targetTime"].toString().toLongLong()); // TODO local time shenaningans
    int limitForward = jObj["limitForward"].toInt();
    auto foundUserSessions = dm->getUserPlannedSessions(targetDt, limitForward);

    QJsonObject jResponse;
    jResponse["type"] = "userPlannedSessions";

    QJsonArray jSessionsByUser;
    for(int i = 0; i < foundUserSessions.length(); i++)
        jSessionsByUser << foundUserSessions[i].toJson();
    jResponse["sessionsTable"] = jSessionsByUser;

    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);
}

void KosTcpServer::sendTechLogTableRows(const QJsonObject &jObj, QTcpSocket* socket) //TODO no way to discern time offset
{
    QDateTime targetDt = QDateTime::fromSecsSinceEpoch(jObj["targetTime"].toString().toLongLong()); // TODO local time shenaningans
    int limitBack = jObj["limitBack"].toInt();
    int limitForward = jObj["limitForward"].toInt();
    QString tableType = jObj["tableType"].toString();
    auto foundRows = dm->getTechLogTableRows(targetDt,  limitBack,  limitForward, tableType);

    QJsonObject jResponse;
    jResponse["type"] = "techLogTableRows";

    QJsonArray jRows;
    foreach(auto row, foundRows)
    {
        QJsonArray jRow;
        foreach(auto cellInRow, row)
        {
            jRow << cellInRow;
        }
        jRows << jRow;
    }
    jResponse["tableType"] = tableType;
    jResponse["rows"] = jRows;

    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);

}

void KosTcpServer::sendCompletionStateDictionary(const QJsonObject &jObj, QTcpSocket* socket)
{
    QJsonObject jResponse;
    jResponse["type"] = "completionStateDictionary";

    QVector<CompletionState> cStates {PlannedFixed,
                PlannedRange,
                InProgress,
                Done,
                FailedWeather,
                FailedTech,
                FailedUnknown};
    QJsonArray jStates;
    foreach(auto st, cStates)
    {
        QJsonObject jState;
        jState[getCompletionStateString(st)] = getCompletionStateDisplayString(st);
        jStates << jState;
    }
    jResponse["dictionary"] = jStates;

    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);
}

void KosTcpServer::sendKosState(const QJsonObject &jObj, QTcpSocket* socket)
{
    QJsonObject jResponse;
    jResponse["type"] = "kosState";
    QPair<int, int> stateAndErrors = dm->getKosState();
    jResponse["stateNum"] =  stateAndErrors.first;
    jResponse["errorsCount"] = stateAndErrors.first == 3 ? stateAndErrors.second : 0;
    jResponse["isManualMode"] = dm->getManualMode();

    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);
}

void KosTcpServer::sendManualModeState(const QJsonObject &jObj, QTcpSocket* socket)
{
    QJsonObject jResponse;
    jResponse["type"] = "manualModeState";
    jResponse["isManualMode"] = dm->getManualMode();

    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);
}

void KosTcpServer::writeManualModeState(const QJsonObject &jObj, QTcpSocket* socket)
{
    bool mmState = jObj["manualModeState"].toBool();

    QJsonObject jResponse;
    jResponse["type"] = "writeManualModeStateResult";

    jResponse["ok"] = dm->setManualMode(mmState);
    jResponse["isManualMode"] = dm->getManualMode();

    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);
}

void KosTcpServer::sendManualModeTable(const QJsonObject &jObj, QTcpSocket* socket)
{
    QJsonObject jResponse;
    jResponse["type"] = "manualModeTable";

    auto res = dm->getManualModeTableInfo();

    QJsonArray jRows;
    foreach(auto mmrKey, res.keys())
    {
        QJsonObject jRow;
        jRow["satId"] = mmrKey;
        QJsonObject jVisibility;
        jVisibility["from"] = QString::number(res[mmrKey]->visibility.start.toSecsSinceEpoch());
        jVisibility["to"] = QString::number(res[mmrKey]->visibility.end.toSecsSinceEpoch());
        jRow["visibility"] = jVisibility;
        //jRow["weather"] = getWeatherString(mmr.weatherVal);
        jRow["info"] = res[mmrKey]->infoText;
        jRows << jRow;
    }

    jResponse["manualModeRows"] = jRows;

    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);
}

void KosTcpServer::sendManualModeCurrentInfo(const QJsonObject &jObj, QTcpSocket* socket)
{
    QJsonObject jResponse;
    jResponse["type"] = "manualModeCurrentInfo";

    auto res = dm->getManualModeTableInfo();

    QJsonArray jSats;
    foreach(auto mmrKey, res.keys())
    {
        QJsonObject jSat;
        jSat["satId"] = mmrKey;
        if(QDateTime::currentDateTime() < res[mmrKey]->visibility.start)
            jSat["weather"] = QJsonObject();
        else
            jSat["weather"] = getWeatherString(res[mmrKey]->weatherVal);
        jSats << jSat;
    }
    jResponse["weathers"] = jSats;
    jResponse["curSessionSatId"] = dm->getCurManualSessionSatId();
    jResponse["curSessionInfo"] = dm->getCurManualSessionInfoText();
    jResponse["sessionIsRunning"] = dm->getCurManualSessionIsRunning();


    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);
}

void KosTcpServer::sendClosestSatelliteVisibility(const QJsonObject &jObj, QTcpSocket* socket)
{
    int satId = jObj["satId"].toInt(-1);

    QJsonObject jResponse;
    jResponse["type"] = "satelliteClosestVisibility";
    jResponse["satId"] = satId;
    jResponse["visibility"] = QJsonObject();

    if(satId != -1)
    {
        auto res = dm->getManualModeTableInfo();
        QJsonObject jVisibility;
        jVisibility["from"] = QString::number(res[satId]->visibility.start.toSecsSinceEpoch());
        jVisibility["to"] = QString::number(res[satId]->visibility.end.toSecsSinceEpoch());
        jResponse["visibility"] = jVisibility;
    }
    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);
}

void KosTcpServer::startManualSession(const QJsonObject &jObj, QTcpSocket* socket)
{
    int satId = jObj["satId"].toInt();
    auto startResult = dm->startManualSession(satId);

    QJsonObject jResponse;
    jResponse["type"] = "sessionStartInfo";
    jResponse["satId"] = satId;
    jResponse["ok"] = startResult.first;
    jResponse["infoText"] = startResult.second;

    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);

}

void KosTcpServer::stopManualSession(const QJsonObject &jObj, QTcpSocket* socket)
{
    int satId = -1;
    if(dm->getCurManualSessionIsRunning())
        satId = dm->getCurManualSessionSatId();
    auto stopResult = dm->stopManualSession();
    QJsonObject jResponse;
    jResponse["type"] = "sessionStopInfo";
    jResponse["satId"] = satId;
    jResponse["ok"] = stopResult.first;
    jResponse["infoText"] = stopResult.second;

    writeBytes(QJsonDocument(jResponse).toJson(QJsonDocument::Compact), socket);

}

void KosTcpServer::writeBytes(const QByteArray& bytes, const QList<QTcpSocket*> &socks)
{
    foreach(QTcpSocket* sock, socks)
        writeBytes(bytes, sock);
}

void KosTcpServer::writeBytes(const QByteArray& bytes, QTcpSocket* socket)
{
    if(socket->isWritable())
        socket->write(bytes);
    else
        qWarning() << "Socket write error";

}

QList<QByteArray> KosTcpServer::splitJsons(const QByteArray& input, bool &ok)
{
    QList<QByteArray> res;
    int jStart=0, jEnd=0, counter=0;

    //foreach (QChar smb, input)
    for (int i=0;i<input.length();i++)
    {
        if(input[i] == '{')
        {
            if(counter == 0)
                jStart = i;
            counter++;
        }
        else if (input[i] == '}')
        {
            if(counter > 0) counter--;
            if(counter == 0)
            {
                jEnd = i;
                res << input.mid(jStart, jEnd-jStart+1);
            }
        }
    }

    int sumOfLengths = 0;
    for (auto i : res) {
        sumOfLengths += i.length();
    }
    ok = input.length() == sumOfLengths;

    return res;
}
