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

u16* videoMemoryTop;
u16* videoMemoryBottom;

// הפונט העברי החדש! פיקסל-ארט קריא, נקי ומדויק מ-א' עד ת'
const u8 customFont[256][8] = {
    ['X'] = {0x42,0x24,0x18,0x18,0x24,0x42,0x42,0x00},
    ['O'] = {0x3C,0x42,0x42,0x42,0x42,0x42,0x3C,0x00},
    ['!'] = {0x10,0x10,0x10,0x10,0x00,0x00,0x10,0x00},
    ['.'] = {0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18},
    [':'] = {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00},
    
    // עברית - החל ממיקום 128
    [128] = {0x18, 0x28, 0x48, 0x88, 0x08, 0x08, 0x08, 0x00}, // א
    [129] = {0x78, 0x08, 0x08, 0x08, 0x08, 0x08, 0x7C, 0x00}, // ב
    [130] = {0x18, 0x08, 0x08, 0x08, 0x08, 0x10, 0x30, 0x00}, // ג
    [131] = {0x78, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00}, // ד
    [132] = {0x78, 0x08, 0x08, 0x48, 0x48, 0x48, 0x48, 0x00}, // ה
    [133] = {0x18, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00}, // ו
    [134] = {0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00}, // ז
    [135] = {0x78, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x00}, // ח
    [136] = {0x38, 0x48, 0x48, 0x48, 0x48, 0x48, 0x3C, 0x00}, // ט
    [137] = {0x18, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00}, // י
    [138] = {0x78, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08}, // ך (עם ירידה לשורה הבאה)
    [139] = {0x38, 0x48, 0x08, 0x08, 0x08, 0x48, 0x38, 0x00}, // כ
    [140] = {0x08, 0x10, 0x20, 0x78, 0x48, 0x48, 0x38, 0x00}, // ל
    [141] = {0x78, 0x48, 0x48, 0x48, 0x48, 0x48, 0x78, 0x00}, // ם
    [142] = {0x78, 0x48, 0x48, 0x48, 0x48, 0x08, 0x38, 0x00}, // מ
    [143] = {0x18, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08}, // ן
    [144] = {0x18, 0x08, 0x08, 0x08, 0x08, 0x08, 0x38, 0x00}, // נ
    [145] = {0x38, 0x48, 0x48, 0x48, 0x48, 0x48, 0x38, 0x00}, // ס
    [146] = {0x48, 0x48, 0x48, 0x28, 0x28, 0x10, 0x10, 0x00}, // ע
    [147] = {0x78, 0x48, 0x48, 0x78, 0x08, 0x08, 0x08, 0x08}, // ף
    [148] = {0x78, 0x48, 0x48, 0x78, 0x08, 0x08, 0x78, 0x00}, // פ
    [149] = {0x48, 0x48, 0x28, 0x18, 0x08, 0x08, 0x08, 0x08}, // ץ
    [150] = {0x48, 0x48, 0x28, 0x18, 0x08, 0x08, 0x78, 0x00}, // צ
    [151] = {0x78, 0x48, 0x48, 0x78, 0x40, 0x40, 0x40, 0x40}, // ק
    [152] = {0x78, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00}, // ר
    [153] = {0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x78, 0x00}, // ש
    [154] = {0x78, 0x48, 0x48, 0x48, 0x48, 0x48, 0x4C, 0x00}  // ת
};

