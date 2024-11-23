/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2013-2014 Yue Liu <yue.liu@mail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KoFileDialog.h"
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QImageReader>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QMimeDatabase>
#include <QMimeType>
#include <QUrl>

class Q_DECL_HIDDEN KoFileDialog::Private
{
public:
    Private(QWidget *parent_, KoFileDialog::DialogType dialogType_, const QString &caption_, const QString &defaultDir_, const QString &dialogName_)
        : parent(parent_)
        , type(dialogType_)
        , dialogName(dialogName_)
        , caption(caption_)
        , defaultDirectory(defaultDir_)
        , filterList(QStringList())
        , defaultFilter(QString())
        , useStaticForNative(false)
        , hideDetails(false)
        , swapExtensionOrder(false)
    {
        // Force the native file dialogs on Windows. Except for KDE, the native file dialogs are only possible
        // using the static methods. The Qt documentation is wrong here, if it means what it says " By default,
        // the native file dialog is used unless you use a subclass of QFileDialog that contains the Q_OBJECT
        // macro."
#ifdef Q_OS_WIN
        useStaticForNative = true;
#endif
        // Non-static KDE file is broken when called with QFileDialog::AcceptSave:
        // then the directory above defaultdir is opened, and defaultdir is given as the default file name...
        //
        // So: in X11, use static methods inside KDE, which give working native dialogs, but non-static outside
        // KDE, which gives working Qt dialogs.
        //
        // Only show the GTK dialog in Gnome, where people deserve it
#ifdef HAVE_X11
        if (qgetenv("KDE_FULL_SESSION").size() > 0) {
            useStaticForNative = true;
        }
        if (qgetenv("XDG_CURRENT_DESKTOP") == "GNOME") {
            useStaticForNative = true;
            QClipboard *cb = QApplication::clipboard();
            cb->blockSignals(true);
            swapExtensionOrder = true;
        }

#endif
    }

    ~Private()
    {
        if (qgetenv("XDG_CURRENT_DESKTOP") == "GNOME") {
            useStaticForNative = true;
            QClipboard *cb = QApplication::clipboard();
            cb->blockSignals(false);
        }
    }

    QWidget *parent;
    KoFileDialog::DialogType type;
    QString dialogName;
    QString caption;
    QString defaultDirectory;
    QStringList filterList;
    QString defaultFilter;
    QScopedPointer<QFileDialog> fileDialog;
    QMimeType mimeType;
    bool useStaticForNative;
    bool hideDetails;
    bool swapExtensionOrder;
};

KoFileDialog::KoFileDialog(QWidget *parent, KoFileDialog::DialogType type, const QString &dialogName)
    : d(new Private(parent, type, QLatin1String(""), getUsedDir(dialogName), dialogName))
{
}

KoFileDialog::~KoFileDialog()
{
    delete d;
}

void KoFileDialog::setCaption(const QString &caption)
{
    d->caption = caption;
}

void KoFileDialog::setDefaultDir(const QString &defaultDir, bool override)
{
    if (override || d->defaultDirectory.isEmpty() || !QFile(d->defaultDirectory).exists()) {
        QFileInfo f(defaultDir);
        d->defaultDirectory = f.absoluteFilePath();
    }
}

void KoFileDialog::setOverrideDir(const QString &overrideDir)
{
    d->defaultDirectory = overrideDir;
}

void KoFileDialog::setImageFilters()
{
    QStringList imageMimeTypes;
    foreach (const QByteArray &mimeType, QImageReader::supportedMimeTypes()) {
        imageMimeTypes << QLatin1String(mimeType);
    }
    setMimeTypeFilters(imageMimeTypes);
}

void KoFileDialog::setNameFilter(const QString &filter)
{
    d->filterList.clear();
    if (d->type == KoFileDialog::SaveFile) {
        QStringList mimeList;
        d->filterList << splitNameFilter(filter, &mimeList);
        d->defaultFilter = d->filterList.first();
    } else {
        d->filterList << filter;
    }
}

void KoFileDialog::setNameFilters(const QStringList &filterList, QString defaultFilter)
{
    d->filterList.clear();

    if (d->type == KoFileDialog::SaveFile) {
        QStringList mimeList;
        foreach (const QString &filter, filterList) {
            d->filterList << splitNameFilter(filter, &mimeList);
        }

        if (!defaultFilter.isEmpty()) {
            mimeList.clear();
            QStringList defaultFilters = splitNameFilter(defaultFilter, &mimeList);
            if (defaultFilters.size() > 0) {
                defaultFilter = defaultFilters.first();
            }
        }
    } else {
        d->filterList = filterList;
    }
    d->defaultFilter = defaultFilter;
}

