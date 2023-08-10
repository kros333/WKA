#include "datamanager.h"
#include <QDebug>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "weatherwidget.h"
#include <algorithm>
#include "math.h"

//qDebug() << debug;
//debug++;

DataManager::DataManager(quint16 daysToLeft, quint16 daysToRight)
{



    weatherWidget.show();
    //    weatherWidget.setModal(true);
    //    weatherWidget.exec();




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
//    int LastKaId[40]{-1,-1,-1,-1,-1,-1,-1};
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
//        switch (currentSatellitePlanningSetting.priority)
//        {
//        case 0:
//            break;
//        default:
//            break;
//        }


        vis = VisibilityWindowSummary();
        vis.startEnd.start = crawlDt;
        vis.startEnd.end = crawlDt.addSecs(getRandomInt(30, 240)*60);
        LastKaVis[currentKaId] = vis.startEnd.end;
        //начало окна всегда с солнечной погодой (почему? пока что не так)
        vis.weathers << VisibilityWindowSummary::WeatherSpan(static_cast<Weather>(getRandomInt(2)), TimeSpan(crawlDt, crawlDt.addSecs(getRandomInt(1, vis.startEnd.end.toSecsSinceEpoch()/60 - vis.startEnd.start.toSecsSinceEpoch()/60)*60)));///////////////////////////////////////////////////
        while(vis.weathers.last().span.end != vis.startEnd.end)
        {
            vis.weathers << VisibilityWindowSummary::WeatherSpan(static_cast<Weather>(getRandomInt(2)), TimeSpan(vis.weathers.last().span.end, vis.weathers.last().span.end.addSecs(getRandomInt(1, vis.startEnd.end.toSecsSinceEpoch()/60 - vis.startEnd.start.toSecsSinceEpoch()/60)*60)));///////////////////////////////////////////////////
            if(vis.weathers.last().span.end > vis.startEnd.end)
            {
                vis.weathers.last().span.end = vis.startEnd.end;
            }
        }

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
            if(s.cState == InProgress || s.cState == PlannedFixed || s.cState == PlannedRange) s.cState = Done;

            if(s.factSessionTimeStart > QDateTime::currentDateTime()) s.cState = getRandomInt(1) ? PlannedFixed : PlannedRange;

//            if(globalCurrentSessionArrayPos == 0 && QDateTime::currentDateTime() <= s.factSessionTimeStart)
//                globalCurrentSessionArrayPos = sessions.count();


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
            vis.sessions << qMakePair(TimeSpan(s.factSessionTimeStart, s.factSessionTimeEnd), s.cState);
            sessionsLeftInWindow--;
        }
        windows << qMakePair(currentKaId, vis);


//        // генерация окна
//        // подготовка данных
//        //если в данный момент не генерируем окно и по кубику надо начать генерировать

//        vis = VisibilityWindowSummary();
//        vis.startEnd.start = crawlDt;
//        //начало окна всегда с солнечной погодой
//        vis.weathers << VisibilityWindowSummary::WeatherSpan(static_cast<Weather>(getRandomInt(2)), TimeSpan(crawlDt, crawlDt));///////////////////////////////////////////////////
//        // 3 сессии
//        sessionsLeftInWindow = 3;

//        //обновляем правую границу текущей погоды
//        if(vis.weathers.length() > 0) vis.weathers.last().span.end = crawlDt;

//        //если кубик сказад что надо менять погоду
//        if(!getRandomInt(15))
//        {
//            auto newWeather = static_cast<Weather>(getRandomInt(2));
//            if(newWeather != vis.weathers.last().weather)
//                vis.weathers << VisibilityWindowSummary::WeatherSpan(newWeather, TimeSpan(crawlDt, crawlDt));
//        }

