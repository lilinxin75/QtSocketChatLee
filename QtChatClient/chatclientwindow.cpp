#include "chatclientwindow.h"
#include "ui_chatclientwindow.h"

#include <QTcpSocket>
#include <QNetworkInterface>
#include <QDateTime>
#include <QFileDialog>
#include <QFile>

#define RECEIVE_DIR_STRING "receive"
#define MAX_SINGLE_PACKAGE 256
#define SERVER_STRING "[Server] "
#define CLIENT_STRING "[Client] "

ChatClientWindow::ChatClientWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::ChatClientWindow),
      m_msgSocket(NULL),
      m_fileSocket(NULL),
      m_totalBytes(0),
      m_processedBytes(0),
      m_fileNameBytes(0),
      m_mediumFile(NULL)
{
    ui->setupUi(this);
    init();
    connect(ui->pushButton_start, SIGNAL(clicked()), this, SLOT(slot_connectServer()));
    connect(ui->pushButton_sendMsg, SIGNAL(clicked()), this, SLOT(slot_sendMsg()));
    connect(ui->pushButton_selectFile, SIGNAL(clicked()), this, SLOT(slot_clickSelectFile()));
    connect(ui->pushButton_sendFile, SIGNAL(clicked()), this, SLOT(slot_clickSendFile()));
}

ChatClientWindow::~ChatClientWindow()
{
    delete ui;
}

void ChatClientWindow::init()
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
    ui->lineEdit_ip->setText(ipAddress);

    QRegExp rx("^((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)$");
    QRegExpValidator *m_IPValidator = new QRegExpValidator(rx, this);
    ui->lineEdit_ip->setValidator(m_IPValidator);

    m_msgSocket = new QTcpSocket(this);
    m_fileSocket = new QTcpSocket(this);
    m_mediumFile = new QFile(this);

//    ui->pushButton_selectFile->setEnabled(false);
//    ui->pushButton_sendFile->setEnabled(false);
//    ui->pushButton_sendMsg->setEnabled(false);
    checkReceiveDir();
}

void ChatClientWindow::write2Log(QString str)
{
    QString strDate = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ");
    strDate = strDate + str;
    ui->textEdit_history->append(strDate);
}

void ChatClientWindow::initFileProcessValue()
{
    m_totalBytes = 0;
    m_processedBytes = 0;
    m_fileNameBytes = 0;
    m_fileNameStr.clear();
    ui->progressBar_file->setValue(0);
}

void ChatClientWindow::checkReceiveDir()
{
    QString sourcePath = QCoreApplication::applicationDirPath();
    sourcePath = sourcePath + QDir::separator() + RECEIVE_DIR_STRING;
    QDir dir;
    if (!dir.exists(sourcePath))
    {
        dir.mkdir(sourcePath);
    }
}

void ChatClientWindow::slot_connectServer()
{
    m_msgSocket->abort();
    QString ip = ui->lineEdit_ip->text();
    quint16 port;
    port = ui->lineEdit_port->text().toUInt();
    m_msgSocket->connectToHost(ip, port);
    connect(m_msgSocket, SIGNAL(readyRead()), this, SLOT(slot_receiveMsg()));
    m_fileSocket->connectToHost(ip, port + 1);
    connect(m_fileSocket, SIGNAL(readyRead()), this, SLOT(slot_receiveFile()));
    write2Log("connected to server.");
}

void ChatClientWindow::slot_sendMsg()
{
    QString str = ui->textEdit_input->toPlainText();
    if (str.isEmpty())
    {
        return;
    }
    if (m_msgSocket->write(str.toLatin1()) == str.size())
    {
        str = "[Client] " + str;
        write2Log(str);
        ui->textEdit_input->clear();
    }
}

void ChatClientWindow::slot_receiveMsg()
{
    QString str = m_msgSocket->readAll();
    str = "[Server] " + str;
    write2Log(str);
}

void ChatClientWindow::slot_clickSelectFile()
{
    initFileProcessValue();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), ".", tr("*(*.*);;"));
    ui->label_fileName->setText(fileName);
    m_mediumFile->setFileName(fileName);
}

void ChatClientWindow::slot_clickSendFile()
{
    m_fileSocket->deleteLater();
    m_fileSocket = new QTcpSocket(this);
    m_fileSocket->connectToHost(ui->lineEdit_ip->text(), ui->lineEdit_port->text().toUInt() + 1);
    connect(m_fileSocket, SIGNAL(connected()), this, SLOT(slot_sendFile()));
    connect(m_fileSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(slot_fileSendProgress(qint64)));
}

void ChatClientWindow::slot_sendFile()
{
    QByteArray outBlock;        //发送用数据缓存包. 2021-01-28 https://blog.csdn.net/Bing_Lee
    if (!m_mediumFile->isOpen() && !m_mediumFile->open(QFile::ReadOnly))
    {
        m_mediumFile->close();
        write2Log("[Client] open file error!");
        return;
    }

    m_totalBytes = m_mediumFile->size();
    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_4_8);
    QString currentFileName;
    QFileInfo fileInfo(*m_mediumFile);
    currentFileName = fileInfo.fileName();    //文件总大小、文件名大小、文件名
    sendOut << qint64(0) << qint64(0) << currentFileName;
    m_totalBytes += outBlock.size();
    sendOut.device()->seek(0);
    sendOut << m_totalBytes << qint64(outBlock.size() - sizeof(qint64) * 2);
    m_fileSocket->write(outBlock);

    QString str = "[Client] Sending ";
    str = str + currentFileName;
    write2Log(str);
    outBlock.resize(0);
}

void ChatClientWindow::slot_fileSendProgress(qint64 bytes)
{
    QByteArray outBlock;        //接收用数据缓存包. 2021-01-28 https://blog.csdn.net/Bing_Lee
    m_processedBytes += bytes;
    ui->progressBar_file->setValue(m_processedBytes / (m_totalBytes * 1.0) * 100);
    if (m_processedBytes < m_totalBytes)
    {
        //按照最小数据包发送或发送最后一点剩余数据. 2021-01-29 https://blog.csdn.net/Bing_Lee
        outBlock = m_mediumFile->read(m_totalBytes - m_processedBytes);
        m_fileSocket->write(outBlock);
    }
    if (m_processedBytes == m_totalBytes)
    {
        write2Log("[Client] " + m_fileNameStr + " have sent.");
        m_mediumFile->close();
        m_fileSocket->close();
        initFileProcessValue();
    }
    if (m_processedBytes > m_totalBytes && m_totalBytes != 0)
    {
        m_mediumFile->close();
        m_fileSocket->close();
        write2Log("[Client] receive data more than oaiginal, error!");
        initFileProcessValue();
    }
}

void ChatClientWindow::slot_receiveFile()
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
                QString str = "open file error!";
                str = SERVER_STRING + str;
                write2Log(str);
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
        qDebug() << "m_mediumFile->size() " << m_mediumFile->size();
        qDebug() << "inBlock.size() " << inBlock.size();
        m_processedBytes += inBlock.size();
        inBlock.resize(0);
    }
    ui->progressBar_file->setValue(m_processedBytes / (m_totalBytes * 1.0) * 100);

    // 接收数据完成时
    if (m_processedBytes == m_totalBytes)
    {
        m_mediumFile->close();
        write2Log("[Server] receive " + m_fileNameStr + " finished!");
    }
    if (m_processedBytes > m_totalBytes && m_totalBytes != 0)
    {
        m_mediumFile->close();
        m_mediumFile->remove();
        write2Log("[Server] receive data more than oaiginal, error!");
        initFileProcessValue();
    }
}
