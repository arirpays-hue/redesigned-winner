#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum GameState { MENU, PLAYER_TURN, COMPUTER_TURN, GAME_OVER };
enum Difficulty { EASY = 0, MEDIUM = 1, HARD = 2 };

enum GameState state = MENU;
enum Difficulty difficulty = MEDIUM;

int board[9] = {0}; // 0=ריק, 1=X (שחקן), 2=O (מחשב)
int delayTimer = 0;

u16* videoMemoryTop;
u16* videoMemoryBottom;

// קבועים לאותיות בעברית - מונע ב-100% בעיות קידוד וג'יבריש!
enum HebChars {
    H_SPC = 0,
    H_ALEF = 1, H_BET = 2, H_GIMEL = 3, H_DALET = 4, H_HE = 5, H_VAV = 6, H_ZAYIN = 7,
    H_CHET = 8, H_TET = 9, H_YOD = 10, H_CHAF_S = 11, H_CHAF = 12, H_LAMED = 13,
    H_MEM_S = 14, H_MEM = 15, H_NUN_S = 16, H_NUN = 17, H_SAMECH = 18, H_AYIN = 19,
    H_PE_S = 20, H_PE = 21, H_TSADI_S = 22, H_TSADI = 23, H_KOF = 24, H_RESH = 25,
    H_SHIN = 26, H_TAV = 27,
    H_COLON = 28, H_EXCL = 29, H_DOT = 30, H_X = 31, H_O = 32
};

// פונט 8x8 מעוצב מחדש בבינארי ברור (רוחב כל אות 6 פיקסלים למניעת התנגשויות)
const u8 hebFont[35][8] = {
    [H_SPC]     = {0,0,0,0,0,0,0,0},
    [H_ALEF]    = {0b10001000, 0b10010000, 0b01010000, 0b00100000, 0b01010000, 0b01001000, 0b10001000, 0}, // א
    [H_BET]     = {0b01111000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b11111100, 0}, // ב
    [H_GIMEL]   = {0b01111000, 0b00001000, 0b00001000, 0b00011000, 0b00101000, 0b01001000, 0b10001000, 0}, // ג
    [H_DALET]   = {0b11111100, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0}, // ד
    [H_HE]      = {0b01111000, 0b00001000, 0b00001000, 0b01001000, 0b01001000, 0b01001000, 0b01001000, 0}, // ה
    [H_VAV]     = {0b00110000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0}, // ו
    [H_ZAYIN]   = {0b01111000, 0b00010000, 0b00100000, 0b00100000, 0b00100000, 0b00100000, 0b00100000, 0}, // ז
    [H_CHET]    = {0b01111000, 0b01001000, 0b01001000, 0b01001000, 0b01001000, 0b01001000, 0b01001000, 0}, // ח
    [H_TET]     = {0b01001000, 0b01011000, 0b01001000, 0b01001000, 0b01001000, 0b01001000, 0b00110000, 0}, // ט
    [H_YOD]     = {0b00110000, 0b00010000, 0b00010000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0}, // י
    [H_CHAF_S]  = {0b01111000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000}, // ך
    [H_CHAF]    = {0b01110000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b01110000, 0}, // כ
    [H_LAMED]   = {0b01000000, 0b01000000, 0b01111000, 0b00001000, 0b00001000, 0b00010000, 0b01100000, 0}, // ל
    [H_MEM_S]   = {0b01111000, 0b01001000, 0b01001000, 0b01001000, 0b01001000, 0b01001000, 0b01111000, 0}, // ם
    [H_MEM]     = {0b01110000, 0b10001000, 0b10101000, 0b10101000, 0b00001000, 0b00001000, 0b01111000, 0}, // מ
    [H_NUN_S]   = {0b00110000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000}, // ן
    [H_NUN]     = {0b00110000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b01110000, 0}, // נ
    [H_SAMECH]  = {0b01111000, 0b10000100, 0b10000100, 0b10000100, 0b10000100, 0b10000100, 0b01111000, 0}, // ס
    [H_AYIN]    = {0b01001000, 0b01001000, 0b01001000, 0b01001000, 0b00101000, 0b00011000, 0b01110000, 0}, // ע
    [H_PE_S]    = {0b01111000, 0b01001000, 0b01101000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000}, // ף
    [H_PE]      = {0b01111000, 0b01001000, 0b01101000, 0b00001000, 0b00001000, 0b01001000, 0b01111000, 0}, // פ
    [H_TSADI_S] = {0b01001000, 0b01001000, 0b00101000, 0b00011000, 0b00010000, 0b00010000, 0b00010000, 0b00010000}, // ץ
    [H_TSADI]   = {0b01001000, 0b01001000, 0b00101000, 0b00010000, 0b00010000, 0b00010000, 0b01111000, 0}, // צ
    [H_KOF]     = {0b01111000, 0b00001000, 0b01001000, 0b01010000, 0b01000000, 0b01000000, 0b01000000, 0b01000000}, // ק
    [H_RESH]    = {0b01111000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0}, // ר
    [H_SHIN]    = {0b10010100, 0b10010100, 0b10010100, 0b10110100, 0b10000100, 0b01001000, 0b00110000, 0}, // ש
    [H_TAV]     = {0b01111000, 0b01001000, 0b01001000, 0b01001000, 0b01001000, 0b01001000, 0b11001000, 0}, // ת
    [H_COLON]   = {0, 0b00110000, 0b00110000, 0, 0, 0b00110000, 0b00110000, 0}, // :
    [H_EXCL]    = {0b00110000, 0b00110000, 0b00110000, 0b00110000, 0, 0, 0b00110000, 0}, // !
    [H_DOT]     = {0, 0, 0, 0, 0, 0, 0b00110000, 0}, // .
    [H_X]       = {0b10000010, 0b01000100, 0b00101000, 0b00010000, 0b00101000, 0b01000100, 0b10000010, 0}, // X
    [H_O]       = {0b00111100, 0b01000010, 0b10000001, 0b10000001, 0b10000001, 0b01000010, 0b00111100, 0}  // O
};

