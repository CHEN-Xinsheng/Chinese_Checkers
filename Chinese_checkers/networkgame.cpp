#include "networkgame.h"
#include "ui_networkgame.h"
#include "place.h"

#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMenuBar>
#include <QDialog>
#include <QMessageBox>
#include <QString>
#include <QtNetwork>
#include <QLabel>
#include <QFont>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>
#include <QVector>
#include <QtGlobal> // qAbs()函数
#include <cstdlib> // rand()
#include <ctime> // time()

NetworkGame::NetworkGame(QWidget *_parent, bool _isServer,int _windowWidth, int _windowHeight, QString _addr) :
    QMainWindow(_parent),
    ui(new Ui::NetworkGame),
    parent(_parent),
    isServer(_isServer)
{
    ui->setupUi(this);

    //初始化places数组
    for (int i = 0; i < 17; ++i)
        for(int j = 0; j < 17; ++j)
            places[i][j] = nullptr;

    //窗口基本设置
    this->setAttribute(Qt::WA_DeleteOnClose,1); //当close此窗口时，将其delete
    this->setWindowIcon(QIcon(":/image/board.png"));
    this->setWindowTitle("网络对战跳棋小程序 by cxs");
    this->setFixedSize(_windowWidth,_windowHeight);
    //返回开始界面按钮
    QPushButton* backButton = new QPushButton(this);
    backButton->setText("返回开始界面");
    backButton->move(this->width() - backButton->width(), this->height() - backButton->height());
    connect(backButton, &QPushButton::clicked, this, [=](){
        qDebug() << "点击了“返回开始界面”按钮";
        //点击返回按钮，回到开始界面，并且把this析构掉
        backToStart();
    });

    //设置棋盘参数
    int diam = 35;
    int offsetX = 260;
    int offsetY = 30;
    int gapX = 45;
    int gapY = gapX * 1.732 / 2.0;
    //画出所有落子点
    for(int i = 0 ; i < 17; ++i){
        for(int j = 0; j < 17; ++j){
            if (isAPlace(i, j)){
                places[i][j] = new Place(":/image/place.jpg",i, j, diam, diam);
                places[i][j]->setParent(this);
                places[i][j]->move(offsetX - i*gapX/2 + j*gapX, offsetY + i*gapY);
                //设置好按钮被点击时的函数
                connect(places[i][j], &QPushButton::clicked, this, [=](){
                    clickPiece(i, j);
                });
            }
        }
    }


    //如果this是服务器
    if(isServer){
        //服务器为1号（红色）玩家
        m_playerId = PlayerId::Red;
        //显示"等待连接中..."
        ui->infoLabel->setText("等待连接中...");
        //创建服务器，并等候连接
        m_tcpServer = new QTcpServer(this);
        m_tcpServer->listen(QHostAddress::Any, 9999);
        connect(m_tcpServer, &QTcpServer::newConnection, this, &NetworkGame::newConnectionSlot);
    }
    //如果this不是服务器
    else{
        //客户端为2号（绿色）玩家
        m_playerId = PlayerId::Green;
        m_tcpSocket = new QTcpSocket(this);
        m_tcpSocket->connectToHost(QHostAddress(_addr), 9999);
        //设置好读取信息的函数
        connect(m_tcpSocket, &QTcpSocket::readyRead, this, &NetworkGame::receive);
        //等待连接服务器，最多3秒
        if (m_tcpSocket->waitForConnected(3000)){
            qDebug() << "成功连接到服务器，服务器地址：" << _addr;
            ui->infoLabel->setText("成功连接到服务器，服务器地址：" + _addr);

        }
        else{
            qDebug() << "连接服务器失败，输入的服务器地址：" << _addr;
            ui->infoLabel->setText("连接服务器失败，2秒后自动返回开始界面。");
            backToStart(2000);//返回开始界面
                    //问题：backToStart()函数里面如果没有QTImer，会连同开始窗口一起关掉；如果有QTimer，即使定时设置为0，也能正常显示开始窗口
        }
    }

}
//inline void NetworkGame::displayText(QString text){
//    qDebug() << "调用了NetworkGame::displayText(QString text), text = " << text;
//    infoLabel->setText(text);
//}
bool NetworkGame::isAPlace(int row, int col){//判断一个坐标（斜坐标系中）是否属于棋盘
    if (0 <= row && row <= 12)
        if (4 <= col && col <= 4 + row)
            return true;
    if (4 <= row && row <= 16)
        if (row - 4 <= col && col <= 12)
            return true;
    return false;
}