// --- מנוע ציור טקסט ---
void drawCharScale(u16* vram, int x, int y, int c, u16 color, int scale) {
    if (c < 0 || c > 255) return;
    const u8* glyph = customFont[c];
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

// זיהוי ותרגום העברית לכיוון הנכון (RTL) עם רווחים מתוקנים
void drawHebrewRTL(u16* vram, int startX, int y, const char* str, u16 color, int scale) {
    int cx = startX;
    for (int i = 0; str[i] != '\0'; i++) {
        unsigned char c = (unsigned char)str[i];
        if (c == 0xD7) { 
            i++;
            unsigned char hebCode = (unsigned char)str[i] - 0x90;
            if (hebCode <= 26) {
                drawCharScale(vram, cx, y, 128 + hebCode, color, scale);
                cx -= 6 * scale; 
            }
        } else if (c == ' ') {
            cx -= 4 * scale;
        } else {
            drawCharScale(vram, cx, y, c, color, scale);
            cx -= 6 * scale;
        }
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

// --- בינה מלאכותית למשחק (AI) ---
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

int minimax(int b[9], int depth, int isMax) {
    int score = checkWinArr(b);
    if (score == 2) return 10 - depth; 
    if (score == 1) return -10 + depth;
    if (isFull(b)) return 0; 

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
    int freeCells[9], numFree = 0;
    for(int i=0; i<9; i++) if(board[i] == 0) freeCells[numFree++] = i;
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
        for(int i=0; i<9; i++) {
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
    if(bestMove != -1) board[bestMove] = 2;
}

// --- ציור הממשקים ---
void updateTopScreen() {
    fillScreen(videoMemoryTop, ARGB16(1, 2, 2, 3));
    
    u16 white = ARGB16(1, 31, 31, 31);
    u16 green = ARGB16(1, 0, 31, 10);
    u16 red   = ARGB16(1, 31, 5, 5);
    u16 yellow= ARGB16(1, 31, 31, 0);
    u16 blue  = ARGB16(1, 10, 20, 31);
    
    drawHebrewRTL(videoMemoryTop, 218, 25, "איקס עיגול", blue, 3);
    
    if (state == MENU) {
        drawHebrewRTL(videoMemoryTop, 200, 90, "בחר רמת קושי", white, 2);
        drawHebrewRTL(videoMemoryTop, 194, 120, "במסך התחתון", white, 2);
    } 
    else if (state == GAME_OVER) {
        int w = checkWinArr(board);
        if(w == 1) drawHebrewRTL(videoMemoryTop, 182, 90, "ניצחת!", green, 3);
        else if(w == 2) drawHebrewRTL(videoMemoryTop, 227, 90, "המחשב ניצח!", red, 3);
        else drawHebrewRTL(videoMemoryTop, 173, 90, "תיקו!", yellow, 3);
        
        drawHebrewRTL(videoMemoryTop, 194, 160, "גע במסך כדי להתחיל שוב", white, 1);
    } 
    else {
        if (difficulty == EASY) drawHebrewRTL(videoMemoryTop, 170, 70, "רמה: קל", white, 2);
        if (difficulty == MEDIUM) drawHebrewRTL(videoMemoryTop, 194, 70, "רמה: בינוני", white, 2);
        if (difficulty == HARD) drawHebrewRTL(videoMemoryTop, 176, 70, "רמה: קשה", white, 2);
        
        if (state == PLAYER_TURN) drawHebrewRTL(videoMemoryTop, 164, 120, "תורך", green, 3);
        else drawHebrewRTL(videoMemoryTop, 206, 120, "המחשב חושב...", red, 2);
    }
}

void drawBottomScreen() {
    u16 bgCol = ARGB16(1, 2, 2, 3);
    fillScreen(videoMemoryBottom, bgCol);

    if (state == MENU) {
        u16 btnCol = ARGB16(1, 6, 8, 12);
        u16 textCol = ARGB16(1, 31, 31, 31);
        
        drawRoundRect(videoMemoryBottom, 50, 30, 156, 40, 10, btnCol);
        drawHebrewRTL(videoMemoryBottom, 140, 42, "קל", textCol, 2);
        
        drawRoundRect(videoMemoryBottom, 50, 80, 156, 40, 10, btnCol);
        drawHebrewRTL(videoMemoryBottom, 164, 92, "בינוני", textCol, 2);
        
        drawRoundRect(videoMemoryBottom, 50, 130, 156, 40, 10, btnCol);
        drawHebrewRTL(videoMemoryBottom, 146, 142, "קשה", textCol, 2);
    } 
    else {
        u16 gridCol = ARGB16(1, 20, 20, 20);
        int ox = 53, oy = 21, cellSize = 50;
        
        for(int i=0; i<150; i++) {
            for(int w=0; w<3; w++) {
                videoMemoryBottom[(oy+i)*256 + (ox+cellSize+w)] = gridCol;
                videoMemoryBottom[(oy+i)*256 + (ox+cellSize*2+w)] = gridCol;
                videoMemoryBottom[(oy+cellSize+w)*256 + (ox+i)] = gridCol;
                videoMemoryBottom[(oy+cellSize*2+w)*256 + (ox+i)] = gridCol;
            }
        }
        
        u16 colorX = ARGB16(1, 0, 31, 10);
        u16 colorO = ARGB16(1, 31, 5, 5);
        for(int i=0; i<9; i++) {
            if(board[i] != 0) {
                int row = i / 3;
                int col = i % 3;
                int px = ox + col*cellSize + 16;
                int py = oy + row*cellSize + 13;
                if(board[i] == 1) drawHebrewRTL(videoMemoryBottom, px, py, "X", colorX, 3);
                else drawHebrewRTL(videoMemoryBottom, px, py, "O", colorO, 3);
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
    srand(time(NULL)); 
    
    videoSetMode(MODE_5_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    int bgTop = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    videoMemoryTop = bgGetGfxPtr(bgTop);

    videoSetModeSub(MODE_5_2D);
    vramSetBankC(VRAM_C_SUB_BG);
    int bgBottom = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    videoMemoryBottom = bgGetGfxPtr(bgBottom);

    updateTopScreen();
    drawBottomScreen();

    while(1) {
        scanKeys();
        int pressedKeys = keysDown();

        if (state == COMPUTER_TURN) {
            delayTimer++;
            if (delayTimer > 40) { 
                doComputerMove();
                if (checkWinArr(board) || isFull(board)) state = GAME_OVER;
                else state = PLAYER_TURN;
                
                updateTopScreen();
                drawBottomScreen();
            }
        }
        
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
                state = MENU; 
                updateTopScreen();
                drawBottomScreen();
            }
        }
        swiWaitForVBlank();
    }
    return 0;
}
