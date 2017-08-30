#include <QtGui/QApplication>
#include "server.h"
#include "mysql.h"
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
    MySql sql;
    sql.initsql();

    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date = current_date_time.toString("yyyy-MM-dd hh:mm:ss");

    QFile file("交易记录.txt");
    if(!file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qDebug()<<file.errorString();
    }
    QString Record = QString("%1\t\t中国银行\t\t交易记录\n\n").arg(current_date);
    file.write(Record.toAscii());
    file.close();

    server w;
    w.show();
    
    return a.exec();
}