void NetworkGame::backToStart(int msec){
    //msec（默认为0）毫秒后回到开始页面，并且把当前所处的NetworkGame析构掉
    qDebug() << "调用了NetworkGame::backToStart()";

    QTimer* timer = new QTimer(this);
    timer->start(msec);
    connect(timer, &QTimer::timeout, this, [=](){
        this->hide();
        parent->setGeometry(this->geometry());
        parent->show();
        this->~NetworkGame();
    });

}
void NetworkGame::paintEvent(QPaintEvent* e){
//    qDebug() << "调用了NetworkGame::paintEvent(QPaintEvent* e)";
    //呈现初始场景
    QPainter painter(this);
    //画出背景图片
    painter.drawPixmap(0,0,this->width(),this->height(),QPixmap(":/image/background.jpg"));
            //bug 2 :QPainter语句必须在paintEvent中

//    //画出所有棋子
//    for (int i = 0; i < 17; ++i)
//        for(int j = 0; j < 17; ++j)
//            if (places[i][j] != nullptr){

//            }

}

void NetworkGame::newConnectionSlot(){
    //如果当前已经有连接，拒绝新连接
    if(m_tcpSocket != nullptr)
        return;
    //否则，连接上
    qDebug() << "调用了NetworkGame::newConnectionSlot()";
    m_tcpSocket = m_tcpServer->nextPendingConnection();
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &NetworkGame::receive);
    ui->infoLabel->setText("成功连接到客户端");
    //抽签
    srand(time(0));
    int randnum = rand() % 2 + 1;
    send(randnum, 0, 0, 0, 0);
    readyToBeginGame(randnum);
}

void NetworkGame::readyToBeginGame(int _firstMovePlayerId){
    //如果我方是服务器
    if (this->isServer){
        //监测是否传来服务器已经准备好的信息
        bool clientIsReady = false;
        connect(this, &NetworkGame::clientIsReady, this,[=]()mutable{
            clientIsReady = true;
        });
        //如果我方点击了“Ok”（表示已经准备好开始游戏）
        if (QMessageBox::Ok == displayDrawResult(_firstMovePlayerId)){
            //如果此时客户端已经准备好
            if (clientIsReady){
                //直接开始游戏
                playGame();
            }
            //如果此时客户端还没有准备好
            else{
                //等待客户端准备好
                connect(this, &NetworkGame::clientIsReady, this, [=](){
                    //先向客户端发送游戏开始的信息
                    send(1, 0, 2, 0, 0);
                    //本地开始游戏
                    playGame();
                });
            }
        }
    }
    //如果我方是客户端，那么这个函数是在收到信息之后被调用的
    else{
        //如果我方点击了“Ok”（表示已经准备好开始游戏）
        if (QMessageBox::Ok == displayDrawResult(_firstMovePlayerId)){
            //向服务器发送我方已准备好的信息
            send(2, 0, 1, 0, 0);
            //等待服务器开始游戏的信号
            connect(this, &NetworkGame::gameHasBegun, this, &NetworkGame::playGame);
        }
    }
}
QMessageBox::StandardButton NetworkGame::displayDrawResult(int _firstMovePlayerId){//显示抽签结果，等待“准备”以开始游戏
    this->firstMovePlayerId = PlayerId(_firstMovePlayerId);
    QString colorName[3] = {"", "红色", "绿色"};
    return QMessageBox::information(this,"抽签结果",
                            QString("我方是 %1 号玩家，持子颜色为[%2]；\n先手玩家是 %3 号玩家，持子颜色为[%4]。\n若已准备好开始游戏，请点击“OK”。")
                             .arg((int)m_playerId).arg(colorName[(int)m_playerId])
                            .arg(_firstMovePlayerId).arg(colorName[_firstMovePlayerId]));
}

