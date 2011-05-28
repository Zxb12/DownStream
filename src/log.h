#ifndef LOG_H
#define LOG_H

#include <QFile>
#include <QString>

#define LOG_NAME "log.log"

#define sLog Log::instance()
#define pNbr QString::number

class Log
{
public:
    static Log* instance();
    static void free();

    void out(const QString &str);
    void out(const QString &str, const QString &ar1);
    void out(const QString &str, const QString &ar1, const QString &ar2);
    void out(const QString &str, const QString &ar1, const QString &ar2, const QString &ar3);
    void out(const QString &str, const QString &ar1, const QString &ar2, const QString &ar3,
             const QString &ar4);
    void out(const QString &str, const QString &ar1, const QString &ar2, const QString &ar3,
             const QString &ar4, const QString &ar5);
    void out(const QString &str, const QString &ar1, const QString &ar2, const QString &ar3,
             const QString &ar4, const QString &ar5, const QString &ar6);
    void out(const QString &str, const QString &ar1, const QString &ar2, const QString &ar3,
             const QString &ar4, const QString &ar5, const QString &ar6, const QString &ar7);
    void out(const QString &str, const QString &ar1, const QString &ar2, const QString &ar3,
             const QString &ar4, const QString &ar5, const QString &ar6, const QString &ar7,
             const QString &ar8);
    void out(const QString &str, const QString &ar1, const QString &ar2, const QString &ar3,
             const QString &ar4, const QString &ar5, const QString &ar6, const QString &ar7,
             const QString &ar8, const QString &ar9);

private:
    Log();
    ~Log();

    inline void print(const QString &str);

    static Log *_p;
    QFile m_file;
};

#endif // LOG_H