//        //если еще надо сессий и кубик скозал
//        if(sessionsLeftInWindow > 0 && !getRandomInt(9))
//        {
//            auto cs = static_cast<CompletionState>(getRandomInt(6));
//            //if(cs == InProgress) cs = Done;
//            if(crawlDt > QDateTime::currentDateTime()) cs = getRandomInt(1) ? PlannedFixed : PlannedRange;
//            vis.sessions << qMakePair(TimeSpan(crawlDt, crawlDt.addSecs(180)), cs);
//            sessionsLeftInWindow--;
//        }
//        // определение конца окна
//        if(sessionsLeftInWindow == 0 && !getRandomInt(6))
//        {
//            vis.startEnd.end = crawlDt;
//            if(vis.weathers[0].span.start == vis.weathers[0].span.end) vis.weathers.removeAt(0);
//            if(vis.weathers[vis.weathers.length()-1].span.start == vis.weathers[vis.weathers.length()-1].span.end) vis.weathers.removeAt(vis.weathers.length()-1);

//        }


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
        qDebug() << "InProgress";
        sessions[globalCurrentSessionArrayPos].cState = InProgress;
        sessions[globalCurrentSessionArrayPos].answers = getRandomInt(0, 140);
        qDebug() << sessions[globalCurrentSessionArrayPos].answers;
    }
    else
    {

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
        mid++;
    }
    return result;
}

QMap<int /*satId*/, QList<VisibilityWindowSummary>>DataManager::getSessionsRibbon(QDateTime targetDt, int limitBackInHours, int limitForwardInHours)
{
    //    QMap<int /*satId*/, QList<VisibilityWindowSummary>> result;
    //    targetDt.setTime(QTime(targetDt.time().hour(),0)); // отбрасываем минуты запроенного времени. Становимся на начало часа
    //    QDateTime leftestHour = targetDt.addSecs(limitBackInHours * 60 * 60 * -1); //hours * 60 = minutes * 60 = seconds
    //    QDateTime rightestHour = targetDt.addSecs(limitForwardInHours * 60 * 60).addSecs(60 * 60 -1); //эта граница считается так: начало часа targetDt + кол-во часов справа(lmitForwardInHours) + 59 мин 59 сек

    //    int maxSatellites = getRandomInt(10) + 30;

    //    for(int i=0;i<maxSatellites;i++)
    //    {
    //        int windowsAmount = getRandomInt(2) + 1; // 1..3 windows
    //        qint64 diff = leftestHour.secsTo(rightestHour);
    //        for(int k = 0; k < windowsAmount; k++)
    //        {
    //            VisibilityWindowSummary vis;
    //            vis.startEnd.start = leftestHour.addSecs(((double)diff/(double)windowsAmount) - (((double)diff/(double)windowsAmount)/4.0*3.0));
    //            vis.startEnd.end = leftestHour.addSecs(((double)diff/(double)windowsAmount) - (((double)diff/(double)windowsAmount)/4.0*1.0));

    //            // weather generating
    //            QDateTime curDtInWindow = vis.startEnd.start;
    //            while(curDtInWindow < vis.startEnd.end)
    //            {
    //                int dice = getRandomInt(9);
    //                if(dice == 0)
    //                {
    //                    Weather newWeather = static_cast<Weather>(getRandomInt(2));
    //                    if(vis.clear.length() > 0) vis.clear.last().end = curDtInWindow;
    //                    if(vis.rainy.length() > 0) vis.rainy.last().end = curDtInWindow;
    //                    if(vis.cloudy.length() > 0) vis.cloudy.last().end = curDtInWindow;
    //                    switch (newWeather) {
    //                    case Weather::Cloudy:
    //                        vis.cloudy.append(TimeSpan(curDtInWindow, vis.startEnd.end)); // end is temporary val , supposed to be overriden. or not.
    //                        break;
    //                    case Weather::Rain:
    //                        vis.rainy.append(TimeSpan(curDtInWindow, vis.startEnd.end)); // end is temporary val , supposed to be overriden. or not.
    //                        break;
    //                    case Weather::Clear:
    //                        vis.clear.append(TimeSpan(curDtInWindow, vis.startEnd.end)); // end is temporary val , supposed to be overriden. or not.
    //                        break;
    //                    }

    //                }
    //                curDtInWindow = curDtInWindow.addSecs(60);
    //            }

    //            //sessions generating
    //            vis.sessions.append(qMakePair(TimeSpan(vis.startEnd.start.addSecs((double)(vis.startEnd.start.secsTo(vis.startEnd.end))/4.0*1.0), vis.startEnd.start.addSecs((double)(vis.startEnd.start.secsTo(vis.startEnd.end))/4.0*1.0).addSecs(600)), static_cast<CompletionState>(getRandomInt(5))));
    //            vis.sessions.append(qMakePair(TimeSpan(vis.startEnd.start.addSecs((double)(vis.startEnd.start.secsTo(vis.startEnd.end))/4.0*2.0), vis.startEnd.start.addSecs((double)(vis.startEnd.start.secsTo(vis.startEnd.end))/4.0*2.0).addSecs(600)), static_cast<CompletionState>(getRandomInt(5))));
    //            vis.sessions.append(qMakePair(TimeSpan(vis.startEnd.start.addSecs((double)(vis.startEnd.start.secsTo(vis.startEnd.end))/4.0*3.0), vis.startEnd.start.addSecs((double)(vis.startEnd.start.secsTo(vis.startEnd.end))/4.0*3.0).addSecs(600)), static_cast<CompletionState>(getRandomInt(5))));

    //            result[satellites[i]->id] << vis;
    //        }

    //    }
    //    return result;


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

    SatellitePlanningRule rule;
    rule.priority = priorityVal;
    rule.maxSessions = newMaxSessions;
    rule.minSessions = newMinSessions;
    rule.pointsAmount = newPointsAmount;
    rule.minSessionInterval = newMinSessionInterval;

    satellitePlanningSettings[sat] = rule;
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

    QDateTime dt = QDateTime::currentDateTime();
    dt.setTime(QTime(dt.time().hour() + getRandomInt(1,9),0,0));

    for(int i=0; i<10; i++)
    {
        result << TimeSpan(dt,dt.addSecs(3600));
        dt = dt.addSecs(3600 * getRandomInt(12,24));
    }

    return result;
}

