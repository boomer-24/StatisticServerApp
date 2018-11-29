#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QFileDialog>
#include <QTime>
#include <QMessageBox>
#include <QNetworkInterface>
#include <QtXml>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void Initialize(const QString& _xmlPath);
    void AutoRun();

public slots:
    void slotNewConnection();
    void slotReadClient();
    void slotStart();
    void slotClientDisconnected();

private:
    Ui::MainWindow *ui_;
    QTcpServer* tcpServer_;
    quint32 nextBlockSize_;
    QByteArray byteArray_;
    QDataStream inDataStream_;

    QMap<int, QTcpSocket*> mapClientSockets_;
    QTcpSocket* client_;
    QString storageDirPath_;

    const quint16 CHECK_CONNECTION_IDENTIFIER = 0;
    const quint16 TEXT_IDENTIFIER = 1;
    const quint16 CSV_IDENTIFIER = 2;

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // MAINWINDOW_H
