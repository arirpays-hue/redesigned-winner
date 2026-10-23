#include <nds.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char currentOperand[16] = "0";
char previousOperand[16] = "";
char operation = ' ';
int isNewNumber = 1;

PrintConsole* topConsole;
u16* videoMemory;

// מיני-גופן פרטי שמונע לחלוטין את הגליצ'ים הגרפיים!
const u8 calcFont[128][8] = {
    ['0'] = {0x38,0x44,0x44,0x44,0x44,0x44,0x44,0x38},
    ['1'] = {0x10,0x30,0x10,0x10,0x10,0x10,0x10,0x38},
    ['2'] = {0x38,0x44,0x04,0x08,0x10,0x20,0x44,0x7C},
    ['3'] = {0x38,0x44,0x04,0x18,0x04,0x04,0x44,0x38},
    ['4'] = {0x08,0x18,0x28,0x48,0x7C,0x08,0x08,0x08},
    ['5'] = {0x7C,0x40,0x40,0x78,0x04,0x04,0x44,0x38},
    ['6'] = {0x38,0x40,0x40,0x78,0x44,0x44,0x44,0x38},
    ['7'] = {0x7C,0x04,0x08,0x10,0x20,0x20,0x20,0x20},
    ['8'] = {0x38,0x44,0x44,0x38,0x44,0x44,0x44,0x38},
    ['9'] = {0x38,0x44,0x44,0x44,0x3C,0x04,0x04,0x38},
    ['A'] = {0x38,0x44,0x44,0x44,0x7C,0x44,0x44,0x44},
    ['C'] = {0x38,0x44,0x40,0x40,0x40,0x40,0x44,0x38},
    ['D'] = {0x78,0x44,0x44,0x44,0x44,0x44,0x44,0x78},
    ['E'] = {0x7C,0x40,0x40,0x78,0x40,0x40,0x40,0x7C},
    ['L'] = {0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x7C},
    ['.'] = {0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30},
    ['+'] = {0x00,0x10,0x10,0x7C,0x10,0x10,0x00,0x00},
    ['-'] = {0x00,0x00,0x00,0x7C,0x00,0x00,0x00,0x00},
    ['*'] = {0x00,0x44,0x28,0x10,0x28,0x44,0x00,0x00}, 
    ['/'] = {0x00,0x04,0x08,0x10,0x20,0x40,0x00,0x00},
    ['='] = {0x00,0x00,0x7C,0x00,0x7C,0x00,0x00,0x00},
    ['%'] = {0x62,0x64,0x08,0x10,0x20,0x26,0x46,0x00}
};

void updateDisplay() {
    consoleClear();
    printf("\n\n\n\n\n\n\n\n"); 
    
    if (operation != ' ') {
        char buf[32];
        snprintf(buf, sizeof(buf), "%s %c", previousOperand, operation);
        int padding = 30 - strlen(buf);
        for(int i = 0; i < padding; i++) printf(" ");
        printf("\x1b[30;1m%s\n\n\x1b[0m", buf);
    } else {
        printf("\n\n");
    }
    
    int padding = 30 - strlen(currentOperand);
    for(int i = 0; i < padding; i++) printf(" ");
    printf("\x1b[37m%s\n\x1b[0m", currentOperand);
}

