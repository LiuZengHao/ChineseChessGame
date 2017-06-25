#ifndef CHESSCLIENT_H
#define CHESSCLIENT_H

#include <QWidget>
#include "chess.h"
#include "config.h"

struct GameControl {
    bool isAgainstComputer;  // 是否与电脑下棋
    int palyerSide;          // 玩家阵营
    int computerSide;        // 电脑阵营
    unsigned int select;     // 选中棋子的索引, 如果没有选中, 则值为0
    bool paused;             // 是否暂停
};

class ChessClient : public QWidget
{
    Q_OBJECT
public:

    explicit ChessClient(QWidget *parent = 0);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);

signals:
    void statusChanged(const char *status, int timeout = 0);
    void stepsChanged(const QString & item, int side);
    // type 认输，求和;载入;保存; 1：红方悔棋， 2：黑方悔棋
    void clearSteps(const QString & type);

public slots:
    void man2man();
    void man2machine();
    void start();
    void stop();
    void undo();
    void save();
    void load();
    void peace();
    void submit();

private:
    void afterMove();
    void restart();
    void showStep(int move);

    GameControl game;
    Chess chess;
};


#endif // CHESSCLIENT_H