//TODO хранить длительность для CompletionState::PlannedRange сеансов
bool DataManager::writeNewSatelliteUserPlannedSession(int satId, QDateTime start, QDateTime end, bool isApproximateTime, int durationInSeconds, qint64 editSessionId)
{
    Satellite* sat = getSatellite(satId);
    if(sat == nullptr) return false;

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

    s.visibilityStart = s.factSessionTimeStart.addSecs(-3600);
    s.visibilityEnd = s.factSessionTimeStart.addSecs(3600);

    if(editSessionId != -1)
    {
        for(int i=0; i < userPlannedSessions.length();i++)
        {
            if(userPlannedSessions[i].id == editSessionId)
            {
                userPlannedSessions[i] = s;
                return true;
            }
        }
        return false; // если в цикле не нашлось такого id
    }

    userPlannedSessions << s;

    return true;
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
    if(secsToUpdateWeather == currentSecsToUpdateWeather)
    {
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
        if(cloud_count == 400)
        {
            currentKosState = 5;
        }
        cloud_count = 0;
        currentSecsToUpdateWeather = 1;
    }
    else
    {
        currentSecsToUpdateWeather++;
    }
    //закончили обновление погоды






    if(sessions[globalCurrentSessionArrayPos].factSessionTimeEnd > QDateTime::currentDateTime() && sessions[globalCurrentSessionArrayPos].factSessionTimeStart <= QDateTime::currentDateTime() && matrix[sessions[globalCurrentSessionArrayPos].ka->posY][sessions[globalCurrentSessionArrayPos].ka->posX] == false && sessions[globalCurrentSessionArrayPos].cState == InProgress)
    {
        sessions[globalCurrentSessionArrayPos].answers += getRandomInt(1, 10);
        currentKosState = 1;
    }
    else
    {
        if(currentKosState != 5)
        {
            if(sessions[globalCurrentSessionArrayPos].factSessionTimeEnd > QDateTime::currentDateTime() && matrix[sessions[globalCurrentSessionArrayPos].ka->posY][sessions[globalCurrentSessionArrayPos].ka->posX] == true && sessions[globalCurrentSessionArrayPos].cState == InProgress)
            {
                currentKosState = 6;
            }
            else
            {
                if(sessions[globalCurrentSessionArrayPos].cState != 0 && sessions[globalCurrentSessionArrayPos].cState != 1 && sessions[globalCurrentSessionArrayPos].cState != 2 && QDateTime::currentDateTime() < sessions[globalCurrentSessionArrayPos + 1].factSessionTimeStart)
                {
                    currentKosState = 4;
                }
                else
                {
                    if(QDateTime::currentDateTime() > sessions[globalCurrentSessionArrayPos + 1].factSessionTimeStart && matrix[sessions[globalCurrentSessionArrayPos + 1].ka->posY][sessions[globalCurrentSessionArrayPos + 1].ka->posX] == true)
                    {
                        currentKosState = 2;
                    }
                    else
                    {
                        currentKosState = 3;
                    }
                }
            }
        }
    }

    // если закончился текущий сеанс
    if(sessions[globalCurrentSessionArrayPos].factSessionTimeEnd <= QDateTime::currentDateTime() || sessions[globalCurrentSessionArrayPos].answers >= sessions[globalCurrentSessionArrayPos].currentPlanningRule.pointsAmount * 180)
    {
        //qDebug() << "cur session " << sessions[globalCurrentSessionArrayPos].factSessionTimeStart;
        //qDebug() << "nxt session " << sessions[globalCurrentSessionArrayPos + 1].factSessionTimeStart;

        // этот сеанс заканчиваем
        if(!(sessions[globalCurrentSessionArrayPos].cState == Done) && !(sessions[globalCurrentSessionArrayPos].cState == FailedUnknown) && !(sessions[globalCurrentSessionArrayPos].cState == FailedTech) && !(sessions[globalCurrentSessionArrayPos].cState == FailedWeather))
        {
            if(sessions[globalCurrentSessionArrayPos].answers >= sessions[globalCurrentSessionArrayPos].currentPlanningRule.pointsAmount * 180) //если ответов достаточно
            {
                sessions[globalCurrentSessionArrayPos].cState = Done; //вероятнее всего, статус будет Выполнен
                if(!getRandomInt(10)) //но кубик может решить иначе
                {
                    sessions[globalCurrentSessionArrayPos].cState = static_cast<CompletionState>(getRandomInt(3) + 3);
                }
            }
            else //если ответов недостаточно
            {
                if(currentKosState == 5 || matrix[sessions[globalCurrentSessionArrayPos].ka->posY][sessions[globalCurrentSessionArrayPos].ka->posX] == true)
                {
                    sessions[globalCurrentSessionArrayPos].cState = static_cast<CompletionState>(4);
                }
                else
                {
                    sessions[globalCurrentSessionArrayPos].cState = static_cast<CompletionState>(getRandomInt(1) + 5);   //кубик решает, какой будет статус из "неудачных"
                    currentKosState = 3;
                }
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
    //        for(int i = 1; (globalCurrentSessionArrayPos - i >= 0) && (sessions[globalCurrentSessionArrayPos - i].factSessionTimeEnd > QDateTime::currentDateTime().addSecs(60)); i++)
    //        {
    //            if((sessions[globalCurrentSessionArrayPos - i].cState == PlannedFixed || sessions[globalCurrentSessionArrayPos - i].cState == PlannedRange) && matrix[sessions[globalCurrentSessionArrayPos - i].ka->posY][sessions[globalCurrentSessionArrayPos - i].ka->posX] == false)
    //            {
    //                newGlobalCurrentSessionArrayPos = globalCurrentSessionArrayPos - i;
    //            }
    //        }
    //        if(newGlobalCurrentSessionArrayPos == 0)
    //        {
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
                        if((sessions[globalCurrentSessionArrayPos + i].cState == PlannedFixed || sessions[globalCurrentSessionArrayPos + i].cState == PlannedRange) && matrix[sessions[globalCurrentSessionArrayPos + i].ka->posY][sessions[globalCurrentSessionArrayPos + i].ka->posX] == false)
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
    //        }
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

    if(currentKosState == 3)
    {
        errorsCount++;
    }
    return qMakePair(currentKosState, errorsCount);
}



