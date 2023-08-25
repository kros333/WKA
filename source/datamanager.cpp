#include "datamanager.h"
#include <QDebug>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "weatherwidget.h"
#include <algorithm>
#include "math.h"

DataManager::DataManager(quint16 daysToLeft, quint16 daysToRight, int weatherUpdFreq)
{
    secsToUpdateWeather = weatherUpdFreq;

    weatherWidget.show();

    // Создание КА
    int kaLastId = 1;
    const int kaGlonassAmount = 30; // количество аппаратов ГЛОНАСС
    for(int i=0; i < kaGlonassAmount; i++)
    {
        satellites.append(new Satellite(kaLastId, "ГЛОНАСС " + QString::number(kaLastId)));
        kaLastId++;
    }
    const int kaLageosAmount = 10;
    for(int i=0; i < kaLageosAmount; i++)
    {
        satellites.append(new Satellite(kaLastId, "ЛАГЕОС " + QString::number(kaLastId)));
        kaLastId++;
    }

        for(int i = 0; i < satellites.count(); i++)
            satellitePlanningSettings.insert(satellites[i], SatellitePlanningRule());


    // Создание сеансов (если по 3 мин то 480 в сутки)
    const QDateTime historyStartDt = QDateTime::currentDateTime().addDays(daysToLeft * -1).addSecs(-1 * (historyStartDt.currentSecsSinceEpoch() % 60));
    const QDateTime futureStopDt = QDateTime::currentDateTime().addDays(daysToRight).addSecs(-1 * (historyStartDt.currentSecsSinceEpoch() % 60));

    qDebug() << "Generated history from " << historyStartDt.toString("dd.MM.yyyy hh:mm:ss") << " to " << futureStopDt.toString("dd.MM.yyyy hh:mm:ss");

    //генерим начальную погоду
    srand(time(0));
    // заполняем матрицу случайными значениями
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            // генерируем случайное число от 0 до 3
            int randomNum = rand() % 4;

            // присваиваем true или false в зависимости от случайного числа
            matrix[i][j] = (randomNum == 1);
            new_matrix[i][j] = matrix[i][j];
        }
    }


    VisibilityWindowSummary vis;
    QDateTime crawlDt = historyStartDt;
    QList<QDateTime> LastKaVis;
    for (int i = 0; i < satellites.length(); i++)
    {
        LastKaVis << QDateTime::currentDateTime().addDays(-3000);
    }
    while (crawlDt < futureStopDt)
    {
        int sessionsLeftInWindow = 0;
        crawlDt = crawlDt.addSecs(getRandomInt(3, 30)*60);
        int currentKaId;
        bool kaOk = false;
        while(!kaOk)
        {
            //            if(kaPos >= 30 && getRandomInt(64)) kaPos = getRandomInt(29);
            currentKaId = getRandomInt(satellites.length()-1);
            kaOk = false;

            if(LastKaVis[currentKaId].addSecs(7200) < crawlDt)
            {
                kaOk = true;
            }

        }
        SatellitePlanningRule currentSatellitePlanningRule = satellitePlanningSettings.find(satellites[currentKaId]).value();

        vis = VisibilityWindowSummary();
        vis.kaId = currentKaId;
        vis.startEnd.start = crawlDt;
        vis.startEnd.end = crawlDt.addSecs(getRandomInt(30, 240)*60);
        LastKaVis[currentKaId] = vis.startEnd.end;
        //начало окна всегда с солнечной погодой (почему? пока что не так)
        vis.weathers << VisibilityWindowSummary::WeatherSpan(static_cast<Weather>(getRandomInt(2)), TimeSpan(crawlDt, crawlDt.addSecs(getRandomInt(1, vis.startEnd.end.toSecsSinceEpoch()/60 - vis.startEnd.start.toSecsSinceEpoch()/60)*60)));///////////////////////////////////////////////////
        while(vis.weathers.last().span.end != vis.startEnd.end && vis.weathers.last().span.end < QDateTime::currentDateTime().addSecs(10))
        {
            vis.weathers << VisibilityWindowSummary::WeatherSpan(static_cast<Weather>(getRandomInt(2)), TimeSpan(vis.weathers.last().span.end, vis.weathers.last().span.end.addSecs(getRandomInt(1, vis.startEnd.end.toSecsSinceEpoch()/60 - vis.startEnd.start.toSecsSinceEpoch()/60)*60)));///////////////////////////////////////////////////
            if(vis.weathers.last().span.end > QDateTime::currentDateTime().addSecs(10))
            {
                vis.weathers.last().span.end = QDateTime::currentDateTime().addSecs(10);
            }
            if(vis.weathers.last().span.end > vis.startEnd.end)
            {
                vis.weathers.last().span.end = vis.startEnd.end;
            }
        }
        if(currentSatellitePlanningRule.priority != 0)
        {
            // определяем количество сессий в окне
            for (int i = currentSatellitePlanningRule.maxSessions; i >= currentSatellitePlanningRule.minSessions && sessionsLeftInWindow == 0; i--)
            {
                if(i * currentSatellitePlanningRule.pointsAmount * 180 + (i - 1) * currentSatellitePlanningRule.minSessionInterval * 60 <= vis.startEnd.end.toSecsSinceEpoch() - vis.startEnd.start.toSecsSinceEpoch())
                {
                    sessionsLeftInWindow = i;
                    if(sessionsLeftInWindow - 1 > 0)
                    {
                        currentSatellitePlanningRule.minSessionInterval = floor((vis.startEnd.end.toSecsSinceEpoch() - vis.startEnd.start.toSecsSinceEpoch() - (sessionsLeftInWindow * currentSatellitePlanningRule.pointsAmount * 180))/((sessionsLeftInWindow - 1) * 60));
                    }
                }
            }
            if(sessionsLeftInWindow == 0)
            {
                for (int i = currentSatellitePlanningRule.minSessionInterval; i >= 0 && sessionsLeftInWindow == 0; i--)
                {
                    if(currentSatellitePlanningRule.minSessions * currentSatellitePlanningRule.pointsAmount * 180 + (currentSatellitePlanningRule.minSessions - 1) * i * 60 <= vis.startEnd.end.toSecsSinceEpoch() - vis.startEnd.start.toSecsSinceEpoch())
                    {
                        sessionsLeftInWindow = currentSatellitePlanningRule.minSessions;
                        currentSatellitePlanningRule.minSessionInterval = i;
                    }
                }
                if(sessionsLeftInWindow == 0)
                {
                    for (int i = currentSatellitePlanningRule.minSessions; i >= 1 && sessionsLeftInWindow == 0; i--)
                    {
                        if(currentSatellitePlanningRule.pointsAmount * i * 180 <= vis.startEnd.end.toSecsSinceEpoch() - vis.startEnd.start.toSecsSinceEpoch())
                        {
                            sessionsLeftInWindow = i;
                            currentSatellitePlanningRule.minSessionInterval = 0;
                        }
                    }
                }
            }
            QDateTime startCurSession = vis.startEnd.start;
            while(sessionsLeftInWindow > 0)
            {
                Session s;
                s.currentPlanningRule = currentSatellitePlanningRule;
                s.id = ++globalSessionLastUsedId;
                s.ka = satellites.at(currentKaId);
                s.factSessionTimeStart = startCurSession;
                s.factSessionTimeEnd = startCurSession.addSecs(s.currentPlanningRule.pointsAmount * 180);
                s.visibilityStart = vis.startEnd.start;
                s.visibilityEnd = vis.startEnd.end;
                s.cState = static_cast<CompletionState>(getRandomInt(6));
                s.WindowId = windows.length();
                if(s.cState == InProgress || s.cState == PlannedFixed || s.cState == PlannedRange) s.cState = Done;

                if(s.factSessionTimeStart > QDateTime::currentDateTime())
                {
//                    if(getRandomInt(20) == 1)
//                    {
//                        s.cState = PlannedFixed;
//                    }
//                    else
//                    {
//                        s.cState = PlannedRange;
//                    }
                    s.cState = PlannedRange;
                }

                switch (s.cState)
                {
                case 3:
                    s.answers = s.currentPlanningRule.pointsAmount * 180 + getRandomInt(0,9);
                    break;
                case 4:
                case 5:
                case 6:
                    if(getRandomInt(0,1))
                    {
                        s.answers = getRandomInt(s.currentPlanningRule.pointsAmount * 180 - 10);
                    }
                    else
                    {
                        s.answers = 0;
                    }
                    break;
                default:
                    break;
                }
                startCurSession = startCurSession.addSecs(s.currentPlanningRule.pointsAmount * 180 + s.currentPlanningRule.minSessionInterval * 60);
                sessions << s;
                vis.sessions << qMakePair(TimeSpan(s.factSessionTimeStart, s.factSessionTimeEnd), qMakePair(s.id, s.cState));
                sessionsLeftInWindow--;
            }
        }
        windows << qMakePair(currentKaId, vis);

        //ТЕХ СОСТ + МЕТЕО
        if(crawlDt <= QDateTime::currentDateTime() && !getRandomInt(40))
            journal.append({QString::number(journal.length() + 1), QString::number(crawlDt.toSecsSinceEpoch()), "Сообщение №" + QString::number(journal.length() + 1)});
        if(crawlDt <= QDateTime::currentDateTime() && !getRandomInt(100))
            tests.append({QString::number(tests.length() + 1), QString::number(crawlDt.toSecsSinceEpoch()), getRandomInt(1)?"В наличии":"Отсутствует", getRandomInt(1)?"В наличии":"Отсутствует",QString::number(tests.length() + 1)});
        if(crawlDt <= QDateTime::currentDateTime() && !getRandomInt(50))
            fnks.append({QString::number(fnks.length() + 1), QString::number(crawlDt.toSecsSinceEpoch()), getRandomInt(1)?"Да":"Нет", getRandomInt(1)?"Да":"Нет", "#" + QString::number(fnks.length() + 1)});
        if(crawlDt <= QDateTime::currentDateTime() && !getRandomInt(50))
            meteos.append({QString::number(meteos.length() + 1), QString::number(crawlDt.toSecsSinceEpoch()), QString::number(getRandomInt(-20,40)), QString::number(meteos.length() + 1) + "м/с"});


    }
    std::sort(sessions.begin(), sessions.end(), [](const Session& s1, const Session& s2)
    {
        return s1.factSessionTimeStart < s2.factSessionTimeStart;
    });

    while(sessions[globalCurrentSessionArrayPos].cState > 1 && globalCurrentSessionArrayPos < sessions.length())
    {
        globalCurrentSessionArrayPos++;
    }
    globalCurrentSessionArrayPos--;
    if(sessions[globalCurrentSessionArrayPos].factSessionTimeEnd > QDateTime::currentDateTime().addSecs(60))
    {
        qDebug() << "Session in progress";
        sessions[globalCurrentSessionArrayPos].cState = InProgress;
        sessions[globalCurrentSessionArrayPos].answers = getRandomInt(0, 140);
    }
    else {
        qDebug() << "No one session in progress";

    }

