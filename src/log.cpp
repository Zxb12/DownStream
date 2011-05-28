#include "log.h"

#include <QDebug>
#include <QTime>

Log *Log::_p = NULL;

Log::Log()
{
    m_file.setFileName(LOG_NAME);
    m_file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Unbuffered);

    out("--- Log start ---");
}

Log::~Log()
{
    if (m_file.isOpen())
    {
        out("---  Log end  ---\r\n");
        m_file.close();
    }
    _p = 0;
}

Log* Log::instance()
{
    if (!_p)
        _p = new Log();
    return _p;
}

void Log::free()
{
    if (_p)
    {
        delete _p;
        _p = NULL;
    }
}

void Log::out(const QString &str)
{
    print(str);
}

void Log::out(const QString &str, const QString &ar1)
{
    print(str.arg(ar1));
}

void Log::out(const QString &str, const QString &ar1, const QString &ar2)
{
    print(str.arg(ar1, ar2));
}

void Log::out(const QString &str, const QString &ar1, const QString &ar2, const QString &ar3)
{
    print(str.arg(ar1, ar2, ar3));
}

void Log::out(const QString &str, const QString &ar1, const QString &ar2, const QString &ar3,
         const QString &ar4)
{
    print(str.arg(ar1, ar2, ar3, ar4));
}

void Log::out(const QString &str, const QString &ar1, const QString &ar2, const QString &ar3,
         const QString &ar4, const QString &ar5)
{
    print(str.arg(ar1, ar2, ar3, ar4, ar5));
}

void Log::out(const QString &str, const QString &ar1, const QString &ar2, const QString &ar3,
         const QString &ar4, const QString &ar5, const QString &ar6)
{
    print(str.arg(ar1, ar2, ar3, ar4, ar5, ar6));
}

void Log::out(const QString &str, const QString &ar1, const QString &ar2, const QString &ar3,
         const QString &ar4, const QString &ar5, const QString &ar6, const QString &ar7)
{
    print(str.arg(ar1, ar2, ar3, ar4, ar5, ar6, ar7));
}

void Log::out(const QString &str, const QString &ar1, const QString &ar2, const QString &ar3,
         const QString &ar4, const QString &ar5, const QString &ar6, const QString &ar7,
         const QString &ar8)
{
    print(str.arg(ar1, ar2, ar3, ar4, ar5, ar6, ar7, ar8));
}

void Log::out(const QString &str, const QString &ar1, const QString &ar2, const QString &ar3,
         const QString &ar4, const QString &ar5, const QString &ar6, const QString &ar7,
         const QString &ar8, const QString &ar9)
{
     print(str.arg(ar1, ar2, ar3, ar4, ar5, ar6, ar7, ar8, ar9));
}

void Log::print(const QString &str)
{
    QString fStr = QTime::currentTime().toString("hh:mm:ss.zzz ") + str;
    qDebug() << qPrintable(fStr);
    m_file.write(qPrintable(fStr + "\r\n"));
}
