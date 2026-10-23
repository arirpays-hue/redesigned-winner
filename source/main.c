#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// מצבי משחק
enum GameState { MENU, PLAYER_TURN, COMPUTER_TURN, GAME_OVER };
enum Difficulty { EASY = 0, MEDIUM = 1, HARD = 2 };

enum GameState state = MENU;
enum Difficulty difficulty = MEDIUM;

int board[9] = {0}; // 0=ריק, 1=X (שחקן), 2=O (מחשב)
int delayTimer = 0;

PrintConsole* topConsole;
u16* videoMemory;

// מיני-גופן מורחב הכולל את האותיות הנדרשות לתפריטים ולמשחק
const u8 customFont[128][8] = {
    ['E'] = {0x7E,0x40,0x40,0x7C,0x40,0x40,0x7E,0x00},
    ['A'] = {0x3C,0x42,0x42,0x7E,0x42,0x42,0x42,0x00},
    ['S'] = {0x3C,0x40,0x40,0x3C,0x02,0x02,0x3C,0x00},
    ['Y'] = {0x42,0x42,0x42,0x3C,0x18,0x18,0x18,0x00},
    ['M'] = {0x42,0x66,0x5A,0x42,0x42,0x42,0x42,0x00},
    ['D'] = {0x7C,0x42,0x42,0x42,0x42,0x42,0x7C,0x00},
    ['H'] = {0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x00},
    ['R'] = {0x7C,0x42,0x42,0x7C,0x48,0x44,0x42,0x00},
    ['X'] = {0x42,0x24,0x18,0x18,0x24,0x42,0x42,0x00},
    ['O'] = {0x3C,0x42,0x42,0x42,0x42,0x42,0x3C,0x00}
};

// --- מנוע גרפי בסיסי ---
void drawCharScale(u16* vram, int x, int y, char c, u16 color, int scale) {
    if (c < 0 || c > 127) return;
    const u8* glyph = customFont[(int)c];
    for (int ty = 0; ty < 8; ty++) {
        u8 row = glyph[ty];
        for (int tx = 0; tx < 8; tx++) {
            if (row & (1 << (7 - tx))) {
                for(int sy = 0; sy < scale; sy++) {
                    for(int sx = 0; sx < scale; sx++) {
                        int px = x + tx * scale + sx;
                        int py = y + ty * scale + sy;
                        if(px >= 0 && px < 256 && py >= 0 && py < 192) {
                            vram[py * 256 + px] = color;
                        }
                    }
                }
            }
        }
    }
}

void drawString(u16* vram, int x, int y, const char* str, u16 color, int scale) {
    while (*str) {
        drawCharScale(vram, x, y, *str, color, scale);
        x += 8 * scale;
        str++;
    }
}

void drawRoundRect(u16* vram, int x, int y, int w, int h, int r, u16 color) {
    for (int iy = 0; iy < h; iy++) {
        for (int ix = 0; ix < w; ix++) {
            int draw = 1;
            if (ix < r && iy < r) {
                if ((ix - r)*(ix - r) + (iy - r)*(iy - r) > r*r) draw = 0;
            } else if (ix >= w - r && iy < r) {
                if ((ix - (w - r - 1))*(ix - (w - r - 1)) + (iy - r)*(iy - r) > r*r) draw = 0;
            } else if (ix < r && iy >= h - r) {
                if ((ix - r)*(ix - r) + (iy - (h - r - 1))*(iy - (h - r - 1)) > r*r) draw = 0;
            } else if (ix >= w - r && iy >= h - r) {
                if ((ix - (w - r - 1))*(ix - (w - r - 1)) + (iy - (h - r - 1))*(iy - (h - r - 1)) > r*r) draw = 0;
            }
            if (draw) vram[(y + iy) * 256 + (x + ix)] = color;
        }
    }
}

void fillScreen(u16* vram, u16 color) {
    for (int i = 0; i < 256 * 192; i++) vram[i] = color;
}