//    if(globalCurrentSessionArrayPos > 0) globalCurrentSessionArrayPos--; //unfortunate hack to get current session
    qDebug() << "Current session starts at " << sessions[globalCurrentSessionArrayPos].factSessionTimeStart << ", id = " << sessions[globalCurrentSessionArrayPos].id;

    auto curDt = QDateTime::currentDateTime();
    foreach(const auto &sat, satellites)
    {
        ManualModeRecord* mmr = new ManualModeRecord();
        mmr->infoText = "";
        //50/50 chanche for availible window or window in future
        if(getRandomInt(1))
            mmr->visibility = TimeSpan(curDt.addSecs(-1 * getRandomInt(600)),curDt.addSecs(getRandomInt(599) + 1));
        else
            mmr->visibility = TimeSpan(curDt.addSecs(getRandomInt(600)),curDt.addSecs(getRandomInt(599) + 1 + 600));
        mmr->weatherVal = static_cast<Weather>(getRandomInt(1));
        manualModeInfos[sat->id] = mmr;
    }

    oneHzTimer.setInterval(1000);
    connect(&oneHzTimer, &QTimer::timeout, this, &DataManager::oneHzTimerElapsed);
    oneHzTimer.start();
}

//DataManager::~DataManager() {}

const QVector<Session> DataManager::getSessions(QDateTime targetTime, int limitBack, int limitForward, SessionsTableFilterOptions filter)
{
    //filtering
    QVector<Session> filteredSessions;
    if(!filter.sessionFrom.isValid() &&
            !filter.sessionTo.isValid() &&
            !filter.visibilityFrom.isValid() &&
            !filter.visibilityTo.isValid() &&
            filter.selectedCStates.isEmpty() &&
            filter.selectedSatelliteIds.isEmpty() ) //если нет фильтра, а конткретно если все даты не валидны и все списки пустые
    {
        filteredSessions = sessions; //copy on write should protect from copying maybe
    }
    else
    {
        for(int i = 0; i< sessions.length()-1; i++)
        {
            if(!filter.selectedSatelliteIds.isEmpty() && !filter.selectedSatelliteIds.contains(sessions[i].ka->id))
                continue;

            if(!filter.selectedCStates.isEmpty() && !filter.selectedCStates.contains(sessions[i].cState))
                continue;

            if(filter.sessionFrom.isValid() && sessions[i].factSessionTimeEnd < filter.sessionFrom)
                continue;

            if(filter.sessionTo.isValid() && sessions[i].factSessionTimeStart > filter.sessionTo)
                continue;

            if(filter.visibilityFrom.isValid() && sessions[i].visibilityEnd < filter.visibilityFrom)
                continue;

            if(filter.visibilityTo.isValid() && sessions[i].visibilityStart > filter.visibilityTo)
                continue;

            filteredSessions << sessions[i];

        }
    }

    if(filteredSessions.length() == 0) return filteredSessions;

    if(targetTime < filteredSessions[0].factSessionTimeStart)
        return filteredSessions.mid(0, limitForward);
    if(targetTime > filteredSessions[filteredSessions.length() - 1].factSessionTimeStart)
        return filteredSessions.mid(filteredSessions.length() - 1 - limitBack, limitBack);

    int left = 0;
    int right = filteredSessions.length() - 1;
    int mid;

    while (left <= right) {
        mid = (left + right) / 2;

        if (filteredSessions[mid].factSessionTimeStart <= targetTime && (mid == filteredSessions.length()-1 || filteredSessions[mid+1].factSessionTimeStart > targetTime)) {
            return filteredSessions.mid(mid - limitBack + 1, limitBack + limitForward);
        }
        else if (filteredSessions[mid].factSessionTimeStart <= targetTime) {
            left = mid + 1;
        }
        else {
            right = mid - 1;
        }
    }

    //return -1; // if no such index is found
    return filteredSessions.mid(globalCurrentSessionArrayPos-limitBack, limitBack + limitForward + 1); //+1 for current
}

QMap<QDate /*day*/, CompletedSessionsBySatId> DataManager::getSessionsGrid(QDate targetDate, int limitBackInDays, int limitForwardInDays)
{
    QMap<QDate /*day*/, CompletedSessionsBySatId> result;
    QDate leftestDate = targetDate.addDays(limitBackInDays * -1);
    QDate rightestDate = targetDate.addDays(limitForwardInDays);
    QDate curDate = leftestDate;
    while(curDate <= rightestDate)
    {
        result.insert(curDate,{});
        curDate = curDate.addDays(1);
    }

    if(sessions.length() == 0) return result;

    //binary search of approximate pos
    int left = 0;
    int right = sessions.length() - 1;
    int mid = 0;
    while (left <= right) {
        mid = (left + right) / 2;

        if (sessions[mid].factSessionTimeStart.date() == leftestDate) {
            break;
        }
        else if (sessions[mid].factSessionTimeStart.date() < leftestDate) {
            left = mid + 1;
        }
        else {
            right = mid - 1;
        }
    }

    if(left > right && left > sessions.length() - 1) // all sessions are earlier than requested date
    {
        // absolutely nothing to return
        return result;
    }


    while(mid >= 0 && sessions[mid].factSessionTimeStart.date() == leftestDate) // moveing left until we find first session of leftest date
        mid--;
    mid++;  //reset to 0 if nothing found OR move 1 to right to first session of day

    if(sessions[mid].factSessionTimeStart.date() > rightestDate)
    {
        // nothing to return, all sesssions are later than rightest date
        return result;
    }


    while(mid <= sessions.length()-1 && sessions[mid].factSessionTimeStart.date() >= leftestDate && sessions[mid].factSessionTimeStart.date() <= rightestDate)
    { //текущий сеанс входит в нужный диапазон
        result[sessions[mid].factSessionTimeStart.date()][sessions[mid].ka->id].sessionsTotal++;
        if(sessions[mid].cState == Done)
            result[sessions[mid].factSessionTimeStart.date()][sessions[mid].ka->id].sessionsDone++;
        if(sessions[mid].cState <= 2)
            result[sessions[mid].factSessionTimeStart.date()][sessions[mid].ka->id].colored = false; // если хоть один статус говорит о том, что сессия не в прошлом
        mid++;
    }
    return result;
}

