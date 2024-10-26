#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

class IRequestInterceptor;
class QSslConfiguration;

class HttpManager : public QObject
{
    Q_OBJECT

public:
    enum Method {
        POST,
        GET,
        PATCH,
        PUT,
        DELETE,
    };
    Q_ENUM(Method)

    using completeFunctionType = std::function<void(const QByteArray &, const QList<QNetworkReply::RawHeaderPair> &)>;
    using errorFunctionType = std::function<void(const QString &)>;

public:
    explicit HttpManager(const QString &host,
                         int port,
                         bool isHttps,
                         QSslConfiguration *sslConfiguration = nullptr,
                         QObject *parent = nullptr);

    virtual ~HttpManager();

    void addRequestInterceptor(IRequestInterceptor *requestInterceptor);

    void sendRequest(Method method,
                     const QString &apiString,
                     const completeFunctionType &onCompleteCallback,
                     const errorFunctionType &onErrorCallback,
                     const QByteArray &data = QByteArray(),
                     const QVariantMap &queries = QVariantMap(),
                     const QVariantMap &headers = QVariantMap());

    bool isNoError(QNetworkReply *reply) const;

    QString getHttpErrorString(QNetworkReply *reply) const;

private:
    QNetworkRequest createRequest(const QString &apiStr,
                                  const QVariantMap &queries = QVariantMap(),
                                  const QVariantMap &additionalHeaders = QVariantMap()) const;

    QNetworkReply *sendCustomRequest(QNetworkAccessManager *manager,
                                     QNetworkRequest &request,
                                     Method type,
                                     const QByteArray &data) const;
private:
    QString mHost;
    int mPort;
    bool mIsHttps;
    QSslConfiguration *mSslConfiguration;
    int mUserTimeuotMs = 30 * 1000;

    QVector<IRequestInterceptor *> mRequestInterceptors;

    QNetworkAccessManager mManager;
};
