#ifndef CHESS_H
#define CHESS_H

#include <vector>
#include "commonuse.h"
#include "config.h"

class Chess
{
public:
    Chess();
    void initBoard();
    bool save(const char *filepath); // 保存到filepath
    bool load(const char *filepath); // 从filepath载入
    int getPiece(int index) const; // 返回棋子, 见枚举量Pieces(声明在commonuse.h)
    int getType(int index) const; // 返回棋子类型, 见枚举量PieceType
    int getSide(int index) const; // 返回棋子属于哪一方, 见枚举量PieceSide
    int lastMove() const; // 返回上一步走棋
    int lastCap() const; // 返回上一步吃子
    int currentSide() const; // 当前应该走棋的一方
    bool sameSide(int i, int j) const; // 索引i 和 j 处的棋子是否处于同一方

    bool humanMove(int move); // 如果move是一个合理的走法, 那么走棋并返回true, 否则返回false
    void computerMove(); // 电脑走棋
    void undoMove(); // 撤销一步
    int checked(); // 返回被将军一方将的索引, 如果没有将军就返回0
    bool isMate(); // 是否将死

private:
    void changeSide();
    void addPiece(int index, int piece);
    void delPiece(int index);
    bool makeMove(int move);
    bool validMove(int move);
    int evaluate() const; // 局面评价函数
    int generateMoves(int mvs[]); // 生成所有走法
    int searchMoves(int alpha, int beta, int depth); // 搜索最佳走法

    char board[256];
    int side;
    std::vector<int> moves[2];
    std::vector<char> caps[2];
    int value[2];
};

#endif // CHESS_H
