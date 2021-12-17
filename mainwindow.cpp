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

static const QMap<QString, QString> weatherMap {
    {"NA", "999"},
    {"晴", "100"},
    {"多云", "101"},
    {"少云", "102"},
    {"阴", "104"},
    {"阵雨", "300"},
    {"雷阵雨", "302"},
    {"小雨", "305"},
    {"中雨", "306"},
    {"大雨", "307"},
    {"小到中雨", "314"},
    {"中到大雨", "315"},
    {"大到暴雨", "316"},
    {"暴雨到大暴雨", "317"},
    {"雨", "399"},
    {"雪", "499"},
    {"小雪", "400"},
    {"中雪", "401"},
    {"大雪", "402"},
    {"暴雪", "404"},
    {"雨夹雪", "404"},
    {"霾", "502"},
    {"雾", "501"},
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->scrollArea->setFrameShape(QFrame::NoFrame);
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
    if(cityKey.isEmpty()){
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
    }

    url.setUrl(webApi + "101281601");
    reply = manager.get(QNetworkRequest(url));//获取文件
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    QByteArray weather = reply->readAll();
    if(!weather.isEmpty())
        analyWeatherXML(weather); //解析天气信息的数据

    //    mNetRequest->setUrl(QUrl(webApi + cityKey));
    //    mNetRequest->setHeader(QNetworkRequest::UserAgentHeader, "RT-Thread ART");
    //    mNetManager->get(*mNetRequest);
    //    connect(mNetManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));
    //    qDebug() << QTime::currentTime().toString();
}

QString MainWindow::getCityKey(QString city_name)
{
    QString cityKey;

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
            cityKey = line.left(line.indexOf("="));
            qDebug() << "所在城市key:"+cityKey;
            break;
        }
    }

    if(QString(cityKey).isEmpty())
    {
        QMessageBox::warning(this, QString("警告"),QString("没有找到" + city_name + "的城市ID" + "\n请检查重试"));
        return NULL;
    }

    return cityKey;
}

void MainWindow::analyWeatherXML(QByteArray json)
{
    if(json.isEmpty())
        return ;

    QString hightemp[7] = {"NULL"};//存储一周当天最高气温和一周最高温度
    QString lowtemp[7] = {"NULL"};//存储一周当天最低气温一周最低温度
    QString date[7] = {"NULL"}; //存储日期
    QString weeks[7] = {NULL};//存储星期
    QString wtype;
    QString icon_path;

    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json, &err);

    QJsonObject jsonObj = jsonDoc.object().value("data").toObject();
    QJsonArray forecast = jsonObj.value("forecast").toArray();
    QJsonObject cityInfo = jsonDoc.object().value("cityInfo").toObject();
    ui->labelCity->setText(cityInfo.value("parent").toString()+cityInfo.value("city").toString());
    ui->labelTime->setText("最后更新于：" +
                           jsonDoc.object().value("time").toString());
    //昨天
    QJsonObject yesterday = jsonObj.value("yesterday").toObject(); //昨天
    hightemp[0] = yesterday.value("high").toString();//昨天最高气温
    lowtemp[0] = yesterday.value("low").toString();
    date[0] = yesterday.value("date").toString();
    weeks[0] = yesterday.value("week").toString();

    //今天
    QJsonObject today = forecast[0].toObject();
    wtype = today.value("type").toString(); //获取天气类型