void NetworkGame::playGame(){
    //参数初始化
    isPlaying = true; //bug3 遗漏此语句
    moveSequence.clear();
    currentPlayerId = firstMovePlayerId;
    turnCnt2 = 0;
    currTurn = 1;
    myTimeoutCnt = 0;
    oppTImeoutCnt = 0;

    //显示界面初始布局
    initBoard();
    ui->lcdNumber->display(timePerTurn);
    ui->infoLabel->setText("游戏进行中");
    ui->myPlayerLabel->setText(QString("我方：%1号，%2").arg((int)m_playerId)
                               .arg(m_playerId == PlayerId::Red ? "红色" : "绿色"));
    ui->currentPlayerLabel->setText(QString("当前回合玩家：%1 号").arg((int)currentPlayerId));
    ui->currTurnLabel->setText(QString("当前是第 %1 回合").arg(currTurn));
    ui->myTimeoutCntLabel->setText(QString("我方超时次数：%1").arg(myTimeoutCnt));

    //设置我方认输按钮
    connect(ui->admitDefeatButton, &QPushButton::clicked, this, [=](){
        qDebug() << QString("点击了“认输”按钮，当前是第 %1 回合。").arg(currTurn);
        //在第1-20回合不可认输（投降）
        if (currTurn <= 20)
            return;
        //告诉对方
        send((int)m_playerId, 2, 0, 0, 0);
        //在本地呈现结果
        disPlayWinLose(PlayerId(3 - (int)m_playerId), QString("%1 号玩家认输。").arg((int)m_playerId));
    });

    //开始游戏
    timer = new QTimer(this);
    timer->start(timePerTurn * 1000);
    //时间框设置
    timer1 = new QTimer(this);
    timer1->start(1000);
    displayTime = timePerTurn;
    connect(timer1, &QTimer::timeout, this,[=]()mutable{
        ui->lcdNumber->display(--displayTime);
    });

    //如果对方超时（对方传来超时的信息），takeTurn
    connect(this, &NetworkGame::oppTimeout, this, [=](){
        //记录一次超时
        oppTImeoutCnt++;
        //takeTurn（包括胜负检查）
        takeTurn();
    });

    //如果我方超时，记录一次超时并且takeTurn，同时告诉对方
    connect(timer,&QTimer::timeout,this, [=]()mutable{
        //如果当前回合不是我方，不进行处理。（对方是否超时以对方时钟为准）
        if (currentPlayerId != m_playerId)
            return;

        qDebug() << "我方超时！";
        //记录一次超时
        myTimeoutCnt++;
        //告诉对方
        send((int)m_playerId, 1, 0, 0, 0);

        //界面更新
        ui->myTimeoutCntLabel->setText(QString("我方超时次数：%1").arg(myTimeoutCnt)); //bug 在networkgame.ui中增加myTimeoutCntLabel后没有保存，报错：Ui::NetworkGame没有此成员

        //takeTurn（包括检查胜负）
        takeTurn();
    });
}


void NetworkGame::takeTurn(){
    //累计操作次数+1
    turnCnt2++;

    //在每一回合结束，进入下一回合之前，检查胜负
    checkWinLose();

    //如果胜负未分，方可进入下一回合
    currTurn = turnCnt2 / 2 + 1;
    currentPlayerId = PlayerId(3 - (int)currentPlayerId);
    timer->start(timePerTurn * 1000);
    displayTime = timePerTurn;
    moveSequence.clear();

    qDebug() << "【takeTurn()】，轮到" << (m_playerId == currentPlayerId ? "我方" : "对方");

    //界面相关
    ui->lcdNumber->display(timePerTurn);
    ui->currentPlayerLabel->setText(QString("当前回合玩家：%1 号").arg((int)currentPlayerId));
    ui->currTurnLabel->setText(QString("当前是第 %1 回合").arg(currTurn));
    timer1->start(1000);
}

