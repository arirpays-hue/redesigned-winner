#include "game.h"
#include <stdlib.h>

Particle particles[MAX_PARTICLES];

// הפונט העברי הבינארי (6 פיקסלים רוחב לכל אות)
static const u8 hebFont[35][8] = {
    [H_SPC]     = {0,0,0,0,0,0,0,0},
    [H_ALEF]    = {0b10001000, 0b10010000, 0b01010000, 0b00100000, 0b01010000, 0b01001000, 0b10001000, 0},
    [H_BET]     = {0b01111000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b11111100, 0},
    [H_GIMEL]   = {0b01111000, 0b00001000, 0b00001000, 0b00011000, 0b00101000, 0b01001000, 0b10001000, 0},
    [H_DALET]   = {0b11111100, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0},
    [H_HE]      = {0b01111000, 0b00001000, 0b00001000, 0b01001000, 0b01001000, 0b01001000, 0b01001000, 0},
    [H_VAV]     = {0b00110000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0},
    [H_ZAYIN]   = {0b01111000, 0b00010000, 0b00100000, 0b00100000, 0b00100000, 0b00100000, 0b00100000, 0},
    [H_CHET]    = {0b01111000, 0b01001000, 0b01001000, 0b01001000, 0b01001000, 0b01001000, 0b01001000, 0},
    [H_TET]     = {0b01001000, 0b01011000, 0b01001000, 0b01001000, 0b01001000, 0b01001000, 0b00110000, 0},
    [H_YOD]     = {0b00110000, 0b00010000, 0b00010000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0},
    [H_CHAF_S]  = {0b01111000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000},
    [H_CHAF]    = {0b01110000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b01110000, 0},
    [H_LAMED]   = {0b01000000, 0b01000000, 0b01111000, 0b00001000, 0b00001000, 0b00010000, 0b01100000, 0},
    [H_MEM_S]   = {0b01111000, 0b01001000, 0b01001000, 0b01001000, 0b01001000, 0b01001000, 0b01111000, 0},
    [H_MEM]     = {0b01110000, 0b10001000, 0b10101000, 0b10101000, 0b00001000, 0b00001000, 0b01111000, 0},
    [H_NUN_S]   = {0b00110000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000},
    [H_NUN]     = {0b00110000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b01110000, 0},
    [H_SAMECH]  = {0b01111000, 0b10000100, 0b10000100, 0b10000100, 0b10000100, 0b10000100, 0b01111000, 0},
    [H_AYIN]    = {0b01001000, 0b01001000, 0b01001000, 0b01001000, 0b00101000, 0b00011000, 0b01110000, 0},
    [H_PE_S]    = {0b01111000, 0b01001000, 0b01101000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000},
    [H_PE]      = {0b01111000, 0b01001000, 0b01101000, 0b00001000, 0b00001000, 0b01001000, 0b01111000, 0},
    [H_TSADI_S] = {0b01001000, 0b01001000, 0b00101000, 0b00011000, 0b00010000, 0b00010000, 0b00010000, 0b00010000},
    [H_TSADI]   = {0b01001000, 0b01001000, 0b00101000, 0b00010000, 0b00010000, 0b00010000, 0b01111000, 0},
    [H_KOF]     = {0b01111000, 0b00001000, 0b01001000, 0b01010000, 0b01000000, 0b01000000, 0b01000000, 0b01000000},
    [H_RESH]    = {0b01111000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0},
    [H_SHIN]    = {0b10010100, 0b10010100, 0b10010100, 0b10110100, 0b10000100, 0b01001000, 0b00110000, 0},
    [H_TAV]     = {0b01111000, 0b01001000, 0b01001000, 0b01001000, 0b01001000, 0b01001000, 0b11001000, 0},
    [H_COLON]   = {0, 0b00110000, 0b00110000, 0, 0, 0b00110000, 0b00110000, 0},
    [H_EXCL]    = {0b00110000, 0b00110000, 0b00110000, 0b00110000, 0, 0, 0b00110000, 0},
    [H_DOT]     = {0, 0, 0, 0, 0, 0, 0b00110000, 0},
    [H_X]       = {0b10000010, 0b01000100, 0b00101000, 0b00010000, 0b00101000, 0b01000100, 0b10000010, 0},
    [H_O]       = {0b00111100, 0b01000010, 0b10000001, 0b10000001, 0b10000001, 0b01000010, 0b00111100, 0}
};

static const u8 STR_TITLE[]     = {H_ALEF, H_YOD, H_KOF, H_SAMECH, H_SPC, H_AYIN, H_YOD, H_GIMEL, H_VAV, H_LAMED, 0xFF};
static const u8 STR_CHOOSE[]    = {H_BET, H_CHET, H_RESH, H_SPC, H_RESH, H_MEM, H_TAV, H_SPC, H_KOF, H_VAV, H_SHIN, H_YOD, 0xFF};
static const u8 STR_WIN[]       = {H_NUN, H_YOD, H_TSADI, H_CHET, H_TAV, H_EXCL, 0xFF};
static const u8 STR_LOSE[]      = {H_HE, H_MEM, H_CHET, H_SHIN, H_BET, H_SPC, H_NUN, H_YOD, H_TSADI, H_CHET, H_EXCL, 0xFF};
static const u8 STR_TIE[]       = {H_TAV, H_YOD, H_KOF, H_VAV, H_EXCL, 0xFF};
static const u8 STR_RESTART[]   = {H_GIMEL, H_AYIN, H_SPC, H_BET, H_MEM, H_SAMECH, H_CHAF_S, H_SPC, H_LAMED, H_MEM, H_SHIN, H_CHET, H_KOF, H_SPC, H_CHET, H_DALET, H_SHIN, 0xFF};
static const u8 STR_YOUR_TURN[] = {H_TAV, H_VAV, H_RESH, H_CHAF_S, 0xFF};
static const u8 STR_THINKING[]  = {H_HE, H_MEM, H_CHET, H_SHIN, H_BET, H_SPC, H_CHET, H_VAV, H_SHIN, H_BET, H_DOT, H_DOT, H_DOT, 0xFF};
static const u8 STR_BTN_EASY[]  = {H_KOF, H_LAMED, 0xFF};
static const u8 STR_BTN_MED[]   = {H_BET, H_YOD, H_NUN, H_VAV, H_NUN, H_YOD, 0xFF};
static const u8 STR_BTN_HARD[]  = {H_KOF, H_SHIN, H_HE, 0xFF};

