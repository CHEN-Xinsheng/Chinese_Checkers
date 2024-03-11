#ifndef NETWORKGAME_H
#define NETWORKGAME_H

#include "place.h"
#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QPaintEvent>
#include <QString>
#include <QLabel>
#include <QVector>
#include <Qtimer>
#include <QMessageBox>

namespace Ui {
class NetworkGame;
}

class NetworkGame : public QMainWindow
{
    Q_OBJECT

public:
    explicit NetworkGame(QWidget *_parent, bool _isServer, int _windowWidth, int _windowHeight, QString _addr);
    ~NetworkGame();
    void newConnectionSlot();
    void send(char id, char info1, char info2, char info3, char info4);
    void receive();
    void backToStart(int mesc = 0);//返回开始界面
    //初始布局相关
    void initBoard();//显示初始布局
    void paintEvent(QPaintEvent* e);
    bool isAPlace(int row, int col);
    //抽签
//    void draw();
    void readyToBeginGame(int _firstMovePlayerId);
    QMessageBox::StandardButton displayDrawResult(int _firstMovePlayerId); //显示抽签结果，等待“准备”以开始游戏
    void playGame();
    //棋局计算相关
    void clickPiece(int row, int col);
    void movePiece(PlayerId movePlayerId, Place* from, Place* to);
    bool canMoveNear(Place* from, Place* to);
    bool canMoveNear(int fromRow, int fromCol, int toRow, int toCol);
    bool canJump(Place* from, Place* to);
    bool canJump(int fromRow, int fromCol, int toRow, int toCol);
    bool canJumpAgain(Place* from, Place* to);
    bool canJumpAgain(int fromRow, int fromCol, int toRow, int toCol);
    void takeTurn();
    void checkWinLose(); //胜负检查
    PlayerId checkFinished(); //检查是否有玩家已占领对方营地
    void checkLeaveEnough(); //检查在指定回合离开大本营的棋子是否足够
    void disPlayWinLose(PlayerId winner, QString info);

signals:
    //信号，返回值是void，只需要声明，不需要实现
    void clientIsReady(); //服务器端使用的信号。当服务器收到客户端已经准备好的信息时，emit此信号
    void gameHasBegun(); //客户端使用的信号。当客户端收到服务器端已经开始游戏的信息时，emit此信号
    void oppTimeout(); //当我方收到对手传来其超时的信息时，emit此信号
    void oppAdmitDefeat(); //当我方收到对手传来其认输的信息时，emit此信号
//    void oppTimeoutLose(); //当我方收到对手传来其因为超时3次而失败的信息时，emit此信号

private:
    Ui::NetworkGame *ui;
    QWidget* parent;

    //网络相关
    QTcpServer* m_tcpServer = nullptr;
    QTcpSocket* m_tcpSocket = nullptr;
//    Qt::GlobalColor playerColor;
    //棋盘相关
    Place* places[17][17];
    //游戏操作相关
    bool isPlaying = false;//游戏是否正在进行
    bool isServer; //自己是服务器（true）还是客户端（false）
    PlayerId m_playerId;
    QVector<Place*> moveSequence;//记录跳棋移动位置序列，每回合开始时清空
    //游戏规则相关
    PlayerId firstMovePlayerId = PlayerId::Blank;
    PlayerId currentPlayerId = PlayerId::Blank;//当前回合玩家
    QTimer* timer = nullptr;
    QTimer* timer1 = nullptr;
    const int timePerTurn = 20;
    int displayTime = timePerTurn;
    int turnCnt2 = 0; //回合数（双方操作次数之和，超时也视为操作）
    int currTurn = 0;
    int myTimeoutCnt = 0;
    int oppTImeoutCnt = 0;

    //界面相关
//    const QString gameRule = QString("游戏规则：\n"
//                                     "   ·棋子可以移动到相邻的空位；\n"
//                                     "   ·如果某个相邻位置有棋子，且该方向上下一个位置为空，可以跳跃至该位置上。\n"
//                                     "操作方法：\n"
//                                     "   ·选中自己的一颗棋子，并依次点击移动路径中的位置.\n"
//                                     "      -如果无法再移动，即自动落子；\n"
//                                     "      -在仍可移动的情况下，也可通过再次点击同一位置来确认落子。\n"
//                                     "自动判负的情形：\n"
//                                     "   ·超时3次；\n"
//                                     "   ·在第20、25、30回合结束时，离开大本营棋子数目不足5、8、10颗；\n"
//                                     "   ·如果双方在同一回合触发上述条件， 自动判平局。");
};

#endif // NETWORKGAME_H
