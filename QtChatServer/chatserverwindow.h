#ifndef CHATSERVERWINDOW_H
#define CHATSERVERWINDOW_H

#include <QMainWindow>

class QTcpServer;
class QTcpSocket;
class QFile;

namespace Ui {
class ChatServerWindow;
}

class ChatServerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChatServerWindow(QWidget *parent = 0);
    ~ChatServerWindow();

protected:
    void init();
    void write2Log(QString str);
    void initFileProcessValue();
    void checkReceiveDir();

private slots:
    void slot_clickStart();
    void slot_acceptMsgConnection();
    void slot_receiveMsg();
    void slot_sendMsg();
    void slot_clickSelectFile();
    void slot_sendFile();
    void slot_acceptFileConnection();
    void slot_fileReceiveProgress();

    void slot_fileSendProgress(qint64 bytes);

private:
    Ui::ChatServerWindow *ui;

    QTcpServer *m_msgServer;
    QTcpSocket *m_msgSocket;
    QTcpServer *m_fileServer;
    QTcpSocket *m_fileSocket;

    qint64 m_totalBytes;           //传输文件总大小. 2021-01-28 https://blog.csdn.net/Bing_Lee
    qint64 m_processedBytes;        //已接收文件大小. 2021-01-28 https://blog.csdn.net/Bing_Lee
    qint64 m_fileNameBytes;        //文件名所占大小. 2021-01-28 https://blog.csdn.net/Bing_Lee
    QString m_fileNameStr;        //传输文件名. 2021-01-28 https://blog.csdn.net/Bing_Lee

    QFile *m_mediumFile;        //内部传输打开的文件对象指针. 2021-01-29 https://blog.csdn.net/Bing_Lee
};

#endif // CHATSERVERWINDOW_H