QMap<int /*satId*/, QList<VisibilityWindowSummary>>DataManager::getSessionsRibbon(QDateTime targetDt, int limitBackInHours, int limitForwardInHours)
{
    QMap<int /*satId*/, QList<VisibilityWindowSummary>> result;
    targetDt.setTime(QTime(targetDt.time().hour(),0)); // отбрасываем минуты запроенного времени. Становимся на начало часа
    QDateTime leftestHour = targetDt.addSecs(limitBackInHours * 60 * 60 * -1); //hours * 60 = minutes * 60 = seconds
    QDateTime rightestHour = targetDt.addSecs(limitForwardInHours * 60 * 60).addSecs(60 * 60 -1); //эта граница считается так: начало часа targetDt + кол-во часов справа(lmitForwardInHours) + 59 мин 59 сек
    for(int i=0;i<windows.length();i++)
    {
        if(windows[i].second.startEnd.end >= leftestHour && windows[i].second.startEnd.start <= rightestHour)
        {
            result[windows[i].first] << windows[i].second;
        }
    }
    return result;
}

bool DataManager::updateSatellitePlanningRule(int satId, QString newPriority, int newPointsAmount, int newMinSessions, int newMaxSessions, int newMinSessionInterval)
{
    bool firstClear = true;
    if(satId < 0 ||
            newPriority == "" ||
            newPointsAmount <= 0 ||
            newMinSessions <= 0 ||
            newMaxSessions <= 0 ||
            newMinSessionInterval < 0 ||
            newMinSessions > newMaxSessions ||
            newPointsAmount > 10)
        return false;

    bool ok;
    auto priorityVal = getSatellitePlanningPriorityVal(newPriority, &ok);
    if(!ok) return false;

    Satellite* sat = getSatellite(satId);
    if(sat == nullptr) return false;

    SatellitePlanningRule newrule;
    newrule.priority = priorityVal;
    newrule.maxSessions = newMaxSessions;
    newrule.minSessions = newMinSessions;
    newrule.pointsAmount = newPointsAmount;
    newrule.minSessionInterval = newMinSessionInterval;

    satellitePlanningSettings[sat] = newrule;
    QDateTime crawlDt = QDateTime::currentDateTime();
    for (int i = 0; i < windows.length(); i++)
    {
        if(windows[i].second.startEnd.start > QDateTime::currentDateTime() && windows[i].first + 1 == satId)
        {
            //удаляем запланированные по старым правилам сессии
            if(firstClear == true)
            {
                for (int j = 0; j < sessions.length(); j++)
                {
                    if(sessions[j].ka->id == satId && sessions[j].visibilityStart >= windows[i].second.startEnd.start.addSecs(-60))
                    {
                        sessions.remove(j);
                        j--;
                    }
                }
                firstClear = false;
            }
            VisibilityWindowSummary vis = windows[i].second;
            vis.sessions.clear();
            int sessionsLeftInWindow = 0;
            SatellitePlanningRule rule = newrule;
            if(rule.priority != 0)
            {
                // определяем количество сессий в окне
                for (int i = rule.maxSessions; i >= rule.minSessions && sessionsLeftInWindow == 0; i--)
                {
                    if(i * rule.pointsAmount * 180 + (i - 1) * rule.minSessionInterval * 60 <= vis.startEnd.end.toSecsSinceEpoch() - vis.startEnd.start.toSecsSinceEpoch())
                    {
                        sessionsLeftInWindow = i;
                        if(sessionsLeftInWindow - 1 > 0)
                        {
                            rule.minSessionInterval = floor((vis.startEnd.end.toSecsSinceEpoch() - vis.startEnd.start.toSecsSinceEpoch() - (sessionsLeftInWindow * rule.pointsAmount * 180))/((sessionsLeftInWindow - 1) * 60));
                            qDebug() << "зашли 1";
                        }
                    }
                }
                if(sessionsLeftInWindow == 0)
                {
                    for (int i = rule.minSessionInterval; i >= 0 && sessionsLeftInWindow == 0; i--)
                    {
                        if(rule.minSessions * rule.pointsAmount * 180 + (rule.minSessions - 1) * i * 60 <= vis.startEnd.end.toSecsSinceEpoch() - vis.startEnd.start.toSecsSinceEpoch())
                        {
                            sessionsLeftInWindow = rule.minSessions;
                            rule.minSessionInterval = i;
                            qDebug() << "зашли 2";
                        }
                    }
                    if(sessionsLeftInWindow == 0)
                    {
                        for (int i = rule.minSessions; i >= 1 && sessionsLeftInWindow == 0; i--)
                        {
                            if(rule.pointsAmount * i * 180 <= vis.startEnd.end.toSecsSinceEpoch() - vis.startEnd.start.toSecsSinceEpoch())
                            {
                                sessionsLeftInWindow = i;
                                rule.minSessionInterval = 0;
                                qDebug() << "зашли 3";
                            }
                        }
                    }
                }
                QDateTime startCurSession = vis.startEnd.start;
                while(sessionsLeftInWindow > 0)
                {
                    Session s;
                    s.currentPlanningRule = rule;
                    s.id = ++globalSessionLastUsedId;
                    s.ka = satellites.at(satId - 1);
                    s.factSessionTimeStart = startCurSession;
                    s.factSessionTimeEnd = startCurSession.addSecs(s.currentPlanningRule.pointsAmount * 180);
                    s.visibilityStart = vis.startEnd.start;
                    s.visibilityEnd = vis.startEnd.end;
                    s.cState = static_cast<CompletionState>(getRandomInt(6));
                    s.WindowId = i;
                    if(s.cState == InProgress || s.cState == PlannedFixed || s.cState == PlannedRange) s.cState = Done;

                    if(s.factSessionTimeStart > QDateTime::currentDateTime())
                    {
    //                    if(getRandomInt(20) == 1)
    //                    {
    //                        s.cState = PlannedFixed;
    //                    }
    //                    else
    //                    {
    //                        s.cState = PlannedRange;
    //                    }
                        s.cState = PlannedRange;
                    }


                    switch (s.cState)
                    {
                    case 3:
                        s.answers = s.currentPlanningRule.pointsAmount * 180 + getRandomInt(0,9);
                        break;
                    case 4:
                    case 5:
                    case 6:
                        if(getRandomInt(0,1))
                        {
                            s.answers = getRandomInt(s.currentPlanningRule.pointsAmount * 180 - 10);
                        }
                        else
                        {
                            s.answers = 0;
                        }
                        break;
                    default:
                        break;
                    }
                    startCurSession = startCurSession.addSecs(s.currentPlanningRule.pointsAmount * 180 + s.currentPlanningRule.minSessionInterval * 60);
                    sessions << s;
                    qDebug() << sessions.last().ka->name;
                    vis.sessions << qMakePair(TimeSpan(s.factSessionTimeStart, s.factSessionTimeEnd), qMakePair(s.id, s.cState));
                    sessionsLeftInWindow--;
                }
                windows[i].second = vis;
            }
        }
    }
    std::sort(sessions.begin(), sessions.end(), [](const Session& s1, const Session& s2)
    {
        return s1.factSessionTimeStart < s2.factSessionTimeStart;
    });
    return true;
}

int DataManager::getSessionsCount()
{
    return sessions.length();
}

const QMap<Satellite*, SatellitePlanningRule> DataManager::getSatellitePlanningRules()
{
    return satellitePlanningSettings;
}

const QVector<Satellite*> DataManager::getSatelliteDictionary()
{
    return satellites;
}

const Satellite* DataManager::getSatelliteName(int satId)
{
    for(int i = 0;i<satellites.length();i++)
        if(satellites[i]->id == satId)
            return satellites[i];
    return nullptr;
}

const QVector<TimeSpan> DataManager::getSatelliteWindows(int satId)
{
    QVector<TimeSpan> result;
    if(satId == -1) return result;

    for (int i = 0; i < windows.length(); i++)
    {
        if(windows[i].second.kaId + 1 == satId && windows[i].second.startEnd.start > QDateTime::currentDateTime().addSecs(180))
        {
            result << windows[i].second.startEnd;
        }
    }
    return result;
}

