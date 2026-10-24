#include "game.h"

void init3D(void) {
    // הפעלת מנוע התלת-ממד החומרתי במסך העליון
    videoSetMode(MODE_0_3D);
    glInit();
    
    glEnable(GL_ANTIALIAS); // החלקת קצוות (Anti-Aliasing) בחומרה
    
    // צבע רקע חלל עמוק (כחול-שחור כהה)
    glClearColor(1, 1, 3, 31);
    glClearPolyID(63);
    glClearDepth(0x7FFF);
    
    glViewport(0, 0, 255, 191);
    
    // הגדרת מצלמת פרספקטיבה תלת-ממדית
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, 256.0 / 192.0, 0.1, 40.0);
    
    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
}

// פונקציית עזר לציור תיבה תלת-ממדית (3D Box) עם תאורה מדומה לכל פאה
static void drawBox3D(float x, float y, float z, float w, float h, float d, int r, int g, int b) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(w, h, d);

    glBegin(GL_QUADS);
    // פאה קדמית (מוארת)
    glColor3b(r, g, b);
    glVertex3v16(floattov16(-0.5), floattov16(-0.5), floattov16(0.5));
    glVertex3v16(floattov16(0.5),  floattov16(-0.5), floattov16(0.5));
    glVertex3v16(floattov16(0.5),  floattov16(0.5),  floattov16(0.5));
    glVertex3v16(floattov16(-0.5), floattov16(0.5),  floattov16(0.5));

    // פאה אחורית (מוצללת)
    glColor3b(r/2, g/2, b/2);
    glVertex3v16(floattov16(-0.5), floattov16(-0.5), floattov16(-0.5));
    glVertex3v16(floattov16(-0.5), floattov16(0.5),  floattov16(-0.5));
    glVertex3v16(floattov16(0.5),  floattov16(0.5),  floattov16(-0.5));
    glVertex3v16(floattov16(0.5),  floattov16(-0.5), floattov16(-0.5));

    // פאה עליונה (בהירה במיוחד)
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

// ציור מודל X תלת-ממדי מסתובב
static void draw3DX(float cx, float cy, int angle, int isWinner) {
    glPushMatrix();
    glTranslatef(cx, cy, 0.25f);
    glRotateZi(angle); // סיבוב באנימציה
    
    int r = isWinner ? 255 : 0;
    int g = 255;
    int b = isWinner ? 0 : 120;

    glPushMatrix();
    glRotateZi(4096); // הטיה של 45 מעלות
    drawBox3D(0, 0, 0, 1.35f, 0.26f, 0.26f, r, g, b);
    glPopMatrix(1);

    glPushMatrix();
    glRotateZi(-4096); // הטיה של -45 מעלות
    drawBox3D(0, 0, 0, 1.35f, 0.26f, 0.26f, r, g, b);
    glPopMatrix(1);

    glPopMatrix(1);
}

// ציור מודל O תלת-ממדי מסתובב
static void draw3DO(float cx, float cy, int angle, int isWinner) {
    glPushMatrix();
    glTranslatef(cx, cy, 0.25f);
    glRotateYi(angle); // סיבוב תלת-ממדי סביב ציר Y
    
    int r = 255;
    int g = isWinner ? 255 : 50;
    int b = isWinner ? 0   : 80;

    drawBox3D(0,  0.42f, 0, 0.95f, 0.22f, 0.25f, r, g, b);
    drawBox3D(0, -0.42f, 0, 0.95f, 0.22f, 0.25f, r, g, b);
    drawBox3D(-0.42f, 0, 0, 0.22f, 0.95f, 0.25f, r, g, b);
    drawBox3D( 0.42f, 0, 0, 0.22f, 0.95f, 0.25f, r, g, b);

    glPopMatrix(1);
}

void render3DScene(void) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // מיקום המצלמה בחלל וסיבובה לפי קלט המשתמש
    glTranslatef(0.0f, 0.0f, camZoom);
    glRotatef(camRotX, 1.0f, 0.0f, 0.0f);
    glRotatef(camRotY, 0.0f, 1.0f, 0.0f);

    // במצב תפריט - הלוח מסתובב לאט מעצמו ברקע!
    if (state == MENU) {
        camRotY += 0.6f;
        if (camRotY > 360.0f) camRotY -= 360.0f;
    }

    // משטח בסיס כהה מתחת ללוח
    drawBox3D(0.0f, 0.0f, -0.2f, 5.2f, 5.2f, 0.15f, 20, 30, 55);

    // 4 קורות תלת-ממדיות זוהרות שמרכיבות את רשת הלוח
    drawBox3D(-0.8f, 0.0f, 0.0f, 0.16f, 4.6f, 0.25f, 0, 180, 255);
    drawBox3D( 0.8f, 0.0f, 0.0f, 0.16f, 4.6f, 0.25f, 0, 180, 255);
    drawBox3D( 0.0f,-0.8f, 0.0f, 4.6f, 0.16f, 0.25f, 0, 180, 255);
    drawBox3D( 0.0f, 0.8f, 0.0f, 4.6f, 0.16f, 0.25f, 0, 180, 255);

    // ציור כלי המשחק ב-3D על הלוח
    for (int i = 0; i < 9; i++) {
        if (board[i] != 0) {
            int col = i % 3;
            int row = i / 3;
            float cx = (col - 1) * 1.6f;
            float cy = (1 - row) * 1.6f;
            
            pieceAngle[i] += 180; // סיבוב מתמיד של הכלים
            int isWinPiece = (i == winLine[0] || i == winLine[1] || i == winLine[2]);
            
            if (board[i] == 1) draw3DX(cx, cy, pieceAngle[i], isWinPiece);
            else if (board[i] == 2) draw3DO(cx, cy, pieceAngle[i], isWinPiece);
        }
    }

    // אם יש ניצחון - ציור קרן לייזר מוזהבת שמחברת את השלישייה!
    if (state == GAME_OVER && winLine[0] != -1) {
        int c0 = winLine[0] % 3, r0 = winLine[0] / 3;
        int c2 = winLine[2] % 3, r2 = winLine[2] / 3;
        float x0 = (c0 - 1) * 1.6f, y0 = (1 - r0) * 1.6f;
        float x2 = (c2 - 1) * 1.6f, y2 = (1 - r2) * 1.6f;
        float midX = (x0 + x2) / 2.0f;
        float midY = (y0 + y2) / 2.0f;

        glPushMatrix();
        glTranslatef(midX, midY, 0.55f);
        if (r0 == r2) {
            drawBox3D(0, 0, 0, 4.4f, 0.18f, 0.18f, 255, 230, 0);
        } else if (c0 == c2) {
            drawBox3D(0, 0, 0, 0.18f, 4.4f, 0.18f, 255, 230, 0);
        } else if (winLine[0] == 0) {
            glRotateZi(-4096);
            drawBox3D(0, 0, 0, 5.8f, 0.18f, 0.18f, 255, 230, 0);
        } else {
            glRotateZi(4096);
            drawBox3D(0, 0, 0, 5.8f, 0.18f, 0.18f, 255, 230, 0);
        }
        glPopMatrix(1);
    }

    glFlush(0);
}
