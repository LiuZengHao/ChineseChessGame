#include "chessclient.h"
#include "commonuse.h"

#include <QByteArray>
#include <QTextStream>

#include <QString>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QMessageBox>
#include <QSound>

static const int CHESSBOARD_WIDTH = 521;
static const int CHESSBOARD_HEIGHT = 577;
static const int CHESSBOARD_STARTX = 250;
static const int CHESSBOARD_STARTY = 100;
static const int SQUARE_SIZE = 57;
static const int BOARD_EDGE = 4;

static const char *SAVE_FILE_NAME = "chess.dat";


static const QString filePath(":/images/chesspieces/");
static const QString picName[] = {
    "OO.GIF", "RK.GIF", "RA.GIF", "RB.GIF", "RN.GIF", "RR.GIF", "RC.GIF", "RP.GIF",
    "OO.GIF", "BK.GIF", "BA.GIF", "BB.GIF", "BN.GIF", "BR.GIF", "BC.GIF", "BP.GIF"
};
static const QString backgroundPic(":/images/BackGround.GIF");
static const QString selectPic(":/images/chesspieces/OOS.GIF");
static const QString checkedPicName[] = { "RKM.GIF", "BKM.GIF" };

static const QString chooseSound(":/audios/Choose.wav");
static const QString drawSound(":/audios/Draw.wav");
static const QString checkSound(":/audios/Check.wav");
static const QString eatSound(":/audios/Eat.wav");
static const QString forbidSound(":/audios/Forbid.wav");
static const QString moveSound(":/audios/Move.wav");
static const QString winSound(":/audios/Win.wav");
static const QString newSound(":/audios/NewGame.wav");

// 根据绘图点获得棋子索引
inline int getIndex(const QPoint &point) {
    int row = (point.y() - BOARD_EDGE)/SQUARE_SIZE;
    int col = (point.x() - BOARD_EDGE)/SQUARE_SIZE;
    return getIndex(row, col);
}
// 根据行数和列数获得绘图点
inline QPoint getPoint(int row, int col) {
    return QPoint(BOARD_EDGE + col*SQUARE_SIZE, BOARD_EDGE + row*SQUARE_SIZE);
}
// 根据棋子索引获得绘图点
inline QPoint getPoint(int index) {
    int row = getRow(index);
    int col = getCol(index);
    return getPoint(row, col);
}

ChessClient::ChessClient(QWidget *parent) : QWidget(parent)
{
    resize(CHESSBOARD_WIDTH, CHESSBOARD_HEIGHT);
    move(CHESSBOARD_STARTX, CHESSBOARD_STARTY);

    // 默认设置
    game.isAgainstComputer = true;
    game.computerSide = BLACK;
    game.palyerSide = RED;
    game.select = 0;
    game.paused = true;
}

void ChessClient::paintEvent(QPaintEvent *event)
{
    // 画棋盘
    QPainter painter(this);
    painter.drawImage(0, 0, QImage(backgroundPic));
    // 遍历 10x9的棋盘
    for (int row = 0; row < CHESSBOARD_ROW; ++row) {
        for (int col = 0; col < CHESSBOARD_COLUMN; ++col) {
            // 获得图片的路径
            QString picture = filePath + picName[chess.getPiece(getIndex(row, col))];
            // 获得绘图点
            QPoint point = getPoint(row, col);
            // 绘图
            painter.drawImage(point, QImage(picture));
        }
    }
    // 画选中效果
    if (game.select) {
        QPoint point = getPoint(game.select);
        painter.drawImage(point, QImage(selectPic));
    }
    else {
        // 没有选中的话, 就要画出上一步走棋的起点和终点
        int lastmove = chess.lastMove();
        int src = getSrc(lastmove);
        int dst = getDst(lastmove);
        painter.drawImage(getPoint(src), QImage(selectPic));
        painter.drawImage(getPoint(dst), QImage(selectPic));
    }
    // 画将军效果
    int checkedAt = chess.checked();
    if (checkedAt) {
        int side = chess.getSide(checkedAt);
        QString checkPic = filePath + checkedPicName[side];
        QPoint point = getPoint(checkedAt);
        painter.drawImage(point, QImage(checkPic));
    }
}

void ChessClient::mousePressEvent(QMouseEvent *event)
{
    // 游戏处于暂停或者非左键点击, 不响应
    if (game.paused || event->button() != Qt::LeftButton) {
        return;
    }
    // 人机模式下电脑还未走完一步棋, 不响应
    if (game.isAgainstComputer && chess.currentSide() != game.palyerSide) {
        return;
    }
    // 处理玩家点击事件
    int dst = getIndex(event->pos()); // 根据点击的位置获得点击的棋盘的索引
    if (chess.getPiece(dst) != NOPIECE && chess.getSide(dst) == chess.currentSide()) {
        // 如果点击自己的子, 那么选中该子
        game.select = dst;
        // 重绘并播放选子声音
        repaint();
        QSound::play(chooseSound);
    }
    else if (game.select) {
        // 如果点击的不是自己的子并且有子选中, 那么尝试走子
        int move = getMove(game.select, dst);
        if (chess.humanMove(move)) {
            game.select = 0;
            // 显示文字棋谱
            showStep(move);
            // 走完一步棋, 做其他事
            afterMove();
        }
        else {
            // 走法不合理, 播放禁止的声音
            QSound::play(forbidSound);
        }
    }
}

