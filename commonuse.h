#ifndef COMMONUSE_H
#define COMMONUSE_H
#include <QString>
#include "config.h"

// 棋盘的矩阵表示 10X9
static const int CHESSBOARD_ROW = 10;
static const int CHESSBOARD_COLUMN = 9;

// 棋盘范围,闭区间[3.12],[3.11]
const int ROW_TOP = 3;
const int ROW_BOTTOM = 12;
const int COL_LEFT = 3;
const int COL_RIGHT = 11;

enum Pieces {
    NOPIECE,
    R_KING = 1, R_ADVISOR, R_MINISTER, R_HORSE, R_CHARIOT, R_CANNON, R_SOLDIER,
    B_KING = 9, B_ADVISOR, B_MINISTER, B_HORSE, B_CHARIOT, B_CANNON, B_SOLDIER,
};

enum PieceType {
    KING = 1,   // 帥 / 将
    ADVISOR,    // 仕 / 仕
    MINISTER,   // 相 / 象
    HORSE,      // 馬 / 马
    CHARIOT,    // 俥 / 車
    CANNON,     // 炮 / 砲
    SOLDIER     // 兵 / 卒
};

enum PieceSide {
    RED,
    BLACK
};

//根据起点和终点生成走法
inline int getMove(int src, int dst) {
    return src + (dst << 8);
}
//根据走法获得起点
inline int getSrc(int move) {
    return move & 255;
}
//根据走法获得终点
inline int getDst(int move) {
    return move >> 8;
}
// 根据行数和列数获得棋子索引
inline int getIndex(int row, int col) {
    return ((row + ROW_TOP) << 4) + (col + COL_LEFT);
}
// 根据棋子索引获得行数
inline int getRow(int index) {
    return (index >> 4) - ROW_TOP;
}
// 根据棋子索引获得列数
inline int getCol(int index) {
    return (index & 15) - COL_LEFT;
}

#endif // COMMONUSE_H
