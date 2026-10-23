#include <nds.h>
#include <stdio.h>

char board[9] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
int cursor = 0;
char currentPlayer = 'X';

// פונקציה לבדיקת ניצחון
int checkWin() {
    int wins[8][3] = {
        {0,1,2}, {3,4,5}, {6,7,8}, // שורות
        {0,3,6}, {1,4,7}, {2,5,8}, // עמודות
        {0,4,8}, {2,4,6}           // אלכסונים
    };
    for(int i = 0; i < 8; i++) {
        if(board[wins[i][0]] != ' ' && 
           board[wins[i][0]] == board[wins[i][1]] && 
           board[wins[i][1]] == board[wins[i][2]]) {
            return 1;
        }
    }
    return 0;
}

// פונקציה לבדיקת תיקו
int checkDraw() {
    for(int i = 0; i < 9; i++) {
        if(board[i] == ' ') return 0;
    }
    return 1;
}

// ציור הלוח למסך
void drawBoard() {
    consoleClear();
    printf("\n   Tic-Tac-Toe - Nintendo DS\n\n");
    printf("      Player Turn: %c\n\n", currentPlayer);
    
    for(int i = 0; i < 9; i++) {
        if(i % 3 == 0) printf("        ");
        
        // סימון המיקום הנוכחי של הסמן
        if(i == cursor) printf("[%c]", board[i]);
        else printf(" %c ", board[i]);
        
        if(i % 3 != 2) printf("|");
        else if(i != 8) printf("\n       ---+---+---\n");
    }
    printf("\n\n\n  D-Pad: Move  |  A: Place\n  START: Restart");
}

int main(void) {
    consoleDemoInit();
    int gameOver = 0;

    drawBoard();

    while(1) {
        scanKeys();
        int keys = keysDown();

        if (keys & KEY_START) {
            // איפוס המשחק
            for(int i = 0; i < 9; i++) board[i] = ' ';
            currentPlayer = 'X';
            cursor = 0;
            gameOver = 0;
            drawBoard();
        }

        if (!gameOver) {
            if (keys & KEY_UP && cursor > 2) cursor -= 3;
            if (keys & KEY_DOWN && cursor < 6) cursor += 3;
            if (keys & KEY_LEFT && cursor % 3 != 0) cursor -= 1;
            if (keys & KEY_RIGHT && cursor % 3 != 2) cursor += 1;

            if (keys & KEY_A && board[cursor] == ' ') {
                board[cursor] = currentPlayer;
                
                if (checkWin()) {
                    drawBoard();
                    printf("\n\n      Player %c WINS!\n", currentPlayer);
                    gameOver = 1;
                } else if (checkDraw()) {
                    drawBoard();
                    printf("\n\n        It's a DRAW!\n");
                    gameOver = 1;
                } else {
                    // החלפת תור
                    currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
                }
            }
            if(keys) drawBoard();
        }

        swiWaitForVBlank();
    }
    return 0;
}