void NetworkGame::send(char id, char info1, char info2, char info3, char info4){
    qDebug() << QString("发送报文：%1 %2 %3 %4 %5").arg(id).arg(info1).arg(info2).arg(info3).arg(info4);

    char info[6] = {id, info1, info2, info3, info4};
    this->m_tcpSocket->write(info, 5);
}
void NetworkGame::receive(){
    //接收来自对方的信息：玩家id，出发的行列坐标，到达的行列坐标
    QByteArray array = m_tcpSocket->readAll();
    int movePlayerId = array[0];
    qDebug() << QString("收到信息，报文：%1 %2 %3 %4 %5").arg(array[0]).arg(array[1]).arg(array[2]).arg(array[3]).arg(array[4]);


    //特殊类型信息
    //抽签结果 // 表示movePlayerId玩家先手
    if (array[1] == 0 && array[2] == 0){
        qDebug() << "【报文含义】收到了抽签结果，先手玩家为：" << movePlayerId;
        readyToBeginGame(movePlayerId);
    }
    //（仅由客户端发送、服务器端接收的消息）表示客户端已经准备好了
    else if (array[1] == 0 && array[2] == 1){
        //检查信息是否合法
        if (movePlayerId != PlayerId::Green){
            qDebug() << QString("【Wrong报文】 信息传递错误，对方报文的“准备好了”的玩家movePlayerId不是对方。错误信息：%1 %2 %3 %4 %5")
                        .arg(array[0]).arg(array[1]).arg(array[2]).arg(array[3]).arg(array[4]);
        }
        qDebug() << "【报文含义】“客户端已经准备好了”";
        emit clientIsReady();
    }
    //（仅由服务器端发送、客户端接收的消息）表示游戏已经开始
    else if (array[1] == 0 && array[2] == 2){
        //检查信息是否合法
        if (movePlayerId != PlayerId::Red){
            qDebug() << QString("【Wrong报文】 信息传递错误，对方报文的“游戏开始信息”玩家movePlayerId不是对方。错误信息：%1 %2 %3 %4 %5")
                        .arg(array[0]).arg(array[1]).arg(array[2]).arg(array[3]).arg(array[4]);
        }
        qDebug() << "【报文含义】“游戏已经开始”";
        emit gameHasBegun();
    }
    //表示movePlayerId（在信息合法的情况下，只能为对方）玩家本轮超时
    else if (array[1] == 1 && array[2] == 0){
        //检查信息是否合法
        if (movePlayerId != 3 - (int)m_playerId){
            qDebug() << QString("【Wrong报文】信息传递错误，对方传来的信息的超时玩家movePlayerId不是对方。错误信息：%1 %2 %3 %4 %5")
                        .arg(array[0]).arg(array[1]).arg(array[2]).arg(array[3]).arg(array[4]);
        }
        qDebug() << "【报文含义】对方本轮超时";
        emit oppTimeout();
    }

    //表示movePlayerId（在信息合法的情况下，只能为对方）玩家认输
    else if (array[1] == 2 && array[2] == 0){
        //检查信息是否合法
        if (movePlayerId != 3 - (int)m_playerId){
            qDebug() << QString("【Wrong报文】信息传递错误，对方传来的认输信息的玩家movePlayerId不是对方。错误信息：%1 %2 %3 %4 %5")
                        .arg(array[0]).arg(array[1]).arg(array[2]).arg(array[3]).arg(array[4]);
        }
        qDebug() << "【报文含义】【游戏结束】对方认输";
        //在本地呈现结果
        disPlayWinLose(m_playerId, QString("%1 号玩家认输。").arg(movePlayerId));
    }
//    //表示movePlayerId（在信息合法的情况下，只能为对方）玩家因超时3次告负
//    else if (array[1] == 2 && array[2] == 1){
//        //检查信息是否合法
//        if (movePlayerId != 3 - (int)m_playerId){
//            qDebug() << QString("【Wrong报文】信息传递错误，对方传来的超时告负玩家movePlayerId不是对方。错误信息：%1 %2 %3 %4 %5")
//                        .arg(array[0]).arg(array[1]).arg(array[2]).arg(array[3]).arg(array[4]);
//        }
//        qDebug() << "【报文含义】【游戏结束】对方超时3次告负";
//        //在本地呈现胜负结果
//        disPlayWinLose(m_playerId, QString("%1 号玩家超时已达3次。").arg(movePlayerId));
//    }
//    //表示movePlayerId（在信息合法的情况下，只能为对方）玩家因离开大本营棋子数目不足告负
//    else if (array[1] == 2 && array[2] == 2){
//        //检查信息是否合法
//        if (movePlayerId != 3 - (int)m_playerId){
//            qDebug() << QString("【Wrong报文】信息传递错误，对方传来的因离开大本营棋子数目不足告负玩家movePlayerId不是对方。错误信息：%1 %2 %3 %4 %5")
//                        .arg(array[0]).arg(array[1]).arg(array[2]).arg(array[3]).arg(array[4]);
//        }
//        if (array[3] == 1){
//            disPlayWinLose(m_playerId, QString("%1 号玩家在第20回合离开大本营棋子数不足5个。").arg(movePlayerId));
//            qDebug() << "【报文含义】【游戏结束】对方在第20回合离开大本营棋子数不足5个";
//        }
//        else if (array[3] == 2){
//            disPlayWinLose(m_playerId, QString("%1 号玩家在第25回合离开大本营棋子数不足8个。").arg(movePlayerId));
//            qDebug() << "【报文含义】【游戏结束】对方在第25回合离开大本营棋子数不足8个";
//        }
//        else if (array[3] == 3){
//            disPlayWinLose(m_playerId, QString("%1 号玩家在第30回合离开大本营棋子数不足10个。").arg(movePlayerId));
//            qDebug() << "【报文含义】【游戏结束】对方在第30回合离开大本营棋子数不足10个";
//        }
//        else{
//            qDebug() << QString("【Wrong报文】信息传递错误，错误信息：%1 %2 %3 %4 %5")
//                        .arg(array[0]).arg(array[1]).arg(array[2]).arg(array[3]).arg(array[4]);
//        }
//    }
//    //表示movePlayerId（在信息合法的情况下，只能为对方）玩家获胜
//    else if (array[1] == 2 && array[2] == 3){
//        //检查信息是否合法
//        if (movePlayerId != 3 - (int)m_playerId){
//            qDebug() << QString("【Wrong报文】信息传递错误，对方传来的获胜信息玩家movePlayerId不是对方。错误信息：%1 %2 %3 %4 %5")
//                        .arg(array[0]).arg(array[1]).arg(array[2]).arg(array[3]).arg(array[4]);
//        }
//        disPlayWinLose(PlayerId(movePlayerId), QString("%1 号玩家已占领对方营地。").arg(movePlayerId));
//        qDebug() << "【报文含义】【游戏结束】对方占领营地，对方获胜";
//    }


    //移动棋子信息
    else{ //表示movePlayerId玩家进行了移动操作
        int fromRow = array[1];
        int fromCol = array[2];
        int toRow = array[3];
        int toCol = array[4];
//        qDebug() << "Playerid:" << movePlayerId << ", from " << fromRow << " " << fromCol
//                                            <<", to" << toRow << " " << toCol;
        //在本地进行移动操作
        NetworkGame::movePiece(PlayerId(movePlayerId), places[fromRow][fromCol], places[toRow][toCol]);
    }
}

