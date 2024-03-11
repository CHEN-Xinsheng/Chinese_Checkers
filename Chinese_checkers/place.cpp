#include "place.h"

#include <QPushButton>
#include <QString>
#include <QPixmap>
#include <QDebug>
#include <QSize>
Place::Place(QString imagePath, int i, int j, int _width, int _height):
    _row(i), _col(j), width(_width), height(_height)
{
    //创建一个宽为width,高为height,且加载了指定图片的button
    QPixmap pixmap;
    bool ret = pixmap.load(imagePath);
    if(!ret){
        qDebug() << "Place::Place()图片加载失败，图片路径" << imagePath;
    }
    this->setFixedSize(width, height);
    this->setIcon(pixmap);
    this->setIconSize(QSize(width,height));

    //connect
}

void Place::setBelongPlayerID(int id){
    setBelongPlayerID(PlayerId(id));
}


void Place::setBelongPlayerID(PlayerId id){
    this->belongPlayerId = id;

    //重新绘制此位置
    QPixmap pixmap;
    bool ret;
    switch(id){
    case PlayerId::Blank:
        ret = pixmap.load(":/image/place.jpg");
        break;
    case PlayerId::Red:
        ret = pixmap.load(":/image/red.jpg");
        break;
    case PlayerId::Green:
        ret = pixmap.load(":/image/green.jpg");
        break;
    }
    if(!ret){
        qDebug() << "Place::setBelongID()图片加载失败";
    }
    this->setIcon(pixmap);
    this->setIconSize(QSize(width,height));
}

int Place::row(){
    return _row;
}
int Place::col(){
    return _col;
}
Place::~Place(){

}//bug 1: 没实现析构函数，报错："undefined reference to vtable for 'Place'"