void KoFileDialog::setMimeTypeFilters(const QStringList &filterList, QString defaultFilter)
{
    d->filterList = getFilterStringListFromMime(filterList, true);

    if (!defaultFilter.isEmpty()) {
        QStringList defaultFilters = getFilterStringListFromMime(QStringList() << defaultFilter, false);
        if (defaultFilters.size() > 0) {
            defaultFilter = defaultFilters.first();
        }
    }
    d->defaultFilter = defaultFilter;
}

void KoFileDialog::setHideNameFilterDetailsOption()
{
    d->hideDetails = true;
}

QStringList KoFileDialog::nameFilters() const
{
    return d->filterList;
}

QString KoFileDialog::selectedNameFilter() const
{
    if (!d->useStaticForNative) {
        return d->fileDialog->selectedNameFilter();
    } else {
        return d->defaultFilter;
    }
}

QString KoFileDialog::selectedMimeType() const
{
    if (d->mimeType.isValid()) {
        return d->mimeType.name();
    } else {
        return QLatin1String("");
    }
}

void KoFileDialog::createFileDialog()
{
    d->fileDialog.reset(new QFileDialog(d->parent, d->caption, d->defaultDirectory));

    if (d->type == SaveFile) {
        d->fileDialog->setAcceptMode(QFileDialog::AcceptSave);
        d->fileDialog->setFileMode(QFileDialog::AnyFile);
    } else { // open / import

        d->fileDialog->setAcceptMode(QFileDialog::AcceptOpen);

        if (d->type == ImportDirectory || d->type == OpenDirectory) {
            d->fileDialog->setFileMode(QFileDialog::Directory);
            d->fileDialog->setOption(QFileDialog::ShowDirsOnly, true);
        } else { // open / import file(s)
            if (d->type == OpenFile || d->type == ImportFile) {
                d->fileDialog->setFileMode(QFileDialog::ExistingFile);
            } else { // files
                d->fileDialog->setFileMode(QFileDialog::ExistingFiles);
            }
        }
    }

    d->fileDialog->setNameFilters(d->filterList);
    if (!d->defaultFilter.isEmpty()) {
        d->fileDialog->selectNameFilter(d->defaultFilter);
    }

    if (d->type == ImportDirectory || d->type == ImportFile || d->type == ImportFiles || d->type == SaveFile) {
        d->fileDialog->setWindowModality(Qt::WindowModal);
    }

    if (d->hideDetails) {
        d->fileDialog->setOption(QFileDialog::HideNameFilterDetails);
    }

    connect(d->fileDialog.data(), &QFileDialog::filterSelected, this, &KoFileDialog::filterSelected);
}

QString KoFileDialog::filename()
{
    QString url;
    if (!d->useStaticForNative) {
        if (!d->fileDialog) {
            createFileDialog();
        }

        if (d->fileDialog->exec() == QDialog::Accepted) {
            url = d->fileDialog->selectedFiles().first();
        }
    } else {
        switch (d->type) {
        case OpenFile: {
            url = QFileDialog::getOpenFileName(d->parent, d->caption, d->defaultDirectory, d->filterList.join(QStringLiteral(";;")), &d->defaultFilter);
            break;
        }
        case OpenDirectory: {
            url = QFileDialog::getExistingDirectory(d->parent, d->caption, d->defaultDirectory, QFileDialog::ShowDirsOnly);
            break;
        }
        case ImportFile: {
            url = QFileDialog::getOpenFileName(d->parent, d->caption, d->defaultDirectory, d->filterList.join(QStringLiteral(";;")), &d->defaultFilter);
            break;
        }
        case ImportDirectory: {
            url = QFileDialog::getExistingDirectory(d->parent, d->caption, d->defaultDirectory, QFileDialog::ShowDirsOnly);
            break;
        }
        case SaveFile: {
            url = QFileDialog::getSaveFileName(d->parent, d->caption, d->defaultDirectory, d->filterList.join(QStringLiteral(";;")), &d->defaultFilter);
            break;
        }
        default:;
        }
    }

    if (!url.isEmpty()) {
        QMimeDatabase db;
        d->mimeType = db.mimeTypeForFile(url);
        saveUsedDir(url, d->dialogName);
    }
    return url;
}