//TODO хранить длительность для CompletionState::PlannedRange сеансов
bool DataManager::writeNewSatelliteUserPlannedSession(int satId, QDateTime start, QDateTime end, bool isApproximateTime, int durationInSeconds, qint64 editSessionId)
{
    Satellite* sat = getSatellite(satId);
    if(sat == nullptr)
    {
        qDebug() << "ASHIBKA";
        return false;
    }
    VisibilityWindowSummary vis;
    int windowPosition = -1;
    for (int i = 0; i < windows.length(); i++)
    {
        if(satId == windows[i].second.kaId + 1 && windows[i].second.startEnd.start.toSecsSinceEpoch() <= start.toSecsSinceEpoch() && windows[i].second.startEnd.end.toSecsSinceEpoch() >= end.toSecsSinceEpoch())
        {
            vis = windows[i].second;
            windowPosition = i;
        }
    }
    if(windowPosition == -1)
    {
        qDebug() << "ASHIBKA_1";
        return false;
    }
    Session s;
    s.cState = isApproximateTime ? CompletionState::PlannedRange : CompletionState::PlannedFixed;
    int sId = editSessionId == -1 ?
                (userPlannedSessions.isEmpty() ? 1 : userPlannedSessions.last().id + 1) :
                editSessionId;
    s.id = sId;
    s.ka = sat;
    s.factSessionTimeStart = start;

    //если диапазон - то брать end, если фикс, то старт + длительность
    if(isApproximateTime)
        s.factSessionTimeEnd = end;
    else
        s.factSessionTimeEnd = start.addSecs(durationInSeconds);

    s.visibilityStart = vis.startEnd.start;
    s.visibilityEnd = vis.startEnd.end;
    s.currentPlanningRule = getSatellitePlanningRules().find(sat).value();
    s.currentPlanningRule.priority = SatellitePlanningPriority(3);
    s.currentPlanningRule.pointsAmount = -1;
    bool write = false;

    if(editSessionId != -1)
    {
        for(int i=0; i < userPlannedSessions.length();i++)
        {
            if(userPlannedSessions[i].id == editSessionId)
            {
                userPlannedSessions[i] = s;
                for (int i = globalCurrentSessionArrayPos; i < sessions.length(); i++)
                {
                    if(sessions[i].ka->id == satId && (DataManager::CheckIncludingQDT(s.factSessionTimeStart.toSecsSinceEpoch(), sessions[i].factSessionTimeStart.toSecsSinceEpoch(), sessions[i].factSessionTimeEnd.toSecsSinceEpoch()) || DataManager::CheckIncludingQDT(s.factSessionTimeEnd.toSecsSinceEpoch(), sessions[i].factSessionTimeStart.toSecsSinceEpoch(), sessions[i].factSessionTimeEnd.toSecsSinceEpoch())))
                    {
                        if(write == false)
                        {
                            for (int e = 0; e < windows[windowPosition].second.sessions.length(); e++)
                            {
                                if(windows[windowPosition].second.sessions[e].second.first == sessions[i].id)
                                {
                                    windows[windowPosition].second.sessions[e].first.start = s.factSessionTimeStart;
                                    windows[windowPosition].second.sessions[e].first.end = s.factSessionTimeEnd;
                                    windows[windowPosition].second.sessions[e].second.first = s.id;
                                    windows[windowPosition].second.sessions[e].second.second = s.cState;
                                    e = windows[windowPosition].second.sessions.length();
                                }
                            }
                            sessions[i] = s;
                            write = true;
                        }
                        else
                        {
                            for (int e = 0; e < windows[windowPosition].second.sessions.length(); e++)
                            {
                                if(windows[windowPosition].second.sessions[e].second.first == sessions[i].id)
                                {
                                    windows[windowPosition].second.sessions.removeAt(e);
                                    e = windows[windowPosition].second.sessions.length();
                                }
                            }
                            for (int j = i; j < sessions.length(); j++)
                            {
                                sessions[j] = sessions[j + 1];
                            }
                            sessions.remove(sessions.length() - 1);
                        }
                    }
                }
                if(write == false)
                {
                    sessions << s;
                    windows[windowPosition].second.sessions << qMakePair(TimeSpan(s.factSessionTimeStart, s.factSessionTimeEnd), qMakePair(s.id, s.cState));
                }
                std::sort(sessions.begin(), sessions.end(), [](const Session& s1, const Session& s2)
                {
                    return s1.factSessionTimeStart < s2.factSessionTimeStart;
                });
                return true;
            }
        }
        qDebug() << "ASHIBKA_2";
        return false; // если в цикле не нашлось такого id
    }

    userPlannedSessions << s;
    for (int i = globalCurrentSessionArrayPos; i < sessions.length(); i++)
    {
        if(sessions[i].ka->id == satId && (DataManager::CheckIncludingQDT(s.factSessionTimeStart.toSecsSinceEpoch(), sessions[i].factSessionTimeStart.toSecsSinceEpoch(), sessions[i].factSessionTimeEnd.toSecsSinceEpoch()) || DataManager::CheckIncludingQDT(s.factSessionTimeEnd.toSecsSinceEpoch(), sessions[i].factSessionTimeStart.toSecsSinceEpoch(), sessions[i].factSessionTimeEnd.toSecsSinceEpoch())))
        {
            if(write == false)
            {
                for (int e = 0; e < windows[windowPosition].second.sessions.length(); e++)
                {
                    if(windows[windowPosition].second.sessions[e].second.first == sessions[i].id)
                    {
                        windows[windowPosition].second.sessions[e].first.start = s.factSessionTimeStart;
                        windows[windowPosition].second.sessions[e].first.end = s.factSessionTimeEnd;
                        windows[windowPosition].second.sessions[e].second.first = s.id;
                        windows[windowPosition].second.sessions[e].second.second = s.cState;
                        e = windows[windowPosition].second.sessions.length();
                    }
                }
                sessions[i] = s;
                write = true;
            }
            else
            {
                for (int e = 0; e < windows[windowPosition].second.sessions.length(); e++)
                {
                    if(windows[windowPosition].second.sessions[e].second.first == sessions[i].id)
                    {
                        windows[windowPosition].second.sessions.removeAt(e);
                        e = windows[windowPosition].second.sessions.length();
                    }
                }
                for (int j = i; j < sessions.length(); j++)
                {
                    sessions[j] = sessions[j + 1];
                }
                sessions.remove(sessions.length() - 1);
            }
        }
    }
    if(write == false)
    {
        sessions << s;
        windows[windowPosition].second.sessions << qMakePair(TimeSpan(s.factSessionTimeStart, s.factSessionTimeEnd), qMakePair(s.id, s.cState));
    }
    std::sort(sessions.begin(), sessions.end(), [](const Session& s1, const Session& s2)
    {
        return s1.factSessionTimeStart < s2.factSessionTimeStart;
    });
    return true;
}

