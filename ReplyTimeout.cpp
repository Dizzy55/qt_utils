#include <QtNetwork/QNetworkReply>
#include <QBasicTimer>
#include <QTimerEvent>

#include "ReplyTimeout.h"

ReplyTimeout::ReplyTimeout(QNetworkReply *reply, int timeout, ReplyTimeout::HandleMethod method)
: QObject(reply),
  mMethod(method),
  mReply(reply)
{
    Q_ASSERT(reply != nullptr);

    if (reply == nullptr || !reply->isRunning())
        return;

    connect(reply, &QNetworkReply::finished, this, &QObject::deleteLater);

    mTimer.start(timeout, this);
}

void ReplyTimeout::timerEvent(QTimerEvent *event)
{
    if (!mTimer.isActive() || event->timerId() != mTimer.timerId())
        return;

    if (!mReply->isRunning())
        return;

    if (mMethod == HandleMethod::Close)
        mReply->close();
    else if (mMethod == HandleMethod::Abort)
        mReply->abort();

    mTimer.stop();
}
