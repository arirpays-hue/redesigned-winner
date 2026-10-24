#include "game.h"
#include <stdlib.h>

void init3D(void) {
    videoSetMode(MODE_0_3D);
    glInit();
    glEnable(GL_ANTIALIAS);
    
    glClearColor(1, 1, 3, 31);
    glClearPolyID(63);
    glClearDepth(0x7FFF);
    
    glViewport(0, 0, 255, 191);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, 256.0 / 192.0, 0.1, 40.0);
    
    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
}

static void drawBox3D(float x, float y, float z, float w, float h, float d, int r, int g, int b) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(w, h, d);

    glBegin(GL_QUADS);
    // פאה קדמית
    glColor3b(r, g, b);
    glVertex3v16(floattov16(-0.5), floattov16(-0.5), floattov16(0.5));
    glVertex3v16(floattov16(0.5),  floattov16(-0.5), floattov16(0.5));
    glVertex3v16(floattov16(0.5),  floattov16(0.5),  floattov16(0.5));
    glVertex3v16(floattov16(-0.5), floattov16(0.5),  floattov16(0.5));

    // פאה אחורית
    glColor3b(r/2, g/2, b/2);
    glVertex3v16(floattov16(-0.5), floattov16(-0.5), floattov16(-0.5));
    glVertex3v16(floattov16(-0.5), floattov16(0.5),  floattov16(-0.5));
    glVertex3v16(floattov16(0.5),  floattov16(0.5),  floattov16(-0.5));
    glVertex3v16(floattov16(0.5),  floattov16(-0.5), floattov16(-0.5));

    // פאה עליונה
    int rt = (r + 50 > 255) ? 255 : r + 50;
    int gt = (g + 50 > 255) ? 255 : g + 50;
    int bt = (b + 50 > 255) ? 255 : b + 50;
    glColor3b(rt, gt, bt);
    glVertex3v16(floattov16(-0.5), floattov16(0.5),  floattov16(-0.5));
    glVertex3v16(floattov16(-0.5), floattov16(0.5),  floattov16(0.5));
    glVertex3v16(floattov16(0.5),  floattov16(0.5),  floattov16(0.5));
    glVertex3v16(floattov16(0.5),  floattov16(0.5),  floattov16(-0.5));

    // פאה תחתונה
    glColor3b(r/3, g/3, b/3);
    glVertex3v16(floattov16(-0.5), floattov16(-0.5), floattov16(-0.5));
    glVertex3v16(floattov16(0.5),  floattov16(-0.5), floattov16(-0.5));
    glVertex3v16(floattov16(0.5),  floattov16(-0.5), floattov16(0.5));
    glVertex3v16(floattov16(-0.5), floattov16(-0.5), floattov16(0.5));

    // פאות צדדיות
    glColor3b((r*3)/4, (g*3)/4, (b*3)/4);
    glVertex3v16(floattov16(0.5), floattov16(-0.5), floattov16(-0.5));
    glVertex3v16(floattov16(0.5), floattov16(0.5),  floattov16(-0.5));
    glVertex3v16(floattov16(0.5), floattov16(0.5),  floattov16(0.5));
    glVertex3v16(floattov16(0.5), floattov16(-0.5), floattov16(0.5));

    glVertex3v16(floattov16(-0.5), floattov16(-0.5), floattov16(-0.5));
    glVertex3v16(floattov16(-0.5), floattov16(-0.5), floattov16(0.5));
    glVertex3v16(floattov16(-0.5), floattov16(0.5),  floattov16(0.5));
    glVertex3v16(floattov16(-0.5), floattov16(0.5),  floattov16(-0.5));
    glEnd();

    glPopMatrix(1);
}

static void draw3DX(float cx, float cy, float cz, float scale, int angle, int isWinner) {
    glPushMatrix();
    glTranslatef(cx, cy, cz);
    glScalef(scale, scale, scale);
    glRotateZi(angle);
    
    int r = isWinner ? ((winEffectType == 2) ? (effectTimer * 8) & 255 : 255) : 0;
    int g = isWinner ? ((winEffectType == 2) ? (128 + effectTimer * 4) & 255 : 255) : 255;
    int b = isWinner ? ((winEffectType == 2) ? 255 : 0) : 120;

    glPushMatrix();
    glRotateZi(4096);
    drawBox3D(0, 0, 0, 1.35f, 0.26f, 0.26f, r, g, b);
    glPopMatrix(1);

    glPushMatrix();
    glRotateZi(-4096);
    drawBox3D(0, 0, 0, 1.35f, 0.26f, 0.26f, r, g, b);
    glPopMatrix(1);

    glPopMatrix(1);
}

static void draw3DO(float cx, float cy, float cz, float scale, int angle, int isWinner) {
    glPushMatrix();
    glTranslatef(cx, cy, cz);
    glScalef(scale, scale, scale);
    glRotateYi(angle);
    
    int r = 255;
    int g = isWinner ? ((winEffectType == 2) ? (effectTimer * 8) & 255 : 255) : 50;
    int b = isWinner ? ((winEffectType == 2) ? 200 : 0) : 80;

    drawBox3D(0,  0.42f, 0, 0.95f, 0.22f, 0.25f, r, g, b);
    drawBox3D(0, -0.42f, 0, 0.95f, 0.22f, 0.25f, r, g, b);
    drawBox3D(-0.42f, 0, 0, 0.22f, 0.95f, 0.25f, r, g, b);
    drawBox3D( 0.42f, 0, 0, 0.22f, 0.95f, 0.25f, r, g, b);

    glPopMatrix(1);
}