void ChessClient::afterMove()
{
    repaint();// 更新画面
    QSound::play(chess.checked()?checkSound:chess.lastCap()?eatSound:moveSound); // 播放一般走子、吃子或将军声音
    // 判断游戏是否结束
    if (chess.isMate()) {
        QSound::play(winSound);
        QMessageBox::about(this, "结果", chess.currentSide() == RED?"黑方获胜":"红方获胜");
        restart();
    }
    //如果是人机模式那么电脑走一步棋
    if (game.isAgainstComputer && chess.currentSide() == game.computerSide) {

        chess.computerMove();
        //电脑始终为黑方，所以move后，chess.currentSide() == RED
        showStep(chess.lastMove());

        afterMove();
    }
}

//显示下棋的步骤
void ChessClient::showStep(int move){

   // QTextStream out(stdout, QIODevice::WriteOnly);
    QString tmp;
    QString chineseNum = tr("零一二三四五六七八九");
    QString pureNum = tr("0123456789");
    QString chessRed = tr("空帥仕相馬俥炮兵");
    QString chessBlack = tr("空将仕象马車砲卒");
    int type = chess.getType(getDst(move));
    int s1 = getRow(getSrc(move));
    int s2 = getCol(getSrc(move));
    int e1 = getRow(getDst(move));
    int e2 = getCol(getDst(move));
    int side = chess.currentSide() ;


    if(side == BLACK){
        QString piece =  chessRed.mid(type,1);
        if(s1 == e1){
            tmp = piece + chineseNum.mid(9-s2, 1) + tr("平") + chineseNum.mid(9-e2, 1);
        }
        else if(s1 > e1){
            if(type == HORSE || type == MINISTER || type == ADVISOR){
                tmp =  piece + chineseNum.mid(9-s2, 1) + tr("进") + chineseNum.mid(9-e2, 1);
            }
            else{
                tmp = piece + chineseNum.mid(9-s2, 1) + tr("进") + chineseNum.mid(s1-e1, 1);
            }

        }
        else{

            if(type == HORSE || type == MINISTER || type == ADVISOR){
                tmp =  piece + chineseNum.mid(9-s2, 1) + tr("退") + chineseNum.mid(9-e2, 1);
            }
            else{
                tmp = piece + chineseNum.mid(9-s2, 1) + tr("退") + chineseNum.mid(e1-s1, 1);
            }
        }
    }
    else{
        QString piece =  chessBlack.mid(type,1);
        if(s1 == e1){
            tmp = piece + pureNum.mid(1+s2, 1) + tr("平") + pureNum.mid(1+e2, 1);
        }
        else if(s1 > e1){
            if(type == HORSE || type == MINISTER || type == ADVISOR){
                tmp =  piece + pureNum.mid(1+s2, 1) + tr("退") + pureNum.mid(1+e2, 1);
            }
            else{
                tmp = piece + pureNum.mid(1+s2, 1) + tr("退") + pureNum.mid(s1-e1, 1);
            }

        }
        else{

            if(type == HORSE || type == MINISTER || type == ADVISOR){
                tmp =  piece + pureNum.mid(1+s2, 1) + tr("进") + pureNum.mid(1+e2, 1);
            }
            else{
                tmp = piece + pureNum.mid(1+s2, 1) + tr("进") + pureNum.mid(e1-s1, 1);
            }
        }

    }
    emit stepsChanged(tmp, side);
}
inline void ChessClient::restart()
{
    chess.initBoard();
    emit clearSteps("求和或认输");
    repaint();
    QSound::play(newSound);
}

void ChessClient::man2machine()
{
    game.isAgainstComputer = true;
}

void ChessClient::man2man()
{
    game.isAgainstComputer = false;
}

void ChessClient::start()
{
    game.paused = false;
    if (game.isAgainstComputer && chess.currentSide() == game.computerSide) {
        chess.computerMove();
        afterMove();
    }
    emit statusChanged("游戏开始", 2000);
}

void ChessClient::stop()
{
    game.paused = true;
    emit statusChanged("游戏暂停");
}

void ChessClient::undo()
{

    if (game.isAgainstComputer) {
        emit clearSteps(QString::number(chess.currentSide()));
        chess.undoMove();
        emit clearSteps(QString::number(chess.currentSide()));
        chess.undoMove();
    }
    else {
        emit clearSteps(QString::number(chess.currentSide()));
        chess.undoMove();
    }

    repaint();
}

void ChessClient::save()
{

    emit clearSteps(tr("保存"));
    if (chess.save(SAVE_FILE_NAME)) {
        emit statusChanged("保存成功", 2000);
    }
    else{
        emit statusChanged("保存失败", 2000);
    }
}

void ChessClient::load()
{
    if (chess.load(SAVE_FILE_NAME)) {
        emit clearSteps("载入");
        repaint();
        emit statusChanged("载入成功", 2000);

    }
    else {
        emit statusChanged("载入失败", 2000);
    }
}

void ChessClient::peace(){
    if (game.isAgainstComputer) {
        emit statusChanged("与电脑下棋不能求和");
        return;
    }
    QMessageBox::StandardButton rb = QMessageBox::information(NULL, "求和", "你同意对方的求和请求吗?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if(rb == QMessageBox::Yes){
        QSound::play(drawSound);
        QMessageBox::about(NULL,tr("结果"),  tr("平局"));
        restart();
    }
}

void ChessClient::submit(){
    if (game.isAgainstComputer) {
        QMessageBox::about(NULL,tr("结果"),  tr("你输了！再接再厉"));
        restart();
        return;
    }
    QMessageBox::StandardButton rb = QMessageBox::information(NULL, "认输", "确定认输吗?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if(rb == QMessageBox::Yes){
        QMessageBox::about(NULL,tr("结果"),  tr("你输了！再接再厉"));
        restart();
    }
}