bool DataManager::CheckIncludingQDT(int check, int start, int end)
{
    if (check >= start && check <= end)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//TODO хранить длительность для CompletionState::PlannedRange сеансов
bool DataManager::deleteSatelliteUserPlannedSession(qint64 deleteSessionId)
{
    if(deleteSessionId == -1) return false;

    for(int i=0; i < userPlannedSessions.length();i++)
    {
        if(userPlannedSessions[i].id == deleteSessionId)
        {
            userPlannedSessions.removeAt(i);
            for (int j = 0; j < sessions.length(); j++)
            {
                if(sessions[j].id == deleteSessionId)
                {
                    sessions.removeAt(j);
                }
            }
            return true;
        }
    }
    return false;
}

const QVector<Session> DataManager::getUserPlannedSessions(QDateTime targetTime, int limitForward)
{
    if(userPlannedSessions.length() == 0) return userPlannedSessions;

    if(targetTime < userPlannedSessions[0].factSessionTimeStart)
        return userPlannedSessions.mid(0, limitForward);
    if(targetTime > userPlannedSessions[userPlannedSessions.length() - 1].factSessionTimeStart)
        return QVector<Session>();

    int left = 0;
    int right = userPlannedSessions.length() - 1;
    int mid;

    while (left <= right) {
        mid = (left + right) / 2;

        if (userPlannedSessions[mid].factSessionTimeStart <= targetTime && (mid == userPlannedSessions.length()-1 || userPlannedSessions[mid+1].factSessionTimeStart > targetTime)) {
            return userPlannedSessions.mid(mid, limitForward);
        }
        else if (userPlannedSessions[mid].factSessionTimeStart <= targetTime) {
            left = mid + 1;
        }
        else {
            right = mid - 1;
        }
    }

    //return -1; // if no such index is found
    return QVector<Session>();
}

const QVector<QVector<QString>> DataManager::getTechLogTableRows(QDateTime targetTime, int limitBack, int limitForward, QString tableType)
{
    QVector<QVector<QString>>* rows = nullptr;
    if(tableType == "journal")
        rows = &journal;
    else if (tableType == "tsd")
        rows = &tests;
    else if (tableType == "fnk")
        rows = &fnks;
    else
        rows = &meteos;

    if(rows == nullptr || rows->length() == 0) return *rows;

    if(targetTime < QDateTime::fromString((*rows)[0][1], "dd.MM.yy hh:mm:ss"))
        return (*rows).mid(0, limitForward);
    if(targetTime >  QDateTime::fromString((*rows)[rows->length()-1][1], "dd.MM.yy hh:mm:ss"))
        return (*rows).mid(rows->length() - 1 - limitBack, limitBack);

    int left = 0;
    int right = rows->length() - 1;
    int mid;

    while (left <= right) {
        mid = (left + right) / 2;

        if (QDateTime::fromString((*rows)[mid][1],"dd.MM.yy hh:mm:ss") <= targetTime && (mid == rows->length()-1 || QDateTime::fromString((*rows)[mid+1][1],"dd.MM.yy hh:mm:ss") > targetTime)) {
            return rows->mid(mid - limitBack + 1, limitBack + limitForward);
        }
        else if (QDateTime::fromString((*rows)[mid][1],"dd.MM.yy hh:mm:ss") <= targetTime) {
            left = mid + 1;
        }
        else {
            right = mid - 1;
        }
    }

    //return -1; // if no such index is found
    return (*rows).mid(globalCurrentSessionArrayPos-limitBack, limitBack + limitForward + 1); //+1 for current

}

const QMap<int, ManualModeRecord*> DataManager::getManualModeTableInfo()
{
    return manualModeInfos;
}

QPair<bool,QString> DataManager::startManualSession(int satId)
{
    if(!isManualMode || getSatellite(satId) == nullptr || manualSessionIsRunning)
        return qMakePair(false, QString(""));

    const QString infoText = "Сеанс запущен...";
    manualSessionIsRunning = true;
    curManualSessionSatId = satId;
    curManualSessionStart = QDateTime::currentDateTime();

    manualModeInfos[satId]->infoText = infoText;
    return qMakePair(true,  manualModeInfos[satId]->infoText);
}

QPair<bool, QString> DataManager::stopManualSession()
{
    QString infoText = "";
    manualModeInfos[curManualSessionSatId]->sessionsThisWindow++;
    const QString resText = QString("Время: %1 - %2; Длит.: %3; %4 ответов; %5 за окно")
            .arg(curManualSessionStart.toString("hh:mm"))
            .arg(QDateTime::currentDateTime().toString("hh:mm"))
            .arg(QString::number(curManualSessionStart.secsTo(QDateTime::currentDateTime())/60) + ":" + QString::number(curManualSessionStart.secsTo(QDateTime::currentDateTime())%60))
            .arg(curManualSessionPoints)
            .arg(manualModeInfos[curManualSessionSatId]->sessionsThisWindow);
    manualModeInfos[curManualSessionSatId]->infoText = resText;

    manualSessionIsRunning = false;
    curManualSessionSatId = -1;
    curManualSessionPoints = 0;

    return qMakePair(true, resText);
}

QString DataManager::getCurManualSessionInfoText()
{
    if(!manualModeInfos.contains(curManualSessionSatId))
        return QString("");
    return manualModeInfos[curManualSessionSatId]->infoText;
}

Satellite* DataManager::getSatellite(int satId)
{
    Satellite* sat = nullptr;
    for (int i=0;i<satellites.length();i++)
    {
        if(satellites[i]->id == satId)
            sat = satellites[i];
    }
    return sat;
}


void DataManager::oneHzTimerElapsed()
{
    if(secsToUpdateWeather == currentSecsToUpdateWeather || weatherWidget.rain)
    {
        QString sky = "";
        // выводим матрицу на экран
        for (int i = 0; i < rows; i++)
        {
            sky = sky + "<html><body>";
            for (int j = 0; j < cols; j++)
            {
                matrix[i][j] = new_matrix[i][j];
                if (matrix[i][j])
                {
                    if(sessions[globalCurrentSessionArrayPos].ka->posX == j && sessions[globalCurrentSessionArrayPos].ka->posY == i)
                    {
                        sky = sky + "<span style=\"background-color: Gray; color: red;\"><b>" + "[]" + "</b></span>";
                    }
                    else
                    {
                        sky = sky + "<span style=\"background-color: Gray;\">&nbsp;&nbsp;</span>";
                    }
                    cloud_count++;
                }
                else
                {
                    if(sessions[globalCurrentSessionArrayPos].ka->posX == j && sessions[globalCurrentSessionArrayPos].ka->posY == i)
                    {
                        sky = sky + "<span style=\"background-color: LightBlue; color: red;\"><b>" + "[]" + "</b></span>";

                    }
                    else
                    {
                        sky = sky + "<span style=\"background-color: LightBlue	;\">&nbsp;&nbsp;</span>";
                    }
                }
            }
            sky = sky + "</body></html>";
        }
        sky = sky + "    Wind - " + QString::number(wind) + "; Clouds - " + QString::number(cloud_count) + "/" + QString::number(rows*cols) + "<html><body></body></html>" + "Current satellite ID - " + QString::number(sessions[globalCurrentSessionArrayPos].ka->id);
        weatherWidget.updateText(sky);


        //обновляем погоду
        if(!weatherWidget.rain)
        {
            if(firstChangeRain)
            {
                //генерим начальную погоду
                // заполняем матрицу случайными значениями
                for (int i = 0; i < rows; i++) {
                    for (int j = 0; j < cols; j++) {
                        // генерируем случайное число
                        int randomNum = getRandomInt(1,6);

                        // присваиваем true или false в зависимости от случайного числа
                        matrix[i][j] = (randomNum == 1);
                        cloud_count = 0;
                    }
                }
                firstChangeRain = false;
            }
            for (int i = 0; i < rows; i++)
            {
                for (int j = 0; j < cols; j++)
                {
                    //если нет дождя
                    if (rain_duration == 0)
                    {
                        //если ячейка облачная
                        if (matrix[i][j] == true)
                        {
                            //если вокруг только облачные ячейки
                            if (matrix[i - 1][j] == true && matrix[i + 1][j] == true && matrix[i][j - 1] == true && matrix[i][j + 1] == true)
                            {
                                //шанс 1 к 200, что станет чистой
                                int randomNum = rand() % (200 / ratio);
                                new_matrix[i][j] = (randomNum != 1);
                            }
                            else
                            {
                                //если рядом хоть одна облачная ячейка или эта ячейка с краю
                                if (matrix[i - 1][j] == true || matrix[i + 1][j] == true || matrix[i][j - 1] == true || matrix[i][j + 1] == true || i == rows - 1 || i == 0 || j == cols - 1 || j == 0)
                                {
                                    //проверяем ветер
                                    switch (wind)
                                    {
                                    case 0:
                                        if (matrix[i - 1][j] == true)
                                        {
                                            //шанс 1 к 200, что станет ясной
                                            int randomNum = rand() % (200 / ratio);
                                            new_matrix[i][j] = (randomNum != 1);
                                        }
                                        else
                                        {
                                            //если эта ячейка с краю, откуда дует ветер
                                            if (i == 0)
                                            {
                                                //шанс 3 к 10, что станет облачной
                                                int randomNum = rand() % (10 + ratio);
                                                new_matrix[i][j] = (3 - randomNum > 0);
                                                if (matrix[i - 1][j] == true || matrix[i + 1][j] == true || matrix[i][j - 1] == true || matrix[i][j + 1] == true)
                                                {
                                                    //шанс 99 к 100, что станет облачной
                                                    int randomNum = rand() % (100 + ratio * 10);
                                                    new_matrix[i][j] = (98 - randomNum > 0);
                                                }
                                            }
                                            else
                                            {
                                                //шанс 1 к 5, что станет ясной
                                                int randomNum = rand() % 5;
                                                new_matrix[i][j] = (randomNum != 1);
                                            }
                                        }
                                        break;
                                    case 1:
                                        if (matrix[i][j + 1] == true)
                                        {
                                            //шанс 1 к 200, что станет ясной
                                            int randomNum = rand() % (200 / ratio);
                                            new_matrix[i][j] = (randomNum != 1);
                                        }
                                        else
                                        {
                                            //если эта ячейка с краю, откуда дует ветер
                                            if (j == cols - 1)
                                            {
                                                //шанс 3 к 10, что станет облачной
                                                int randomNum = rand() % (10 + ratio);
                                                new_matrix[i][j] = (3 - randomNum > 0);
                                                if (matrix[i - 1][j] == true || matrix[i + 1][j] == true || matrix[i][j - 1] == true || matrix[i][j + 1] == true)
                                                {
                                                    //шанс 99 к 100, что станет облачной
                                                    int randomNum = rand() % (100 + ratio * 10);
                                                    new_matrix[i][j] = (98 - randomNum > 0);
                                                }
                                            }
                                            else
                                            {
                                                //шанс 1 к 5, что станет ясной
                                                int randomNum = rand() % 5;
                                                new_matrix[i][j] = (randomNum != 1);
                                            }
                                        }
                                        break;
                                    case 2:
                                        if (matrix[i + 1][j] == true)
                                        {
                                            //шанс 1 к 200, что станет ясной
                                            int randomNum = rand() % (200 / ratio);
                                            new_matrix[i][j] = (randomNum != 1);
                                        }
                                        else
                                        {
                                            //если эта ячейка с краю, откуда дует ветер
                                            if (i == rows - 1)
                                            {
                                                //шанс 3 к 10, что станет облачной
                                                int randomNum = rand() % (10 + ratio);
                                                new_matrix[i][j] = (3 - randomNum > 0);
                                                if (matrix[i - 1][j] == true || matrix[i + 1][j] == true || matrix[i][j - 1] == true || matrix[i][j + 1] == true)
                                                {
                                                    //шанс 99 к 100, что станет облачной
                                                    int randomNum = rand() % (100 + ratio * 10);
                                                    new_matrix[i][j] = (98 - randomNum > 0);
                                                }
                                            }
                                            else
                                            {
                                                //шанс 1 к 5, что станет ясной
                                                int randomNum = rand() % 5;
                                                new_matrix[i][j] = (randomNum != 1);
                                            }
                                        }
                                        break;
                                    case 3:
                                        if (matrix[i][j - 1] == true)
                                        {
                                            //шанс 1 к 200, что станет ясной
                                            int randomNum = rand() % (200 / ratio);
                                            new_matrix[i][j] = (randomNum != 1);
                                        }
                                        else
                                        {
                                            //если эта ячейка с краю, откуда дует ветер
                                            if (j == 0)
                                            {
                                                //шанс 3 к 10, что станет облачной
                                                int randomNum = rand() % (10 + ratio);
                                                new_matrix[i][j] = (3 - randomNum > 0);
                                                if (matrix[i - 1][j] == true || matrix[i + 1][j] == true || matrix[i][j - 1] == true || matrix[i][j + 1] == true)
                                                {
                                                    //шанс 99 к 100, что станет облачной
                                                    int randomNum = rand() % (100 + ratio * 10);
                                                    new_matrix[i][j] = (98 - randomNum > 0);
                                                }
                                            }
                                            else
                                            {
                                                //шанс 1 к 5, что станет ясной
                                                int randomNum = rand() % 5;
                                                new_matrix[i][j] = (randomNum != 1);
                                            }
                                        }
                                        break;
                                    }
                                }
                                //если вокруг нет облачных ячеек
                                else
                                {
                                    //шанс 1 к 300, что станет чистой
                                    int randomNum = rand() % 300;
                                    new_matrix[i][j] = (randomNum != 1);
                                }
                            }
                        }
                        //если ячейка чистая
                        else
                        {
                            //если вокруг только облачные ячейки
                            if (matrix[i - 1][j] == true && matrix[i + 1][j] == true && matrix[i][j - 1] == true && matrix[i][j + 1] == true)
                            {
                                //шанс 99 к 100, что станет облачной
                                int randomNum = rand() % 100;
                                new_matrix[i][j] = (98 - randomNum > 0);
                            }
                            else
                            {
                                //если рядом хоть одна облачная ячейка или эта ячейка с краю
                                if (matrix[i - 1][j] == true || matrix[i + 1][j] == true || matrix[i][j - 1] == true || matrix[i][j + 1] == true || i == rows - 1 || i == 0 || j == cols - 1 || j == 0)
                                {
                                    //проверяем ветер
                                    switch (wind)
                                    {
                                    case 0:
                                        if (matrix[i - 1][j] == true)
                                        {
                                            //шанс 1 к 5, что станет облачной
                                            int randomNum = rand() % 5;
                                            new_matrix[i][j] = (randomNum == 1);
                                        }
                                        else
                                        {
                                            //шанс 1 к 300, что станет облачной
                                            int randomNum = rand() % 300;
                                            new_matrix[i][j] = (randomNum == 1);
                                            //если эта ячейка с краю, откуда дует ветер
                                            if (i == 0)
                                            {
                                                //шанс 80 к 100, что станет облачной
                                                int randomNum = rand() % 100;
                                                new_matrix[i][j] = (79 - randomNum > 0);
                                                if (matrix[i - 1][j] == true || matrix[i + 1][j] == true || matrix[i][j - 1] == true || matrix[i][j + 1] == true)
                                                {
                                                    //шанс 95 к 100, что станет облачной
                                                    int randomNum = rand() % 100;
                                                    new_matrix[i][j] = (94 - randomNum > 0);
                                                }

                                            }

                                        }
                                        break;
                                    case 1:
                                        if (matrix[i][j + 1] == true)
                                        {
                                            //шанс 1 к 5, что станет облачной
                                            int randomNum = rand() % 5;
                                            new_matrix[i][j] = (randomNum == 1);
                                        }
                                        else
                                        {
                                            //если эта ячейка с краю, откуда дует ветер
                                            if (j == cols - 1)
                                            {
                                                //шанс 80 к 100, что станет облачной
                                                int randomNum = rand() % 100;
                                                new_matrix[i][j] = (79 - randomNum > 0);
                                                if (matrix[i - 1][j] == true || matrix[i + 1][j] == true || matrix[i][j - 1] == true || matrix[i][j + 1] == true)
                                                {
                                                    //шанс 95 к 100, что станет облачной
                                                    int randomNum = rand() % 100;
                                                    new_matrix[i][j] = (94 - randomNum > 0);
                                                }

                                            }
                                        }
                                        break;
                                    case 2:
                                        if (matrix[i + 1][j] == true)
                                        {
                                            //шанс 1 к 5, что станет облачной
                                            int randomNum = rand() % 5;
                                            new_matrix[i][j] = (randomNum == 1);
                                        }
                                        else
                                        {
                                            //шанс 1 к 300, что станет облачной
                                            int randomNum = rand() % 300;
                                            new_matrix[i][j] = (randomNum == 1);
                                            //если эта ячейка с краю, откуда дует ветер
                                            if (i == rows - 1)
                                            {
                                                //шанс 80 к 100, что станет облачной
                                                int randomNum = rand() % 100;
                                                new_matrix[i][j] = (79 - randomNum > 0);
                                                if (matrix[i - 1][j] == true || matrix[i + 1][j] == true || matrix[i][j - 1] == true || matrix[i][j + 1] == true)
                                                {
                                                    //шанс 95 к 100, что станет облачной
                                                    int randomNum = rand() % 100;
                                                    new_matrix[i][j] = (94 - randomNum > 0);
                                                }

                                            }
                                        }
                                        break;
                                    case 3:
                                        if (matrix[i][j - 1] == true)
                                        {
                                            //шанс 1 к 5, что станет облачной
                                            int randomNum = rand() % 5;
                                            new_matrix[i][j] = (randomNum == 1);
                                        }
                                        else
                                        {
                                            //шанс 1 к 300, что станет облачной
                                            int randomNum = rand() % 300;
                                            new_matrix[i][j] = (randomNum == 1);
                                            //если эта ячейка с краю, откуда дует ветер
                                            if (j == 0)
                                            {
                                                //шанс 80 к 100, что станет облачной
                                                int randomNum = rand() % 100;
                                                new_matrix[i][j] = (79 - randomNum > 0);
                                                if (matrix[i - 1][j] == true || matrix[i + 1][j] == true || matrix[i][j - 1] == true || matrix[i][j + 1] == true)
                                                {
                                                    //шанс 95 к 100, что станет облачной
                                                    int randomNum = rand() % 100;
                                                    new_matrix[i][j] = (94 - randomNum > 0);
                                                }

                                            }
                                        }
                                        break;
                                    }
                                }
                                //если вокруг нет облачных ячеек
                                else
                                {
                                    //шанс 1 к 200, что станет облачной
                                    int randomNum = rand() % 200;
                                    new_matrix[i][j] = (randomNum == 1);
                                }
                            }
                        }
                    }
                    //если идет дождь
                    else
                    {
                        new_matrix[i][j] = true;
                        rain_duration--;
                        //если дождь закончился, уменьшаем спавн облаков
                        if (rain_duration == 0)
                        {
                            ratio = 40;
                        }
                    }
                }
            }

            //может поменяться ветер
            if (rand() % 70 == 1)
            {
                int newWind = rand() % 2;
                switch (wind)
                {
                case 0:
                    if(newWind==1)
                    {
                        wind++;
                    }
                    else
                    {
                        wind=3;
                    }
                    break;
                case 1:
                    if(newWind==1)
                    {
                        wind++;
                    }
                    else
                    {
                        wind--;
                    }
                    break;
                case 2:
                    if(newWind==1)
                    {
                        wind++;
                    }
                    else
                    {
                        wind--;
                    }
                    break;
                case 3:
                    if(newWind==1)
                    {
                        wind = 0;
                    }
                    else
                    {
                        wind--;
                    }
                    break;
                }
                wind = rand() % 4;
            }
            //случаи, если стало много облаков и нет дождя
            if (static_cast<double>(cloud_count) / static_cast<double>(cols * rows) >= 0.625 && rain_duration == 0)
            {
                //может пойти дождь, если облаков накопилось слишком много
                if (static_cast<double>(cloud_count) / static_cast<double>(cols * rows) >= 0.8 && rand() % 3 == 1)
                {
                    rain_duration = rand() % 9000 + 3000;
                }
                //может уменьшиться спавн облаков
                else
                {
                    if(rand() % 40 == 1)
                    {
                        ratio = (rand() % 2 + 1) * (rand() % 2 + 1) * 2;
                    }
                }
            }
            //если облаков не много, то шанс спавна облаков может прийти в норму (наибольшая вероятность спавна)
            else
            {
                if (rand() % 80 == 1)
                {
                    ratio = 1;
                }
            }
        }
        else
        {
            // заполняем матрицу дождем
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    new_matrix[i][j] = true;
                }
            }
            firstChangeRain = true;
            cloud_count = rows*cols;
        }
        if(cloud_count >= 400)
        {
            currentKosState = 5; //  идет дождь
        }
        for (int i = 0; i < windows.length(); i++)
        {
            if(windows[i].second.startEnd.start <= QDateTime::currentDateTime() && windows[i].second.startEnd.end >= QDateTime::currentDateTime())
            {
                if(new_matrix[satellites[windows[i].second.kaId]->posY][satellites[windows[i].second.kaId]->posX] == true)
                {
                    if(cloud_count >= 400)
                    {
                        if(windows[i].second.weathers.last().weather == 2)
                        {
                            if(windows[i].second.weathers.last().span.end > QDateTime::currentDateTime().addSecs(secsToUpdateWeather))
                            {
                                windows[i].second.weathers.last().span.end = windows[i].second.weathers.last().span.end.addSecs(secsToUpdateWeather);
                            }
                            else
                            {
                                windows[i].second.weathers.last().span.end = QDateTime::currentDateTime();
                            }
                        }
                        else
                        {
                            if(windows[i].second.weathers.last().span.end > QDateTime::currentDateTime().addSecs(secsToUpdateWeather))
                            {
                                windows[i].second.weathers << VisibilityWindowSummary::WeatherSpan(static_cast<Weather>(2), TimeSpan(windows[i].second.weathers.last().span.end, windows[i].second.weathers.last().span.end.addSecs(secsToUpdateWeather)));
                            }
                            else
                            {
                                windows[i].second.weathers << VisibilityWindowSummary::WeatherSpan(static_cast<Weather>(2), TimeSpan(windows[i].second.weathers.last().span.end, QDateTime::currentDateTime()));
                            }
                        }
                    }
                    else
                    {
                        if(windows[i].second.weathers.last().weather == 1)
                        {
                            if(windows[i].second.weathers.last().span.end > QDateTime::currentDateTime().addSecs(secsToUpdateWeather))
                            {
                                windows[i].second.weathers.last().span.end = windows[i].second.weathers.last().span.end.addSecs(secsToUpdateWeather);
                            }
                            else
                            {
                                windows[i].second.weathers.last().span.end = QDateTime::currentDateTime();
                            }
                        }
                        else
                        {
                            if(windows[i].second.weathers.last().span.end > QDateTime::currentDateTime().addSecs(secsToUpdateWeather))
                            {
                                windows[i].second.weathers << VisibilityWindowSummary::WeatherSpan(static_cast<Weather>(1), TimeSpan(windows[i].second.weathers.last().span.end, windows[i].second.weathers.last().span.end.addSecs(secsToUpdateWeather)));
                            }
                            else
                            {
                                windows[i].second.weathers << VisibilityWindowSummary::WeatherSpan(static_cast<Weather>(1), TimeSpan(windows[i].second.weathers.last().span.end, QDateTime::currentDateTime()));
                            }
                        }
                    }
                }
                else
                {
                    if(windows[i].second.weathers.last().weather == 0)
                    {
                        if(windows[i].second.weathers.last().span.end > QDateTime::currentDateTime().addSecs(secsToUpdateWeather))
                        {
                            windows[i].second.weathers.last().span.end = windows[i].second.weathers.last().span.end.addSecs(secsToUpdateWeather);
                        }
                        else
                        {
                            windows[i].second.weathers.last().span.end = QDateTime::currentDateTime();
                        }
                    }
                    else
                    {
                        if(windows[i].second.weathers.last().span.end > QDateTime::currentDateTime().addSecs(secsToUpdateWeather))
                        {
                            windows[i].second.weathers << VisibilityWindowSummary::WeatherSpan(static_cast<Weather>(0), TimeSpan(windows[i].second.weathers.last().span.end, windows[i].second.weathers.last().span.end.addSecs(secsToUpdateWeather)));
                        }
                        else
                        {
                            windows[i].second.weathers << VisibilityWindowSummary::WeatherSpan(static_cast<Weather>(0), TimeSpan(windows[i].second.weathers.last().span.end, QDateTime::currentDateTime()));
                        }
                    }
                }
            }
        }

        currentSecsToUpdateWeather = 1;
    }
    else
    {
        currentSecsToUpdateWeather++;
    }
    //закончили обновление погоды






    if(sessions[globalCurrentSessionArrayPos].factSessionTimeEnd > QDateTime::currentDateTime() && sessions[globalCurrentSessionArrayPos].factSessionTimeStart <= QDateTime::currentDateTime() && new_matrix[sessions[globalCurrentSessionArrayPos].ka->posY][sessions[globalCurrentSessionArrayPos].ka->posX] == false && sessions[globalCurrentSessionArrayPos].cState == InProgress)
    {
        sessions[globalCurrentSessionArrayPos].answers += getRandomInt(1, 10);
        currentKosState = 1; // ошибок нет, лазер стреляет
    }
    else
    {
        if(currentKosState != 5)
        {
            if(sessions[globalCurrentSessionArrayPos].factSessionTimeEnd > QDateTime::currentDateTime() && new_matrix[sessions[globalCurrentSessionArrayPos].ka->posY][sessions[globalCurrentSessionArrayPos].ka->posX] == true && sessions[globalCurrentSessionArrayPos].cState == InProgress)
            {
                currentKosState = 6; // ошибок нет, лазер стреляет в облако
            }
            else
            {
                if(sessions[globalCurrentSessionArrayPos].cState != 0 && sessions[globalCurrentSessionArrayPos].cState != 1 && sessions[globalCurrentSessionArrayPos].cState != 2 && QDateTime::currentDateTime() < sessions[globalCurrentSessionArrayPos + 1].factSessionTimeStart)
                {
                    currentKosState = 4; // ожидание
                }
                else
                {
                    if(QDateTime::currentDateTime() > sessions[globalCurrentSessionArrayPos + 1].factSessionTimeStart && new_matrix[sessions[globalCurrentSessionArrayPos + 1].ka->posY][sessions[globalCurrentSessionArrayPos + 1].ka->posX] == true)
                    {
                        currentKosState = 2; // ошибок нет, облачно
                    }
                    else
                    {
                        currentKosState = 3; // есть ошибки
                    }
                }
            }
        }
    }
    if(cloud_count < 400 && currentKosState == 5)
    {
        currentKosState = 3; // есть ошибки
    }
    cloud_count = 0;
    // если закончился текущий сеанс
    if(sessions[globalCurrentSessionArrayPos].factSessionTimeEnd <= QDateTime::currentDateTime() || (sessions[globalCurrentSessionArrayPos].currentPlanningRule.pointsAmount != -1 && sessions[globalCurrentSessionArrayPos].answers >= sessions[globalCurrentSessionArrayPos].currentPlanningRule.pointsAmount * 180))
    {
        // этот сеанс заканчиваем
        if(!(sessions[globalCurrentSessionArrayPos].cState == Done) && !(sessions[globalCurrentSessionArrayPos].cState == FailedUnknown) && !(sessions[globalCurrentSessionArrayPos].cState == FailedTech) && !(sessions[globalCurrentSessionArrayPos].cState == FailedWeather))
        {
            if(sessions[globalCurrentSessionArrayPos].answers >= sessions[globalCurrentSessionArrayPos].currentPlanningRule.pointsAmount * 180 || sessions[globalCurrentSessionArrayPos].currentPlanningRule.pointsAmount == -1) //если ответов достаточно
            {
                sessions[globalCurrentSessionArrayPos].cState = Done; //вероятнее всего, статус будет Выполнен
                if(!getRandomInt(10)) //но кубик может решить иначе
                {
                    sessions[globalCurrentSessionArrayPos].cState = static_cast<CompletionState>(getRandomInt(3) + 3);
                }
            }
            else //если ответов недостаточно
            {
                if(sessions[globalCurrentSessionArrayPos].currentPlanningRule.pointsAmount != -1)
                {
                    if(currentKosState == 5 || new_matrix[sessions[globalCurrentSessionArrayPos].ka->posY][sessions[globalCurrentSessionArrayPos].ka->posX] == true)
                    {
                        sessions[globalCurrentSessionArrayPos].cState = static_cast<CompletionState>(4);
                    }
                    else
                    {
                        sessions[globalCurrentSessionArrayPos].cState = static_cast<CompletionState>(getRandomInt(1) + 5);   //кубик решает, какой будет статус из "неудачных"
                        currentKosState = 3;
                    }
                }
                else
                {
                    sessions[globalCurrentSessionArrayPos].cState = Done;
                }
            }
        }
        // записываем новый статус сеанса в его окно
        for (int i = 0; i < windows[sessions[globalCurrentSessionArrayPos].WindowId].second.sessions.length(); i++)
        {
            if(windows[sessions[globalCurrentSessionArrayPos].WindowId].second.sessions[i].second.first == sessions[globalCurrentSessionArrayPos].id)
            {
                windows[sessions[globalCurrentSessionArrayPos].WindowId].second.sessions[i].second.second = sessions[globalCurrentSessionArrayPos].cState;
            }
        }

        //если следующий сеанс существует
        if(globalCurrentSessionArrayPos + 1 <= sessions.length() - 1)
        {
            //если по времени следующий сеанс уже можно начать
            if(QDateTime::currentDateTime() >= sessions[globalCurrentSessionArrayPos + 1].factSessionTimeStart)
            {
                //ищем следующий спутник в зоне видимости
                int newGlobalCurrentSessionArrayPos = 0;
                Session newCurrentSession;
                bool init = false;
                for(int i = 1; newGlobalCurrentSessionArrayPos == 0 && (globalCurrentSessionArrayPos + i <= sessions.length() - 1) && (sessions[globalCurrentSessionArrayPos + i].factSessionTimeStart <= QDateTime::currentDateTime()) && newGlobalCurrentSessionArrayPos == 0; i++)
                {
                    if(sessions[globalCurrentSessionArrayPos + i].factSessionTimeEnd <= QDateTime::currentDateTime()) // если в запланированных сеансах попался тот, время которого истекло
                    {
                        sessions[globalCurrentSessionArrayPos + i].cState = static_cast<CompletionState>(getRandomInt(1) + 5);   //кубик решает, какой будет статус из "неудачных"
                    }
                    else
                    {
                        if((sessions[globalCurrentSessionArrayPos + i].cState == PlannedFixed || sessions[globalCurrentSessionArrayPos + i].cState == PlannedRange) && new_matrix[sessions[globalCurrentSessionArrayPos + i].ka->posY][sessions[globalCurrentSessionArrayPos + i].ka->posX] == false)
                        {
                            switch (satellitePlanningSettings.find(sessions[globalCurrentSessionArrayPos + i].ka).value().priority)
                            {
                            case 3:
                            {
                                newGlobalCurrentSessionArrayPos = globalCurrentSessionArrayPos + i;
                                Session tempNewSession = sessions[newGlobalCurrentSessionArrayPos];
                                while(i > 1)
                                {
                                    sessions[globalCurrentSessionArrayPos + i] = sessions[globalCurrentSessionArrayPos + i - 1];
                                    i--;
                                }
                                sessions[globalCurrentSessionArrayPos + 1] = tempNewSession;
                                break;
                            }
                            case 2:
                            {
                                if(init && satellitePlanningSettings.find(newCurrentSession.ka).value().priority < 2)
                                {
                                    Session newCurrentSession = sessions[globalCurrentSessionArrayPos + i];
                                }
                                else
                                {
                                    if(!init)
                                    {
                                        Session newCurrentSession = sessions[globalCurrentSessionArrayPos + i];
                                    }
                                }
                                if(sessions[globalCurrentSessionArrayPos + i + 1].factSessionTimeStart >= QDateTime::currentDateTime())
                                {
                                    newGlobalCurrentSessionArrayPos = globalCurrentSessionArrayPos + i;
                                    Session tempNewSession = sessions[newGlobalCurrentSessionArrayPos];
                                    while(i > 1)
                                    {
                                        sessions[globalCurrentSessionArrayPos + i] = sessions[globalCurrentSessionArrayPos + i - 1];
                                        i--;
                                    }
                                    sessions[globalCurrentSessionArrayPos + 1] = tempNewSession;                                     }
                                break;
                            }
                            case 1:
                            {
                                if(!init)
                                {
                                    Session newCurrentSession = sessions[globalCurrentSessionArrayPos + i];
                                }
                                if(sessions[globalCurrentSessionArrayPos + i + 1].factSessionTimeStart >= QDateTime::currentDateTime())
                                {
                                    newGlobalCurrentSessionArrayPos = globalCurrentSessionArrayPos + i;
                                    Session tempNewSession = sessions[newGlobalCurrentSessionArrayPos];
                                    while(i > 1)
                                    {
                                        sessions[globalCurrentSessionArrayPos + i] = sessions[globalCurrentSessionArrayPos + i - 1];
                                        i--;
                                    }
                                    sessions[globalCurrentSessionArrayPos + 1] = tempNewSession;                                     }
                                break;
                            }
                            default:
                                break;
                            }

                        }
                    }
                }
                globalCurrentSessionArrayPos++;
                // следущему присваиваем состояние в процессе
                sessions[globalCurrentSessionArrayPos].cState = InProgress;
                sessions[globalCurrentSessionArrayPos].answers = 0;
                qDebug() << "Started new session " << sessions[globalCurrentSessionArrayPos].factSessionTimeStart.toString("hh:mm:ss");

            }
        }
    }



    auto curDt = QDateTime::currentDateTime();
    //manual mode windows and weather
    foreach(auto satId, manualModeInfos.keys())
    {
        if(manualModeInfos[satId]->visibility.end < curDt)
        {
            if(getRandomInt(1))
                manualModeInfos[satId]->visibility = TimeSpan(curDt,curDt.addSecs(getRandomInt(599) + 1));
            else
                manualModeInfos[satId]->visibility = TimeSpan(curDt.addSecs(getRandomInt(600)),curDt.addSecs(getRandomInt(599) + 1 + 600));
            manualModeInfos[satId]->infoText = "";
            manualModeInfos[satId]->sessionsThisWindow = 0;
        }

        if(getRandomInt(50))
            manualModeInfos[satId]->weatherVal = static_cast<Weather>(getRandomInt(1));
    }

    //manual session points
    if(manualSessionIsRunning)
    {
        curManualSessionPoints += getRandomInt(20);
        if(manualModeInfos.contains(curManualSessionSatId))
            manualModeInfos[curManualSessionSatId]->infoText = QString("Идут измерения: %1 точек").arg(curManualSessionPoints);
    }
}

QPair<int, int> DataManager::getKosState()
{
    if(weatherWidget.errors)
    {
        return qMakePair(3, getRandomInt(1,4));
    }
    if(currentKosState == 3)
    {
        errorsCount++;
    }
    return qMakePair(currentKosState, errorsCount);
}



