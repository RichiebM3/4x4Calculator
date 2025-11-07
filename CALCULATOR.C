/////////////////////// RICHARD A BRUCE ///////////////////////
/////////// 4X4 KEYPAD CALCULATOR DESIGN CONNECTED TO /////////
/////////// A 16X2 LCD SCREEN                 ////////////////


#include <stdio.h>
#include "tm4c123gh6pm.h"
#include <string.h>

// LCD Commands
#define clear_display        0x01
#define return_cursor        0x02
#define move_cursor_right    0x06
#define move_cursor_left     0x08
#define display_right        0x1C
#define display_left         0x18
#define cursor_blink         0x0F
#define cursor_off           0x0C
#define cursor_on            0x0E
#define set_4bit             0x28
#define set_8bit             0x38
#define entry_mode           0x06
#define func_8bit            0x32
#define set5x7font           0x20
#define firstRow             0x80
#define secondRow            0xC0 // Moving to second line
#define ROWS 4
#define COLUMNS 4

// Port Definitions
#define GPIO_PORTA_DATA_R       (*((volatile unsigned long *)0x400043FC))
#define GPIO_PORTA_DIR_R        (*((volatile unsigned long *)0x40004400))
#define GPIO_PORTA_AFSEL_R      (*((volatile unsigned long *)0x40004420))
#define GPIO_PORTA_PUR_R        (*((volatile unsigned long *)0x40004510))
#define GPIO_PORTA_DEN_R        (*((volatile unsigned long *)0x4000451C))

#define GPIO_PORTB_DATA_R       (*((volatile unsigned long *)0x400053FC))
#define GPIO_PORTB_DIR_R        (*((volatile unsigned long *)0x40005400))
#define GPIO_PORTB_AFSEL_R      (*((volatile unsigned long *)0x40005420))
#define GPIO_PORTB_PUR_R        (*((volatile unsigned long *)0x40005510))
#define GPIO_PORTB_DEN_R        (*((volatile unsigned long *)0x4000551C))

#define GPIO_PORTE_DATA_R       (*((volatile unsigned long *)0x400243FC))
#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_PUR_R        (*((volatile unsigned long *)0x40024510))
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_LOCK_R       (*((volatile unsigned long *)0x40024520))

// Globals
int firstDigit = 0;
int secondDigit = 0;
int resultDigit = 0;
int waitSecondDigit = 0;
char operation = '+'; // Default operation
char buffer[16];

// Function Prototypes
void delay_ms(int n);
void delay_us(int n);
void LCD_init(void);
void LCD_Cmd(unsigned char cmd);
void LCD_write_char(unsigned char data);
void LCD_write_nibble(unsigned char data, unsigned char cmd);
void LCD_string(char *str);
void Timer0A_usd(void);
void Timer1A_msd(void);
unsigned char getKey(void);
void displayCalculation(void);
unsigned char detectKeyInColumn(unsigned char column, int row);

// Keypad Mapping
int keymap[ROWS][COLUMNS] = {
    { 1, 2, 3, 10 },  // 10 for '*'
    { 4, 5, 6, 11 },  // 11 for '/'
    { 7, 8, 9, 12 },  // 12 for '='
    { 14, 0, 15, 13 } // 14 for 'C' (clear), 15 for 'A' (add)
};

// Port Initialization
void PortABE_Init() {
    SYSCTL_RCGC2_R |= 0x0000001B;  // Enable clock for Port A, B, and E

    // Initialize Port E (Rows as output)
    GPIO_PORTE_LOCK_R = 0x4C4F434B;
    GPIO_PORTE_CR_R = 0x0F;  // PE0 - PE3 
    GPIO_PORTE_AMSEL_R = 0x00;
    GPIO_PORTE_PCTL_R = 0x00000000;
    GPIO_PORTE_DIR_R = 0x0F;  // Set PE0-PE3 as output
    GPIO_PORTE_AFSEL_R = 0x00;
    GPIO_PORTE_PUR_R = 0x00;
    GPIO_PORTE_DEN_R = 0x0F;   // Enable digital function

    // Initialize Port A (Columns as input)
    GPIO_PORTA_LOCK_R = 0x4C4F434B;
    GPIO_PORTA_CR_R = 0xF0; // SET PORTS PA4-PA7
    GPIO_PORTA_AMSEL_R = 0x00;
    GPIO_PORTA_PCTL_R = 0x00000000;
    GPIO_PORTA_DIR_R = 0x00; //INPUT 
    GPIO_PORTA_AFSEL_R = 0x00;
    GPIO_PORTA_PUR_R = 0xF0;
    GPIO_PORTA_DEN_R = 0xF0;

    // Initialize Port B (LCD Data Pins)
    GPIO_PORTB_LOCK_R = 0x4C4F434B;
    GPIO_PORTB_CR_R = 0xF7;  // PB0-PB2 Control data & PB4-PB7 for LCD Data
    GPIO_PORTB_AMSEL_R = 0x00;
    GPIO_PORTB_PCTL_R = 0x00000000;
    GPIO_PORTB_DIR_R = 0xF7;  // Set PB0-PB2 & PB4-PB7 as output
    GPIO_PORTB_AFSEL_R = 0x00;
    GPIO_PORTB_PUR_R = 0x00;
    GPIO_PORTB_DEN_R = 0xF7;   // Enable digital function
}