//胜负检查
PlayerId NetworkGame::checkFinished(){
    //检查服务器（1号，红方，大本营在下方）是否获胜
    bool hasWon = true;
    for (int i = 0; i <= 3; ++i)
        for (int j = 4; j <= 4 + i; ++j)
            if(places[i][j]->belongPlayerId != PlayerId::Red)
                hasWon = false;
    if(hasWon)
        return PlayerId::Red;

    //检查客户端（2号，绿方，大本营在上方）是否获胜
    hasWon = true;
    for (int i = 13; i <= 16; ++i)
        for (int j = i - 4; j <= 12; ++j)
            if(places[i][j]->belongPlayerId != PlayerId::Green)
                hasWon = false;
    if(hasWon)
        return PlayerId::Green;

    //无人获胜
    return PlayerId::Blank;

}
void NetworkGame::checkLeaveEnough(){ //检查在指定回合离开大本营的棋子是否足够
    //统计离开大本营的棋子数目
    int outCnt1 = 0;
    int outCnt2 = 0;
    //服务器（1号，红方）大本营在下方
    for (int i = 13; i <= 16; ++i)
        for (int j = i - 4; j <= 12; ++j)
            if(places[i][j]->belongPlayerId != PlayerId::Red)
                ++outCnt1;
    //客户端（绿方，2号玩家）大本营在上方
    for (int i = 0; i <= 3; ++i)
        for (int j = 4; j <= 4 + i; ++j)
            if(places[i][j]->belongPlayerId != PlayerId::Green)
                ++outCnt2;

    //在本地呈现胜负结果
    if (20 <= currTurn && currTurn < 25){
        if (outCnt1 < 5 && outCnt2 >= 5){
            disPlayWinLose(PlayerId::Green, QString("1 号玩家在第20回合离开大本营棋子数为 %1个，不足5个。").arg(outCnt1));
        }
        else if (outCnt1 >= 5 && outCnt2 < 5){
            disPlayWinLose(PlayerId::Red, QString("2 号玩家在第20回合离开大本营棋子数为 %1个，不足5个。").arg(outCnt2));
        }
        else if (outCnt1 < 5 && outCnt2 < 5){
            disPlayWinLose(PlayerId::Blank, QString("1 号和2 号玩家在第20回合离开大本营棋子数分别为 %1，%2 个，均不足5个。").arg(outCnt1).arg(outCnt2));
        }
    }
    else if (25 <= currTurn && currTurn < 30){
        if (outCnt1 < 8 && outCnt2 >= 8){
            disPlayWinLose(PlayerId::Green, QString("1 号玩家在第25回合离开大本营棋子数为 %1个，不足8个。").arg(outCnt1));
        }
        else if (outCnt1 >= 8 && outCnt2 < 8){
            disPlayWinLose(PlayerId::Red, QString("2 号玩家在第25回合离开大本营棋子数为 %1个，不足8个。").arg(outCnt2));
        }
        else if (outCnt1 < 8 && outCnt2 < 8){
            disPlayWinLose(PlayerId::Blank, QString("1 号和2 号玩家在第25回合离开大本营棋子数分别为 %1，%2 个，均不足8个。").arg(outCnt1).arg(outCnt2));
        }
    }
    else if (30 <= currTurn){
        if (outCnt1 < 10 && outCnt2 >= 10){
            disPlayWinLose(PlayerId::Green, QString("1 号玩家在第30回合离开大本营棋子数为 %1个，不足10个。").arg(outCnt1));
        }
        else if (outCnt1 >= 10 && outCnt2 < 10){
            disPlayWinLose(PlayerId::Red, QString("2 号玩家在第30回合离开大本营棋子数为 %1个，不足10个。").arg(outCnt2));
        }
        else if (outCnt1 < 10 && outCnt2 < 10){
            disPlayWinLose(PlayerId::Blank, QString("1 号和2 号玩家在第30回合离开大本营棋子数分别为 %1，%2 个，均不足10个。").arg(outCnt1).arg(outCnt2));
        }
    }

}

