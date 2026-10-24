#ifndef GAME_H
#define GAME_H

#include <nds.h>

// מצבי משחק ורמות קושי
enum GameState { MENU, PLAYER_TURN, COMPUTER_TURN, GAME_OVER };
enum Difficulty { EASY = 0, MEDIUM = 1, HARD = 2 };

extern enum GameState state;
extern enum Difficulty difficulty;
extern int board[9];
extern int winLine[3]; // שומר את 3 המשבצות המנצחות לאפקט הלייזר התלת-ממדי

// משתני זווית ומצלמה לתלת-ממד
extern float camRotX;
extern float camRotY;
extern float camZoom;
extern int pieceAngle[9];

// מערכת חלקיקים (Particles) לאפקטים במסך המגע
typedef struct {
    float x, y;
    float vx, vy;
    u16 color;
    int life;
} Particle;

#define MAX_PARTICLES 80
extern Particle particles[MAX_PARTICLES];

// קבועים לאותיות בעברית - חסין מתקלות קידוד
enum HebChars {
    H_SPC = 0,
    H_ALEF = 1, H_BET = 2, H_GIMEL = 3, H_DALET = 4, H_HE = 5, H_VAV = 6, H_ZAYIN = 7,
    H_CHET = 8, H_TET = 9, H_YOD = 10, H_CHAF_S = 11, H_CHAF = 12, H_LAMED = 13,
    H_MEM_S = 14, H_MEM = 15, H_NUN_S = 16, H_NUN = 17, H_SAMECH = 18, H_AYIN = 19,
    H_PE_S = 20, H_PE = 21, H_TSADI_S = 22, H_TSADI = 23, H_KOF = 24, H_RESH = 25,
    H_SHIN = 26, H_TAV = 27,
    H_COLON = 28, H_EXCL = 29, H_DOT = 30, H_X = 31, H_O = 32
};

// הצהרת פונקציות מכל הקבצים
int checkWinArr(int b[9]);
void updateWinLine(void);
int isFull(int b[9]);
void doComputerMove(void);

void init3D(void);
void render3DScene(void);

void drawBottomUI(u16* vram);
void spawnParticles(int x, int y, u16 color);
void updateAndDrawParticles(u16* vram);

#endif