// Detect Key in Column
unsigned char detectKeyInColumn(unsigned char column, int row) {
    if (column == 0xE0) return keymap[row][0]; // Column 0
    if (column == 0xD0) return keymap[row][1]; // Column 1
    if (column == 0xB0) return keymap[row][2]; // Column 2
    if (column == 0x70) return keymap[row][3]; // Column 3
    return 0; // Safety return
}

// Get Key from Keypad
unsigned char getKey(void) {
    GPIO_PORTE_DATA_R = 0x00; // Clear row outputs
    unsigned char column = GPIO_PORTA_DATA_R & 0xF0;

    if (column == 0xF0) {
        return 0; // No key pressed
    }

    while (1) { // Find out which row has been activated
        for (int row = 0; row < ROWS; row++) {
            GPIO_PORTE_DATA_R = ~(1 << row); // Activate one row at a time
            delay_ms(50); // Debounce delay
            column = GPIO_PORTA_DATA_R & 0xF0;
            if (column != 0xF0) {
                return detectKeyInColumn(column, row); // Return detected key
            }
        }
    }
}

// Display Calculation on LCD 
void displayCalculation(void) {
    LCD_Cmd(secondRow); // Move to second row

    if (!waitSecondDigit) {
        sprintf(buffer, "%d", firstDigit); // Display first digit
    } else if (waitSecondDigit && resultDigit == 0) {
        sprintf(buffer, "%d %c %d", firstDigit, operation, secondDigit); // Display operation
    } else {
        sprintf(buffer, "%d %c %d = %d", firstDigit, operation, secondDigit, resultDigit); // Display result
    }

    LCD_string(buffer); // Send the buffer to the LCD

    // Clear the rest of the line
    for (int i = strlen(buffer); i < 16; i++) {
        LCD_write_char(' '); // Clear remaining space
    }
}

// LCD Initialization
void LCD_init(void) {
    SYSCTL_RCGC2_R |= 0x22; // Enable clock for LCD

    // Initialize control pins
    GPIO_PORTB_DATA_R &= ~0x07; // Clear control pins
    LCD_Cmd(0x20);  // Set 5x7 font
    LCD_Cmd(0x28);  // Set 4-bit mode
    LCD_Cmd(0x06);  // Move cursor right
    LCD_Cmd(0x01);  // Clear display
    LCD_Cmd(0x0F);  // Cursor blink
}

// LCD Command Function
void LCD_Cmd(unsigned char cmd) {
    GPIO_PORTB_DATA_R &= ~0x02; // RW = 0
    GPIO_PORTB_DATA_R &= ~0x01; // RS = 0
    LCD_write_nibble(cmd & 0xF0, 0);  // Send high nibble
    LCD_write_nibble(cmd << 4, 0); // Send low nibble

    // If command takes more time
    if (cmd < 4) {
        delay_ms(2);
    } else {
        delay_us(40);
    }
}

// Write Nibble to LCD
void LCD_write_nibble(unsigned char data, unsigned char cmd) {
    data &= 0xF0; // Keep only the upper nibble
    cmd &= 0x0F; // Control bits
    GPIO_PORTB_DATA_R = data | cmd | 0x04;  // Send data and command to LCD
    GPIO_PORTB_DATA_R |= data; // Ensure data is sent
    GPIO_PORTB_DATA_R = 0; // Done writing to LCD
}

// Write Character to LCD
void LCD_write_char(unsigned char data) {
    GPIO_PORTB_DATA_R |= 0x01; // RS = 1 for data
    LCD_write_nibble(data & 0xF0, 0x01);
    LCD_write_nibble(data << 4, 0x01);
    delay_us(40);
}

// Write String to LCD
void LCD_string(char *str) {
    for (int i = 0; str[i] != 0; i++) {
        LCD_write_char(str[i]);
    }
}

