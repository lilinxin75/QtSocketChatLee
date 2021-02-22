#ifndef SERVER_H
#define SERVER_H

#include <QObject>

class QTcpSocket;
class QTcpServer;
class QFile;

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server();
    ~Server();

    void SelectOption();
    bool start();

protected:
    void initFileProcessValue();
    void checkReceiveDir();

private slots:
    void slot_acceptFileConnection();
    void slot_fileReceiveProgress();

private:
    QTcpServer *m_fileServer;
    QTcpSocket *m_fileSocket;

    qint64 m_totalBytes;           //传输文件总大小. 2021-01-28 https://blog.csdn.net/Bing_Lee
    qint64 m_processedBytes;        //已接收文件大小. 2021-01-28 https://blog.csdn.net/Bing_Lee
    qint64 m_fileNameBytes;        //文件名所占大小. 2021-01-28 https://blog.csdn.net/Bing_Lee
    QString m_fileNameStr;        //传输文件名. 2021-01-28 https://blog.csdn.net/Bing_Lee

    QFile *m_mediumFile;        //内部传输打开的文件对象指针. 2021-01-29 https://blog.csdn.net/Bing_Lee
};

#endif // SERVER_H
