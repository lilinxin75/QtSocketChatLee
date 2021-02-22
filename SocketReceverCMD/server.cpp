#include "server.h"
#include <iostream>
#include <QTcpSocket>
#include <QTcpServer>
#include <QDataStream>
#include <QFile>
#include <QNetworkInterface>
#include <QCoreApplication>
#include <QDir>

#define RECEIVE_DIR_STRING "receive"

Server::Server()
    : m_fileServer(NULL),
      m_fileSocket(NULL),
      m_totalBytes(0),
      m_processedBytes(0),
      m_fileNameBytes(0),
      m_mediumFile(NULL)
{
    m_fileServer = new QTcpServer(this);
    m_mediumFile = new QFile(this);
    checkReceiveDir();
    SelectOption();
}

Server::~Server()
{
    delete m_mediumFile;
    delete m_fileServer;
}

void Server::SelectOption()
{
    int selected = 0;
    std::cout<< "//////////////////OPTION///////////////////////"<< std::endl;
    std::cout<< "// 1   Start Server                          //"<< std::endl;
    std::cout<< "///////////////////////////////////////////////"<< std::endl;
    std::cin >> selected;
    {
        bool startflg = false;
        while(!startflg)
        {
            startflg = start();
        }
    }
}

bool Server::start()
{
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i)
    {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
                ipAddressesList.at(i).toIPv4Address())
        {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
    {
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    }

    if(!m_fileServer->listen(QHostAddress::Any))
    {
        std::cout << m_fileServer->errorString().toStdString();
        return false;
    }
    else
    {
        connect(m_fileServer, SIGNAL(newConnection()), this, SLOT(slot_acceptFileConnection()));
        quint16 port = m_fileServer->serverPort();
        std::cout << "IP: " << ipAddress.toStdString() <<
                     " PORT: " << QString::number(port).toStdString() <<
                     std::endl;
    }
    std::cout<< "Listening." << std::endl;
    return true;
}

void Server::initFileProcessValue()
{
    m_totalBytes = 0;
    m_processedBytes = 0;
    m_fileNameBytes = 0;
    m_fileNameStr.clear();
}

void Server::checkReceiveDir()
{
    QString sourcePath = QCoreApplication::applicationDirPath();
    sourcePath = sourcePath + QDir::separator() + RECEIVE_DIR_STRING;
    QDir dir;
    if (!dir.exists(sourcePath))
    {
        dir.mkdir(sourcePath);
    }
}

void Server::slot_acceptFileConnection()
{
    initFileProcessValue();
    m_fileSocket = m_fileServer->nextPendingConnection();
    connect(m_fileSocket, SIGNAL(readyRead()), this, SLOT(slot_fileReceiveProgress()));
    connect(m_fileSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(slot_fileReceiveProgress()));
    std::cout << "client socket connected." << std::endl;
}

void Server::slot_fileReceiveProgress()
{
    QDataStream in(m_fileSocket);
    in.setVersion(QDataStream::Qt_4_8);
    // 如果接收到的数据小于16个字节，保存到来的文件头结构
    QByteArray inBlock;        //接收用数据缓存包. 2021-01-28 https://blog.csdn.net/Bing_Lee
    if (m_processedBytes <= sizeof(qint64) * 2)
    {
        if ((m_fileSocket->bytesAvailable() >= sizeof(qint64) * 2) &&
                (m_fileNameBytes == 0))
        {
            // 接收数据总大小信息和文件名大小信息
            in >> m_totalBytes >> m_fileNameBytes;
            m_processedBytes += sizeof(qint64) * 2;
        }
        if ((m_fileSocket->bytesAvailable() >= m_fileNameBytes) &&
                (m_fileNameBytes != 0))
        {
            // 接收文件名，并建立文件
            in >> m_fileNameStr;
            m_processedBytes += m_fileNameBytes;
            m_mediumFile->setFileName(m_fileNameStr);
            QString sourcePath = QCoreApplication::applicationDirPath();
            QDir::setCurrent(sourcePath + QDir::separator() + RECEIVE_DIR_STRING);
            if (!m_mediumFile->open(QFile::WriteOnly))
            {
                std::cout << "open file error!" << std::endl;
                initFileProcessValue();
                return;
            }
        }
        else
        {
            return;
        }
    }
    // 如果接收的数据小于总数据，那么写入文件
    else if (m_processedBytes < m_totalBytes)
    {
        inBlock = m_fileSocket->readAll();
        m_mediumFile->write(inBlock);
        m_processedBytes += inBlock.size();
        inBlock.resize(0);
    }
    // 接收数据完成时
    if (m_processedBytes == m_totalBytes)
    {
        m_mediumFile->close();
        std::cout << "[Server] receive " << m_fileNameStr.toStdString() << " finished!" << std::endl;
    }
    if (m_processedBytes > m_totalBytes && m_totalBytes != 0)
    {
        m_mediumFile->close();
        m_mediumFile->remove();
        std::cout << "[Server] receive data more than oaiginal, error!" << std::endl;
        initFileProcessValue();
    }
}