//    qDebug() << wtype;
    icon_path = ":/weather-icon/" + weatherMap[wtype] + ".png";
    QPixmap pixmap1(icon_path);
    ui->labelDay1WeatherImg->setPixmap(pixmap1.scaled(70,70)); //展示天气图片
    ui->labelTemperature->setText(jsonObj.value("wendu").toString()+"℃");
    ui->labelWeather->setText(today.value("type").toString());
    ui->labelHumidity->setText(jsonObj.value("shidu").toString());
    ui->labelWind->setText(today.value("fx").toString()+today.value("fl").toString());
    ui->labelAirIndex->setText(QString::number(today.value("aqi").toInt()));
    ui->labelAirQuality->setText(jsonObj.value("quality").toString());
    ui->labelDay1->setText("今天");
    ui->labelDay1Temperature->setText(today.value("low").toString().replace("低温","")+"/"+today.value("high").toString().replace("高温",""));
    hightemp[1] = today.value("high").toString();
    lowtemp[1] = today.value("low").toString();
    date[1] = today.value("date").toString();
    weeks[1] = today.value("week").toString();

    //明天
    QJsonObject tomorrow = forecast[1].toObject();
    wtype = tomorrow.value("type").toString(); //获取天气类型
    icon_path = ":/weather-icon/" + weatherMap[wtype] + ".png";
    QPixmap pixmap2(icon_path);
    ui->labelDay2WeatherImg->setPixmap(pixmap2.scaled(50,50)); //展示天气图片
    ui->labelDay2->setText("明天");
    ui->labelDay2Temperature->setText(tomorrow.value("low").toString().replace("低温","")+"/"+tomorrow.value("high").toString().replace("高温",""));
    hightemp[2] = tomorrow.value("high").toString();
    lowtemp[2] = tomorrow.value("low").toString();
    date[2] = tomorrow.value("date").toString();
    weeks[2] = tomorrow.value("week").toString();

    //后天
    QJsonObject day_3 = forecast[2].toObject();
    wtype = day_3.value("type").toString(); //获取天气类型
    icon_path = ":/weather-icon/" + weatherMap[wtype] + ".png";
    QPixmap pixmap3(icon_path);
    ui->labelDay3WeatherImg->setPixmap(pixmap3.scaled(50,50)); //展示天气图片
    ui->labelDay3->setText(day_3.value("week").toString());
    ui->labelDay3Temperature->setText(day_3.value("low").toString().replace("低温","")+"/"+day_3.value("high").toString().replace("高温",""));
    hightemp[3] = day_3.value("high").toString();
    lowtemp[3] = day_3.value("low").toString();
    date[3] = day_3.value("date").toString();
    weeks[3] = day_3.value("week").toString();

    //第四天
    QJsonObject day_4 = forecast[3].toObject();
    wtype = day_4.value("type").toString(); //获取天气类型
    icon_path = ":/weather-icon/" + weatherMap[wtype] + ".png";
    QPixmap pixmap4(icon_path);
    ui->labelDay4WeatherImg->setPixmap(pixmap4.scaled(50,50)); //展示天气图片
    ui->labelDay4->setText(day_4.value("week").toString());
    ui->labelDay4Temperature->setText(day_4.value("low").toString().replace("低温","")+"/"+day_4.value("high").toString().replace("高温",""));
    hightemp[4] = day_4.value("high").toString();
    lowtemp[4] = day_4.value("low").toString();
    date[4] = day_4.value("date").toString();
    weeks[4] = day_4.value("week").toString();

    //第五天
    QJsonObject day_5 = forecast[4].toObject();
    wtype = day_5.value("type").toString(); //获取天气类型
    icon_path = ":/weather-icon/" + weatherMap[wtype] + ".png";
    QPixmap pixmap5(icon_path);
    ui->labelDay5WeatherImg->setPixmap(pixmap5.scaled(50,50)); //展示天气图片
    ui->labelDay5->setText(day_5.value("week").toString());
    ui->labelDay5Temperature->setText(day_5.value("low").toString().replace("低温","")+"/"+day_5.value("high").toString().replace("高温",""));
    hightemp[5] = day_5.value("high").toString();
    lowtemp[5] = day_5.value("low").toString();
    date[5] = day_5.value("date").toString();
    weeks[5] = day_5.value("week").toString();

    //第六天
    QJsonObject day_6 = forecast[5].toObject();
    hightemp[6] = day_6.value("high").toString();
    lowtemp[6] = day_6.value("low").toString();
    date[6] = day_6.value("date").toString();
    weeks[6] = day_6.value("week").toString();

    createChart(hightemp,lowtemp,date,weeks);
}

