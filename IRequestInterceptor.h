#pragma once

#include <QtNetwork/QNetworkRequest>

class IRequestInterceptor
{
public:
    virtual void operator () (QNetworkRequest &reply) const = 0;
};
