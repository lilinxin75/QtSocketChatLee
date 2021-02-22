#ifndef CHATCLIENTWINDOW_H
#define CHATCLIENTWINDOW_H

#include <QMainWindow>

class QTcpSocket;
class QFile;

namespace Ui {
class ChatClientWindow;
}

class ChatClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChatClientWindow(QWidget *parent = 0);
    ~ChatClientWindow();

protected:
    void init();
    void write2Log(QString str);
    void initFileProcessValue();
    void checkReceiveDir();

private slots:
    void slot_connectServer();
    void slot_sendMsg();
    void slot_receiveMsg();
    void slot_clickSelectFile();
    void slot_clickSendFile();
    void slot_sendFile();
    void slot_fileSendProgress(qint64 bytes);
    void slot_receiveFile();

private:
    Ui::ChatClientWindow *ui;

    QTcpSocket *m_msgSocket;
    QTcpSocket *m_fileSocket;

    qint64 m_totalBytes;           //传输文件总大小. 2021-01-28 https://blog.csdn.net/Bing_Lee
    qint64 m_processedBytes;        //已接收或发送的文件大小. 2021-01-28 https://blog.csdn.net/Bing_Lee
    qint64 m_fileNameBytes;        //文件名所占大小. 2021-01-28 https://blog.csdn.net/Bing_Lee
    QString m_fileNameStr;        //传输文件名. 2021-01-28 https://blog.csdn.net/Bing_Lee

    QFile *m_mediumFile;        //内部传输打开的文件对象指针. 2021-01-29 https://blog.csdn.net/Bing_Lee
};

#endif // CHATCLIENTWINDOW_H