// --- לוגיקת AI למשחק ---
int checkWinArr(int b[9]) {
    int wins[8][3] = {{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};
    for(int i=0; i<8; i++) {
        if(b[wins[i][0]] != 0 && b[wins[i][0]] == b[wins[i][1]] && b[wins[i][1]] == b[wins[i][2]])
            return b[wins[i][0]];
    }
    return 0;
}

int isFull(int b[9]) {
    for(int i=0; i<9; i++) if(b[i] == 0) return 0;
    return 1;
}

// אלגוריתם Minimax לחישוב המהלך המושלם
int minimax(int b[9], int depth, int isMax) {
    int score = checkWinArr(b);
    if (score == 2) return 10 - depth; // ניצחון למחשב
    if (score == 1) return -10 + depth; // הפסד למחשב
    if (isFull(b)) return 0; // תיקו

    if (isMax) {
        int best = -1000;
        for(int i=0; i<9; i++) {
            if(b[i]==0) {
                b[i] = 2; 
                int val = minimax(b, depth+1, 0);
                b[i] = 0;
                if(val > best) best = val;
            }
        }
        return best;
    } else {
        int best = 1000;
        for(int i=0; i<9; i++) {
            if(b[i]==0) {
                b[i] = 1;
                int val = minimax(b, depth+1, 1);
                b[i] = 0;
                if(val < best) best = val;
            }
        }
        return best;
    }
}

void doComputerMove() {
    int freeCells[9];
    int numFree = 0;
    for(int i=0; i<9; i++) {
        if(board[i] == 0) freeCells[numFree++] = i;
    }
    if(numFree == 0) return;

    // חישוב אקראיות בהתאם לרמת הקושי
    int doRandom = 0;
    if (difficulty == EASY) doRandom = (rand() % 100 < 80); // 80% טעויות
    else if (difficulty == MEDIUM) doRandom = (rand() % 100 < 35); // 35% טעויות
    else doRandom = 0; // 0% טעויות (Hard)

    int bestMove = -1;

    if (doRandom) {
        bestMove = freeCells[rand() % numFree];
    } else {
        int bestVal = -1000;
        for(int i=0; i<9; i++) {
            if(board[i] == 0) {
                board[i] = 2;
                int moveVal = minimax(board, 0, 0);
                board[i] = 0;
                if(moveVal > bestVal) {
                    bestMove = i;
                    bestVal = moveVal;
                } else if (moveVal == bestVal && (rand() % 2 == 0)) {
                    bestMove = i; // גיוון כשיש מהלכים שווים בערכם
                }
            }
        }
    }
    if(bestMove != -1) board[bestMove] = 2;
}

// --- ממשק וגרפיקה ---
void updateTopScreen() {
    consoleClear();
    printf("\n\n   TIC-TAC-TOE : AI EDITION\n");
    printf("   ========================\n\n\n");
    
    if (state == MENU) {
        printf("     Select Difficulty\n");
        printf("     on the touch screen!\n");
    } else if (state == GAME_OVER) {
        int w = checkWinArr(board);
        if(w == 1) printf("   \x1b[32mYOU WIN! AMAZING!\x1b[0m\n");
        else if(w == 2) printf("   \x1b[31mCOMPUTER WINS!\x1b[0m\n");
        else printf("   \x1b[33mIT'S A TIE!\x1b[0m\n");
        printf("\n\n   Tap bottom screen to restart.");
    } else {
        printf("   Difficulty: ");
        if(difficulty==EASY) printf("EASY\n");
        if(difficulty==MEDIUM) printf("MEDIUM\n");
        if(difficulty==HARD) printf("HARD\n");
        printf("\n\n");
        
        if (state == PLAYER_TURN) printf("   \x1b[32mYOUR TURN (X)\x1b[0m\n");
        else printf("   \x1b[31mCOMPUTER IS THINKING...\x1b[0m\n");
    }
}

void drawBottomScreen() {
    u16 bgCol = ARGB16(1, 2, 2, 3);
    fillScreen(videoMemory, bgCol);

    if (state == MENU) {
        u16 btnCol = ARGB16(1, 6, 8, 12);
        u16 textCol = ARGB16(1, 31, 31, 31);
        
        drawRoundRect(videoMemory, 50, 30, 156, 40, 10, btnCol);
        drawString(videoMemory, 95, 42, "EASY", textCol, 2);
        
        drawRoundRect(videoMemory, 50, 80, 156, 40, 10, btnCol);
        drawString(videoMemory, 95, 92, "MED", textCol, 2);
        
        drawRoundRect(videoMemory, 50, 130, 156, 40, 10, btnCol);
        drawString(videoMemory, 95, 142, "HARD", textCol, 2);
    } 
    else {
        // ציור לוח המשחק (רשת)
        u16 gridCol = ARGB16(1, 20, 20, 20);
        int ox = 53, oy = 21, cellSize = 50;
        
        for(int i=0; i<150; i++) {
            for(int w=0; w<3; w++) {
                videoMemory[(oy+i)*256 + (ox+cellSize+w)] = gridCol;
                videoMemory[(oy+i)*256 + (ox+cellSize*2+w)] = gridCol;
                videoMemory[(oy+cellSize+w)*256 + (ox+i)] = gridCol;
                videoMemory[(oy+cellSize*2+w)*256 + (ox+i)] = gridCol;
            }
        }
        
        // ציור האיקסים והעיגולים
        u16 colorX = ARGB16(1, 0, 31, 10);
        u16 colorO = ARGB16(1, 31, 5, 5);
        for(int i=0; i<9; i++) {
            if(board[i] != 0) {
                int row = i / 3;
                int col = i % 3;
                int px = ox + col*cellSize + 13;
                int py = oy + row*cellSize + 13;
                if(board[i] == 1) drawString(videoMemory, px, py, "X", colorX, 3);
                else drawString(videoMemory, px, py, "O", colorO, 3);
            }
        }
    }
}

void initGame() {
    for(int i=0; i<9; i++) board[i] = 0;
    state = PLAYER_TURN;
    updateTopScreen();
    drawBottomScreen();
}

int main(void) {
    srand(time(NULL)); // אתחול מנוע אקראיות
    
    videoSetMode(MODE_0_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    topConsole = consoleInit(NULL, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);

    videoSetModeSub(MODE_5_2D);
    vramSetBankC(VRAM_C_SUB_BG);
    int bg3 = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    videoMemory = bgGetGfxPtr(bg3);

    updateTopScreen();
    drawBottomScreen();

    while(1) {
        scanKeys();
        int pressedKeys = keysDown();

        // תור המחשב (עם השהיה קלה לתחושה טבעית)
        if (state == COMPUTER_TURN) {
            delayTimer++;
            if (delayTimer > 40) { // ממתין כחצי שנייה
                doComputerMove();
                if (checkWinArr(board) || isFull(board)) state = GAME_OVER;
                else state = PLAYER_TURN;
                
                updateTopScreen();
                drawBottomScreen();
            }
        }
        
        // זיהוי לחיצות מגע
        if (pressedKeys & KEY_TOUCH) {
            touchPosition touch;
            touchRead(&touch);

            if (state == MENU) {
                if(touch.px >= 50 && touch.px <= 206) {
                    if(touch.py >= 30 && touch.py <= 70) { difficulty = EASY; initGame(); }
                    else if(touch.py >= 80 && touch.py <= 120) { difficulty = MEDIUM; initGame(); }
                    else if(touch.py >= 130 && touch.py <= 170) { difficulty = HARD; initGame(); }
                }
            } 
            else if (state == PLAYER_TURN) {
                int ox = 53, oy = 21, cellSize = 50;
                if (touch.px >= ox && touch.px < ox + 150 && touch.py >= oy && touch.py < oy + 150) {
                    int col = (touch.px - ox) / cellSize;
                    int row = (touch.py - oy) / cellSize;
                    int cellIndex = row * 3 + col;
                    
                    if (board[cellIndex] == 0) {
                        board[cellIndex] = 1;
                        if (checkWinArr(board) || isFull(board)) state = GAME_OVER;
                        else {
                            state = COMPUTER_TURN;
                            delayTimer = 0;
                        }
                        updateTopScreen();
                        drawBottomScreen();
                    }
                }
            }
            else if (state == GAME_OVER) {
                state = MENU; // חזרה לתפריט בסיום
                updateTopScreen();
                drawBottomScreen();
            }
        }
        swiWaitForVBlank();
    }
    return 0;
}
