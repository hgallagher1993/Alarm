/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/***********************************************************************************************
 * Hugh Gallagher
 * 5 - 10 - 2015
 *
 * This program is a basic alarm system controlled and monitored using a serial terminal.
 * The alarm is armed when the password is entered and triggered when the switch (SW1) is pressed
 * or when an incorrect password is entered in the ARMED state, a Red L.E.D flashes while triggered.
 * The password can be changed by entering an ADMIN state.
 *
 * Hugh Gallagher
 * V.01
 * 05 - 10 2015
 *
 * -INIT STATE
 * -ARMED STATE
 * -TRIGGERED STATE
 *
 * Hugh Gallagher
 * V.02
 * 08 - 10 2015
 *
 * New Features
 * - Alarm lockout
 * - Admin state
 *
 **********************************************************************************************/
#include "fsl_device_registers.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "gpio.h"
#include <string.h>

//Function Prototypes
void my_delay();
char char_received();

// Global variables
char password[] = "1234";
char admin[] = "0000";
char user_entry[10];

int main(void)
{
	enum STATES {INIT, ARMED, TRIGGERED, LOCKOUT, ADMIN};
	enum STATES currentState = INIT;
	int count = 0, ii = 0;

    //Initialise the FRDM-KL26Z Board
	hardware_init();
	//Configure LED pins
	FRDM_KL26Z_LEDs_Configure();
	//Configure SW1
	FRDM_KL26Z_SW1_Configure(PULLUP, 0);


    while(1)
    {
    	switch(currentState)
    	{
    		// Enter the password to enter the ARMED state
    		// If the wrong password is entered 3 times the system is locked out
    		// Enter the ADMIN password to enter the ADMIN state and change the password
			case INIT:
				PRINTF("Enter Password To Enable Alarm\n\r");
				SCANF("%s", &user_entry);
				if(strcmp(user_entry, password) == 0)
				{
					PRINTF("Correct Password\n\n\r");
					currentState = ARMED;
					count = 0;
				}
				else if(strcmp(user_entry, admin) == 0)
				{
					currentState = ADMIN;
					count = 0;
				}
				else
				{
					PRINTF("Incorrect password entered\n\n\r");
					count++;
				}

				if(count > 2)
				{
					currentState = LOCKOUT;
					count = 0;
				}
				break;

			// Enter the password to turn off disarm the alarm
			// Alarm is triggered when SW1 is pressed or when a wrong password is entered
			case ARMED:
				PRINTF(". . .ALARM ARMED. . .\n\r");
				PRINTF("Hit any Key and Then Enter The Password to Disarm\n\n\r");

				while(SW1_read() == 1)
				{
					if(char_received())
					{
						SCANF("%s", &user_entry);
						if(strcmp(user_entry, password) == 0)
						{
							PRINTF("Correct Password\n\n\r");
							currentState = INIT;
							break;
						}
						else
						{
							PRINTF("Incorrect password entered\n\n\r");
							break;
						}
					}
				}

				if(currentState == ARMED)
				{
					currentState = TRIGGERED;
					PRINTF(". . .ALARM TRIGGERED. . .\n\n\r");
					PRINTF("Hit any Key and Then Enter The Password to Disarm\n\n\r");
				}
				break;

			// Alarm stays triggered until the correct password is entered
			// A Red LED flashed every half second while triggered
			case TRIGGERED:
				LED_set(RED, TOGGLE);
				my_delay();

				if(char_received())
				{
					SCANF("%s", &user_entry);
					if(strcmp(user_entry, password) == 0)
					{
						PRINTF("Correct Password\n\n\r");
						PRINTF(". . .Alarm is Disabled. . .\n\n\r");
						currentState = INIT;
						LED_set(RED, OFF);
						break;
					}
					else
					{
						PRINTF("Incorrect password entered\n\n\r");
					}
				}
				break;

			// System is locked out for 30 seconds and then returns to the INIT state
			case LOCKOUT:
				PRINTF("Incorrect password entered 3 times\n\n\r");
				PRINTF(". . .SYSTEM LOCKED OUT. . .\n\n\r");

				for(ii = 0; ii < 25; ii++)
				{
					my_delay();
				}

				currentState = INIT;
				break;

			// Original password is changed here
			case ADMIN:
				PRINTF(". . . ADMIN STATE. . .");
				PRINTF("Enter new password\n\n\r");
				SCANF("%s", &user_entry);

				strcpy(password, user_entry);

				currentState = INIT;
				break;

			default:
				currentState = INIT;
				break;
    	}
    }
    /* Never leave main */
    return 0;
}

//Function to determine if the UART has received a character
//Returns a 1 if the UART receive buffer contains a character
char char_received()
{
	if(UART0_S1 & 0x20)
		return 1;
	else
		return 0;
}

//Delay function
void my_delay()
{
	long int xx;
	for(xx = 0; xx < 2000000; xx++);
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
