/*
 * This file is part of the W1209 firmware replacement project
 * (https://github.com/mister-grumbler/w1209-firmware).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * Control functions for the seven-segment display (SSD).
 */

#include "display.h"
#include "stm8s003/gpio.h"

/* Definitions for display */
// Port A controls segments: B, F
// 0000 0110
#define SSD_SEG_BF_PORT     PD_ODR
#define SSD_BF_PORT_MASK    0b01001000
// Port A controls segments: C, G
// 1100 0000
#define SSD_SEG_CG_PORT     PA_ODR
#define SSD_CG_PORT_MASK    0b00001010
// Port D controls segments: A, E, D, P
// 0010 1110
#define SSD_SEG_AEDP_PORT   PC_ODR
#define SSD_AEDP_PORT_MASK  0b10111000

// PC.7
#define SSD_SEG_A_BIT       0x80
// PD.6
#define SSD_SEG_B_BIT       0x40
// PA.1
#define SSD_SEG_C_BIT       0x02
// PC.5
#define SSD_SEG_D_BIT       0x20
// PC.4
#define SSD_SEG_E_BIT       0x10
// PD.3
#define SSD_SEG_F_BIT       0x08
// PA.3
#define SSD_SEG_G_BIT       0x08
// PC.3
#define SSD_SEG_P_BIT       0x08

// Port B controls digits: 1
#define SSD_DIGIT_1_PORT   PC_ODR
// Port D controls digit: 2,3
#define SSD_DIGIT_23_PORT    PD_ODR

// PC.6
#define SSD_DIGIT_1_BIT     0x40
// PD.5
#define SSD_DIGIT_2_BIT     0x20
// PD.4
#define SSD_DIGIT_3_BIT     0x10

const unsigned char Hex2CharMap[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A',
                                     'B', 'C', 'D', 'E', 'F'
                                    };

static unsigned char activeDigitId;
static unsigned char displayA[3];
static unsigned char displayC[3];
static unsigned char displayD[3];

static void enableDigit (unsigned char);
static void setDigit (unsigned char, unsigned char, bool);

static bool displayOff;
static bool testMode;

/**
 * @brief Configure appropriate bits for GPIO ports, initialize static
 *  variables and set test mode for display.
 */
void initDisplay()
{
    PD_DDR |= SSD_SEG_B_BIT | SSD_SEG_F_BIT | SSD_DIGIT_2_BIT | SSD_DIGIT_3_BIT;
    PD_CR1 |= SSD_SEG_B_BIT | SSD_SEG_F_BIT | SSD_DIGIT_2_BIT | SSD_DIGIT_3_BIT;

    PA_DDR |= SSD_SEG_C_BIT | SSD_SEG_G_BIT;
    PA_CR1 |= SSD_SEG_C_BIT | SSD_SEG_G_BIT;

    PC_DDR |= SSD_SEG_A_BIT | SSD_SEG_D_BIT | SSD_SEG_E_BIT | SSD_SEG_P_BIT | SSD_DIGIT_1_BIT;
    PC_CR1 |= SSD_SEG_A_BIT | SSD_SEG_D_BIT | SSD_SEG_E_BIT | SSD_SEG_P_BIT | SSD_DIGIT_1_BIT;
    displayOff = false;
    activeDigitId = 0;
    setDisplayTestMode (true, "");
}

/**
 * @brief This function is being called during timer's interrupt
 *  request so keep it extremely small and fast. During this call
 *  the data from display's buffer being used to drive appropriate
 *  GPIO pins of microcontroller.
 */
