/* Swinder - Portable library for spreadsheet
   SPDX-FileCopyrightText: 2003-2005 Ariya Hidayat <ariya@kde.org>
   SPDX-FileCopyrightText: 2006, 2009 Marijn Kruisselbrink <mkruisselbrink@kde.org>
   SPDX-FileCopyrightText: 2009, 2010 Sebastian Sauer <sebsauer@kdab.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef SWINDER_UTILS_H
#define SWINDER_UTILS_H

#include "value.h"
#include <map>

#include <QLoggingCategory>
#include <QRect>
#include <QString>
#include <QUuid>
#include <QtEndian>

//"calligra.filter.sidewinder"
Q_DECLARE_LOGGING_CATEGORY(lcSidewinder)

namespace Swinder
{

// The minimal number of rows and columns. This is used to fill remaining rows and columns with the
// default style what is needed cause Excel does always define the default for all rows and columns
// while ODF does only for those that are explicit defined.
static const uint minimumColumnCount = 1024;
static const uint minimumRowCount = 32768;

// The maximal number of rows and columns. This allows us to cut rows and columns away that would
// not be handled by the consumer application anyway cause they reached the applications limited.
static const uint maximalColumnCount = 32768;
static const uint maximalRowCount = 65536;

// Returns A for 1, B for 2, C for 3, etc.
QString columnName(uint column);
// Returns the escaped sheet-name.
QString encodeSheetName(const QString &name);
// Returns an encoded cell-address like e.g. "Sheet1!A1".
QString encodeAddress(const QString &sheetName, uint column, uint row);
// Returns an encoded cell-range like e.g. "Sheet1!A1:B2".
QString encodeAddress(const QString &sheetName, const QRect &rect);

class Workbook;
class XlsRecordOutputStream;

Value errorAsValue(int errorCode);

static inline unsigned long readU16(const void *p)
{
    const unsigned char *ptr = (const unsigned char *)p;
    return ptr[0] + (ptr[1] << 8);
}

static inline unsigned readU8(const void *p)
{
    const unsigned char *ptr = (const unsigned char *)p;
    return ptr[0];
}

static inline long readS16(const void *p)
{
    long val = readU16(p);
    if (val & 0x8000)
        val = val - 0x10000;
    return val;
}

static inline long readS8(const void *p)
{
    const unsigned char *ptr = (const unsigned char *)p;
    long val = *ptr;
    if (val & 0x80)
        val = val - 0x100;
    return val;
}

static inline unsigned long readU32(const void *p)
{
    const unsigned char *ptr = (const unsigned char *)p;
    return ptr[0] + (ptr[1] << 8) + (ptr[2] << 16) + (ptr[3] << 24);
}

static inline long readS32(const void *p)
{
    long val = readU32(p);
    if (val & 0x800000)
        val = val - 0x1000000;
    return val;
}

static inline double readFixed32(const void *p)
{
    const unsigned char *ptr = (const unsigned char *)p;
    unsigned a = readU16(ptr);
    unsigned b = readU16(ptr + 2);
    return a + (b / 65536.0);
}

static inline QUuid readUuid(const void *p)
{
    const unsigned char *ptr = (const unsigned char *)p;
    return QUuid(readU32(ptr), readU16(ptr + 4), readU16(ptr + 6), ptr[9], ptr[9], ptr[10], ptr[11], ptr[12], ptr[13], ptr[14], ptr[15]);
}

// FIXME check that double is 64 bits
static inline double readFloat64(const void *p)
{
    quint64 val = qFromLittleEndian<quint64>(reinterpret_cast<const uchar *>(p));
    double num = 0.0;
    memcpy(&num, &val, sizeof num);
    return num;
}

// RK value is special encoded integer or floating-point
// see any documentation of Excel file format for detail description
static inline void decodeRK(unsigned rkvalue, bool &isInteger, int &i, double &f)
{
    double factor = (rkvalue & 0x01) ? 0.01 : 1;
    if (rkvalue & 0x02) {
        // FIXME check that int is 32 bits ?
        isInteger = true;
        i = *((int *)&rkvalue) >> 2;
        if (rkvalue & 0x01) {
            if (i % 100 == 0) {
                i /= 100;
            } else {
                isInteger = false;
                f = i * 0.01;
            }
        }
    } else {
        // TODO ensure double takes 8 bytes
        isInteger = false;
        rkvalue = qFromLittleEndian<quint32>(rkvalue);
        unsigned char *s = (unsigned char *)&rkvalue;
        unsigned char *r = (unsigned char *)&f;
        r[0] = r[1] = r[2] = r[3] = 0;
        r[4] = s[0] & 0xfc;
        r[5] = s[1];
        r[6] = s[2];
        r[7] = s[3];
        f *= factor;
    }
}

/**
 * Supported Excel document version.
 */
