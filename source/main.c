#include <nds.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char currentOperand[16] = "0";
char previousOperand[16] = "";
char operation = ' ';
int isNewNumber = 1;

PrintConsole topScreen;
PrintConsole bottomScreen;

// פונקציה לעדכון המסך העליון בדומה ל-updateDisplay ב-JS
void updateDisplay() {
    consoleSelect(&topScreen);
    consoleClear();
    printf("\n\n      NDS Calculator\n");
    printf("      --------------\n\n\n\n");
    
    if (operation != ' ') {
        printf("      %s %c\n\n", previousOperand, operation);
    } else {
        printf("\n\n");
    }
    printf("      %s\n", currentOperand);
}

// ציור ממשק הכפתורים למסך התחתון
void drawButtons() {
    consoleSelect(&bottomScreen);
    consoleClear();
    printf("\n\n");
    printf("   AC   |  DEL  |   %%   |   /   \n");
    printf("--------------------------------\n");
    printf("   7    |   8   |   9   |   *   \n");
    printf("--------------------------------\n");
    printf("   4    |   5   |   6   |   -   \n");
    printf("--------------------------------\n");
    printf("   1    |   2   |   3   |   +   \n");
    printf("--------------------------------\n");
    printf("        0       |   .   |   =   \n");
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
        // מניעת כפילות של נקודה עשרונית
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
    if (strlen(previousOperand) > 0 && !isNewNumber) {
        compute();
    }
    operation = op;
    strcpy(previousOperand, currentOperand);
    isNewNumber = 1;
    updateDisplay();
}

int main(void) {
    // הגדרת מסכים
    videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    vramSetBankC(VRAM_C_SUB_BG);

    // אתחול מסך עליון לתוצאה ומסך תחתון לכפתורים
    consoleInit(&topScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
    consoleInit(&bottomScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);

    drawButtons();
    updateDisplay();

    while(1) {
        scanKeys();
                int pressedKeys = keysDown();

        // זיהוי לחיצת מגע
        if (pressedKeys & KEY_TOUCH) {
            
            touchPosition touch;
            touchRead(&touch);

            // חלוקת המסך התחתון (256x192) לגריד של 4 עמודות ו-5 שורות
            int col = touch.px / 64;
            int row = touch.py / 38;

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
                if (col <= 1) appendNumber('0'); // span-two כמו ב-HTML
                else if (col == 2) appendNumber('.');
                else if (col == 3) compute();
            }
        }

        swiWaitForVBlank();
    }
    return 0;
}
