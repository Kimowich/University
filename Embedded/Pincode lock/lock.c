#include <avr/io.h>

#include <avr/interrupt.h> /* interrupt flags e.g. TIMSK */

#define true 1
#define false 0

void initialize (void);
void changeState(void);

/* global variables */
unsigned char state, task, select;
unsigned int one, two, three, four;

int main(int argn, char * argv[]) { /* initialize */
  initialize();
  while (true);
  return 0;
};

void changeState(void) {
  switch (state) {
    case 0: // g or s
      if (key == 's' || key == 'g')	{
		task = key;
		state = 1;
	  } else { state = 0; }
      break;
    case 1:
      if (key == 'e')	{ state = 2; }
	  else				{ state = 0; }
      break;
    case 2:
      if (key == 't')	{ state = 3; }
	  else				{ state = 0; }
      break;
    case 3:
      if (key == ' ')	{ state = 4; }
	  else				{ state = 0; }
      break;
    case 4:
      if (key == 'a' || key == 'b')	{
		select = key;
		state = 5;
	  } else { state = 0; }
      break;
	case 5:
      if      (key == ' ')	{ state = 7; }
	  else if (key == 13)	{ state = 6; } // return
	  else					{ state = 0; }
      break;
	case 6:
										// Return stuff
	  break;
	case 7: // First number
	  key = key-48; // De-ASCIIfying
	  if (key < 10) {
	    one = key;
		state = 8
	  } else { state = 0; }
	  break;
	case 8: // Second number
	  key = key-48; // De-ASCIIfying
	  if (key < 10) {
	    two = key;
		state = 9
	  } else { state = 0; }
	  break;
	case 9: // Third number
	  key = key-48; // De-ASCIIfying
	  if (key < 10) {
	    three = key;
		state = 10
	  } else { state = 0; }
	  break;
	case 10: // Fourth number
	  key = key-48; // De-ASCIIfying
	  if (key < 10) {
	    four = key;
		state = 11
	  } else { state = 0; }
	  break;
	case 11:
	  if (key == 13)	{
										// FINISH STUFF
	  }
	  else { state = 0; }
      break;
    default:
      state = 0;
      break;
  }
  delay++; /* advance time */
}

void initialize (void) 
{ /* Configuration setup */
  delay = 0;
  state = 1; /* start in state 1 */
  /* Set direction of port B */
  DDRB = 0xFF;
  PORTB = 0xfe; /* turn lamp 0 on only */
  /* set timer 0 (tmr0) prescaler 1024 */
  //TCCR0 = _BV (CS02) | _BV (CS00);
  TCCR0 = 0x05; // Prescaler = 1024
  //TCCR0 = _BV (CS00); /* prescale 1 for debug */
  /* enable timer 0 interrupts */
  TIMSK = _BV (TOIE0);
  /* enable interrupts */
  sei ();
}
