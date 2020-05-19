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
 * Implementation of application menu.
 */

#include "menu.h"
#include "buttons.h"
#include "display.h"
#include "relay.h"
#include "params.h"
#include "timer.h"

#define MENU_1_SEC_PASSED   32
#define MENU_3_SEC_PASSED   MENU_1_SEC_PASSED * 3
#define MENU_10_SEC_PASSED   MENU_1_SEC_PASSED * 10
#define MENU_AUTOINC_DELAY  MENU_1_SEC_PASSED / 8

static unsigned char menuDisplay;
static unsigned char menuState;
static unsigned int timer;

/**
 * @brief Initialization of local variables.
 */
void initMenu()
{
    timer = 0;
    menuState = menuDisplay = MENU_ROOT;
}

/**
 * @brief Gets menu state for displaying appropriate value on the SSD.
 * @return
 */
unsigned char getMenuDisplay()
{
    return menuDisplay;
}

/**
 * @brief Updating state of application's menu and displaying info when new
 *  event is received. Possible states of menu and displaying are:
 *  MENU_ROOT
 *  MENU_SELECT_PARAM
 *  MENU_CHANGE_PARAM
 *  MENU_SET_TIMER
 *
 * @param event is one of:
 *  MENU_EVENT_PUSH_BUTTON1
 *  MENU_EVENT_PUSH_BUTTON2
 *  MENU_EVENT_PUSH_BUTTON3
 *  MENU_EVENT_RELEASE_BUTTON1
 *  MENU_EVENT_RELEASE_BUTTON2
 *  MENU_EVENT_RELEASE_BUTTON3
 *  MENU_EVENT_CHECK_TIMER
 */
void feedMenu (unsigned char event)
{
    if (menuState == MENU_ROOT) {
        switch (event) {
        case MENU_EVENT_PUSH_BUTTON1:
            timer = 0;
	    /* menuDisplay = MENU_SET_TIMER;  */
            break;

        case MENU_EVENT_RELEASE_BUTTON1:
            if (timer < MENU_3_SEC_PASSED) {
	      changeRelayState();
            }
            timer = 0;
            break;

        case MENU_EVENT_PUSH_BUTTON2:
            timer = 0;
	    /* menuDisplay = MENU_SET_TIMER;  */
            break;

        case MENU_EVENT_RELEASE_BUTTON2:
            if (timer < MENU_3_SEC_PASSED) {
	      resetRelayTimer();
            }
            timer = 0;
            break;
	    
        case MENU_EVENT_CHECK_TIMER:
            if (getButton1() ) {
                if (timer > MENU_3_SEC_PASSED) {
                    setParamId (0);
                    timer = 0;
                    menuState = menuDisplay = MENU_SELECT_PARAM;
                }
            }

            break;

        default:
            if (timer > MENU_10_SEC_PASSED) {
                timer = 0;
                menuState = menuDisplay = MENU_ROOT;
            }

            break;
        }
    } else if (menuState == MENU_SELECT_PARAM) {
        switch (event) {
	case MENU_EVENT_PUSH_BUTTON1:
	  timer = 0;
	  /* menuDisplay = MENU_SET_TIMER;  */
	  break;

        case MENU_EVENT_RELEASE_BUTTON1:
	  if (timer < MENU_3_SEC_PASSED) 
	    incParamId();
	  timer = 0;
	  break;

        case MENU_EVENT_PUSH_BUTTON2:
	  decParamId();
	    
        case MENU_EVENT_RELEASE_BUTTON2:
	  timer = 0;
	  break;

        case MENU_EVENT_CHECK_TIMER:
	  if (getButton1() && timer > MENU_3_SEC_PASSED) {
	      timer = 0;
	      menuState = menuDisplay = MENU_CHANGE_PARAM;
	      break;
	  }

	  
	  if (timer > MENU_10_SEC_PASSED) {
	    timer = 0;
	    setParamId (0);
	    storeParams();
	    menuState = menuDisplay = MENU_ROOT;
	    break;
	  }
	  break;
	  
	default:
	  break;
	}
    } else if (menuState == MENU_CHANGE_PARAM) {
        switch (event) {
	case MENU_EVENT_PUSH_BUTTON1:
	  timer = 0;
	  /* menuDisplay = MENU_SET_TIMER;  */
	  break;

        case MENU_EVENT_RELEASE_BUTTON1:
	  if (timer < MENU_3_SEC_PASSED) 
	    incParam();
	  timer = 0;
	  break;

        case MENU_EVENT_PUSH_BUTTON2:
            decParam();

        case MENU_EVENT_RELEASE_BUTTON2:
            timer = 0;
            break;

        case MENU_EVENT_CHECK_TIMER:
            if (getButton1() && timer > MENU_3_SEC_PASSED) {
                timer = 0;
                menuState = menuDisplay = MENU_SELECT_PARAM;
                break;
            }

            if (timer > MENU_10_SEC_PASSED) {
                timer = 0;
                storeParams();
                menuState = menuDisplay = MENU_ROOT;
		break;
            }

            break;

        default:
            break;
        }
    }
}

/**
 * @brief This function is being called during timer's interrupt
 *  request so keep it extremely small and fast.
 *  During this call all time-related functionality of application
 *  menu is handled. For example: fast value change while holding
 *  a button, return to root menu when no action is received from
 *  user within a given time.
 */
void refreshMenu()
{
    timer++;
    feedMenu (MENU_EVENT_CHECK_TIMER);
}
