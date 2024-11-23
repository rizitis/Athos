/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>

#include <core/kdeconnectplugin.h>

#include "mprisremoteplayer.h"

#define PACKET_TYPE_MPRIS_REQUEST QStringLiteral("kdeconnect.mpris.request")
#define PACKET_TYPE_MPRIS QStringLiteral("kdeconnect.mpris")

class MprisRemotePlugin : public KdeConnectPlugin
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kdeconnect.device.mprisremote")
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY propertiesChanged)
    Q_PROPERTY(int length READ length NOTIFY propertiesChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY propertiesChanged)
    Q_PROPERTY(int position READ position WRITE setPosition NOTIFY propertiesChanged)
    Q_PROPERTY(QStringList playerList READ playerList NOTIFY propertiesChanged)
    Q_PROPERTY(QString player READ player WRITE setPlayer)
    Q_PROPERTY(QString title READ title NOTIFY propertiesChanged)
    Q_PROPERTY(QString artist READ artist NOTIFY propertiesChanged)
    Q_PROPERTY(QString album READ album NOTIFY propertiesChanged)
    Q_PROPERTY(bool canSeek READ canSeek NOTIFY propertiesChanged)

public:
    using KdeConnectPlugin::KdeConnectPlugin;

    long position() const;
    int volume() const;
    int length() const;
    bool isPlaying() const;
    QStringList playerList() const;
    QString player() const;
    QString title() const;
    QString artist() const;
    QString album() const;
    bool canSeek() const;

    void setVolume(int volume);
    void setPosition(int position);
    void setPlayer(const QString &player);

    void receivePacket(const NetworkPacket &np) override;
    QString dbusPath() const override;

    Q_SCRIPTABLE void seek(int offset) const;
    Q_SCRIPTABLE void requestPlayerList();
    Q_SCRIPTABLE void sendAction(const QString &action);

Q_SIGNALS:
    Q_SCRIPTABLE void propertiesChanged();

private:
    void requestPlayerStatus(const QString &player);

    QString m_currentPlayer;
    QMap<QString, MprisRemotePlayer *> m_players;
};