// כל משפט במשחק מוגדר מראש - חסין מתקלות UTF-8
const u8 STR_TITLE[]     = {H_ALEF, H_YOD, H_KOF, H_SAMECH, H_SPC, H_AYIN, H_YOD, H_GIMEL, H_VAV, H_LAMED, 0xFF};
const u8 STR_CHOOSE[]    = {H_BET, H_CHET, H_RESH, H_SPC, H_RESH, H_MEM, H_TAV, H_SPC, H_KOF, H_VAV, H_SHIN, H_YOD, 0xFF};
const u8 STR_BOTTOM[]    = {H_BET, H_MEM, H_SAMECH, H_CHAF_S, H_SPC, H_HE, H_TAV, H_CHET, H_TAV, H_VAV, H_NUN_S, 0xFF};
const u8 STR_WIN[]       = {H_NUN, H_YOD, H_TSADI, H_CHET, H_TAV, H_EXCL, 0xFF};
const u8 STR_LOSE[]      = {H_HE, H_MEM, H_CHET, H_SHIN, H_BET, H_SPC, H_NUN, H_YOD, H_TSADI, H_CHET, H_EXCL, 0xFF};
const u8 STR_TIE[]       = {H_TAV, H_YOD, H_KOF, H_VAV, H_EXCL, 0xFF};
const u8 STR_RESTART[]   = {H_GIMEL, H_AYIN, H_SPC, H_BET, H_MEM, H_SAMECH, H_CHAF_S, H_SPC, H_LAMED, H_MEM, H_SHIN, H_CHET, H_KOF, H_SPC, H_CHET, H_DALET, H_SHIN, 0xFF};
const u8 STR_LVL_EASY[]  = {H_RESH, H_MEM, H_HE, H_COLON, H_SPC, H_KOF, H_LAMED, 0xFF};
const u8 STR_LVL_MED[]   = {H_RESH, H_MEM, H_HE, H_COLON, H_SPC, H_BET, H_YOD, H_NUN, H_VAV, H_NUN, H_YOD, 0xFF};
const u8 STR_LVL_HARD[]  = {H_RESH, H_MEM, H_HE, H_COLON, H_SPC, H_KOF, H_SHIN, H_HE, 0xFF};
const u8 STR_YOUR_TURN[] = {H_TAV, H_VAV, H_RESH, H_CHAF_S, 0xFF};
const u8 STR_THINKING[]  = {H_HE, H_MEM, H_CHET, H_SHIN, H_BET, H_SPC, H_CHET, H_VAV, H_SHIN, H_BET, H_DOT, H_DOT, H_DOT, 0xFF};
const u8 STR_BTN_EASY[]  = {H_KOF, H_LAMED, 0xFF};
const u8 STR_BTN_MED[]   = {H_BET, H_YOD, H_NUN, H_VAV, H_NUN, H_YOD, 0xFF};
const u8 STR_BTN_HARD[]  = {H_KOF, H_SHIN, H_HE, 0xFF};

