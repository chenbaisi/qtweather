#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QMessageBox>
#include <QJsonParseError>
#include <QDebug>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QPointF>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->labelDay1Temperature->adjustSize();
    getWeather();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::getWeather()
{
    QUrl url;
    QNetworkAccessManager manager;
    QEventLoop loop;
    QJsonDocument JD;
    QJsonParseError JPE;
    QString city_name;

    QString webApi = "http://t.weather.itboy.net/api/weather/city/";
    if(!cityKey.isEmpty())
        mNetRequest->setUrl(QUrl(webApi + cityKey));
    else{
        QString surl = "http://ip-api.com/json/?lang=zh-CN";//获取当地ip地址
        url.setUrl(surl);
        reply = manager.get(QNetworkRequest(url));
        //请求结束并下载完成后，退出子事件循环
        QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        //开启子事件循环
        loop.exec();
        QByteArray localCity = reply->readAll();
        JD = QJsonDocument::fromJson(localCity, &JPE);
        if(JPE.error == QJsonParseError::NoError) {
            if(JD.isObject()) {
                QJsonObject obj = JD.object();
                if(obj.contains("city")) {
                    QJsonValue JV_city = obj.take("city");
                    if(JV_city.isString()) {
                        city_name = JV_city.toString().replace("市","");
                        qDebug() << "所在城市"+city_name;
                    }
                }
            }
        }
        cityKey = getCityKey(city_name);

        url.setUrl(webApi + "101281601");
        reply = manager.get(QNetworkRequest(url));//获取文件
        QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
        QByteArray weather = reply->readAll();
        if(!weather.isEmpty()){
            analyWeatherXML(weather); //解析天气信息的数据
        }
        //        mNetRequest->setUrl(QUrl(webApi + cityKey));
    }


    //    mNetRequest->setHeader(QNetworkRequest::UserAgentHeader, "RT-Thread ART");
    //    mNetManager->get(*mNetRequest);
    //    connect(mNetManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));
    //    qDebug() << QTime::currentTime().toString();
}

QString MainWindow::getCityKey(QString city_name)
{
    QString cityID;

    QFile file("D:/Development/Qt-projects/Lab/Finalproject/weather/cityID.txt");  //文件为城市代码
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, QString("警告"),QString("无法加载城市数据库！\n请联系开发者"));
        return NULL;
    }

    QTextStream TS(&file);
    QString s = TS.readAll();
    file.close();
    QStringList SL = s.split("\n");
    for(int i=0; i<SL.length(); i++){
        QString line = SL.at(i);
        //            qDebug() << line;
        if (line.contains(city_name)) {
            cityID = line.left(line.indexOf("="));
            qDebug() << "所在城市key"+cityID;
            break;
        }
    }

    if(QString(cityID).isEmpty())
    {
        QMessageBox::warning(this, QString("警告"),QString("没有找到" + city_name + "的城市ID" + "\n请检查重试"));
        return NULL;
    }

    return cityID;
}