void render3DScene(void) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, camZoom);
    glRotatef(camRotX, 1.0f, 0.0f, 0.0f);
    glRotatef(camRotY, 0.0f, 1.0f, 0.0f);

    if (state == MENU) {
        camRotY += 0.6f;
        if (camRotY > 360.0f) camRotY -= 360.0f;
    } else if (state == GAME_OVER && winLine[0] != -1 && winEffectType == 3) {
        // אפקט 3: סופת דיסקו - הלוח מסתובב מהר בחלל!
        camRotY += 3.2f;
        if (camRotY > 360.0f) camRotY -= 360.0f;
    }

    // משטח בסיס
    drawBox3D(0.0f, 0.0f, -0.2f, 5.2f, 5.2f, 0.15f, 20, 30, 55);

    // צבע רשת הלוח (משתנה לאורות דיסקו באפקט 3)
    int gr = 0, gg = 180, gb = 255;
    if (state == GAME_OVER && winLine[0] != -1 && winEffectType == 3) {
        gr = (effectTimer * 12) & 255;
        gg = (effectTimer * 7 + 85) & 255;
        gb = (effectTimer * 19 + 170) & 255;
    }

    drawBox3D(-0.8f, 0.0f, 0.0f, 0.16f, 4.6f, 0.25f, gr, gg, gb);
    drawBox3D( 0.8f, 0.0f, 0.0f, 0.16f, 4.6f, 0.25f, gr, gg, gb);
    drawBox3D( 0.0f,-0.8f, 0.0f, 4.6f, 0.16f, 0.25f, gr, gg, gb);
    drawBox3D( 0.0f, 0.8f, 0.0f, 4.6f, 0.16f, 0.25f, gr, gg, gb);

    // ציור הכלים עם אפקטי הניצחון השונים
    for (int i = 0; i < 9; i++) {
        if (board[i] != 0) {
            int col = i % 3;
            int row = i / 3;
            float cx = (col - 1) * 1.6f;
            float cy = (1 - row) * 1.6f;
            float cz = 0.25f;
            float scale = 1.0f;
            
            int isWinPiece = (i == winLine[0] || i == winLine[1] || i == winLine[2]);
            pieceAngle[i] += isWinPiece ? 420 : 180;

            if (state == GAME_OVER && isWinPiece) {
                if (winEffectType == 1) {
                    // אפקט 1: הכלים המנצחים קופצים לגובה בגל!
                    int phase = (effectTimer + i * 10) % 40;
                    int bounce = (phase < 20) ? phase : (40 - phase);
                    cz += bounce * 0.06f;
                } else if (winEffectType == 2) {
                    // אפקט 2: הכלים המנצחים פועמים בגודלם (Pulse)!
                    int phase = (effectTimer * 2) % 40;
                    int pulse = (phase < 20) ? phase : (40 - phase);
                    scale = 0.85f + (pulse * 0.03f);
                }
            }
            
            if (board[i] == 1) draw3DX(cx, cy, cz, scale, pieceAngle[i], isWinPiece);
            else if (board[i] == 2) draw3DO(cx, cy, cz, scale, pieceAngle[i], isWinPiece);
        }
    }

    // קרן לייזר (מוצגת באפקט 0 או באפקט 3 עם צבעים מתחלפים)
    if (state == GAME_OVER && winLine[0] != -1 && (winEffectType == 0 || winEffectType == 3)) {
        int c0 = winLine[0] % 3, r0 = winLine[0] / 3;
        int c2 = winLine[2] % 3, r2 = winLine[2] / 3;
        float x0 = (c0 - 1) * 1.6f, y0 = (1 - r0) * 1.6f;
        float x2 = (c2 - 1) * 1.6f, y2 = (1 - r2) * 1.6f;
        float midX = (x0 + x2) / 2.0f;
        float midY = (y0 + y2) / 2.0f;

        int lr = 255, lg = 230, lb = 0;
        if (winEffectType == 3) {
            lr = 255; lg = (effectTimer * 15) & 255; lb = 255;
        }

        glPushMatrix();
        glTranslatef(midX, midY, 0.55f);
        if (r0 == r2) {
            drawBox3D(0, 0, 0, 4.4f, 0.18f, 0.18f, lr, lg, lb);
        } else if (c0 == c2) {
            drawBox3D(0, 0, 0, 0.18f, 4.4f, 0.18f, lr, lg, lb);
        } else if (winLine[0] == 0) {
            glRotateZi(-4096);
            drawBox3D(0, 0, 0, 5.8f, 0.18f, 0.18f, lr, lg, lb);
        } else {
            glRotateZi(4096);
            drawBox3D(0, 0, 0, 5.8f, 0.18f, 0.18f, lr, lg, lb);
        }
        glPopMatrix(1);
    }

    glFlush(0);
}