// --- מנוע גרפי ---
void drawGlyph(u16* vram, int x, int y, u8 id, u16 color, int scale) {
    if (id >= 35) return;
    const u8* glyph = hebFont[id];
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

// מצייר משפט בעברית מימין לשמאל וממרכז אותו אוטומטית סביב centerX
void drawCenteredHeb(u16* vram, int centerX, int y, const u8* codes, u16 color, int scale) {
    int totalWidth = 0;
    for (int i = 0; codes[i] != 0xFF; i++) {
        totalWidth += (codes[i] == H_SPC) ? (4 * scale) : (7 * scale);
    }
    
    int curRight = centerX + (totalWidth / 2);
    for (int i = 0; codes[i] != 0xFF; i++) {
        int step = (codes[i] == H_SPC) ? (4 * scale) : (7 * scale);
        curRight -= step;
        if (codes[i] != H_SPC) {
            drawGlyph(vram, curRight, y, codes[i], color, scale);
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

// --- לוגיקת משחק ו-AI ---
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

// --- עדכון מסכים ---
void updateTopScreen() {
    fillScreen(videoMemoryTop, ARGB16(1, 2, 2, 3));
    
    u16 white  = ARGB16(1, 31, 31, 31);
    u16 green  = ARGB16(1, 0, 31, 10);
    u16 red    = ARGB16(1, 31, 5, 5);
    u16 yellow = ARGB16(1, 31, 31, 0);
    u16 blue   = ARGB16(1, 10, 20, 31);
    
    drawCenteredHeb(videoMemoryTop, 128, 25, STR_TITLE, blue, 3);
    
    if (state == MENU) {
        drawCenteredHeb(videoMemoryTop, 128, 90, STR_CHOOSE, white, 2);
        drawCenteredHeb(videoMemoryTop, 128, 120, STR_BOTTOM, white, 2);
    } 
    else if (state == GAME_OVER) {
        int w = checkWinArr(board);
        if(w == 1) drawCenteredHeb(videoMemoryTop, 128, 85, STR_WIN, green, 3);
        else if(w == 2) drawCenteredHeb(videoMemoryTop, 128, 85, STR_LOSE, red, 2);
        else drawCenteredHeb(videoMemoryTop, 128, 85, STR_TIE, yellow, 3);
        
        drawCenteredHeb(videoMemoryTop, 128, 145, STR_RESTART, white, 1);
    } 
    else {
        if (difficulty == EASY) drawCenteredHeb(videoMemoryTop, 128, 70, STR_LVL_EASY, white, 2);
        if (difficulty == MEDIUM) drawCenteredHeb(videoMemoryTop, 128, 70, STR_LVL_MED, white, 2);
        if (difficulty == HARD) drawCenteredHeb(videoMemoryTop, 128, 70, STR_LVL_HARD, white, 2);
        
        if (state == PLAYER_TURN) drawCenteredHeb(videoMemoryTop, 128, 120, STR_YOUR_TURN, green, 3);
        else drawCenteredHeb(videoMemoryTop, 128, 120, STR_THINKING, red, 2);
    }
}

void drawBottomScreen() {
    u16 bgCol = ARGB16(1, 2, 2, 3);
    fillScreen(videoMemoryBottom, bgCol);

    if (state == MENU) {
        u16 btnCol  = ARGB16(1, 6, 8, 12);
        u16 textCol = ARGB16(1, 31, 31, 31);
        
        drawRoundRect(videoMemoryBottom, 50, 30, 156, 40, 10, btnCol);
        drawCenteredHeb(videoMemoryBottom, 128, 42, STR_BTN_EASY, textCol, 2);
        
        drawRoundRect(videoMemoryBottom, 50, 80, 156, 40, 10, btnCol);
        drawCenteredHeb(videoMemoryBottom, 128, 92, STR_BTN_MED, textCol, 2);
        
        drawRoundRect(videoMemoryBottom, 50, 130, 156, 40, 10, btnCol);
        drawCenteredHeb(videoMemoryBottom, 128, 142, STR_BTN_HARD, textCol, 2);
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
                int px = ox + col*cellSize + 13;
                int py = oy + row*cellSize + 13;
                if(board[i] == 1) drawGlyph(videoMemoryBottom, px, py, H_X, colorX, 3);
                else drawGlyph(videoMemoryBottom, px, py, H_O, colorO, 3);
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