void MainWindow::analyWeatherXML(QByteArray json)
{
    if(json.isEmpty())
        return ;

    QString hightemp[7] = {"NULL"};//存储一周当天最高气温
    QString lowtemp[7] = {"NULL"};//存储一周当天最低气温
    QString date[7] = {"NULL"}; //存储日期

    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json, &err);

    QJsonObject jsonObj = jsonDoc.object().value("data").toObject();
    QJsonObject yesterday = jsonObj.value("yesterday").toObject(); //昨天
    hightemp[0] = yesterday.value("high").toString();//昨天最高气温
    lowtemp[0] = yesterday.value("low").toString();
    QJsonArray forecast = jsonObj.value("forecast").toArray();

    QJsonObject cityInfo = jsonDoc.object().value("cityInfo").toObject();
    ui->labelCity->setText(cityInfo.value("parent").toString() +
                           cityInfo.value("city").toString());
    ui->labelTime->setText("最后更新于：" +
                           jsonDoc.object().value("time").toString());

    QJsonObject today = forecast[0].toObject(); //今天
    ui->labelTemperature->setText(jsonObj.value("wendu").toString()+"℃");
    ui->labelWeather->setText(today.value("type").toString());
    ui->labelHumidity->setText(jsonObj.value("shidu").toString());
    ui->labelWind->setText(today.value("fx").toString()+today.value("fl").toString());
    ui->labelAirIndex->setText(QString::number(today.value("aqi").toInt()));
    ui->labelAirQuality->setText(jsonObj.value("quality").toString());
    ui->labelDay1->setText("今天");
    ui->labelDay1Temperature->setText(today.value("low").toString()+"/"+today.value("high").toString());
    hightemp[1] = today.value("high").toString();
    lowtemp[1] = today.value("low").toString();


    QJsonObject tomorrow = forecast[1].toObject(); //明天
    ui->labelDay2->setText("明天");
    ui->labelDay2Temperature->setText(tomorrow.value("low").toString().replace("低温","")+"/"+tomorrow.value("high").toString().replace("高温",""));
    hightemp[2] = tomorrow.value("high").toString();
    lowtemp[2] = tomorrow.value("low").toString();

    QJsonObject day_3 = forecast[2].toObject(); //后天
    ui->labelDay3->setText(day_3.value("week").toString());
    ui->labelDay3Temperature->setText(day_3.value("low").toString().replace("低温","")+"/"+day_3.value("high").toString().replace("高温",""));
    hightemp[3] = day_3.value("high").toString();
    lowtemp[3] = day_3.value("low").toString();

    QJsonObject day_4 = forecast[3].toObject(); //第四天
    ui->labelDay4->setText(day_3.value("week").toString());
    ui->labelDay4Temperature->setText(day_4.value("low").toString().replace("低温","")+"/"+day_4.value("high").toString().replace("高温",""));
    hightemp[4] = day_4.value("high").toString();
    lowtemp[4] = day_4.value("low").toString();

    QJsonObject day_5 = forecast[4].toObject(); //第五天
    ui->labelDay5->setText(day_5.value("week").toString());
    ui->labelDay5Temperature->setText(day_5.value("low").toString().replace("低温","")+"/"+day_5.value("high").toString().replace("高温",""));
    hightemp[5] = day_5.value("high").toString();
    lowtemp[5] = day_5.value("low").toString();

    QJsonObject day_6 = forecast[5].toObject(); //第六天
    hightemp[6] = day_6.value("high").toString();
    lowtemp[6] = day_6.value("low").toString();

    createChart(hightemp,lowtemp);
}

void MainWindow::createChart(QString hightemp[],QString lowtemp[])
{ //创建图表
    QChart *chart = new QChart();
    chart->legend()->hide();
    ui->chartView->setChart(chart);

    axisY = new QValueAxis;
    axisY->setRange(-20, 40);
    axisY->setTitleText("温度");
    axisY->setTickCount(9);
    axisY->setLabelFormat("%d"); //标签格式
    axisY->setMinorTickCount(2);//4

    axisX1 = new QCategoryAxis;
    axisX1->setRange(0, 6);
    axisX1->setTitleText("日期");
    axisX1->append("昨天",0);
    axisX1->append("今天",1);
    axisX1->append("明天",2);
    axisX1->append("后天",3);
    axisX1->append("大后天",4);
    axisX1->append("大大后天",5);
    axisX1->append("大大大后天",6);
    axisX1->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);

    chart->addAxis(axisX1,Qt::AlignTop);
    chart->addAxis(axisY,Qt::AlignLeft);
    axisX1->setGridLineVisible(false);
    axisY->setGridLineVisible(false);
    QLineSeries *serieshigh = new QLineSeries();
    QLineSeries *serieslow = new QLineSeries();

    for (int i = 0;i < 7 ;i++ ) {
        hightemp[i].replace("高温","");
        hightemp[i].replace("℃","");
        lowtemp[i].replace("低温","");
        lowtemp[i].replace("℃","");

//        qDebug() << hightemp[i].toDouble();
//        qDebug() << lowtemp[i].toDouble();
    }

    *serieshigh << QPointF(0,hightemp[0].toDouble()) << QPointF(1,hightemp[1].toDouble())
            << QPointF(2,hightemp[2].toDouble()) << QPointF(3,hightemp[3].toDouble())
            << QPointF(4,hightemp[4].toDouble()) << QPointF(5,hightemp[5].toDouble())
            << QPointF(6,hightemp[6].toDouble());
    *serieslow << QPointF(0,lowtemp[0].toDouble()) << QPointF(1,lowtemp[1].toDouble())
            << QPointF(2,lowtemp[2].toDouble()) << QPointF(3,lowtemp[3].toDouble())
            << QPointF(4,lowtemp[4].toDouble()) << QPointF(5,lowtemp[5].toDouble())
            << QPointF(6,lowtemp[6].toDouble());

    chart->addSeries(serieshigh);
    chart->addSeries(serieslow);

    chart->setAxisX(axisX1,serieshigh);
    chart->setAxisY(axisY,serieshigh);
    chart->setAxisX(axisX1,serieslow);
    chart->setAxisY(axisY,serieslow);
}






