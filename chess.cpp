#include "chess.h"
#include "chessdata.h"
#include <algorithm>
#include <fstream>

const int MAX_GEN_MOVES = 128; // 最大生成的走法数
const int LIMIT_DEPTH = 6;    // 最大的搜索深度
const int MATE_VALUE = 10000;  // 最高分值，即将死的分值
const int ADVANCED_VALUE = 3;  // 先行权分值

// Alpha-Beta搜索相关全局变量
struct {
    int history[65536];
    int bestmove;
    void init() {
        std::fill(history, history + 65536, 0);
        bestmove = 0;
    }
} search;
// 根据历史表排序的函数
static bool comparator(int i, int j) {
    return search.history[i] > search.history[j];
}

// 行方向上的距离
inline int rdist(int src, int dst) {
    int r1 = (src >> 4);
    int r2 = (dst >> 4);
    return abs(r1 - r2);
}
// 列方向上的距离
inline int cdist(int src, int dst) {
    int c1 = (src & 15);
    int c2 = (dst & 15);
    return abs(c1 - c2);
}
// 行列上的距离之和
inline int dist(int src, int dst) {
    return rdist(src, dst) + cdist(src, dst);
}
// 是否在河的同一边
inline bool sameHalf(int src, int dst) {
    return ((src ^ dst) & 0x80) == 0;
}
// 是否已过河
inline bool awayHalf(int src, int side) {
    return (src & 0x80) == (side << 7);
}
// 是否同一行
inline bool sameRow(int src, int dst) {
    return (src >> 4) == (dst >> 4);
}
// 是否同一列
inline bool sameCol(int src, int dst) {
    return (src & 15) == (dst & 15);
}
// 两点间棋子数
static int countBetween(const char *board, int src, int dst) {
    int start = (src < dst)?src:dst;
    int end = src + dst - start;
    int step = sameRow(src, dst)?1:16;
    int n = 0;
    for (int i = start + step; i < end; i += step) {
        n += (board[i] != NOPIECE);
    }
    return n;
}
// 小兵前进一格
inline int squareForward(int src, int side) {
    return src - 16 + (side << 5);
}

Chess::Chess()
{
    initBoard();
}

void Chess::initBoard()
{
    side = RED;
    for (int i = 0; i < 2; i++) {
        value[i] = 0;
        moves[i].clear();
        caps[i].clear();
    }
    for (int i = 0; i < 256; i++) {
        addPiece(i, STARTUP_LAYOUT[i]);
    }

}

int Chess::getPiece(int index) const
{
    return board[index];
}

int Chess::getType(int index) const
{
    return board[index] & 7;
}

int Chess::getSide(int index) const
{
    return (board[index] >> 3) & 1;
}

int Chess::lastMove() const
{
    auto &mvs = moves[side ^ 1];
    return mvs.size()?mvs.back():0;
}

int Chess::lastCap() const
{
    auto &captures = caps[side ^ 1];
    return captures.size()?captures.back():0;
}

int Chess::currentSide() const
{
    return side;
}

bool Chess::sameSide(int i, int j) const
{
    return getPiece(i) && getPiece(j) && getSide(i) == getSide(j);
}

bool Chess::humanMove(int move)
{
    if (validMove(move)) {
        return makeMove(move);
    }
    else {
        return false;
    }
}

void Chess::computerMove()
{
    search.init();
    searchMoves(-MATE_VALUE, MATE_VALUE, LIMIT_DEPTH);
    makeMove(search.bestmove);
}

void Chess::undoMove()
{
    int move, src, dst, capture;
    changeSide();
    if (moves[side].size() == 0) {
        return;
    }
    move = moves[side].back();
    capture = caps[side].back();
    moves[side].pop_back();
    caps[side].pop_back();
    src = getSrc(move);
    dst = getDst(move);
    addPiece(src, board[dst]);
    delPiece(dst);
    addPiece(dst, capture);
}

// 返回根据步长找到的下一个棋子的索引
static int findNextPiece(const char *board, int src, int step) {
    int dst = src + step;
    while (InBoard[dst]) {
        if (board[dst] != NOPIECE) {
            break;
        }
        dst += step;
    }
    return dst;
}

int Chess::checked()
{
    int i, j;
    for (i = 0; i < 256; i++) {
        if (getType(i) == KING && getSide(i) == side) {
            break;
        }
    }
    // 判断是否被对方的兵将军
    if (getType(squareForward(i, side)) == SOLDIER || getType(i - 1) == SOLDIER || getType(i + 1) == SOLDIER) {
        return i;
    }
    // 判断是否被对方的马将军
    for (j = 0; j < 8; j++) {
        int dst = i + horse_step[j];
        int pin = i + horse_pin2[j];
        if (getType(dst) == HORSE && getSide(dst) != side && getPiece(pin) == NOPIECE) {
            return i;
        }
    }
    // 判断是否被对方的车(或帅)(或炮）将军
    for (j = 0; j < 4; j++) {
        int step = king_step[j];
        int dst;
        dst = findNextPiece(board, i, step);
        if (InBoard[dst] && (getType(dst) == CHARIOT || getType(dst) == KING) && getSide(dst) != side) {
            return i;
        }
        dst = findNextPiece(board, dst, step);
        if (InBoard[dst] && getType(dst) == CANNON && getSide(dst) != side) {
            return i;
        }
    }
    return 0;
}