void refreshDisplay()
{
    enableDigit (3);

    if (displayOff) {
        return;
    }

    SSD_SEG_BF_PORT &= ~SSD_BF_PORT_MASK;
    SSD_SEG_BF_PORT |= displayD[activeDigitId] & SSD_BF_PORT_MASK;
    SSD_SEG_CG_PORT &= ~SSD_CG_PORT_MASK;
    SSD_SEG_CG_PORT |= displayA[activeDigitId] & SSD_CG_PORT_MASK;
    SSD_SEG_AEDP_PORT &= ~SSD_AEDP_PORT_MASK;
    SSD_SEG_AEDP_PORT |= displayC[activeDigitId] & SSD_AEDP_PORT_MASK;
    enableDigit (activeDigitId);

    if (activeDigitId > 1) {
        activeDigitId = 0;
    } else {
        activeDigitId++;
    }
}

/**
 * @brief Enables/disables a test mode of SSDisplay. While in this mode
 *  the test message will be displayed and any attempts to update
 *  display's buffer will be ignored.
 * @param val
 *  value to be set: true - enable test mode, false - disable test mode.
 */
void setDisplayTestMode (bool val, char* str)
{
  testMode = val;
    if (!testMode) {
        if (*str == 0) {
            setDisplayStr ("888");
        } else {
            setDisplayStr (str);
        }
    }


}

/**
 * @brief Enable/disable display.
 * @param val
 *  value to be set: true - display off, false - display on.
 */
void setDisplayOff (bool val)
{
    displayOff = val;
}

/**
 * @brief Sets dot in the buffer of display at position pointed by id
 *  to the state defined by val.
 * @param id
 *  identifier of digit 0..2
 * @param val
 *  state of dot to be set: true - enable, false - disable.
 */
void setDisplayDot (unsigned char id, bool val)
{
    if (val) {
        displayC[id] |= SSD_SEG_P_BIT;
    } else {
        displayC[id] &= ~SSD_SEG_P_BIT;
    }
}

/**
 * @brief Sets symbols of given null-terminated string into display's buffer.
 * @param val
 *  pointer to the null-terminated string.
 */
void setDisplayStr (const unsigned char* val)
{
    unsigned char i, d;

    // get number of display digit(s) required to show given string.
    for (i = 0, d = 0; * (val + i) != 0; i++, d++) {
        if (* (val + i) == '.' && i > 0 && * (val + i - 1) != '.') d--;
    }

    // at this point d = required digits
    // but SSD have 3 digits only. So rest is doesn't matters.
    if (d > 3) {
        d = 3;
    }

    // disable the digit if it is not needed.
    for (i = 3 - d; i > 0; i--) {
        setDigit (3 - i, ' ', false);
    }

    // set values for digits.
    for (i = 0; d != 0 && *val + i != 0; i++, d--) {
        if (* (val + i + 1) == '.') {
            setDigit (d - 1, * (val + i), true);
            i++;
        } else {
            setDigit (d - 1, * (val + i), false);
        }
    }
}

/**
 * @brief
 * Enable the digit with given ID on SSD and rest of digits are disabled.
 *
 * @param id
 * The ID = 0 corresponds to the right most digit on the display.
 * Accepted values are: 0, 1, 2, any other value will disable display.
 */
static void enableDigit (unsigned char id)
{
    switch (id) {
    case 2:
        SSD_DIGIT_23_PORT &= ~SSD_DIGIT_3_BIT;
        SSD_DIGIT_23_PORT |= SSD_DIGIT_2_BIT;
        SSD_DIGIT_1_PORT |= SSD_DIGIT_1_BIT;
        break;

    case 1:
        SSD_DIGIT_23_PORT &= ~SSD_DIGIT_2_BIT;
        SSD_DIGIT_23_PORT |= SSD_DIGIT_3_BIT;
        SSD_DIGIT_1_PORT |= SSD_DIGIT_1_BIT;
        break;

    case 0:
        SSD_DIGIT_1_PORT &= ~SSD_DIGIT_1_BIT;
        SSD_DIGIT_23_PORT |= SSD_DIGIT_3_BIT | SSD_DIGIT_2_BIT;
        break;

    default:
        SSD_DIGIT_23_PORT |= SSD_DIGIT_3_BIT | SSD_DIGIT_2_BIT;
        SSD_DIGIT_1_PORT |= SSD_DIGIT_1_BIT;
        break;
    }
}