// פונקציית הציור החדשה שמשתמשת במיני-גופן שלנו
void drawCharScale(u16* vram, int x, int y, char c, u16 color, int scale) {
    if (c < 0 || c > 127) return;
    const u8* glyph = calcFont[(int)c];
    
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

void drawCalculatorUI(u16* vram) {
    u16 bgCol      = ARGB16(1, 2, 2, 2);     
    u16 btnDark    = ARGB16(1, 5, 5, 5);     
    u16 btnTop     = ARGB16(1, 7, 7, 7);     
    u16 btnClear   = ARGB16(1, 7, 4, 4);     
    u16 btnOp      = ARGB16(1, 3, 7, 11);    
    u16 btnEq      = ARGB16(1, 0, 25, 10);   
    
    u16 textWhite  = ARGB16(1, 31, 31, 31);
    u16 textRed    = ARGB16(1, 31, 10, 10);  
    u16 textGreen  = ARGB16(1, 0, 28, 14);   
    u16 textBlue   = ARGB16(1, 12, 22, 30);  
    u16 textBlack  = ARGB16(1, 0, 0, 0);

    for (int i = 0; i < 256 * 192; i++) vram[i] = bgCol;
    
    drawRoundRect(vram, 4, 4, 59, 33, 8, btnClear);
    drawString(vram, 4 + 13, 4 + 8, "AC", textRed, 2);
    
    drawRoundRect(vram, 67, 4, 59, 33, 8, btnTop);
    drawString(vram, 67 + 5, 4 + 8, "DEL", textGreen, 2);
    
    drawRoundRect(vram, 130, 4, 59, 33, 8, btnTop);
    drawString(vram, 130 + 21, 4 + 8, "%", textGreen, 2);
    
    drawRoundRect(vram, 193, 4, 59, 33, 8, btnOp);
    drawString(vram, 193 + 21, 4 + 8, "/", textBlue, 2);
    
    for(int i=0; i<3; i++) {
        drawRoundRect(vram, 4 + i*63, 41, 59, 33, 8, btnDark);
        char s[2] = {'7'+i, 0};
        drawString(vram, 4 + i*63 + 21, 41 + 8, s, textWhite, 2);
    }
    drawRoundRect(vram, 193, 41, 59, 33, 8, btnOp);
    drawString(vram, 193 + 21, 41 + 8, "*", textBlue, 2);
    
    for(int i=0; i<3; i++) {
        drawRoundRect(vram, 4 + i*63, 78, 59, 33, 8, btnDark);
        char s[2] = {'4'+i, 0};
        drawString(vram, 4 + i*63 + 21, 78 + 8, s, textWhite, 2);
    }
    drawRoundRect(vram, 193, 78, 59, 33, 8, btnOp);
    drawString(vram, 193 + 21, 78 + 8, "-", textBlue, 2);
    
    for(int i=0; i<3; i++) {
        drawRoundRect(vram, 4 + i*63, 115, 59, 33, 8, btnDark);
        char s[2] = {'1'+i, 0};
        drawString(vram, 4 + i*63 + 21, 115 + 8, s, textWhite, 2);
    }
    drawRoundRect(vram, 193, 115, 59, 33, 8, btnOp);
    drawString(vram, 193 + 21, 115 + 8, "+", textBlue, 2);
    
    drawRoundRect(vram, 4, 152, 122, 33, 8, btnDark); 
    drawString(vram, 4 + 53, 152 + 8, "0", textWhite, 2);
    
    drawRoundRect(vram, 130, 152, 59, 33, 8, btnDark);
    drawString(vram, 130 + 21, 152 + 8, ".", textWhite, 2);
    
    drawRoundRect(vram, 193, 152, 59, 33, 8, btnEq);
    drawString(vram, 193 + 21, 152 + 8, "=", textBlack, 2);
}

void appendNumber(char n) {
    if (isNewNumber) {
        if (n == '.') strcpy(currentOperand, "0.");
        else {
            currentOperand[0] = n;
            currentOperand[1] = '\0';
        }
        isNewNumber = 0;
    } else {
        if (n == '.' && strchr(currentOperand, '.')) return;
        if (strlen(currentOperand) < 12) {
            int len = strlen(currentOperand);
            currentOperand[len] = n;
            currentOperand[len+1] = '\0';
        }
    }
    updateDisplay();
}

void clearAll() {
    strcpy(currentOperand, "0");
    strcpy(previousOperand, "");
    operation = ' ';
    isNewNumber = 1;
    updateDisplay();
}

void deleteLast() {
    int len = strlen(currentOperand);
    if (len == 1 || strcmp(currentOperand, "Error") == 0) {
        strcpy(currentOperand, "0");
        isNewNumber = 1;
    } else {
        currentOperand[len-1] = '\0';
    }
    updateDisplay();
}

void compute() {
    double prev = atof(previousOperand);
    double curr = atof(currentOperand);
    double result = 0;

    if (operation == '+') result = prev + curr;
    else if (operation == '-') result = prev - curr;
    else if (operation == '*') result = prev * curr;
    else if (operation == '/') {
        if (curr == 0) {
            strcpy(currentOperand, "Error");
            strcpy(previousOperand, "");
            operation = ' ';
            isNewNumber = 1;
            updateDisplay();
            return;
        }
        result = prev / curr;
    }
    else if (operation == '%') result = (prev / 100.0) * curr;
    else return;

    snprintf(currentOperand, 16, "%.6g", result);
    operation = ' ';
    strcpy(previousOperand, "");
    isNewNumber = 1;
    updateDisplay();
}

void chooseOperation(char op) {
    if (strcmp(currentOperand, "Error") == 0) clearAll();
    if (strlen(previousOperand) > 0 && !isNewNumber) compute();
    operation = op;
    strcpy(previousOperand, currentOperand);
    isNewNumber = 1;
    updateDisplay();
}

int main(void) {
    videoSetMode(MODE_0_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    topConsole = consoleInit(NULL, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);

    videoSetModeSub(MODE_5_2D);
    vramSetBankC(VRAM_C_SUB_BG);
    int bg3 = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    videoMemory = bgGetGfxPtr(bg3);

    drawCalculatorUI(videoMemory);
    updateDisplay();

    while(1) {
        scanKeys();
        int pressedKeys = keysDown();

        if (pressedKeys & KEY_TOUCH) {
            touchPosition touch;
            touchRead(&touch);

            int col = -1, row = -1;
            if (touch.py >= 4 && touch.py <= 37) row = 0;
            else if (touch.py >= 41 && touch.py <= 74) row = 1;
            else if (touch.py >= 78 && touch.py <= 111) row = 2;
            else if (touch.py >= 115 && touch.py <= 148) row = 3;
            else if (touch.py >= 152 && touch.py <= 185) row = 4;
            
            if (touch.px >= 4 && touch.px <= 63) col = 0;
            else if (touch.px >= 67 && touch.px <= 126) col = 1;
            else if (touch.px >= 130 && touch.px <= 189) col = 2;
            else if (touch.px >= 193 && touch.px <= 252) col = 3;
            
            if (row == 4 && col == 1) col = 0; 
            
            if (row != -1 && col != -1) {
                if (row == 0) {
                    if (col == 0) clearAll();
                    else if (col == 1) deleteLast();
                    else if (col == 2) chooseOperation('%');
                    else if (col == 3) chooseOperation('/');
                }
                else if (row == 1) {
                    if (col == 3) chooseOperation('*');
                    else appendNumber('7' + col);
                }
                else if (row == 2) {
                    if (col == 3) chooseOperation('-');
                    else appendNumber('4' + col);
                }
                else if (row == 3) {
                    if (col == 3) chooseOperation('+');
                    else appendNumber('1' + col);
                }
                else if (row == 4) {
                    if (col == 0) appendNumber('0'); 
                    else if (col == 2) appendNumber('.');
                    else if (col == 3) compute();
                }
            }
        }
        swiWaitForVBlank();
    }
    return 0;
}
