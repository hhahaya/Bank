#ifndef SERVER_H
#define SERVER_H

#include <QDialog>
#include <QTcpServer>
#include <QVector>
#include "connect.h"

namespace Ui {
class server;
}

class server : public QDialog
{
    Q_OBJECT
    
public:
    explicit server(QWidget *parent = 0);
    ~server();
    QTcpServer *tcp_server;
    QVector<WMger> ARR;
    Ui::server *ui;
    QTcpSocket *cfd;
    Send_MSG message;                                   //接收信息
    quint16 blocksize;

public slots:
    void newconnect();
    void deal_client();
    void send_back();
};


#endif // SERVER_H
