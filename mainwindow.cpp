#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);
    this->setWindowTitle("Server");
    this->Initialize(QCoreApplication::applicationDirPath().append("/ini.xml"));
    this->tcpServer_ = new QTcpServer(this);

    QObject::connect(this->tcpServer_, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
    this->nextBlockSize_ = 0;
    this->slotStart();
}

MainWindow::~MainWindow()
{
    this->AutoRun();
    delete ui_;
}

void MainWindow::Initialize(const QString &_xmlPath)
{
    QDomDocument domDoc;
    QFile file(_xmlPath);
    if (file.open(QIODevice::ReadOnly))
    {
        if (domDoc.setContent(&file))
        {
            QDomElement domElement = domDoc.documentElement();
            QDomNode domNode = domElement.firstChild();
            while(!domNode.isNull())
            {
                if (domNode.isElement())
                {
                    QDomElement domElement = domNode.toElement();
                    if (!domElement.isNull())
                    {
                        if (domElement.tagName() == "pathToStorageDir")
                        {
                            this->storageDirPath_=domElement.text();
                        }
                    }
                    domNode = domNode.nextSibling();
                }
            }
        }
    }
}

void MainWindow::AutoRun()
{
    QSettings setting("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
//    QString path(QDir::toNativeSeparators(QApplication::applicationFilePath()));
    setting.setValue("StatisticSystemServer", QDir::toNativeSeparators(QApplication::applicationFilePath()));
    setting.sync();
}

void MainWindow::slotNewConnection()
{
    QTcpSocket* clientSocket = this->tcpServer_->nextPendingConnection();
    int idClient = clientSocket->socketDescriptor();
    this->mapClientSockets_[idClient] = clientSocket;
//    this->mapClientSockets_.insert(idClient, clientSocket);

    QObject::connect(this->mapClientSockets_[idClient], SIGNAL(disconnected()),
                     this->mapClientSockets_[idClient], SLOT(deleteLater()));
    QObject::connect(this->mapClientSockets_[idClient], SIGNAL(disconnected()),
                     this, SLOT(slotClientDisconnected()));
    QObject::connect(this->mapClientSockets_[idClient], SIGNAL(readyRead()),
                     this, SLOT(slotReadClient()));
    this->ui_->textBrowser->append(QTime::currentTime().toString().append(":   new client"));

//    this->client_ = this->tcpServer_->nextPendingConnection();
//    QObject::connect(this->client_, SIGNAL(disconnected()), this->client_, SLOT(deleteLater()));
//    QObject::connect(this->client_, SIGNAL(disconnected()), this, SLOT(slotClientDisconnected()));
//    QObject::connect(this->client_, SIGNAL(readyRead()), this, SLOT(slotReadClient()));
//    this->ui_->textBrowser->append(QTime::currentTime().toString().append(":   ПОДКЛЮЧИЛСЯ КЛИЕНТ!"));
}

void MainWindow::slotReadClient()
{
    QTcpSocket* clientSocket = (QTcpSocket*)sender();
    int idSender = clientSocket->socketDescriptor();
    this->inDataStream_.setDevice(clientSocket);

    QString testerNumber;
    int bytesAvailable;
    this->inDataStream_.setVersion(QDataStream::Qt_5_8);
    for ( ; ; ) {
        if (!this->nextBlockSize_)
        {
            bytesAvailable = clientSocket->bytesAvailable();
            if (bytesAvailable < sizeof(quint32))
                break;
            this->inDataStream_ >> this->nextBlockSize_;
        }
        bytesAvailable = clientSocket->bytesAvailable();
        if (clientSocket->bytesAvailable() < this->nextBlockSize_)
        {
            break;
        }

        this->inDataStream_ >> testerNumber;
        this->ui_->textBrowser->append("\n");
        this->ui_->textBrowser->append(QString("Tester № : ").append(testerNumber).append(": "));

        quint16 packageContent;
        this->inDataStream_ >> packageContent;
//        this->ui_->textBrowser->append(QString("packageContent: ").append(QString::number(packageContent)));

        if (packageContent == this->CHECK_CONNECTION_IDENTIFIER)
        {

//            QByteArray arrBlock;
//            QDataStream out(&arrBlock, QIODevice::WriteOnly);
//            out.setVersion(QDataStream::Qt_5_8);
//            out << quint32(0) << this->CHECK_CONNECTION_IDENTIFIER;
//            out.device()->seek(0);
//            out << quint32(arrBlock.size() - sizeof(quint32));
////            this->mapClientSockets_[idSender]->write(arrBlock);
//            this->client_->write(arrBlock);
        } else
            if (packageContent == this->TEXT_IDENTIFIER)
            {
                QString textFromClient;
                this->inDataStream_ >> textFromClient;
                this->nextBlockSize_ = 0;
//                this->ui_->textBrowser->append("Client: ");
                this->ui_->textBrowser->append(textFromClient);
            } else
                if (packageContent == this->CSV_IDENTIFIER)
                {
                    QString fileName;
                    this->inDataStream_ >> fileName;

                    QString path(this->storageDirPath_);
                    path.append("/");
                    path.append(fileName);
                    QFile csvFile(path);
                    if (!csvFile.open(QFile::WriteOnly))
                        this->ui_->textBrowser->setText("csv-file not open");
                    else
                    {
//                        QTextCodec* codec = QTextCodec::codecForName("Windows - 1251");
                        QByteArray byteArrayContent;
                        this->inDataStream_ >> byteArrayContent;
//                        QString strContent(codec->toUnicode(byteArrayContent));
//                        csvFile.write(strContent.toStdString().data());
                        csvFile.write(byteArrayContent);
                        csvFile.close();
                        this->ui_->textBrowser->append(QTime::currentTime().toString().
                                                       append(QString(": received csv: ").append(fileName)));
                        this->nextBlockSize_ = 0;
                    }
                }
    }
}

void MainWindow::slotStart()
{
    if (!this->tcpServer_->listen(QHostAddress::Any, 5000))
    {
        QMessageBox::critical(0, "Server Error", "Unable to start the server:" + this->tcpServer_->errorString());
        this->tcpServer_->close();
        return;
    } else
    {
        this->ui_->textBrowser->append(QTime::currentTime().toString().append(":   Server is running"));
    }
    //    foreach(QNetworkInterface netInterface, QNetworkInterface::allInterfaces())
    //    {
    //        // Return only the first non-loopback MAC Address
    //        if (!(netInterface.flags() & QNetworkInterface::IsLoopBack))
    //            qDebug() <<  netInterface.hardwareAddress();
    //    }
    //    QList<QHostAddress> addressList = QNetworkInterface::allAddresses();
    //    foreach( QHostAddress address, addressList )
    //        qDebug() << address.toString();

}

void MainWindow::slotClientDisconnected()
{
    this->ui_->textBrowser->append("\n");
    this->ui_->textBrowser->append(QTime::currentTime().toString().append(":   tester disconnected"));
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    this->ui_->textBrowser->resize(this->width() - 20, this->height() - 50);
}
