#include "server.h"
#include "connect.h"
#include "mysql.h"
#include "ui_server.h"

server::server(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::server)
{
    ui->setupUi(this);
    tcp_server = new QTcpServer(this);
    if(! tcp_server->listen(QHostAddress::LocalHost, SERV_PORT))
    {
        qDebug()<<tcp_server->errorString();
        close();
    }
    connect(tcp_server, SIGNAL(newConnection()), this, SLOT(newconnect()));
}

server::~server()
{
    delete ui;
}

void server::newconnect()
{
    Send_MSG message;
    blocksize = 0;
    cfd = tcp_server->nextPendingConnection();
    cfd->read((char *)&message, sizeof(Send_MSG));

    connect(cfd, SIGNAL(readyRead()), this, SLOT(deal_client()));

}

void server::deal_client()
{
    QDataStream in(cfd);

    in.setVersion(QDataStream::Qt_4_6);                 //���ܴ���
    if(blocksize == 0)
    {
        if(cfd->bytesAvailable() < (int)sizeof(quint16))
        {
            return;
        }
        in>>blocksize;
    }
    if(cfd->bytesAvailable() < blocksize)
    {
        return;
    }
    in>>message;                                        //�������Ĵ���message
    send_back();
}

void server::send_back()
{
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date = current_date_time.toString("yyyy-MM-dd hh:mm:ss");

    QFile file("���׼�¼.txt");
    if(!file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append))
    {
        qDebug()<<file.errorString();
    }

    Recv_MSG message_recv;                              //���ͻ�ȥ�Ľṹ��
    bool ret;
    int flag = 0;
    switch(message.cmd)
    {
        case WORKER_INIT:                               //ְ��ע��
        {
            MySql M;
            qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
            message.work_num = (int)(qrand())%899999+100000;               //����һ��6λ���˺�
            ret = M.addWorker(&message, &message_recv);
            if(ret == false)
            {
                QString str = QString("worker %1 register fail...").arg(message.work_name);
                ui->label->setText(str);
            }
            else
            {
                QString str = QString("worker %1 register success...").arg(message.work_name);
                ui->label->setText(str);
            }

            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_0);
            out<<(quint16)0;
            out<<message_recv;
            out.device()->seek(0);
            out<<(quint16)(block.size()-sizeof(quint16));
            cfd->write(block, block.length());

            break;
        }
        case WORKER_LOG:                                //ְ����¼
        {
            QSqlQuery query;
            int value0;
            bool ret;
            QString value1;
            QString str1 = QString("select num, name from worker where num = %1 and passward = %2").arg(message.work_num).arg(message.work_passward);
            ret = query.exec(str1);
            query.last();
            int record = query.at()+1;
            qDebug()<<"all records:"<<record;
            if(record == -1)
            {
                message_recv.cmd = ERROR;
                QString str = QString("worker %1 log fail...").arg(query.value(1).toString());
                ui->label->setText(str);

                QByteArray block;
                QDataStream out(&block, QIODevice::WriteOnly);
                out.setVersion(QDataStream::Qt_4_0);
                out<<(quint16)0;
                out<<message_recv;
                out.device()->seek(0);
                out<<(quint16)(block.size()-sizeof(quint16));
                cfd->write(block, block.length());
                break;
            }


            value0 = query.value(0).toInt();
            qDebug()<<value0;
            value1 = query.value(1).toString();
            qDebug()<<value1;

            for(int i = 0; i < ARR.size(); i++)
            {
                if(ARR[i].work_num == value0 && ARR[i].work_name == value1)
                {
                    flag = 1;
                    QString str = QString("worker %1 has logged, so log fail...").arg(value1);
                    ui->label->setText(str);
                }
            }

            if(flag == 1)
            {
                message_recv.cmd = LOG_EXIST;                   //�˺��ѵ�¼
            }
            else
            {
                WMger link;
                link.work_name = value1;
                link.work_num = value0;
                ARR.push_back(link);                        //�����û��Ĺ���

                message_recv.cmd = LOG_SUCCESS;
                message_recv.work_num = value0;
                message_recv.work_name = value1;
                qDebug()<<value1;
                QString str = QString("worker %1 log success...").arg(value1);
                qDebug()<<str;
                ui->label->setText(str);
            }


            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_0);
            out<<(quint16)0;
            out<<message_recv;
            out.device()->seek(0);
            out<<(quint16)(block.size()-sizeof(quint16));
            cfd->write(block, block.length());

            break;
        }
        case EXIT:                                      //ְ���˳�
        {
            for(int i = 0; i < ARR.size(); i++)
            {
                if(ARR[i].work_name == message.work_name)
                {
                    ARR.erase(ARR.begin()+i);
                }
            }

            break;
        }
        case USER_INIT:                                 //����
        {
            MySql M;
            qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
            message.user_num = (int)(qrand())%899999+100000;               //����һ��6λ���˺�

            ret = M.adduser(&message, &message_recv);
            if(ret == false)
            {
                QString str = QString("user %1 register fail...").arg(message.user_name);
                ui->label->setText(str);
            }
            else
            {
                QString str = QString("user %1 register success...").arg(message.user_name);
                ui->label->setText(str);
                QString Record = QString("\n%1\t\t%2�û�ע��ɹ�\t�˺ţ�%3").arg(current_date).arg(message.user_name).arg(message.user_num);
                file.write(Record.toAscii());
            }

            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_0);
            out<<(quint16)0;
            out<<message_recv;
            out.device()->seek(0);
            out<<(quint16)(block.size()-sizeof(quint16));
            cfd->write(block, block.length());

            break;
        }
        case USER_CLOSE:                                //����
        {
            MySql M;
            ret = M.deleteuser(&message, &message_recv);
            if(ret == false)
            {
                QString str = QString("user_num %1 close account fail...").arg(message.user_num);
                ui->label->setText(str);
            }
            else
            {
                QString str = QString("user_num %1 close account success...").arg(message.user_num);
                ui->label->setText(str);

                QString Record = QString("\n%1\t\t%2�û������ɹ�").arg(current_date).arg(message.user_num);
                file.write(Record.toAscii());
            }                  

            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_0);
            out<<(quint16)0;
            out<<message_recv;
            out.device()->seek(0);
            out<<(quint16)(block.size()-sizeof(quint16));
            cfd->write(block, block.length());

            break;
        }
        case USER_SAVEMONEY:                            //���
        {
            MySql M;
            ret = M.save_money(&message, &message_recv);
            if(ret == false)
            {
                QString str = QString("user_num %1 save money fail...").arg(message.user_num);
                ui->label->setText(str);
            }
            else
            {
                QString str = QString("user_num %1 save money success...").arg(message.user_num);
                ui->label->setText(str);
                QString Record = QString("\n%1\t\t%2�û�\t���\t%3Ԫ").arg(current_date).arg(message.user_num).arg(message.first_money);
                file.write(Record.toAscii());
            }

            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_0);
            out<<(quint16)0;
            out<<message_recv;
            out.device()->seek(0);
            out<<(quint16)(block.size()-sizeof(quint16));
            cfd->write(block, block.length());

            break;
        }
        case USER_GETMONEY:                             //ȡ��
    {
            MySql M;
            M.get_money(&message, &message_recv);
            if(ret == false)
            {
                QString str = QString("user_num %1 get money fail...").arg(message.user_num);
                ui->label->setText(str);
            }
            else
            {
                QString str = QString("user_num %1 get money success...").arg(message.user_num);
                ui->label->setText(str);
                QString Record = QString("\n%1\t\t%2�û�\tȡ��\t%3Ԫ").arg(current_date).arg(message.user_num).arg(message.first_money);
                file.write(Record.toAscii());
            }

            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_0);
            out<<(quint16)0;
            out<<message_recv;
            out.device()->seek(0);
            out<<(quint16)(block.size()-sizeof(quint16));
            cfd->write(block, block.length());

            break;
        }
        case USER_TRANMONEY:                             //ת��
        {
            MySql M;
            ret = M.tran_money(&message, &message_recv);
            if(ret == false)
            {
                QString str = QString("user_num %1 tran money fail...").arg(message.user_num);
                ui->label->setText(str);
            }
            else
            {
                QString str = QString("user_num %1 tran money success...").arg(message.user_num);
                ui->label->setText(str);
                QString Record = QString("\n%1\t\t%2�û�\tת��\t%3�û�\t%4Ԫ").arg(current_date).arg(message.user_num).arg(message.recv_people).arg(message.first_money);
                file.write(Record.toAscii());
            }

            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_0);
            out<<(quint16)0;
            out<<message_recv;
            out.device()->seek(0);
            out<<(quint16)(block.size()-sizeof(quint16));
            cfd->write(block, block.length());

            break;
        }
        case USER_LOOKMONEY:                            //�鿴���
        {
            MySql M;
            qDebug()<<"num = "<<message.user_num;
            qDebug()<<"id = "<<message.user_id;
            qDebug()<<"passward = "<<message.user_passward;
            ret = M.look_money(&message, &message_recv);
            if(ret == false)
            {
                QString str = QString("user_num %1 look money fail...").arg(message.user_num);
                ui->label->setText(str);
            }
            else
            {
                QString str = QString("user_num %1 look money success...").arg(message.user_num);
                ui->label->setText(str);
            }

            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_0);
            out<<(quint16)0;
            out<<message_recv;
            out.device()->seek(0);
            out<<(quint16)(block.size()-sizeof(quint16));
            cfd->write(block, block.length());

            break;
        }
        case USER_CHANGE:                                   //�޸�����
        {
            MySql M;
            ret = M.change_passward(&message, &message_recv);
            if(ret == false)
            {
                QString str = QString("user_num %1 change passward fail...").arg(message.user_num);
                ui->label->setText(str);
            }
            else
            {
                QString str = QString("user_num %1 change passward success...").arg(message.user_num);
                ui->label->setText(str);
            }

            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_0);
            out<<(quint16)0;
            out<<message_recv;
            out.device()->seek(0);
            out<<(quint16)(block.size()-sizeof(quint16));
            cfd->write(block, block.length());

            break;
        }
        case USER_ACCOUNT_DETAIL:                               //�鿴�˻���ϸ
        {
            MySql M;
            ret = M.account_detail(&message, &message_recv);
            if(ret == false)
            {
                QString str = QString("user_num %1 show account detail fail...").arg(message.user_num);
                ui->label->setText(str);
            }
            else
            {
                QString str = QString("user_num %1 show account detail success...").arg(message.user_num);
                ui->label->setText(str);
            }

            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_0);
            out<<(quint16)0;
            out<<message_recv;
            out.device()->seek(0);
            out<<(quint16)(block.size()-sizeof(quint16));
            cfd->write(block, block.length());

            break;
        }
        case LOOK_RECORD:                                       //�鿴���׼�¼
        {
            MySql M;

            ret = M.look_record(&message, &message_recv);
            if(ret == false)
            {
                QString str = QString("worker_num %1 look record fail...").arg(message.work_num);
                ui->label->setText(str);
            }
            else
            {
                QFile file1("���׼�¼.txt");
                if(!file1.open(QIODevice::ReadOnly))
                {
                    qDebug()<<file1.errorString();
                }

                QString str = QString("worker_num %1 look record success...").arg(message.work_num);
                ui->label->setText(str);
                message_recv.record = file1.readAll();
                file1.close();
            }

            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_0);
            out<<(quint16)0;
            out<<message_recv;
            out.device()->seek(0);
            out<<(quint16)(block.size()-sizeof(quint16));
            cfd->write(block, block.length());

            break;
        }
    }
    file.close();

}