/**
 * @brief Sets bits within display's buffer appropriate to given value.
 *  So this symbol will be shown on display during refreshDisplay() call.
 *  When test mode is enabled the display's buffer will not be updated.
 *
 * The list of segments as they located on display:
 *  _2_       _1_       _0_
 *  <A>       <A>       <A>
 * F   B     F   B     F   B
 *  <G>       <G>       <G>
 * E   C     E   C     E   C
 *  <D> (P)   <D> (P)   <D> (P)
 *
 * @param id
 *  Identifier of character's position on display.
 *  Accepted values are: 0, 1, 2.
 * @param val
 *  Character to be represented on SSD at position being designated by id.
 *  Due to limited capabilities of SSD some characters are shown in a very
 *  schematic manner.
 *  Accepted values are: ANY.
 *  But only actual characters are defined. For the rest of values the
 *  '_' symbol is shown.
 * @param dot
 *  Enable dot (decimal point) for the character.
 *  Accepted values true/false.
 *
 */
static void setDigit (unsigned char id, unsigned char val, bool dot)
{

    if (id > 2) return;

    if (testMode) return;
    
    switch (val) {
    case '-':
      displayA[id] = SSD_SEG_G_BIT;
      displayC[id] = 0;
      displayD[id] = 0;
      break;
      
    case ' ':
      displayA[id] = 0;
      displayC[id] = 0;
      displayD[id] = 0;
      break;
      
    case '0':
      displayA[id] = SSD_SEG_C_BIT;
      displayD[id] = SSD_SEG_B_BIT | SSD_SEG_F_BIT ;
      displayC[id] = SSD_SEG_A_BIT | SSD_SEG_D_BIT | SSD_SEG_E_BIT;
      break;
      
    case '1':
      displayD[id] = SSD_SEG_B_BIT;
      displayA[id] = SSD_SEG_C_BIT;
      displayC[id] = 0;
      break;
      
    case '2':
      displayD[id] = SSD_SEG_B_BIT ;
      displayA[id] = SSD_SEG_G_BIT ;
      displayC[id] = SSD_SEG_A_BIT | SSD_SEG_D_BIT | SSD_SEG_E_BIT;
      break;
      
    case '3':
      displayD[id] = SSD_SEG_B_BIT; 
      displayA[id] = SSD_SEG_C_BIT | SSD_SEG_G_BIT;
      displayC[id] = SSD_SEG_A_BIT | SSD_SEG_D_BIT;
      break;

    case '4':
      displayD[id] = SSD_SEG_B_BIT | SSD_SEG_F_BIT ;
      displayA[id] = SSD_SEG_C_BIT | SSD_SEG_G_BIT;
      displayC[id] = 0;
      break;

    case '5':
        displayD[id] = SSD_SEG_F_BIT ;
        displayA[id] = SSD_SEG_C_BIT | SSD_SEG_G_BIT;
        displayC[id] = SSD_SEG_A_BIT | SSD_SEG_D_BIT;
        break;

    case '6':
        displayD[id] = SSD_SEG_F_BIT ;
        displayA[id] = SSD_SEG_C_BIT | SSD_SEG_G_BIT;
        displayC[id] = SSD_SEG_A_BIT | SSD_SEG_D_BIT | SSD_SEG_E_BIT;
        break;

    case '7':
        displayD[id] = SSD_SEG_B_BIT ;
        displayA[id] = SSD_SEG_C_BIT;
        displayC[id] = SSD_SEG_A_BIT;
        break;

    case '8':
        displayD[id] = SSD_SEG_B_BIT | SSD_SEG_F_BIT;
        displayA[id] = SSD_SEG_C_BIT | SSD_SEG_G_BIT;
        displayC[id] = SSD_SEG_A_BIT | SSD_SEG_D_BIT | SSD_SEG_E_BIT;
        break;

    case '9':
        displayD[id] = SSD_SEG_B_BIT | SSD_SEG_F_BIT;
        displayA[id] = SSD_SEG_C_BIT |SSD_SEG_G_BIT;
        displayC[id] = SSD_SEG_A_BIT | SSD_SEG_D_BIT;
        break;

    case 'A':
        displayD[id] = SSD_SEG_B_BIT | SSD_SEG_F_BIT;
        displayA[id] = SSD_SEG_C_BIT | SSD_SEG_G_BIT;
        displayC[id] = SSD_SEG_A_BIT | SSD_SEG_E_BIT;
        break;

    case 'B':
        displayD[id] = SSD_SEG_F_BIT;
        displayA[id] = SSD_SEG_C_BIT | SSD_SEG_G_BIT;
        displayC[id] = SSD_SEG_D_BIT | SSD_SEG_E_BIT;
        break;

    case 'C':
        displayD[id] = SSD_SEG_F_BIT;
        displayA[id] = 0;
        displayC[id] = SSD_SEG_A_BIT | SSD_SEG_D_BIT | SSD_SEG_E_BIT;
        break;

    case 'D':
        displayD[id] = SSD_SEG_B_BIT ;
        displayA[id] = SSD_SEG_C_BIT | SSD_SEG_G_BIT;
        displayC[id] = SSD_SEG_D_BIT | SSD_SEG_E_BIT;
        break;

    case 'E':
        displayD[id] = SSD_SEG_F_BIT ;
        displayA[id] = SSD_SEG_G_BIT;
        displayC[id] = SSD_SEG_A_BIT | SSD_SEG_D_BIT | SSD_SEG_E_BIT;
        break;

    case 'F':
        displayD[id] = SSD_SEG_F_BIT;
        displayA[id] = SSD_SEG_G_BIT;
        displayC[id] = SSD_SEG_A_BIT | SSD_SEG_E_BIT;
        break;

    case 'H':
        displayD[id] = SSD_SEG_B_BIT | SSD_SEG_F_BIT;
        displayA[id] = SSD_SEG_C_BIT | SSD_SEG_G_BIT;
        displayC[id] = SSD_SEG_E_BIT;
        break;

    case 'L':
        displayD[id] = SSD_SEG_F_BIT;
        displayA[id] = 0;
        displayC[id] = SSD_SEG_D_BIT | SSD_SEG_E_BIT;
        break;

    case 'N':
        displayD[id] = SSD_SEG_B_BIT | SSD_SEG_F_BIT ;
        displayA[id] = SSD_SEG_C_BIT;
        displayC[id] = SSD_SEG_A_BIT | SSD_SEG_E_BIT;
        break;

    case 'O':
        displayD[id] = SSD_SEG_B_BIT | SSD_SEG_F_BIT;
        displayA[id] = SSD_SEG_C_BIT;
        displayC[id] = SSD_SEG_A_BIT | SSD_SEG_D_BIT | SSD_SEG_E_BIT;
        break;

    case 'P':
        displayD[id] = SSD_SEG_B_BIT | SSD_SEG_F_BIT;
        displayA[id] = SSD_SEG_G_BIT;
        displayC[id] = SSD_SEG_A_BIT | SSD_SEG_E_BIT;
        break;

    case 'R':
        displayD[id] = SSD_SEG_F_BIT;
        displayA[id] = 0;
        displayC[id] = SSD_SEG_A_BIT | SSD_SEG_E_BIT;
        break;

    default:
        displayD[id] = 0;
        displayA[id] = 0;
        displayC[id] = SSD_SEG_D_BIT;
    }

    if (dot) {
        displayC[id] |= SSD_SEG_P_BIT;
    } else {
        displayC[id] &= ~SSD_SEG_P_BIT;
    }

    return;
}
