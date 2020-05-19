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
 * Control functions for relay.
 */

#include "relay.h"
#include "stm8s003/gpio.h"
//#include "adc.h"
#include "params.h"

#define RELAY_PORT              PA_ODR
#define RELAY_BIT               0x04
#define RELAY_TIMER_MULTIPLIER  1

static unsigned int timer;
static bool state;

/**
 * @brief Configure appropriate bits for GPIO port A, reset local timer
 *  and reset state.
 */
void initRelay()
{
    PA_DDR |= RELAY_BIT;
    PA_CR1 |= RELAY_BIT;
    timer = 0;
    state = false;
}

/**
 * @brief Sets state of the relay.
 * @param on - true, off - false
 */
void setRelay (bool on)
{
    if (on) {
        RELAY_PORT |= RELAY_BIT;
    } else {
        RELAY_PORT &= ~RELAY_BIT;
    }

}

/**
 * @brief Change state of the relay.
 * @param on - true, off - false
 */
void changeRelayState()
{
  state = !state;
}

void resetRelayTimer ()
{
   timer = 0;
}

bool getRelayState()
{
  if (state)
    return true;
  else
    return false;
}

unsigned int getResidualTime()
{
    return PARAM_RELAY_TIMER-(timer>>RELAY_TIMER_MULTIPLIER); 
}

/**
 * @brief This function is being called during timer's interrupt
 *  request so keep it extremely small and fast.
 */
void refreshRelay()
{  
  bool mode = getParamById (PARAM_RELAY_MODE);

  if (state) { // Relay state is enabled 
    if (mode) //Timer mode
      {
	timer++;
	if ((getParamById (PARAM_RELAY_TIMER) << RELAY_TIMER_MULTIPLIER) < timer)  
	  {
	    state=!state;
	    timer=0;
	  }
      }
  }
  setRelay(state);
}
