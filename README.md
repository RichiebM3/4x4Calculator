# 4x4Calculator
# 4x4 Keypad Calculator

## Overview
This project implements a simple calculator using a 4x4 keypad and a 16x2 LCD screen. The calculator can perform basic arithmetic operations (addition, multiplication, and division) and displays the results on the LCD.

## Author
**Richard A Bruce**

## Components
- **Microcontroller:** TM4C123GH6PM
- **Keypad:** 4x4 matrix keypad
- **Display:** 16x2 LCD
- **Programming Language:** C

## Features
- Basic arithmetic operations: addition, multiplication, and division.
- User-friendly interface with an LCD display.
- Debounce handling for keypad inputs.
- Error handling for division by zero.

## Circuit Diagram
*(Include a circuit diagram here if available)*

## Code Structure
The code is organized into several key functions:

- **LCD Functions:** Initialize and control the LCD display.
- **Keypad Functions:** Detect key presses and map them to corresponding values.
- **Calculation Functions:** Handle arithmetic operations and display results.

### Key Functions
- `LCD_init()`: Initializes the LCD display.
- `getKey()`: Reads input from the keypad.
- `displayCalculation()`: Updates the LCD with the current calculation.
- `main()`: The main program loop that handles user input and performs calculations.

## Setup Instructions
1. **Hardware Setup:**
   - Connect the 4x4 keypad to the appropriate GPIO pins of the TM4C123GH6PM.
   - Connect the 16x2 LCD to the microcontroller as per the defined pin mappings in the code.

2. **Software Setup:**
   - Make sure to have the appropriate development environment set up for programming the TM4C123GH6PM (e.g., Keil, Code Composer Studio).
   - Copy the provided code into your development environment.

3. **Compiling and Uploading:**
   - Compile the code and upload it to the microcontroller using the appropriate programmer.

## Usage Instructions
1. Power on the calculator.
2. The LCD will display "*Calculator*" followed by "Press Keys".
3. Use the keypad to enter numbers and select operations:
   - Press numbers (0-9) to input values.
   - Press '*' for multiplication or '/' for division.
   - Press '=' to calculate the result.
   - Press 'C' to clear the current input.

4. The result will be displayed for 2 seconds before resetting for the next calculation.

## Error Handling
- If division by zero is attempted, the display will show "Cannot divide" for 2 seconds before resetting.

## Conclusion
This 4x4 keypad calculator is a simple yet effective project to demonstrate interfacing with a keypad and an LCD using the TM4C123GH6PM microcontroller. It serves as a great introduction to embedded systems programming and hardware interfacing.

## License
This project is licensed under the MIT License. See the LICENSE file for details.