static void drawGlyph(u16* vram, int x, int y, u8 id, u16 color, int scale) {
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

static void drawCenteredHeb(u16* vram, int centerX, int y, const u8* codes, u16 color, int scale) {
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

static void drawRoundRect(u16* vram, int x, int y, int w, int h, int r, u16 color) {
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

void spawnParticles(int x, int y, u16 color) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].x = (float)x;
        particles[i].y = (float)y;
        particles[i].vx = ((rand() % 100) - 50) / 14.0f;
        particles[i].vy = ((rand() % 100) - 65) / 14.0f;
        particles[i].color = color;
        particles[i].life = 20 + (rand() % 20);
    }
}

void updateAndDrawParticles(u16* vram) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life > 0) {
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            particles[i].vy += 0.18f; // כוח משיכה לחלקיקים
            particles[i].life--;

            int px = (int)particles[i].x;
            int py = (int)particles[i].y;
            if (px >= 1 && px < 254 && py >= 1 && py < 190) {
                vram[py * 256 + px] = particles[i].color;
                vram[py * 256 + px + 1] = particles[i].color;
                vram[(py + 1) * 256 + px] = particles[i].color;
                vram[(py + 1) * 256 + px + 1] = particles[i].color;
            }
        }
    }
}

void drawBottomUI(u16* vram) {
    // רקע מדורג (Gradient) יפהפה במסך התחתון
    for (int y = 0; y < 192; y++) {
        u16 rowCol = ARGB16(1, 1, 2 + (y / 32), 4 + (y / 16));
        for (int x = 0; x < 256; x++) {
            vram[y * 256 + x] = rowCol;
        }
    }

    u16 white  = ARGB16(1, 31, 31, 31);
    u16 green  = ARGB16(1, 0, 31, 12);
    u16 red    = ARGB16(1, 31, 6, 6);
    u16 yellow = ARGB16(1, 31, 31, 0);
    u16 cyan   = ARGB16(1, 0, 26, 31);

    if (state == MENU) {
        drawCenteredHeb(vram, 128, 12, STR_TITLE, cyan, 2);
        drawCenteredHeb(vram, 128, 34, STR_CHOOSE, white, 1);

        u16 btnCol = ARGB16(1, 5, 9, 16);
        drawRoundRect(vram, 55, 52, 146, 36, 9, btnCol);
        drawCenteredHeb(vram, 128, 62, STR_BTN_EASY, green, 2);

        drawRoundRect(vram, 55, 98, 146, 36, 9, btnCol);
        drawCenteredHeb(vram, 128, 108, STR_BTN_MED, yellow, 2);

        drawRoundRect(vram, 55, 144, 146, 36, 9, btnCol);
        drawCenteredHeb(vram, 128, 154, STR_BTN_HARD, red, 2);
    } else {
        // סרגל סטטוס עליון בעברית במסך המגע
        if (state == PLAYER_TURN) {
            drawCenteredHeb(vram, 128, 14, STR_YOUR_TURN, green, 2);
        } else if (state == COMPUTER_TURN) {
            drawCenteredHeb(vram, 128, 14, STR_THINKING, red, 2);
        } else if (state == GAME_OVER) {
            int w = checkWinArr(board);
            if (w == 1) drawCenteredHeb(vram, 128, 10, STR_WIN, green, 2);
            else if (w == 2) drawCenteredHeb(vram, 128, 10, STR_LOSE, red, 2);
            else drawCenteredHeb(vram, 128, 10, STR_TIE, yellow, 2);
            drawCenteredHeb(vram, 128, 32, STR_RESTART, white, 1);
        }

        // לוח מגע אינטראקטיבי (3x3)
        int ox = 68, oy = 52, cellSize = 40;
        u16 gridCol = ARGB16(1, 0, 20, 28);

        for (int i = 0; i < 120; i++) {
            for (int w = 0; w < 2; w++) {
                vram[(oy + i) * 256 + (ox + cellSize + w)] = gridCol;
                vram[(oy + i) * 256 + (ox + cellSize * 2 + w)] = gridCol;
                vram[(oy + cellSize + w) * 256 + (ox + i)] = gridCol;
                vram[(oy + cellSize * 2 + w) * 256 + (ox + i)] = gridCol;
            }
        }

        for (int i = 0; i < 9; i++) {
            if (board[i] != 0) {
                int r = i / 3;
                int c = i % 3;
                int px = ox + c * cellSize + 12;
                int py = oy + r * cellSize + 12;
                if (board[i] == 1) drawGlyph(vram, px, py, H_X, green, 2);
                else drawGlyph(vram, px, py, H_O, red, 2);
            }
        }
    }

    // ציור חלקיקים מתפוצצים מעל הממשק
    updateAndDrawParticles(vram);
}
