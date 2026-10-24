#include "game.h"
#include <stdlib.h>
#include <time.h>

enum GameState state = MENU;
enum Difficulty difficulty = MEDIUM;
int board[9] = {0};
int winLine[3] = {-1, -1, -1};

float camRotX = -28.0f;
float camRotY = 0.0f;
float camZoom = -5.8f;
int pieceAngle[9] = {0};

static int delayTimer = 0;
static u16* videoMemoryBottom;
static u16 backBuffer[256 * 192]; // באפר כפול למניעת הבהובים במסך התחתון

static void initGame(void) {
    for (int i = 0; i < 9; i++) {
        board[i] = 0;
        pieceAngle[i] = 0;
    }
    winLine[0] = -1; winLine[1] = -1; winLine[2] = -1;
    camRotX = -28.0f;
    camRotY = 0.0f;
    state = PLAYER_TURN;
}

int main(void) {
    srand(time(NULL));

    // אתחול המסך העליון לתלת-ממד (3D)
    init3D();

    // אתחול המסך התחתון לדו-ממד (2D Bitmap)
    videoSetModeSub(MODE_5_2D);
    vramSetBankC(VRAM_C_SUB_BG);
    int bgSub = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    videoMemoryBottom = bgGetGfxPtr(bgSub);

    while (1) {
        scanKeys();
        int down = keysDown();
        int held = keysHeld();

        // שליטה בזווית המצלמה התלת-ממדית באמצעות החצים וכפתורי L/R!
        if (held & KEY_LEFT)  camRotY -= 2.0f;
        if (held & KEY_RIGHT) camRotY += 2.0f;
        if (held & KEY_UP)    camRotX -= 1.5f;
        if (held & KEY_DOWN)  camRotX += 1.5f;
        if (held & KEY_L)     camZoom -= 0.08f;
        if (held & KEY_R)     camZoom += 0.08f;

        // תור המחשב
        if (state == COMPUTER_TURN) {
            delayTimer++;
            if (delayTimer > 35) {
                doComputerMove();
                updateWinLine();
                if (checkWinArr(board) || isFull(board)) {
                    state = GAME_OVER;
                } else {
                    state = PLAYER_TURN;
                }
            }
        }

        // זיהוי נגיעות במסך התחתון
        if (down & KEY_TOUCH) {
            touchPosition touch;
            touchRead(&touch);

            if (state == MENU) {
                if (touch.px >= 55 && touch.px <= 201) {
                    if (touch.py >= 52 && touch.py <= 88) {
                        difficulty = EASY;
                        spawnParticles(touch.px, touch.py, ARGB16(1, 0, 31, 12));
                        initGame();
                    } else if (touch.py >= 98 && touch.py <= 134) {
                        difficulty = MEDIUM;
                        spawnParticles(touch.px, touch.py, ARGB16(1, 31, 31, 0));
                        initGame();
                    } else if (touch.py >= 144 && touch.py <= 180) {
                        difficulty = HARD;
                        spawnParticles(touch.px, touch.py, ARGB16(1, 31, 6, 6));
                        initGame();
                    }
                }
            } else if (state == PLAYER_TURN) {
                int ox = 68, oy = 52, cellSize = 40;
                if (touch.px >= ox && touch.px < ox + 120 && touch.py >= oy && touch.py < oy + 120) {
                    int col = (touch.px - ox) / cellSize;
                    int row = (touch.py - oy) / cellSize;
                    int idx = row * 3 + col;

                    if (board[idx] == 0) {
                        board[idx] = 1;
                        pieceAngle[idx] = 0;
                        spawnParticles(touch.px, touch.py, ARGB16(1, 0, 31, 12));
                        updateWinLine();

                        if (checkWinArr(board) || isFull(board)) {
                            state = GAME_OVER;
                        } else {
                            state = COMPUTER_TURN;
                            delayTimer = 0;
                        }
                    }
                }
            } else if (state == GAME_OVER) {
                state = MENU;
            }
        }

        // 1. רינדור הזירה התלת-ממדית במסך העליון
        render3DScene();

        // 2. ציור הממשק והחלקיקים לבאפר והעתקה חלקה למסך התחתון
        drawBottomUI(backBuffer);
        swiWaitForVBlank();
        dmaCopy(backBuffer, videoMemoryBottom, 256 * 192 * 2);
    }
    return 0;
}
