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

#ifndef MENU_H
#define MENU_H

/* Menu sections */
enum menuS { MENU_ROOT=0, MENU_SET_TIMER, MENU_SELECT_PARAM, MENU_CHANGE_PARAM };
/* Menu events */
enum menuE { MENU_EVENT_PUSH_BUTTON1=0, MENU_EVENT_PUSH_BUTTON2, MENU_EVENT_PUSH_BUTTON3, MENU_EVENT_RELEASE_BUTTON1, MENU_EVENT_RELEASE_BUTTON2, MENU_EVENT_RELEASE_BUTTON3, MENU_EVENT_CHECK_TIMER};

void initMenu();
void refreshMenu();
unsigned char getMenuDisplay();
void feedMenu (unsigned char event);

#endif
