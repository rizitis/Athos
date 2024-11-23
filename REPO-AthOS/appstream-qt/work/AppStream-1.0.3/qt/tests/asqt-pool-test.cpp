/*
 * Copyright (C) 2016-2024 Matthias Klumpp <matthias@tenstral.net>
 * Copyright (C) 2023 Kai Uwe Broulik <kde@broulik.de>
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the license, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtTest>
#include <QObject>
#include <QTemporaryFile>
#include "pool.h"
#include "testpaths.h"

class PoolReadTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRead01();
    void testLoadAsync();
};

using namespace AppStream;

static std::unique_ptr<Pool> createPool()
{
    // set up the data pool to read our sample data, without localization
    auto pool = std::make_unique<Pool>();

    pool->resetExtraDataLocations();
    pool->setLocale("C");
    pool->setLoadStdDataLocations(false);
    pool->addExtraDataLocation(AS_SAMPLE_DATA_PATH, Metadata::FormatStyleCatalog);

    // don't load system metainfo/desktop files
    auto flags = pool->flags();
    flags &= ~Pool::FlagLoadOsDesktopFiles;
    flags &= ~Pool::FlagLoadOsMetainfo;
    flags &= ~Pool::FlagIgnoreCacheAge;
    pool->setFlags(flags);

    return pool;
}

void PoolReadTest::testRead01()
{
    auto pool = createPool();

    // temporary cache location, so we don't use any system cache ever during tests
    QTemporaryDir cacheDir;
    QVERIFY(cacheDir.isValid());

    // use clean caches
    pool->overrideCacheLocations(cacheDir.path(), nullptr);

    // read metadata
    bool ret = pool->load();
    if (!ret)
        qWarning() << pool->lastError();
    QVERIFY(ret);

    auto cbox = pool->components();
    QCOMPARE(cbox.size(), 20);

    cbox = pool->componentsById("org.neverball.Neverball");
    QCOMPARE(cbox.size(), 1);

    auto cpt = cbox.indexSafe(0);
    QVERIFY(!cpt->id().isEmpty());

    QCOMPARE(cpt->name(), QLatin1String("Neverball"));
}

void PoolReadTest::testLoadAsync()
{
    auto pool = createPool();

    QTemporaryDir cacheDir;
    QVERIFY(cacheDir.isValid());

    pool->overrideCacheLocations(cacheDir.path(), nullptr);

    QSignalSpy loadedSpy(pool.get(), &Pool::loadFinished);
    QVERIFY(loadedSpy.isValid());

    pool->loadAsync();

    QVERIFY(loadedSpy.wait());
    QCOMPARE(loadedSpy.count(), 1);
    const QList<QVariant> arguments = loadedSpy.takeFirst();
    QCOMPARE(arguments.first().toBool(), true);

    auto cbox = pool->components();
    QCOMPARE(cbox.size(), 20);

    auto cpts = pool->componentsById("org.neverball.Neverball").toList();
    QCOMPARE(cpts.size(), 1);

    auto cpt = cpts[0];
    QVERIFY(!cpt.id().isEmpty());

    QCOMPARE(cpt.name(), QLatin1String("Neverball"));
}

QTEST_MAIN(PoolReadTest)

#include "asqt-pool-test.moc"
