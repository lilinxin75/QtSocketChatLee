#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>

class QTcpSocket;
class QFile;

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client();
    ~Client();

protected:
    void selectOption();
    bool setfileSaveDir();
    bool initTcp();
    void initFileProcessValue();
    void getSendFileName();

private slots:
    void slot_clickSendFile();
    void slot_sendFile();
    void slot_fileSendProgress(qint64 bytes);

private:
    QTcpSocket *m_fileSocket;

    qint64 m_totalBytes;           //传输文件总大小. 2021-01-28 https://blog.csdn.net/Bing_Lee
    qint64 m_processedBytes;        //已接收或发送的文件大小. 2021-01-28 https://blog.csdn.net/Bing_Lee
    qint64 m_fileNameBytes;        //文件名所占大小. 2021-01-28 https://blog.csdn.net/Bing_Lee
    QString m_fileNameStr;        //传输文件名. 2021-01-28 https://blog.csdn.net/Bing_Lee

    QFile *m_mediumFile;        //内部传输打开的文件对象指针. 2021-01-29 https://blog.csdn.net/Bing_Lee

    QString m_hostAddress;
    quint16 m_hostPort;
};

#endif // CLIENT_H
