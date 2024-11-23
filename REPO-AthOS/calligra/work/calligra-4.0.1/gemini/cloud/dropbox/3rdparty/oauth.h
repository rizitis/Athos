/*

    SPDX-FileCopyrightText: 2011 Cuong Le <metacuong@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef OAUTH_H
#define OAUTH_H

#include <QPair>
#include <QString>

#include <QNetworkRequest>

class QUrl;

#define OAUTH_VERSION "1.0"

class OAuth
{
public:
    OAuth();

    QString m_consumer_key;
    QString m_consumer_secret;
    QString m_token;
    QString m_secret;

    void sign(QString method, QNetworkRequest *networkRequest);

private:
    QString oauth_timestamp();
    QString oauth_consumer_key();
    QString oauth_signature_method();
    QString oauth_token();
    QString oauth_version();
    QString oauth_signature(QString method, QUrl *url, QString oAuthHeader);
    QString SHA1(QString base, QString key);
};

#endif // OAUTH_H
