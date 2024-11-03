#include <chrono>

#include <QUrlQuery>
#include <QUrl>
#include <QBuffer>
#include <QSslConfiguration>

#include "IRequestInterceptor.h"
#include "MetaEnum.h"
#include "ReplyTimeout.h"

#include "HttpManager.h"

HttpManager::HttpManager(const QString &host,
                         int port,
                         bool isHttps,
                         QSslConfiguration *sslConfiguration,
                         QObject *parent)
: QObject(parent),
  mHost(host),
  mPort(port),
  mIsHttps(isHttps),
  mSslConfiguration(sslConfiguration)
{
}

HttpManager::~HttpManager()
{
}

void HttpManager::addRequestInterceptor(IRequestInterceptor *requestInterceptor)
{
    mRequestInterceptors.push_back(requestInterceptor);
}

void HttpManager::sendRequest(Method method,
                              const QString &apiString,
                              const completeFunctionType &onCompleteCallback,
                              const errorFunctionType &onErrorCallback,
                              const QByteArray &data,
                              const QVariantMap &queries,
                              const QVariantMap &headers)
{
    qInfo() << QString("**** Request: %1. Type: %2")
                   .arg(apiString)
                   .arg(MetaEnum::enumToString(method));

    QNetworkRequest request = createRequest(apiString, queries, headers);

    QNetworkReply *reply;

    switch (method) {
    case Method::POST: {
        qInfo() << QString("**** Data: %1")
                       .arg(QString::fromUtf8(data.data()).toStdString().c_str());
        reply = mManager.post(request, data);
        break;
    }
    case Method::GET:
        reply = mManager.get(request);
        break;
    case Method::PUT: {
        qInfo() << QString("**** Data: %1")
                       .arg(QString::fromUtf8(data.data()).toStdString().c_str());
        reply = mManager.put(request, data);
        break;
    }
    case Method::DELETE:
        if (data.isEmpty())
            reply = mManager.deleteResource(request);
        else
            reply = sendCustomRequest(&mManager, request, method, data);
        break;
    case Method::PATCH:
        reply = sendCustomRequest(&mManager, request, method, data);
        break;
    default:
        reply = nullptr;
        Q_ASSERT(false);
    }

    // Enable user timeout
    new ReplyTimeout(reply, mUserTimeuotMs);

    auto start = std::chrono::high_resolution_clock::now();

    auto onReplyFinished = [this, onCompleteCallback, onErrorCallback, reply, start] {
        std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start;
        qInfo() << QString("%1 response elapsed time: %2 s.")
                   .arg(reply->url().toString())
                   .arg(elapsed.count());

        // Check HTTP error
        if (isNoError(reply)) {
            qInfo() << "Reply acquired";

            if (onCompleteCallback != nullptr)
                onCompleteCallback(reply->readAll(), reply->rawHeaderPairs());

        } else {
            QString errorString = getHttpErrorString(reply);

            QNetworkReply::NetworkError replyError = reply->error();
            int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            qWarning() << "**** Reply error:" << errorString;

            if (onErrorCallback != nullptr)
                onErrorCallback(replyError, httpCode, errorString);
        }

        reply->close();
        reply->deleteLater();
    };

    connect(reply, &QNetworkReply::finished, this, onReplyFinished);
}

QString HttpManager::getHttpErrorString(QNetworkReply *reply) const
{
    QNetworkReply::NetworkError replyNetworkError = reply->error();

    if (replyNetworkError == QNetworkReply::NoError)
        return "";

    // For translation purpose. See enum NetworkError
    const QMap<QNetworkReply::NetworkError, QString> descriptionNetworkErrors = {
        // network layer errors [relating to the destination server] (1-99):
        {QNetworkReply::HostNotFoundError, tr("Host not found")},           // 3
        {QNetworkReply::TimeoutError, tr("Timeout")},                       // 4
        {QNetworkReply::OperationCanceledError, tr("Timeout")},             // 5 - User timeout)
        {QNetworkReply::UnknownNetworkError, tr("Connection unavailable")}, // 99
        // proxy errors (101-199):
        // ...
        // content errors (201-299):
        {QNetworkReply::ContentNotFoundError, tr("Content not found")},     // 203
        // protocol errors (301-399)
        {QNetworkReply::ProtocolInvalidOperationError, tr("Protocol invalid operation")}, // 302
        // Server side errors (401-499)
        {QNetworkReply::InternalServerError, tr("Internal server error")},  // 401
    };

    if (descriptionNetworkErrors.contains(replyNetworkError))
        return descriptionNetworkErrors[replyNetworkError];

    int httpError = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return tr("Server HTTP error code: %1 (Internal code: %2)")
        .arg(httpError)
        .arg(replyNetworkError);
}

QNetworkRequest HttpManager::createRequest(const QString &apiStr,
                                           const QVariantMap &queries,
                                           const QVariantMap &additionalHeaders) const
{
    const QString httpTemplate = "http%1://%2:%3%4";

    const QString urlString = QString(httpTemplate)
                                .arg(mIsHttps ? "s" : "")
                                .arg(mHost)
                                .arg(mPort)
                                .arg(apiStr);
    QUrl url(urlString);

    QUrlQuery query;
    for (const QString &key : queries.keys())
        query.addQueryItem(key, queries.value(key).toString());
    url.setQuery(query.query());

    QNetworkRequest request(url);

    // Common headers
    for (IRequestInterceptor *inretceptor : mRequestInterceptors)
        inretceptor->operator ()(request);

    // Additional headers
    request.setRawHeader("Content-Type", "application/json");
    for (const QString &key : additionalHeaders.keys()) {
        QString value = additionalHeaders.value(key).toString();
        request.setRawHeader(QByteArray(key.toStdString().c_str(), key.length()),
                             QByteArray(value.toStdString().c_str(), value.length()));
    }

    if (mSslConfiguration != nullptr)
        request.setSslConfiguration(*mSslConfiguration);

    qInfo() << "**** Request created:" << request.url();
    return request;
}

QNetworkReply *HttpManager::sendCustomRequest(QNetworkAccessManager *manager,
                                              QNetworkRequest &request,
                                              Method method,
                                              const QByteArray &data) const
{
    QString methodString = MetaEnum::enumToString(method);
    request.setRawHeader("HTTP", methodString.toUtf8());

    QBuffer *buffer = new QBuffer();
    buffer->setData(data);
    buffer->open(QIODevice::ReadOnly);
    QNetworkReply *reply =  manager->sendCustomRequest(request,
                                                       methodString.toUtf8(),
                                                       buffer);
    buffer->setParent(reply);

    return reply;
}

bool HttpManager::isNoError(QNetworkReply *reply) const
{
    QNetworkReply::NetworkError replyError = reply->error();
    int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    return replyError == QNetworkReply::NoError && httpCode == 200;
}
