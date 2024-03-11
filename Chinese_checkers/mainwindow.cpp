#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMenuBar>
#include <QDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QString>
#include <QtNetwork>
#include <regex>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //窗口基本设置
    this->setWindowIcon(QIcon(":/image/board.png"));
    this->setWindowTitle("网络对战跳棋小程序 by cxs");
    this->setFixedSize(900,700);

    //“创建服务器”与“连接服务器”按钮设置
    connect(ui->createServerButton, &QPushButton::clicked,this,[=](){
        qDebug() << "点击了“创建服务器(Create server)”按钮";

        //获取本机的IP信息
        QString localHostName = QHostInfo::localHostName();
        QHostInfo info = QHostInfo::fromName(localHostName);
        QString text("Host IP: ");
        foreach(QHostAddress address,info.addresses()){
            if(address.protocol() == QAbstractSocket::IPv4Protocol)
                text += address.toString();
        }
        text += "\n点击“OK”以创建服务器，点击“Cancel”以取消。";

        //如果选择OK，创建服务器，跳转页面
        if (QMessageBox::Ok == QMessageBox::question(this, "创建服务器", text,QMessageBox::Ok|QMessageBox::Cancel)){
            this->m_pNetworkGame = new NetworkGame(this, true, this->width(), this->height(), "");
            this->hide();
            m_pNetworkGame->setGeometry(this->geometry());
            m_pNetworkGame->show();
        }
    });
    connect(ui->connectToServerButton, &QPushButton::clicked, this, [=](){
        qDebug() << "点击了“连接服务器(Connect to server)”按钮";

        //输入拟连接的服务器的IPv4地址
        bool ok; //如果点击“OK”，则ok = true；否则（点击了“Cancel”或关闭页面），ok = false
        QString text = QInputDialog::getText(this, "连接服务器", "请输入服务器IPv4地址，并等候3秒以连接：", QLineEdit::Normal, "127.0.0.1", &ok);

        if (ok && !text.isEmpty()){
            //检查所输入的字符串是否符合IPv4地址格式要求
            std::regex pattern(R"(^((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})(\.((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})){3}$)");
            if (!std::regex_match(text.toStdString(), pattern)){
                //如果不符合IPv4格式要求，出现提示弹窗
                qDebug() << "所输入的字符串不符合IPv4地址格式要求，输入内容 text = " << text;
                QMessageBox::warning(this, "错误信息", "所输入的字符串不符合IPv4地址格式要求！");
            }
            else{
                qDebug() << "在“连接服务器”窗口输入了：" << text << " ok =" << ok ;
                this->m_pNetworkGame = new NetworkGame(this, false, this->width(), this->height(), text);
                this->hide();
                m_pNetworkGame->setGeometry(this->geometry());
                m_pNetworkGame->show();
            }
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

