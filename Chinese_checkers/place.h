#ifndef PLACE_H
#define PLACE_H

#include <QPushButton>
#include <QString>

enum PlayerId {Blank = 0, Red = 1, Green = 2};

class Place : QPushButton
{
public:
    Place(QString imagePath, int x, int y, int _width, int _height);
    void setBelongPlayerID(int id);
    void setBelongPlayerID(PlayerId id);
    int row();
    int col();

    ~Place();
private:
    int _row;
    int _col;
    PlayerId belongPlayerId = PlayerId::Blank;

    //大小参数
    int width;
    int height;

friend class NetworkGame;
};

#endif // PLACE_H
