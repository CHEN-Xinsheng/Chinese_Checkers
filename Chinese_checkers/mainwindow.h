#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "networkgame.h"
#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:

    Ui::MainWindow *ui;
    NetworkGame* m_pNetworkGame = nullptr;

};


#endif // MAINWINDOW_H
