#include "game.h"
#include <stdlib.h>

static const int wins[8][3] = {
    {0,1,2}, {3,4,5}, {6,7,8}, // שורות
    {0,3,6}, {1,4,7}, {2,5,8}, // עמודות
    {0,4,8}, {2,4,6}           // אלכסונים
};

int checkWinArr(int b[9]) {
    for(int i = 0; i < 8; i++) {
        if(b[wins[i][0]] != 0 && 
           b[wins[i][0]] == b[wins[i][1]] && 
           b[wins[i][1]] == b[wins[i][2]]) {
            return b[wins[i][0]];
        }
    }
    return 0;
}

void updateWinLine(void) {
    winLine[0] = -1; winLine[1] = -1; winLine[2] = -1;
    for(int i = 0; i < 8; i++) {
        if(board[wins[i][0]] != 0 && 
           board[wins[i][0]] == board[wins[i][1]] && 
           board[wins[i][1]] == board[wins[i][2]]) {
            winLine[0] = wins[i][0];
            winLine[1] = wins[i][1];
            winLine[2] = wins[i][2];
            return;
        }
    }
}

int isFull(int b[9]) {
    for(int i = 0; i < 9; i++) if(b[i] == 0) return 0;
    return 1;
}

static int minimax(int b[9], int depth, int isMax) {
    int score = checkWinArr(b);
    if (score == 2) return 10 - depth; 
    if (score == 1) return -10 + depth;
    if (isFull(b)) return 0; 

    if (isMax) {
        int best = -1000;
        for(int i = 0; i < 9; i++) {
            if(b[i] == 0) {
                b[i] = 2; 
                int val = minimax(b, depth + 1, 0);
                b[i] = 0;
                if(val > best) best = val;
            }
        }
        return best;
    } else {
        int best = 1000;
        for(int i = 0; i < 9; i++) {
            if(b[i] == 0) {
                b[i] = 1;
                int val = minimax(b, depth + 1, 1);
                b[i] = 0;
                if(val < best) best = val;
            }
        }
        return best;
    }
}

void doComputerMove(void) {
    int freeCells[9], numFree = 0;
    for(int i = 0; i < 9; i++) if(board[i] == 0) freeCells[numFree++] = i;
    if(numFree == 0) return;

    int doRandom = 0;
    if (difficulty == EASY) doRandom = (rand() % 100 < 80); 
    else if (difficulty == MEDIUM) doRandom = (rand() % 100 < 35); 
    else doRandom = 0; 

    int bestMove = -1;
    if (doRandom) {
        bestMove = freeCells[rand() % numFree];
    } else {
        int bestVal = -1000;
        for(int i = 0; i < 9; i++) {
            if(board[i] == 0) {
                board[i] = 2;
                int moveVal = minimax(board, 0, 0);
                board[i] = 0;
                if(moveVal > bestVal) {
                    bestMove = i;
                    bestVal = moveVal;
                } else if (moveVal == bestVal && (rand() % 2 == 0)) {
                    bestMove = i; 
                }
            }
        }
    }
    if(bestMove != -1) {
        board[bestMove] = 2;
        pieceAngle[bestMove] = 0;
        // יצירת פיצוץ חלקיקים אדום במיקום שבו המחשב שיחק במסך התחתון
        int col = bestMove % 3;
        int row = bestMove / 3;
        spawnParticles(68 + col * 40 + 20, 52 + row * 40 + 20, ARGB16(1, 31, 6, 6));
    }
}
