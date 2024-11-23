/**
 * SPDX-FileCopyrightText: 2018 Nicolas Fella <nicolas.fella@gmx.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#pragma once

#include <QDBusConnection>
#include <QString>

class NetworkPacket;
class MprisRemotePlugin;

class MprisRemotePlayer : public QObject
{
    Q_OBJECT

public:
    explicit MprisRemotePlayer(QString id, MprisRemotePlugin *plugin);
    ~MprisRemotePlayer() override;

    void parseNetworkPacket(const NetworkPacket &np);
    long position() const;
    void setPosition(long position);
    int volume() const;
    long length() const;
    bool playing() const;
    QString title() const;
    QString artist() const;
    QString album() const;
    QString identity() const;

    bool canSeek() const;
    bool canPlay() const;
    bool canPause() const;
    bool canGoPrevious() const;
    bool canGoNext() const;

    QDBusConnection &dbus();

Q_SIGNALS:
    void controlsChanged();
    void trackInfoChanged();
    void positionChanged();
    void volumeChanged();
    void playingChanged();

private:
    QString id;
    bool m_playing;
    bool m_canPlay;
    bool m_canPause;
    bool m_canGoPrevious;
    bool m_canGoNext;
    int m_volume;
    long m_length;
    long m_lastPosition;
    qint64 m_lastPositionTime;
    QString m_title;
    QString m_artist;
    QString m_album;
    bool m_canSeek;

    // Use an unique connection for every player, otherwise we can't distinguish which mpris player is being controlled
    QString m_dbusConnectionName;
    QDBusConnection m_dbusConnection;
};
