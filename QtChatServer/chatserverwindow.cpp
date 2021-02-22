#include "chatserverwindow.h"
#include "ui_chatserverwindow.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QDateTime>
#include <QFileDialog>
#include <QFile>

#define RECEIVE_DIR_STRING "receive"
#define SERVER_STRING "[Server] "
#define CLIENT_STRING "[Client] "

ChatServerWindow::ChatServerWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::ChatServerWindow),
      m_msgServer(NULL),
      m_msgSocket(NULL),
      m_fileServer(NULL),
      m_fileSocket(NULL),
      m_totalBytes(0),
      m_processedBytes(0),
      m_fileNameBytes(0),
      m_mediumFile(NULL)
{
    ui->setupUi(this);
    init();
    connect(ui->pushButton_start, SIGNAL(clicked()), this, SLOT(slot_clickStart()));
    connect(ui->pushButton_sendMsg, SIGNAL(clicked()), this, SLOT(slot_sendMsg()));
    connect(ui->pushButton_selectFile, SIGNAL(clicked()), this, SLOT(slot_clickSelectFile()));
    connect(ui->pushButton_sendFile, SIGNAL(clicked()), this, SLOT(slot_sendFile()));
}

ChatServerWindow::~ChatServerWindow()
{
    delete ui;
    delete m_msgServer;
    delete m_fileServer;
    delete m_mediumFile;
}

void ChatServerWindow::init()
{
    m_msgServer = new QTcpServer(this);
    m_fileServer = new QTcpServer(this);
    m_mediumFile = new QFile(this);

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
    ui->label_ip->setText("IP " + ipAddress);
//    ui->pushButton_selectFile->setEnabled(false);
//    ui->pushButton_sendFile->setEnabled(false);
//    ui->pushButton_sendMsg->setEnabled(false);
    checkReceiveDir();
}

void ChatServerWindow::write2Log(QString str)
{
    QString strDate = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ");
    strDate = strDate + str;
    ui->textEdit_history->append(strDate);
}

void ChatServerWindow::initFileProcessValue()
{
    m_totalBytes = 0;
    m_processedBytes = 0;
    m_fileNameBytes = 0;
    m_fileNameStr.clear();
    ui->progressBar_file->setValue(0);
}

void ChatServerWindow::checkReceiveDir()
{
    QString sourcePath = QCoreApplication::applicationDirPath();
    sourcePath = sourcePath + QDir::separator() + RECEIVE_DIR_STRING;
    QDir dir;
    if (!dir.exists(sourcePath))
    {
        dir.mkdir(sourcePath);
    }
}

void ChatServerWindow::slot_clickStart()
{
    quint16 port = 0;
    if (ui->lineEdit_port->text() != "0")
    {
        port = ui->lineEdit_port->text().toUInt();
    }
    if (m_msgServer->isListening() && m_msgServer->serverPort() != port)
    {
        m_msgServer->close();
    }
    if (!m_msgServer->listen(QHostAddress::Any, port))
    {
        return;
    }
    else
    {
        write2Log("Message server listening.");
        port = m_msgServer->serverPort();
        ui->lineEdit_port->setText(QString::number(port));
        connect(m_msgServer, SIGNAL(newConnection()), this, SLOT(slot_acceptMsgConnection()));
        ui->pushButton_sendMsg->setEnabled(true);
    }
    if (!m_fileServer->listen(QHostAddress::Any, port + 1))
    {
        write2Log("File server init false, please restart.");
        return;
    }
    else
    {
        write2Log("File server listening.");
        connect(m_fileServer, SIGNAL(newConnection()), this, SLOT(slot_acceptFileConnection()));
        ui->pushButton_selectFile->setEnabled(true);
        ui->pushButton_sendFile->setEnabled(true);
    }
}

void ChatServerWindow::slot_acceptMsgConnection()
{
    QTcpSocket *clientConnection = m_msgServer->nextPendingConnection();
    m_msgSocket = clientConnection;
    connect(m_msgSocket, SIGNAL(readyRead()), this, SLOT(slot_receiveMsg()));
    QString str = m_msgSocket->localAddress().toString() + " client connected.";
    write2Log(str);
}

void ChatServerWindow::slot_receiveMsg()
{
    QTcpSocket *socketTmp = (QTcpSocket*) sender();
    QString str = socketTmp->readAll();
    str = CLIENT_STRING + str;
    write2Log(str);
}

void ChatServerWindow::slot_sendMsg()
{
    if (m_msgSocket == NULL)
    {
        write2Log("client is empty.");
        return;
    }
    QString str = ui->textEdit_input->toPlainText();
    if (str.isEmpty())
    {
        return;
    }
    if (m_msgSocket->write(str.toLatin1()) == str.size())
    {
        str = SERVER_STRING + str;
        write2Log(str);
        ui->textEdit_input->clear();
    }
}

void ChatServerWindow::slot_clickSelectFile()
{
    initFileProcessValue();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), ".", tr("*(*.*);;"));
    ui->label_fileName->setText(fileName);
    m_mediumFile->close();
    m_mediumFile->setFileName(fileName);
}

void ChatServerWindow::slot_sendFile()
{
    if (m_fileSocket == NULL)
    {
        write2Log("client is empty.");
        return;
    }
    if (!m_mediumFile->exists())
    {
        write2Log("file not exist.");
        return;
    }
    m_mediumFile->close();
    if (!m_mediumFile->open(QFile::ReadOnly))
    {
        m_mediumFile->close();
        QString str = "open file error!";
        str = SERVER_STRING + str;
        write2Log(str);
        return;
    }
    connect(m_fileSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(slot_fileSendProgress(qint64)));

    m_totalBytes = m_mediumFile->size();
    QByteArray outBlock;        //发送用数据缓存包. 2021-01-28 https://blog.csdn.net/Bing_Lee
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

    QString str = "Sending ";
    str = SERVER_STRING + str + currentFileName;
    write2Log(str);
    outBlock.resize(0);
}

void ChatServerWindow::slot_acceptFileConnection()
{
    initFileProcessValue();
    m_fileSocket = m_fileServer->nextPendingConnection();
    connect(m_fileSocket, SIGNAL(readyRead()), this, SLOT(slot_fileReceiveProgress()));
    connect(m_fileSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(slot_fileReceiveProgress()));
    write2Log("client file socket connected.");
}

void ChatServerWindow::slot_fileReceiveProgress()
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

void ChatServerWindow::slot_fileSendProgress(qint64 bytes)
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
    if (m_processedBytes == m_totalBytes && m_totalBytes != 0)
    {
        write2Log("[Server] " + m_fileNameStr + " have sent.");
        m_mediumFile->close();
        initFileProcessValue();
        qDebug() << "m_processedBytes " << m_processedBytes;
        qDebug() << "m_totalBytes " << m_totalBytes;
        qDebug() << "ui->progressBar_file->value() " << ui->progressBar_file->value();
    }
    if (m_processedBytes > m_totalBytes && m_totalBytes != 0)
    {
        m_mediumFile->close();
        write2Log("[Server] receive data more than oaiginal, error!");
        initFileProcessValue();
    }
}
