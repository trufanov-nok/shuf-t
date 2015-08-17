#ifndef QFORKEDTEXTSTREAM_H
#define QFORKEDTEXTSTREAM_H

/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include <QtCore/qiodevice.h>
#include <QtCore/qstring.h>
#include <QtCore/qchar.h>
//#include <./qlocale.h>
#include <QLocale>
#include <QtCore/qscopedpointer.h>

#include <stdio.h>

#ifdef Status
#error qforkedtextstream.h must be included before any header file that defines Status
#endif

QT_BEGIN_NAMESPACE

//#define QT_NO_TEXTCODEC_FORKED

class QTextCodec;
class QTextDecoder;

class QForkedTextStreamPrivate;
class QForkedTextStream                                // text stream class
{
    Q_DECLARE_PRIVATE(QForkedTextStream)

public:
    enum RealNumberNotation {
        SmartNotation,
        FixedNotation,
        ScientificNotation
    };
    enum FieldAlignment {
        AlignLeft,
        AlignRight,
        AlignCenter,
        AlignAccountingStyle
    };
    enum Status {
        Ok,
        ReadPastEnd,
        ReadCorruptData,
        WriteFailed
    };
    enum NumberFlag {
        ShowBase = 0x1,
        ForcePoint = 0x2,
        ForceSign = 0x4,
        UppercaseBase = 0x8,
        UppercaseDigits = 0x10
    };
    Q_DECLARE_FLAGS(NumberFlags, NumberFlag)

