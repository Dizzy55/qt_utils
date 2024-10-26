#pragma once

#include <QObject>
#include <QBasicTimer>

class QNetworkReply;
class QBasicTimer;

class ReplyTimeout : public QObject
{
  Q_OBJECT

public:
    enum class HandleMethod
    {
        Abort,
        Close
    };

    ReplyTimeout(QNetworkReply *reply, int timeout, HandleMethod method = HandleMethod::Abort);

private:
    void timerEvent(QTimerEvent *event);

private:
    HandleMethod mMethod;
    QNetworkReply *mReply;

    QBasicTimer mTimer;
};
