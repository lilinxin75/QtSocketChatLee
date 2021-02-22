#include "client.h"
#include <QFile>
#include <iostream>
#include <QDebug>
#include <QTextStream>
#include <QtNetwork>

QTextStream qcin(stdin, QIODevice::ReadOnly);
QTextStream qcout(stdout, QIODevice::WriteOnly);
QTextStream qcerr(stderr, QIODevice::WriteOnly);

Client::Client()
    : m_fileSocket(NULL),
      m_totalBytes(0),
      m_processedBytes(0),
      m_fileNameBytes(0),
      m_mediumFile(NULL)
{
    m_fileSocket = new QTcpSocket(this);
    m_mediumFile = new QFile(this);
    selectOption();
}

Client::~Client()
{
    delete m_fileSocket;
    delete m_mediumFile;
}

void Client::initFileProcessValue()
{
    m_totalBytes = 0;
    m_processedBytes = 0;
    m_fileNameBytes = 0;
    m_fileNameStr.clear();
}

void Client::getSendFileName()
{
    initFileProcessValue();
    bool fileExistFlg = false;
    while (!fileExistFlg)
    {
        fileExistFlg = setfileSaveDir();
    }
    if (fileExistFlg)
    {
        slot_clickSendFile();
    }
}

void Client::slot_clickSendFile()
{
    m_fileSocket->deleteLater();
    m_fileSocket = new QTcpSocket(this);
    m_fileSocket->connectToHost(m_hostAddress, m_hostPort);
    connect(m_fileSocket, SIGNAL(connected()), this, SLOT(slot_sendFile()));
    connect(m_fileSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(slot_fileSendProgress(qint64)));
    std::cout << "success connect to server." << std::endl;
}

void Client::slot_sendFile()
{
    QByteArray outBlock;        //发送用数据缓存包. 2021-01-28 https://blog.csdn.net/Bing_Lee
    if (!m_mediumFile->isOpen() && !m_mediumFile->open(QFile::ReadOnly))
    {
        m_mediumFile->close();
        std::cout << "[Client] open file error!" << std::endl;
        return;
    }

    m_totalBytes = m_mediumFile->size();
    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_4_8);
    QString currentFileName;
    QFileInfo fileInfo(*m_mediumFile);
    currentFileName = fileInfo.fileName();
    //文件总大小、文件名大小、文件名
    sendOut << qint64(0) << qint64(0) << currentFileName;
    m_totalBytes += outBlock.size();
    sendOut.device()->seek(0);
    sendOut << m_totalBytes << qint64(outBlock.size() - sizeof(qint64) * 2);
    m_fileSocket->write(outBlock);

    QString str = "[Client] Sending ";
    str = str + currentFileName;
    std::cout << str.toStdString() << std::endl;
    outBlock.resize(0);
}

void Client::slot_fileSendProgress(qint64 bytes)
{
    QByteArray outBlock;        //接收用数据缓存包. 2021-01-28 https://blog.csdn.net/Bing_Lee
    m_processedBytes += bytes;
    if (m_processedBytes < m_totalBytes)
    {
        //按照最小数据包发送或发送最后一点剩余数据. 2021-01-29 https://blog.csdn.net/Bing_Lee
        outBlock = m_mediumFile->read(m_totalBytes - m_processedBytes);
        m_fileSocket->write(outBlock);
    }
    if (m_processedBytes == m_totalBytes)
    {
        std::cout << "[Client] " << m_fileNameStr.toStdString() << " have sent." << std::endl;
        m_mediumFile->close();
        getSendFileName();
    }
    if (m_processedBytes > m_totalBytes && m_totalBytes != 0)
    {
        m_mediumFile->close();
        std::cout << "[Client] receive data more than oaiginal, error!" << std::endl;
        getSendFileName();
    }
}

bool Client::initTcp()
{
    std::cout<<"//////////////////NOTICE////////////////////////"<<std::endl;
    std::cout<<"//Please input host address like this         //"<<std::endl;
    std::cout<<"//127.0.0.1                                   //"<<std::endl;
    std::cout<<"////////////////////////////////////////////////"<<std::endl;

    qcin >> m_hostAddress;
    std::cout<<"//////////////////NOTICE////////////////////////"<<std::endl;
    std::cout<<"//Please input host port like this            //"<<std::endl;
    std::cout<<"//10086                                       //"<<std::endl;
    std::cout<<"////////////////////////////////////////////////"<<std::endl;
    qcin >> m_hostPort;
    m_fileSocket->disconnectFromHost();
    m_fileSocket->connectToHost(m_hostAddress, m_hostPort, QAbstractSocket::ReadOnly);
    if (m_fileSocket->waitForConnected())
    {
        m_fileSocket->disconnectFromHost();
        return true;
    }
    else
    {
        return false;
    }
}

void Client::selectOption()
{
//    int selected = 0;
//    std::cout<<"//////////////////OPTION///////////////////////"<<std::endl;
//    std::cout<<"// 1   SendDocument                          //"<<std::endl;
//    std::cout<<"///////////////////////////////////////////////"<<std::endl;
    bool addressAccess = false;
    while(!addressAccess)
    {
        addressAccess = initTcp();
    }
    getSendFileName();
}

bool Client::setfileSaveDir()
{
    std::cout<<"Please Input Document Directory: "<<std::endl;
    QString fileNameStr;
    qcin >> fileNameStr;
    m_fileNameStr.clear();
    m_fileNameStr = fileNameStr;
    m_mediumFile->setFileName(m_fileNameStr);
    if(m_mediumFile->exists())
    {
        return true;
    }
    else
    {
        std::cout << "Document did not exist: " << std::endl;
        return false;
    }
}