    QForkedTextStream();
    explicit QForkedTextStream(QIODevice *device);
    explicit QForkedTextStream(FILE *fileHandle, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    explicit QForkedTextStream(QString *string, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    explicit QForkedTextStream(QByteArray *array, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    explicit QForkedTextStream(const QByteArray &array, QIODevice::OpenMode openMode = QIODevice::ReadOnly);
    virtual ~QForkedTextStream();

#ifndef QT_NO_TEXTCODEC_FORKED
    void setCodec(QTextCodec *codec);
    void setCodec(const char *codecName);
    QTextCodec *codec() const;
    void setAutoDetectUnicode(bool enabled);
    bool autoDetectUnicode() const;
    void setGenerateByteOrderMark(bool generate);
    bool generateByteOrderMark() const;
#endif

    void setLocale(const QLocale &locale);
    QLocale locale() const;

    void setDevice(QIODevice *device);
    QIODevice *device() const;

    void setString(QString *string, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    QString *string() const;

    Status status() const;
    void setStatus(Status status);
    void resetStatus();

    bool atEnd() const;
    void reset();
    void flush();
    bool seek(qint64 pos);
    qint64 pos() const;

    void skipWhiteSpace();

    QString readLine(qint64 maxlen = 0);
    QString readAll();
    QString read(qint64 maxlen);

    void setFieldAlignment(FieldAlignment alignment);
    FieldAlignment fieldAlignment() const;

    void setPadChar(QChar ch);
    QChar padChar() const;

    void setFieldWidth(int width);
    int fieldWidth() const;

    void setNumberFlags(NumberFlags flags);
    NumberFlags numberFlags() const;

    void setIntegerBase(int base);
    int integerBase() const;

    void setRealNumberNotation(RealNumberNotation notation);
    RealNumberNotation realNumberNotation() const;

    void setRealNumberPrecision(int precision);
    int realNumberPrecision() const;

    QForkedTextStream &operator>>(QChar &ch);
    QForkedTextStream &operator>>(char &ch);
    QForkedTextStream &operator>>(signed short &i);
    QForkedTextStream &operator>>(unsigned short &i);
    QForkedTextStream &operator>>(signed int &i);
    QForkedTextStream &operator>>(unsigned int &i);
    QForkedTextStream &operator>>(signed long &i);
    QForkedTextStream &operator>>(unsigned long &i);
    QForkedTextStream &operator>>(qlonglong &i);
    QForkedTextStream &operator>>(qulonglong &i);
    QForkedTextStream &operator>>(float &f);
    QForkedTextStream &operator>>(double &f);
    QForkedTextStream &operator>>(QString &s);
    QForkedTextStream &operator>>(QByteArray &array);
    QForkedTextStream &operator>>(char *c);

    QForkedTextStream &operator<<(QChar ch);
    QForkedTextStream &operator<<(char ch);
    QForkedTextStream &operator<<(signed short i);
    QForkedTextStream &operator<<(unsigned short i);
    QForkedTextStream &operator<<(signed int i);
    QForkedTextStream &operator<<(unsigned int i);
    QForkedTextStream &operator<<(signed long i);
    QForkedTextStream &operator<<(unsigned long i);
    QForkedTextStream &operator<<(qlonglong i);
    QForkedTextStream &operator<<(qulonglong i);
    QForkedTextStream &operator<<(float f);
    QForkedTextStream &operator<<(double f);
    QForkedTextStream &operator<<(const QString &s);
    QForkedTextStream &operator<<(QLatin1String s);
    QForkedTextStream &operator<<(const QByteArray &array);
    QForkedTextStream &operator<<(const char *c);
    QForkedTextStream &operator<<(const void *ptr);


    QForkedTextStreamPrivate* impl;
    int getBytesRead();
    qint64 getFastPos();

private:
    Q_DISABLE_COPY(QForkedTextStream)
    friend class QDebugStateSaverPrivate;

    QScopedPointer<QForkedTextStreamPrivate> d_ptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QForkedTextStream::NumberFlags)

/*****************************************************************************
  QForkedTextStream manipulators
 *****************************************************************************/

typedef QForkedTextStream & (*QForkedTextStreamFunction)(QForkedTextStream &);// manipulator function
typedef void (QForkedTextStream::*QFTSMFI)(int); // manipulator w/int argument
typedef void (QForkedTextStream::*QFTSMFC)(QChar); // manipulator w/QChar argument


class QForkedTextStreamManipulator
{
public:
    QForkedTextStreamManipulator(QFTSMFI m, int a) { mf = m; mc = 0; arg = a; }
    QForkedTextStreamManipulator(QFTSMFC m, QChar c) { mf = 0; mc = m; ch = c; arg = -1; }
    void exec(QForkedTextStream &s) { if (mf) { (s.*mf)(arg); } else { (s.*mc)(ch); } }

private:
    QFTSMFI mf;                                        // QTextStream member function
    QFTSMFC mc;                                        // QTextStream member function
    int arg;                                          // member function argument
    QChar ch;
};

inline QForkedTextStream &operator>>(QForkedTextStream &s, QForkedTextStreamFunction f)
{ return (*f)(s); }

inline QForkedTextStream &operator<<(QForkedTextStream &s, QForkedTextStreamFunction f)
{ return (*f)(s); }

inline QForkedTextStream &operator<<(QForkedTextStream &s, QForkedTextStreamManipulator m)
{ m.exec(s); return s; }

QForkedTextStream &bin(QForkedTextStream &s);
QForkedTextStream &oct(QForkedTextStream &s);
QForkedTextStream &dec(QForkedTextStream &s);
QForkedTextStream &hex(QForkedTextStream &s);

QForkedTextStream &showbase(QForkedTextStream &s);
QForkedTextStream &forcesign(QForkedTextStream &s);
QForkedTextStream &forcepoint(QForkedTextStream &s);
QForkedTextStream &noshowbase(QForkedTextStream &s);
QForkedTextStream &noforcesign(QForkedTextStream &s);
QForkedTextStream &noforcepoint(QForkedTextStream &s);

QForkedTextStream &uppercasebase(QForkedTextStream &s);
QForkedTextStream &uppercasedigits(QForkedTextStream &s);
QForkedTextStream &lowercasebase(QForkedTextStream &s);
QForkedTextStream &lowercasedigits(QForkedTextStream &s);

QForkedTextStream &fixed(QForkedTextStream &s);
QForkedTextStream &scientific(QForkedTextStream &s);

QForkedTextStream &left(QForkedTextStream &s);
QForkedTextStream &right(QForkedTextStream &s);
QForkedTextStream &center(QForkedTextStream &s);

QForkedTextStream &endl(QForkedTextStream &s);
QForkedTextStream &flush(QForkedTextStream &s);
QForkedTextStream &reset(QForkedTextStream &s);

QForkedTextStream &bom(QForkedTextStream &s);

QForkedTextStream &ws(QForkedTextStream &s);

inline QForkedTextStreamManipulator qSetForkedFieldWidth(int width)
{
    QFTSMFI func = &QForkedTextStream::setFieldWidth;
    return QForkedTextStreamManipulator(func,width);
}

inline QForkedTextStreamManipulator qSetForkedPadChar(QChar ch)
{
    QFTSMFC func = &QForkedTextStream::setPadChar;
    return QForkedTextStreamManipulator(func, ch);
}

inline QForkedTextStreamManipulator qSetForkedRealNumberPrecision(int precision)
{
    QFTSMFI func = &QForkedTextStream::setRealNumberPrecision;
    return QForkedTextStreamManipulator(func, precision);
}




QT_END_NAMESPACE

#endif // QFORKEDTEXTSTREAM_H