enum { UnknownExcel = 0, Excel95, Excel97, Excel2000 };

QString readByteString(const void *data, unsigned length, unsigned maxSize = -1, bool *error = nullptr, unsigned *size = nullptr);
QString readTerminatedUnicodeChars(const void *data, unsigned *size = nullptr, unsigned maxSize = -1, bool *error = nullptr);
QString readUnicodeChars(const void *data,
                         unsigned length,
                         unsigned maxSize = -1,
                         bool *error = nullptr,
                         unsigned *size = nullptr,
                         unsigned continuePosition = -1,
                         unsigned offset = 0,
                         bool unicode = true,
                         bool asianPhonetics = false,
                         bool richText = false);
QString
readUnicodeString(const void *data, unsigned length, unsigned maxSize = -1, bool *error = nullptr, unsigned *size = nullptr, unsigned continuePosition = -1);
QString
readUnicodeCharArray(const void *p, unsigned length, unsigned maxSize = -1, bool *error = nullptr, unsigned *size = nullptr, unsigned continuePosition = -1);

std::ostream &operator<<(std::ostream &s, const QString &ustring);
std::ostream &operator<<(std::ostream &s, const QByteArray &data);
std::ostream &operator<<(std::ostream &s, const QUuid &uuid);

/**
  Class Record represents a base class for all other type record,
  hence do not use this class in real life.

 */
class Record
{
public:
    /**
      Static ID of the record. Subclasses should override this value
      with the id of the record they handle.
    */
    static const unsigned int id;

    virtual unsigned int rtti() const
    {
        return this->id;
    }

    /**
      Creates a new generic record.
    */
    explicit Record(Workbook *);

    /**
      Destroys the record.
    */
    virtual ~Record();

    /**
     * Record factory, create a new record of specified type.
     */
    static Record *create(unsigned type, Workbook *book);

    void setVersion(unsigned v)
    {
        ver = v;
    }

    unsigned version() const
    {
        return ver;
    }

    void setRecordSize(unsigned size)
    {
        m_size = size;
    }

    unsigned recordSize() const
    {
        return m_size;
    }

    /**
      Sets the data for this record.
     */
    virtual void setData(unsigned size, const unsigned char *data, const unsigned int *continuePositions);

    virtual void writeData(XlsRecordOutputStream &out) const;

    /**
      Sets the position of the record in the OLE stream. Somehow this is
      required to process BoundSheet and BOF(Worksheet) properly.
     */
    void setPosition(unsigned pos);

    /**
      Gets the position of this record in the OLE stream.
     */
    unsigned position() const;

    /**
      Returns the name of the record. For debugging only.
     */
    virtual const char *name() const
    {
        return "Unknown";
    }

    /**
      Dumps record information to output stream. For debugging only.
     */
    virtual void dump(std::ostream &out) const;

    bool isValid() const;

protected:
    void setIsValid(bool isValid);
    // the workbook
    Workbook *m_workbook;
    // position of this record in the OLE stream
    unsigned stream_position;
    // in which version does this record denote ?
    unsigned ver;
    // is the record valid?
    bool valid;
    // size of the record
    unsigned m_size;
};

typedef Record *(*RecordFactory)(Workbook *);
typedef Record *(*RecordFactoryWithArgs)(Workbook *, void *);

class RecordRegistry
{
public:
    static void registerRecordClass(unsigned id, RecordFactory factory);
    static void registerRecordClass(unsigned id, RecordFactoryWithArgs factory, void *args);
    static void unregisterRecordClass(unsigned id);
    static Record *createRecord(unsigned id, Workbook *book);

private:
    RecordRegistry() = default;
    static RecordRegistry *instance();

    std::map<unsigned, RecordFactory> records;
    std::map<unsigned, RecordFactoryWithArgs> recordsWithArgs;
    std::map<unsigned, void *> recordArgs;
};

} // namespace Swinder

#endif // SWINDER_UTILS_H
