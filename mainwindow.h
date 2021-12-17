#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QLineSeries>
#include <QValueAxis>
#include <QtCharts>

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



private slots:
    void on_btnSearch_clicked();

private:
    Ui::MainWindow *ui;

private:
    void getWeather();
    void createChart(QString hightemp[],QString lowtemp[],QString date[],QString weeks[]);

    QString getCityKey(QString city_name);
    void replyFinished(QNetworkReply *reply);
    void analyWeatherXML(QByteArray json);

    QCategoryAxis *axisX1;
    QCategoryAxis *axisX2;
    QValueAxis *axisY;


};
#endif // MAINWINDOW_H