void MainWindow::createChart(QString hightemp[],QString lowtemp[],QString date[],QString weeks[])
{ //创建图表
    int highest = -100;
    int lowest = 100;
    for (int i = 0;i < 7 ;i++ ) {
        hightemp[i].replace("高温","");
        hightemp[i].replace("℃","");
        lowtemp[i].replace("低温","");
        lowtemp[i].replace("℃","");
        if(hightemp[i].toDouble() >= highest) highest = hightemp[i].toDouble();
        if(lowtemp[i].toDouble() <= lowest) lowest = lowtemp[i].toDouble();

    }
//    qDebug() << highest;
//    qDebug() << lowest;

    QChart *chart = new QChart();
    chart->legend()->hide();
    ui->chartView->setChart(chart);
    ui->chartView->setRenderHint(QPainter::Antialiasing);

    //纵坐标轴
    axisY = new QValueAxis;
    axisY->setRange(lowest-3,highest+3);
    //    axisY->setTitleText("温度");
    axisY->setTickCount(9);
    axisY->setLabelFormat("%d"); //标签格式
    axisY->setMinorTickCount(2);//4

    //上横坐标轴
    QDate d = QDate::currentDate();
    axisX1 = new QCategoryAxis;
    axisX1->setRange(0, 6);
    //    axisX1->setTitleText("日期");
    axisX1->append("昨天",0);
    axisX1->append("今天",1);
    axisX1->append("明天",2);
    axisX1->append("后天",3);
    axisX1->append(weeks[4],4);
    axisX1->append(weeks[5],5);
    axisX1->append(weeks[6],6);
    axisX1->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);

    //下横坐标轴
    axisX2 = new QCategoryAxis;
    axisX2->setRange(0, 6);
    axisX2->append(QString::number(d.month())+"/"+date[0],0);
    axisX2->append(QString::number(d.month())+"/"+date[1],1);
    axisX2->append(QString::number(d.month())+"/"+date[2],2);
    axisX2->append(QString::number(d.month())+"/"+date[3],3);
    axisX2->append(QString::number(d.month())+"/"+date[4],4);
    axisX2->append(QString::number(d.month())+"/"+date[5],5);
    axisX2->append(QString::number(d.month())+"/"+date[6],6);
    axisX2->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);

    axisX1->setGridLineVisible(false);// 主网格线不可视
    axisX2->setGridLineVisible(false);
    axisY->setGridLineVisible(false);
    axisY->setMinorGridLineVisible(false); // 次网格线不可视
    axisX1->setMinorGridLineVisible(false);
    axisX2->setMinorGridLineVisible(false);
    axisY->setLabelsVisible(false);//y轴刻度标签不可视
    axisX1->setLineVisible(false);//x轴线不可视
    axisX2->setLineVisible(false);
    axisY->setLineVisible(false);//y轴线不可视

    chart->addAxis(axisX1,Qt::AlignTop);
    chart->addAxis(axisY,Qt::AlignLeft);
    chart->addAxis(axisX2,Qt::AlignBottom);


    QLineSeries *serieshigh = new QLineSeries();
    QLineSeries *serieslow = new QLineSeries();

    *serieshigh << QPointF(0,hightemp[0].toDouble()) << QPointF(1,hightemp[1].toDouble())
            << QPointF(2,hightemp[2].toDouble()) << QPointF(3,hightemp[3].toDouble())
            << QPointF(4,hightemp[4].toDouble()) << QPointF(5,hightemp[5].toDouble())
            << QPointF(6,hightemp[6].toDouble());
    *serieslow << QPointF(0,lowtemp[0].toDouble()) << QPointF(1,lowtemp[1].toDouble())
            << QPointF(2,lowtemp[2].toDouble()) << QPointF(3,lowtemp[3].toDouble())
            << QPointF(4,lowtemp[4].toDouble()) << QPointF(5,lowtemp[5].toDouble())
            << QPointF(6,lowtemp[6].toDouble());

    serieshigh->setPointLabelsClipping(false);//不切割边缘点标签
    serieslow->setPointLabelsClipping(false);
    serieshigh->setPointsVisible(true);//设置点标签显示格式
    serieslow->setPointsVisible(true);
    serieshigh->setPointLabelsVisible(true);//设置折点可见
    serieslow->setPointLabelsVisible(true);
    serieshigh->setPointLabelsFormat("@yPoint℃");//只显示纵坐标
    serieslow->setPointLabelsFormat("@yPoint℃");

    chart->addSeries(serieshigh);
    chart->addSeries(serieslow);

    chart->setAxisX(axisX1,serieshigh);
    chart->setAxisY(axisY,serieshigh);
    chart->setAxisX(axisX1,serieslow);
    chart->setAxisY(axisY,serieslow);
}

void MainWindow::on_btnSearch_clicked()
{
    QString city = ui->lineEditSearch->text();
    qDebug() << city;
    cityKey = getCityKey(city);
    getWeather();
}