void NetworkGame::checkWinLose(){
    //检查是否有玩家已占领对方营地
    switch(checkFinished()){
    case PlayerId::Blank:
        break;
    case PlayerId::Red:
        disPlayWinLose(PlayerId::Red, QString("1 号玩家已占领对方营地。"));
        break;
    case PlayerId::Green:
        disPlayWinLose(PlayerId::Green, QString("2 号玩家已占领对方营地。"));
        break;
    }

    ////如果当前回合只进行到一半，不继续检查以下项
    if(turnCnt2 % 2 != 0)
        return;
    ////以下项仅在每一回合（双方各操作一次为一回合）结束时检查
    //检查是否超时3次
    if (myTimeoutCnt >= 3 && oppTImeoutCnt < 3)
        disPlayWinLose(PlayerId(3 - (int)m_playerId), QString("%1 号玩家超时已达3次。").arg((int)m_playerId));
    else if (myTimeoutCnt < 3 && oppTImeoutCnt >= 3)
        disPlayWinLose(m_playerId, QString("%1 号玩家超时已达3次。").arg(3 - (int)m_playerId));
    else if (myTimeoutCnt >= 3 && oppTImeoutCnt >= 3)
        disPlayWinLose(PlayerId::Blank, QString("双方均在本回合超时达3次。").arg(3 - (int)m_playerId));


    //检查是否因为离开大本营棋子数量不足而告负
    checkLeaveEnough();
}
void NetworkGame::disPlayWinLose(PlayerId winner, QString info){
    qDebug() << QString("【游戏结束】disPlayWinLose() 调用， winner = %1。").arg(winner) << info;

    QString winOrLose;
    if (winner == PlayerId::Blank)
        winOrLose = "平局。";
    else if (winner == m_playerId)
        winOrLose = "我方胜利！";
    else
        winOrLose = "我方失败。";

    //点击“OK”，跳转到开始界面。
    if(QMessageBox::Ok == QMessageBox::information(this, winOrLose, info + winOrLose + "\n请关闭程序。")){
        backToStart(0);
    }

}


void NetworkGame::initBoard(){
    for (int i = 0; i <= 3; ++i)
        for (int j = 4; j <= 4 + i; ++j)
            places[i][j]->setBelongPlayerID(PlayerId::Green);
    for (int i = 13; i <= 16; ++i)
        for (int j = i - 4; j <= 12; ++j)
            places[i][j]->setBelongPlayerID(PlayerId::Red);

}