QStringList KoFileDialog::filenames()
{
    QStringList urls;

    if (!d->useStaticForNative) {
        if (!d->fileDialog) {
            createFileDialog();
        }
        if (d->fileDialog->exec() == QDialog::Accepted) {
            urls = d->fileDialog->selectedFiles();
        }
    } else {
        switch (d->type) {
        case OpenFiles:
        case ImportFiles: {
            urls = QFileDialog::getOpenFileNames(d->parent, d->caption, d->defaultDirectory, d->filterList.join(QStringLiteral(";;")), &d->defaultFilter);
            break;
        }
        default:;
        }
    }
    if (urls.size() > 0) {
        saveUsedDir(urls.first(), d->dialogName);
    }
    return urls;
}

void KoFileDialog::filterSelected(const QString &filter)
{
    // "Windows BMP image ( *.bmp )";
    int start = filter.lastIndexOf(QLatin1String("*.")) + 2;
    int end = filter.lastIndexOf(QLatin1String(" )"));
    int n = end - start;
    QString extension = filter.mid(start, n);
    d->defaultFilter = filter;
    d->fileDialog->setDefaultSuffix(extension);
}

QStringList KoFileDialog::splitNameFilter(const QString &nameFilter, QStringList *mimeList)
{
    Q_ASSERT(mimeList);

    QStringList filters;
    QString description;

    if (nameFilter.contains(QLatin1String("("))) {
        description = nameFilter.left(nameFilter.indexOf(QLatin1String("(")) - 1).trimmed();
    }

    QStringList entries = nameFilter.mid(nameFilter.indexOf(QLatin1String(QLatin1String("("))) + 1).split(QStringLiteral(" "), Qt::SkipEmptyParts);

    foreach (QString entry, entries) {
        entry = entry.remove(QStringLiteral("*"));
        entry = entry.remove(QStringLiteral(")"));

        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForName("bla" + entry);
        if (mime.name() != QLatin1String("application/octet-stream")) {
            if (!mimeList->contains(mime.name())) {
                mimeList->append(mime.name());
                filters.append(mime.comment() + " ( *" + entry + " )");
            }
        } else {
            filters.append(entry.remove(QStringLiteral(".")).toUpper() + " " + description + " ( *." + entry + " )");
        }
    }

    return filters;
}

const QStringList KoFileDialog::getFilterStringListFromMime(const QStringList &mimeList, bool withAllSupportedEntry)
{
    QStringList mimeSeen;

    QStringList ret;
    if (withAllSupportedEntry) {
        ret << QString();
    }

    for (QStringList::ConstIterator it = mimeList.begin(); it != mimeList.end(); ++it) {
        QMimeDatabase db;
        QMimeType mimeType = db.mimeTypeForName(*it);
        if (!mimeType.isValid()) {
            continue;
        }
        if (!mimeSeen.contains(mimeType.name())) {
            QString oneFilter;
            QStringList patterns = mimeType.globPatterns();
            QStringList::ConstIterator jt;
            for (jt = patterns.constBegin(); jt != patterns.constEnd(); ++jt) {
                if (d->swapExtensionOrder) {
                    oneFilter.prepend(*jt + " ");
                    if (withAllSupportedEntry) {
                        ret[0].prepend(*jt + " ");
                    }
                } else {
                    oneFilter.append(*jt + " ");
                    if (withAllSupportedEntry) {
                        ret[0].append(*jt + " ");
                    }
                }
            }
            oneFilter = mimeType.comment() + " ( " + oneFilter + ")";
            ret << oneFilter;
            mimeSeen << mimeType.name();
        }
    }

    // All files
    ret << i18n("All files") + " ( * )";

    if (withAllSupportedEntry) {
        ret[0] = i18n("All supported formats") + " ( " + ret[0] + (")");
    }
    return ret;
}

QString KoFileDialog::getUsedDir(const QString &dialogName)
{
    if (dialogName.isEmpty())
        return QLatin1String("");

    KConfigGroup group = KSharedConfig::openConfig()->group("File Dialogs");
    QString dir = group.readEntry(dialogName);

    return dir;
}

void KoFileDialog::saveUsedDir(const QString &fileName, const QString &dialogName)
{
    if (dialogName.isEmpty())
        return;

    QFileInfo fileInfo(fileName);
    KConfigGroup group = KSharedConfig::openConfig()->group("File Dialogs");
    group.writeEntry(dialogName, fileInfo.absolutePath());
}