// Millisecond Delay
void delay_ms(int n) {
    Timer1A_msd();
    for (int i = 0; i < n; i++) {
        while ((TIMER1_RIS_R & 0x01) == 0); // Wait for TimerA timeout flag
        TIMER1_ICR_R = 0x01; // Clear the TimerA timeout flag
    }
}

// Microsecond Delay
void delay_us(int n) {
    Timer0A_usd();
    for (int i = 0; i < n; i++) {
        while ((TIMER0_RIS_R & 0x01) == 0); // Wait for TimerA timeout flag
        TIMER0_ICR_R = 0x01; // Clear the TimerA timeout flag
    }
}

// Main Function
int main(void) {
    PortABE_Init(); // Initialize ports A, B, and E
    LCD_init(); // Initialize LCD
    Timer0A_usd(); // Initialize Timer0A for microsecond delay
    Timer1A_msd(); // Initialize Timer1A for millisecond delay
    displayCalculation(); // Initial display setup

    // Start Display
    LCD_Cmd(clear_display); // Clear display
    LCD_Cmd(firstRow); // Set cursor to first row
    LCD_string("*Calculator"); // Display title
    delay_ms(750); // Hold for a moment
    LCD_Cmd(clear_display); // Clear display again
    LCD_string("Press Keys"); // Prompt user
    LCD_Cmd(secondRow); // Move to second row
    delay_ms(50); // Short delay

    while (1) {
        unsigned char key = getKey(); // Get key press from keypad

        // Check keypad input
        if (key) {
            if (key <= 9) { // Number keys (0-9)
                if (!waitSecondDigit) {
                    firstDigit = key; // Store first digit
                } else {
                    secondDigit = key; // Store second digit
                }
                displayCalculation(); // Update display with current input
            } else if (key == 10) { // '*' key
                operation = '*'; // Set multiplication operation
                delay_ms(50); // Debounce delay
                waitSecondDigit = 1; // Move to second digit input
                displayCalculation(); // Update display
            } else if (key == 11) { // '/' key
                operation = '/'; // Set division operation
                delay_ms(50); // Debounce delay
                waitSecondDigit = 1; // Move to second digit input
                displayCalculation(); // Update display
            } else if (key == 12) { // '=' key
                // Perform calculation based on the selected operation
                if (operation == '*') {
                    resultDigit = firstDigit * secondDigit; // Multiplication
                } else if (operation == '/') {
                    if (secondDigit != 0) {
                        resultDigit = firstDigit / secondDigit; // Division
                    } else {
                        // Handle division by zero error
                        LCD_Cmd(clear_display); // Clear display
                        LCD_string("Cannot divide"); // Show error message
                        delay_ms(2000); // Display error for 2 seconds

                        // Reset values after displaying error
                        resultDigit = 0; 
                        waitSecondDigit = 0; // Reset to initial state
                        firstDigit = 0;
                        secondDigit = 0;
                        displayCalculation(); // Update display
                        continue; // Skip to next iteration
                    }
                }
                // Show the result
                displayCalculation(); // Update display with result
                delay_ms(2000); // Show result for 2 seconds

                // Reset calculator for the next operation
                waitSecondDigit = 0;
                firstDigit = 0;
                secondDigit = 0;
                resultDigit = 0;
                LCD_Cmd(clear_display); // Clear display before next input
                displayCalculation(); // Update display
            }
        }
    }
}

// Timer Initialization Functions
void Timer0A_usd(void) {
    SYSCTL_RCGCTIMER_R |= 0x01;     /* enable clock to Timer0 */
    TIMER0_CTL_R = 0x00;            /* disable Timer before initialization */
    TIMER0_CFG_R = 0x00;            /* 32-bit option */
    TIMER0_TAMR_R = 0x02;           /* periodic mode and down-counter */
    TIMER0_TAILR_R = 16000 - 1;     /* Timer A interval load value register */
    TIMER0_ICR_R = 0x1;             /* clear the TimerA timeout flag */
    TIMER0_CTL_R |= 0x01;           /* enable Timer A after initialization */
    TIMER0_TAPR_R = 1 - 1;          /* Prescaler value */
}

void Timer1A_msd(void) {
    SYSCTL_RCGCTIMER_R |= 0x02;     /* enable clock to Timer1 */
    TIMER1_CTL_R = 0x00;            /* disable Timer before initialization */
    TIMER1_CFG_R = 0x00;            /* 32-bit option */
    TIMER1_TAMR_R = 0x02;           /* periodic mode and down-counter */
    TIMER1_TAILR_R = 50000 - 1;     /* 10ms interval load value register */
    TIMER1_ICR_R = 0x1;             /* clear the TimerA timeout flag */
    TIMER1_CTL_R |= 0x01;           /* enable Timer A after initialization */
}