//棋子移动判断和操作
void NetworkGame::clickPiece(int row, int col){
    if(!isPlaying){
        qDebug() << "点击了棋盘，坐标：row = " << row << ", col = " << col << "，【无效】游戏尚未开始";
        return;
    }
    if( m_playerId != currentPlayerId){
        qDebug() << "点击了棋盘，坐标：row = " << row << ", col = " << col << "，【无效】当前不是我方回合";
        return;
    }
    if(!isAPlace(row, col)){
        //检测是否误传参数
        qDebug() << "点击了棋盘，坐标：row = " << row << ", col = " << col << "，【无效】clickPiece()接收的参数是无效参数";
        return;
    }

    //如果游戏正在进行，并且是我方回合，则点击合法
    //如果点击了自己的棋子
    if(places[row][col]->belongPlayerId == m_playerId){        
        qDebug() << "点击了棋盘，坐标：row = " << row << ", col = " << col << "，【清空序列并置首】自己的棋子";
        //将序列清空，该棋子置于首位
        this->moveSequence.clear();
        this->moveSequence.push_back(places[row][col]);
    }

    //如果点击了空格
    else if(places[row][col]->belongPlayerId == PlayerId::Blank){
        //如果此时序列为空。序列不能以空落子点开头，否则视为无效操作
        if(this->moveSequence.isEmpty()){
            qDebug() << "点击了棋盘，坐标：row = " << row << ", col = " << col << "，空格，【无效】当前序列为空";
        }

        //如果此时序列长度为1
        else if(this->moveSequence.length() == 1){
            //如果可以相邻地移动
            if (canMoveNear(this->moveSequence.last(), places[row][col])){
                qDebug() << "点击了棋盘，坐标：row = " << row << ", col = " << col << "相邻空格，直接移动";
                //将信息发送给另一方，并落子
                send((int)m_playerId, this->moveSequence.first()->row(), this->moveSequence.first()->col(), row, col);
                movePiece(m_playerId, this->moveSequence.last(), places[row][col]);
            }
            //如果可以跳子
            else if (canJump(this->moveSequence.last(), places[row][col])){
                //如果可以继续跳子，加入序列
                if (canJumpAgain(this->moveSequence.last(), places[row][col])){
                    this->moveSequence.push_back(places[row][col]);
                    qDebug() << "点击了棋盘，坐标：row = " << row << ", col = " << col << "跳子，且可继续跳子";
                }
                //如果不能再跳子，直接落子
                else{
                    qDebug() << "点击了棋盘，坐标：row = " << row << ", col = " << col << "跳子，不可继续跳子，直接落子";
                    //将信息发送给另一方，并落子
                    send((int)m_playerId, this->moveSequence.first()->row(), this->moveSequence.first()->col(), row, col);
                    movePiece(m_playerId, this->moveSequence.last(), places[row][col]);
                }
            }
            //如果不能移动，把序列清空
            else{
                qDebug() << "点击了棋盘，坐标：row = " << row << ", col = " << col << "，空格，【清空序列】因为不能移动到此位置";
                this->moveSequence.clear();
            }
        }

        //如果序列长度已经至少是2，必须每一步都跳子
        else{
            //如果连续第二次点击了某一个空格，视为确认落子
            if(this->moveSequence.last() == places[row][col]){
                qDebug() << "点击了棋盘，坐标：row = " << row << ", col = " << col << "，空格，【手动确认落子】";
                //将信息发送给另一方，并落子
                send((int)m_playerId, this->moveSequence.first()->row(), this->moveSequence.first()->col(), row, col);
                movePiece(m_playerId, this->moveSequence.first(), places[row][col]);
            }
            //如果可以跳子
            else if(canJump(this->moveSequence.last(), places[row][col])){
                //如果可以继续跳子，加入序列
                if (canJumpAgain(this->moveSequence.last(), places[row][col])){
                    this->moveSequence.push_back(places[row][col]);
                    qDebug() << "点击了棋盘，坐标：row = " << row << ", col = " << col << "跳子，且可继续跳子";
                }
                //如果不能再跳子，直接落子
                else{
                    qDebug() << "点击了棋盘，坐标：row = " << row << ", col = " << col << "跳子，不可继续跳子，直接落子";
                    //将信息发送给另一方，并落子
                    send((int)m_playerId, this->moveSequence.first()->row(), this->moveSequence.first()->col(), row, col);
                    movePiece(m_playerId, this->moveSequence.first(), places[row][col]);
                }
            }
            //如果不是当前格子，也不能跳子
            else{
                qDebug() << "点击了棋盘，坐标：row = " << row << ", col = " << col << "，空格，【清空序列】因为不能移动到此位置";
                //将序列清空
                this->moveSequence.clear();
            }
        }
    }

    //其他情形（如果点击了其他玩家的棋子）
    else{
        qDebug() << "点击了棋盘，坐标：row = " << row << ", col = " << col << "，【清空序列】其他玩家的棋子";
        //将序列清空
        this->moveSequence.clear();
    }

//    qDebug() << "-------当前movesequence:";
//    for (auto item : moveSequence)
//        qDebug() << QString("------------- row = %1, col = %2").arg(item->row()).arg(item->col());

}
void NetworkGame::movePiece(PlayerId movePlayerId, Place* from, Place* to){
    qDebug() << QString("【movePiece()】  %1 号玩家的棋子从 row = %2, col = %3，移动到 row = %4, col = %5")
                .arg(movePlayerId).arg(from->row()).arg(from->col())
                .arg(to->row()).arg(to->col());

    //维护棋盘信息（包括变色）
    from->setBelongPlayerID(PlayerId::Blank);
    to->setBelongPlayerID(movePlayerId);

    //takeTurn（包括检查胜负）
    takeTurn();
}