bool Chess::isMate()
{
    int mvs[MAX_GEN_MOVES], counts;
    counts = generateMoves(mvs);
    for (int i = 0; i < counts; i++) {
        if (makeMove(mvs[i])) {
            undoMove();
            return false;
        }
    }
    return true;
}

inline void Chess::changeSide()
{
    side ^= 1;
}

void Chess::addPiece(int index, int piece)
{
    board[index] = piece;
    if (piece < 8) {
        value[RED] += valueTable[piece & 7][index];
    }
    else {
        value[BLACK] += valueTable[piece & 7][254 - index];
    }
}

void Chess::delPiece(int index)
{
    int piece = board[index];
    if (piece < 8) {
        value[RED] -= valueTable[piece & 7][index];
    }
    else {
        value[BLACK] -= valueTable[piece & 7][254 - index];
    }
    board[index] = NOPIECE;
}

bool Chess::makeMove(int move)
{
    int src, dst, capture;
    src = getSrc(move);
    dst = getDst(move);

    capture = board[dst];
    delPiece(dst);
    addPiece(dst, board[src]);

    delPiece(src);
    if (checked()) { // 撤销自杀的走法并返回false
        addPiece(src, board[dst]);
        delPiece(dst);
        addPiece(dst, capture);
        return false;
    }
    else { // 记录走法并返回true

        caps[side].push_back(capture);
        moves[side].push_back(move);
        changeSide();
        return true;
    }
}

bool valid_minister(const char *board, int src, int dst) {
    if (sameHalf(src, dst) && rdist(src, dst) == 2 && cdist(src, dst) == 2) {
        int pin = ((src + dst) >> 1); // 象眼的索引
        return board[pin] == NOPIECE;
    }
    return false;
}

bool valid_horse(const char *board, int src, int dst) {
    if (rdist(src, dst) == 1 && cdist(src, dst) == 2) {
        int pin = (src & 0xf0) | (((dst & 0x0f) + (src & 0x0f)) >> 1); // 马脚的索引
        return board[pin] == NOPIECE;
    }
    else if (rdist(src, dst) == 2 && cdist(src, dst) == 1) {
        int pin = (((src & 0xf0) + (dst & 0xf0)) >> 1) | (src & 0x0f);
        return board[pin] == NOPIECE;
    }
    else {
        return false;
    }
}

bool valid_cannon(const char *board, int src, int dst) {
    if (sameRow(src, dst) || sameCol(src, dst)) {
        if (board[dst]) {
            // 如果目标处有子, 那么一定是隔了一个棋子打炮
            return countBetween(board, src, dst) == 1;
        }
        else {
            return countBetween(board, src, dst) == 0;
        }
    }
    return false;
}

bool valid_soldier(const char *board, int src, int dst) {
    int side = (board[src] >> 3) & 1;
    if (awayHalf(src, side) && ((dst == src - 1) || (dst == src + 1))) {
        // 已经过河并且左右移动一格
        return true;
    }
    return dst == squareForward(src, side);// 目标在前进一格的地方
}

bool Chess::validMove(int move)
{
    int src, dst;
    src = getSrc(move);
    dst = getDst(move);
    if (!sameSide(src, dst)) {
        switch (getType(src)) {
        case KING: // 在九宫格内并且距离为1
            return InFort[dst] && dist(src, dst) == 1;
        case ADVISOR: // 在九宫格内并且行、列上距离为1
            return InFort[dst] && cdist(src, dst) == 1 && rdist(src, dst) == 1;
        case MINISTER:
            return valid_minister(board, src, dst);
        case HORSE:
            return valid_horse(board, src, dst);
        case CHARIOT:
            return (sameRow(src, dst) || sameCol(src, dst)) && countBetween(board, src, dst) == 0;
        case CANNON:
            return valid_cannon(board, src, dst);
        case SOLDIER:
            return valid_soldier(board, src, dst);
        }
    }
    return false;
}

inline int Chess::evaluate() const
{
    return (side == RED)?(value[RED] - value[BLACK] + ADVANCED_VALUE):(value[BLACK] - value[RED] + ADVANCED_VALUE);
}

