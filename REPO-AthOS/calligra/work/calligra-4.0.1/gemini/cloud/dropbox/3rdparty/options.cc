/*

    SPDX-FileCopyrightText: 2011 Cuong Le <metacuong@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "options.h"

#include <QSettings>
#include <QStringList>
#include <QDebug>

Options::Options(QObject *parent) :
    QObject(parent)
{
    get_transfers_auto();
    get_screen_orientation();
    get_push_notification();
}

bool Options::is_transfers_auto() const {
    return m_is_transfers_auto;
}

int  Options::screen_orientation() const {
    return m_screen_orientation;
}

bool  Options::is_push_notification() const {
    return m_is_push_notification;
}


void Options::get_transfers_auto() {
    QSettings settings;
    settings.beginGroup("transfers");

    if (settings.childKeys().indexOf("type") == -1){
        m_is_transfers_auto = false;
        return; //default is manual
    }
    m_is_transfers_auto = settings.value("type").toBool();
}

void Options::get_screen_orientation() {
    QSettings settings;
    settings.beginGroup("screen_orientation");

    if(settings.childKeys().indexOf("type") == -1){
        m_screen_orientation = 2;
        return;//default is auto
    }

    m_screen_orientation = settings.value("type").toInt();
}

void Options::get_push_notification() {
    QSettings settings;
    settings.beginGroup("push_notification");

    if(settings.childKeys().indexOf("type") == -1){
        m_is_push_notification = true;
        return;//default is auto
    }

    m_is_push_notification = settings.value("type").toBool();
}

void Options::set_transfers_auto(const bool &val){
    QSettings settings;
    settings.setValue("transfers/type", val);
    m_is_transfers_auto = val;
}

void Options::set_screen_orientation(const int &val){
    QSettings settings;
    settings.setValue("screen_orientation/type", val);
    m_screen_orientation = val;
}

void Options::set_push_notification(const bool &val){
    QSettings settings;
    settings.setValue("push_notification/type", val);
    m_is_push_notification = val;
}
