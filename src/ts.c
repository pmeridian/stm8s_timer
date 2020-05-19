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

//#include "adc.h"
#include "buttons.h"
#include "display.h"
#include "menu.h"
#include "params.h"
#include "relay.h"
#include "timer.h"

#define INTERRUPT_ENABLE    __asm rim __endasm;
#define INTERRUPT_DISABLE   __asm sim __endasm;
#define WAIT_FOR_INTERRUPT  __asm wfi __endasm;

/**
 * @brief
 */
int main()
{
    static unsigned char* stringBuffer[7];
    unsigned char paramMsg[] = {'P', '0', 0};

    initMenu();  
    initButtons();    
    initParamsEEPROM();   
    resetParamsEEPROM();  
    initDisplay(); 
    /* initADC();   */
    initRelay(); 
    initTimer();

    /* setRelay(true); */
    
    INTERRUPT_ENABLE
    // Loop
    while (true) {
      if (getUptimeSeconds() > 0) {
            setDisplayTestMode (false, "");
        }

      if (getMenuDisplay() == MENU_ROOT) {
	if (getParamById(PARAM_RELAY_MODE) == 0) {
	  if (getRelayState())
	    setDisplayStr ("ON");
	  else
	    setDisplayStr ("OFF");
	} else {
	  itofpa ( getResidualTime(), (char*) stringBuffer, 0);
	  setDisplayStr ( (char*) stringBuffer);
	}
      } else if (getMenuDisplay() == MENU_SET_TIMER) {
	paramToString (PARAM_RELAY_TIMER, (char*) stringBuffer);
	setDisplayStr ( (char*) stringBuffer);
      } else if (getMenuDisplay() == MENU_SELECT_PARAM) {
	paramMsg[1] = '0' + getParamId();
	setDisplayStr ( (unsigned char*) &paramMsg);
      } else if (getMenuDisplay() == MENU_CHANGE_PARAM) {
	paramToString (getParamId(), (char*) stringBuffer);
	setDisplayStr ( (char *) stringBuffer);
      } else {
	setDisplayStr ("ERR");
	setDisplayOff ( (bool) (getUptime() & 0x40) );
	}


      WAIT_FOR_INTERRUPT
    };

}