int Chess::generateMoves(int mvs[])
{
    int counts, i, j, dst, pin;
    counts = 0;
    for (i = 0; i < 256; i++) {
        if (getSide(i) != side) {
            continue;
        }
        switch (getType(i)) {
        case KING:
            for (j = 0; j < 4; j++) {
                dst = i + king_step[j];
                if (InFort[dst] && !sameSide(i, dst)) {
                    mvs[counts++] = getMove(i, dst);
                }
            }
            break;
        case ADVISOR:
            for (j = 0; j < 4; j++) {
                dst = i + advisor_step[j];
                if (InFort[dst] && !sameSide(i, dst)) {
                    mvs[counts++] = getMove(i, dst);
                }
            }
            break;
        case MINISTER:
            for (j = 0; j < 4; j++) {
                dst = i + minister_step[j];
                pin = i + minister_pin[j];
                if (InBoard[dst] && sameHalf(i, dst) && !sameSide(i, dst) && board[pin] == NOPIECE) {
                    mvs[counts++] = getMove(i, dst);
                }
            }
            break;
        case HORSE:
            for (j = 0; j < 8; j++) {
                dst = i + horse_step[j];
                pin = i + horse_pin[j];
                if (InBoard[dst] && !sameSide(i, dst) && board[pin] == NOPIECE) {
                    mvs[counts++] = getMove(i, dst);
                }
            }
            break;
        case CHARIOT:
            for (j = 0; j < 4; j++) {
                dst = i + king_step[j];
                while (InBoard[dst]) {
                    if (!sameSide(i, dst)) {
                        mvs[counts++] = getMove(i, dst);
                    }
                    if (board[dst] != NOPIECE) {
                        break;
                    }
                    dst += king_step[j];
                }
            }
            break;
        case CANNON:
            for (j = 0; j < 4; j++) {
                dst = i + king_step[j];
                while (InBoard[dst] && board[dst] == NOPIECE) {
                    mvs[counts++] = getMove(i, dst);
                    dst += king_step[j];
                }
                dst = findNextPiece(board, dst, king_step[j]);
                if (InBoard[dst] && getSide(dst) != side) {
                    mvs[counts++] = getMove(i, dst);
                }
            }
            break;
        case SOLDIER:
            dst = squareForward(i, side);
            if (InBoard[dst] && !sameSide(i, dst)) {
                mvs[counts++] = getMove(i, dst);
            }
            if (awayHalf(i, side)) {
                for (dst = i - 1; dst <= i + 1; dst += 2)
                if (InBoard[dst] && !sameSide(i, dst)) {
                    mvs[counts++] = getMove(i, dst);
                }
            }
            break;
        }
    }
    return counts;
}

int Chess::searchMoves(int alpha, int beta, int depth)
{
    if (depth == 0) {
        return evaluate();
    }
    int mvs[MAX_GEN_MOVES], counts, value, move;
    // 初始化最佳值和最佳走法
    value = -MATE_VALUE;
    move = 0;
    // 生成并排序走法
    counts = generateMoves(mvs);
    std::sort(mvs, mvs + counts, comparator);
    // 对每一种走法递归调用searchMoves
    for (int i = 0; i < counts && value < beta; i++) {
        if (makeMove(mvs[i])) {
            int v = -searchMoves(-beta, -alpha, depth - 1);
            undoMove();
            if (v > value) {
                value = v;
                move = mvs[i];
                if (value > alpha) { // 缩小搜索边界
                    alpha = value;
                }
            }
        }
    }
    if (value == -MATE_VALUE) { // 搜索出杀棋则返回杀棋的评价
        return -depth - MATE_VALUE + 10;
    }
    search.history[move] += depth*depth; // 将最佳走法记录到历史表
    if (depth == LIMIT_DEPTH) {
        search.bestmove = move;
    }
    return value;
}

bool Chess::save(const char *filepath)
{
    std::ofstream file(filepath, std::ios::binary);
    if (file) {
        // 保存棋盘
        file.write(board, sizeof(char)*256);
        // 保存玩家阵营
        file.write((char*)&side, sizeof(int));
        // 保存双方子力价值
        file.write((char*)value, sizeof(int)*2);
        // 保存走法和吃子的历史
        for (int i = 0; i < 2; i++) {
            auto moves_size = moves[i].size();
            auto caps_size = caps[i].size();
            file.write((char*)&moves_size, sizeof(moves_size));
            file.write((char*)&caps_size, sizeof(caps_size));
            if (moves_size) {
                file.write((char*)moves[i].data(), sizeof(int)*moves_size);
            }
            if (caps_size) {
                file.write(caps[i].data(), sizeof(char)*caps_size);
            }
        }
        return true;
    }
    return false;
}

bool Chess::load(const char *filepath)
{
    std::ifstream file(filepath, std::ios::binary);
    if (file) {
        // 读入棋盘
        file.read(board, sizeof(char)*256);
        // 读入玩家阵营
        file.read((char*)&side, sizeof(int));
        // 读入双方子力价值
        file.read((char*)value, sizeof(int)*2);
        // 读入走法和吃子的历史
        for (int i = 0; i < 2; i++) {
            decltype(moves[i].size()) moves_size;
            decltype(caps[i].size()) caps_size;
            int move;
            char cap;
            moves[i].clear();
            caps[i].clear();
            file.read((char*)&moves_size, sizeof(moves_size));
            file.read((char*)&caps_size, sizeof(caps_size));
            for (decltype(moves_size) j = 0; j < moves_size; j++) {
                file.read((char*)&move, sizeof(int));
                moves[i].push_back(move);
            }
            for (decltype(moves_size) j = 0; j < caps_size; j++) {
                file.read((char*)&cap, sizeof(char));
                caps[i].push_back(cap);
            }
        }
        return true;
    }
    return false;
}