//操作合法性判断
bool NetworkGame::canMoveNear(Place* from, Place* to){
    return canMoveNear(from->row(), from->col(), to->row(), to->col());
}
bool NetworkGame::canMoveNear(int fromRow, int fromCol, int toRow, int toCol){
    //检测是否错误调用
    if (!isAPlace(fromRow, fromCol)){
        qDebug() << "错误调用NetworkGame::canMoveNear()，因为坐标:" << fromRow << " " << fromCol <<"不是棋盘上的落子点";
        return false;
    }
    if (!isAPlace(toRow, toCol)){
        qDebug() << "错误调用NetworkGame::canMoveNear()，因为坐标:" << toRow << " " << toCol <<"不是棋盘上的落子点";
        return false;
    }
    if (places[toRow][toCol]->belongPlayerId != PlayerId::Blank) {
        qDebug() << "错误调用NetworkGame::canMoveNear()，因为places:" << toRow << " " << toCol <<"已经有棋子，棋子所属玩家是："
                        << places[toRow][toCol]->belongPlayerId;
        return false;
    }

    //正确调用（两个点均在棋盘上，且目标点是空格）
    //不跳子，即在相邻的两格移动
    if (qAbs(fromRow - toRow) + qAbs(fromCol - toCol) == 1)
        return true;
    if (toRow == fromRow + 1 && toCol == fromCol + 1)
        return true;
    if (toRow == fromRow - 1 && toCol == fromCol - 1)
        return true;

    return false;
}

bool NetworkGame::canJump(Place* from, Place* to){
    return canJump(from->row(), from->col(), to->row(), to->col());
}
bool NetworkGame::canJump(int fromRow, int fromCol, int toRow, int toCol){

    if (!isAPlace(fromRow, fromCol)){
//        qDebug() << "错误调用NetworkGame::canJump()，因为坐标:" << fromRow << " " << fromCol <<"不是棋盘上的落子点";
        return false;
    }
    if (!isAPlace(toRow, toCol)){
//        qDebug() << "错误调用NetworkGame::canJump()，因为坐标:" << toRow << " " << toCol <<"不是棋盘上的落子点";
        return false;
    }
//   //检测是否错误调用
    if (places[toRow][toCol]->belongPlayerId != PlayerId::Blank) {
//        qDebug() << "错误调用NetworkGame::canJump()，因为places:" << toRow << " " << toCol <<"已经有棋子，棋子所属玩家是："
//                        << places[toRow][toCol]->belongPlayerId;
        return false;
    }
    //跳子
    if (fromRow == toRow && qAbs(fromCol - toCol) == 2)
        if (places[fromRow][(fromCol + toCol) / 2]->belongPlayerId != PlayerId::Blank)
            return true;
    if (qAbs(fromRow - toRow) == 2 && fromCol == toCol)
        if (places[(fromRow + toRow) / 2][fromCol]->belongPlayerId != PlayerId::Blank)
            return true;
    if (toRow == fromRow + 2 && toCol == fromCol + 2)
        if (places[fromRow + 1][fromCol + 1]->belongPlayerId != PlayerId::Blank)
            return true;
    if (toRow == fromRow - 2 && toCol == fromCol - 2)
        if (places[fromRow - 1][fromCol - 1]->belongPlayerId != PlayerId::Blank)
            return true;

    return false;
}

bool NetworkGame::canJumpAgain(Place* from, Place* to){
    return canJumpAgain(from->row(), from->col(), to->row(), to->col());
}
bool NetworkGame::canJumpAgain(int fromRow, int fromCol, int toRow, int toCol){
    int dRow[6] = {0, 2, 2, 0, -2, -2};
    int dCol[6] = {2, 2, 0, -2, -2, 0};
    for (int i = 0; i < 6; ++i){
        if (canJump(toRow, toCol, toRow+dRow[i], toCol+dCol[i])
                && !(fromRow == toRow+dRow[i] && fromCol == toCol+dCol[i]))
            return true;
    }
    return false;
}
NetworkGame::~NetworkGame()
{
    qDebug() << "调用了NetworkGame::~NetworkGame()";
//    if (m_tcpSocket != nullptr){
//        disPlayWinLose(PlayerId::Blank, QString("%1 号玩家退出游戏。").arg(m_playerId));
//    }
    delete ui;
    delete m_tcpServer;
    delete m_tcpSocket;
//    for (int i = 0; i <17; ++i){
//        for (int j = 0; j < 17; ++j)
//            delete places[i][j];
//    }
}
