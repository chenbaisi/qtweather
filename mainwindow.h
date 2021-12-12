#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QString cityKey="";
    QNetworkAccessManager *mNetManager;
    QNetworkRequest *mNetRequest;
    QNetworkReply *reply;



private:
    Ui::MainWindow *ui;

private:
    void getWeather();
    QString getCityKey(QString city_name);
    void replyFinished(QNetworkReply *reply);
    void analyWeatherXML(QByteArray json);


};
#endif // MAINWINDOW_H
