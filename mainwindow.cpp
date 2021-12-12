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

        url.setUrl(webApi + cityKey);
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

        QString date[5] = {"NULL"}; //存储日期
        QJsonParseError err;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(json, &err);

        QJsonObject jsonObj = jsonDoc.object().value("data").toObject();
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


        QJsonObject tomorrow = forecast[1].toObject(); //明天
        ui->labelDay2->setText("明天");
        ui->labelDay2Temperature->setText(tomorrow.value("low").toString().replace("低温","")+"/"+tomorrow.value("high").toString().replace("高温",""));

        QJsonObject day_3 = forecast[2].toObject(); //后天
        ui->labelDay3->setText(day_3.value("week").toString());
        ui->labelDay3Temperature->setText(day_3.value("low").toString().replace("低温","")+"/"+day_3.value("high").toString().replace("高温",""));

        QJsonObject day_4 = forecast[3].toObject(); //大后天
        ui->labelDay4->setText(day_3.value("week").toString());
        ui->labelDay4Temperature->setText(day_4.value("low").toString().replace("低温","")+"/"+day_4.value("high").toString().replace("高温",""));

        QJsonObject day_5 = forecast[4].toObject(); //大后天
        ui->labelDay5->setText(day_5.value("week").toString());
        ui->labelDay5Temperature->setText(day_5.value("low").toString().replace("低温","")+"/"+day_5.value("high").toString().replace("高温",""));
}





