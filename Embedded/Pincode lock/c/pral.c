#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h> /* interrupt flags e.g. TIMSK */

/* Make boolean more readable */
#define true 1
#define false 0

/* Prototype of used functions */
void initialize(void);
void buttonSense(void);
void toggleLEDs(char LED);
int pressButton(void);
void changeState(void);
void message(char* msg);
void error(char key);
void code(int cone, int ctwo, int cthree, int cfour);
int usart_getchar(char c, FILE *stream);
int usart_putchar(char c, FILE *stream);

/* global variables */
unsigned char bstate=0, pressed=0, unpressed, toggled=0;
unsigned char counter=0, locked=0, pressHandled=0;
unsigned char task='0', select='0', key='0', LED;
unsigned int state=0, one, two, three, four, wait=0, waitz=0;
unsigned int aone=0, atwo=0, athree=0, afour=0;
unsigned int bone=0, btwo=0, bthree=0, bfour=0;
FILE uart_str_in  = FDEV_SETUP_STREAM(usart_getchar, NULL, _FDEV_SETUP_READ);
FILE uart_str_out = FDEV_SETUP_STREAM(usart_putchar, NULL, _FDEV_SETUP_WRITE);

/* Main function */
int main(int argn, char * argv[]) { /* initialize */
    initialize();
	while (true){
		while((UCSRA & (1<<RXC))) {
			key = UDR;
			printf("%c", key);
			changeState();
		}
	}
  return 0;
};

ISR (TIMER0_COMP_vect) { // timer 0 interrupt
    TCNT0 = 0;
	buttonSense();
}

void buttonSense(void) { // Run 50x per second.
    switch (bstate) {
        case 0: // First value
            if (toggled == 0) { // Light LED upon entering function.
                toggled = 1;
                toggleLEDs(0x7F);
            }
            one += pressButton(); // Is button pressed?
            if ((pressed == 0) && (counter > 50) && (one > 0)) {
                bstate++; // If we've vaited long enough with no press.
            }
            break;
        case 1: // Second value
            if (toggled == 1) { // Light LED upon entering function.
                toggled = 0;
                toggleLEDs(0x7F);
            }
            two += pressButton(); // Is button pressed?
            if ((pressed == 0) && (counter > 50) && (two > 0)) {
                bstate++; // If we've vaited long enough with no press.
            }
            break;
        case 2: // Third value
            if (toggled == 0) { // Light LED upon entering function.
                toggled = 1;
                toggleLEDs(0x7F);
            }
            three += pressButton(); // Is button pressed?
            if ((pressed == 0) && (counter > 50) && (three > 0)) {
                bstate++; // If we've vaited long enough with no press.
            }
            break;
                
        case 3: // Fourth value
            if (toggled == 1) { // Light LED upon entering function.
                toggled = 0;
                toggleLEDs(0x7F);
            }
            four += pressButton(); // Is button pressed?
            if ((pressed == 0) && (counter > 50) && (four > 0)) {
                bstate++; // If we've vaited long enough with no press.
            }
            break;
        case 4: // Finishing up
            toggled = 0;
			printf("%d%d%d%d",one,two,three,four);
            if (((one == aone)&&(two == atwo)&&(three == athree)&&(four == afour))||
                ((one == bone)&&(two == btwo)&&(three == bthree)&&(four == bfour))) {
                if (locked == 1 ) {
					locked = 0;
					message("Unlocked!");
					PORTB = 0xFF;
				} else { // was unlocked, locking
					locked = 1;
					message("Locked!");
					PORTB = 0xF0;
				}
            } else {
                message("Code wrong!");
            }	
			bstate = 0;
			one = 0;
			two = 0;
			three = 0;
			four = 0;
        default:
            bstate = 0;
            break;
    }
}

void toggleLEDs(char LED) {
	if ((PORTB & LED) == LED) {
		PORTB &= LED;
		for(wait=0;wait<256;wait++){
			for(waitz=0;waitz<256;waitz++);
		}
		PORTB |= ~LED;
	} else {
		PORTB |= ~LED;
		for(wait=0;wait<256;wait++){
			for(waitz=0;waitz<256;waitz++);
		}
		PORTB &= LED;
	}
}

int pressButton(void) {
    if (((PINA|0xFE) == 0xFE)) { // Button Pressed
        if (pressed<200) { // Guard against stupid users overloading variables.
            pressed++;
        }
        counter = 0;
    } else { // Button not pressed.
        pressed = 0;
        pressHandled = 0;
    }
    if ((pressed > 25) && (pressHandled == 0)) { // Button pressed long enough to count.
        toggleLEDs(0xBF);
        pressHandled++;
        counter = 0;
        return 1; // Return 1 because we need to add to the total for that number.
    }
    counter++; // Counts when to move on to next number.
    return 0;
}

void initialize (void) { /* Configuration setup */
    bstate = 0;
    DDRA = 0x00;
    PORTA = 0x00;
    DDRB = 0xFF;
    PORTB = 0xFF;
    TCCR0 = 0x05; // 5 -- Prescaler = 1024
    /* enable timer 0 interrupts */
    TIMSK = 0x02; // _BV (TOIE0);
    OCR0 = 0x48; //48
    /* enable interrupts */
	UCSRB = (1<<TXEN)|(1<<RXEN);
	UBRRL = 1;
	stdout = &uart_str_out;
	stdin  = &uart_str_in;
    sei();
}

int usart_getchar(char c, FILE *stream) {
  while ((1<<RXC) == 1) {printf("X");} // UART Control Status Register A
  printf("_");
  UDR = c;
  changeState();
  return 0;
}

int usart_putchar(char c, FILE *stream) {
  while ((UCSRA & (1<<UDRE)) == 0) {}
  UDR = c;
  return 0;
}

void message(char* msg) {
  printf("\r\n%s\r\n", msg);
}
void error(char key) {
  printf("\r\nBad: %c\r\n", key);
}

void code(int cone, int ctwo, int cthree, int cfour) {
  printf("\r\nCode is: %i%i%i%i\r\n", cone, ctwo, cthree, cfour);
}

void changeState(void) {
  switch (state) {
    case 0: // g or s
      if (key == 's' || key == 'g')	{
		task = key; // Need to remember what we need to do.
		state++;
	  } else {
	    state = 0;
		error(key);
	  }
      break;
    case 1:
      if (key == 'e') {
	    state++;
	  } else {
		state = 0;
		error(key);
	  }
      break;
    case 2:
      if (key == 't') {
	    state++;
	  } else {
	    state = 0;
		error(key);
	  }
      break;
    case 3:
      if (key == ' ') {
	    state++;
	  } else {
	    state = 0;
		error(key);
	  }
      break;
    case 4:
      if (key == 'a' || key == 'b') {
		select = key; // Need to remember which code we're looking at.
		state++;
	  } else {
	    state = 0;
		error(key);
	  }
      break;
	case 5:
      if (key == ' ') { state = 7; } // Space == setting a code.
	  else if (key == 13) { // Return key
	    if (task == 's') { // Task is set - clear code.
	      if (select == 'a') {
		    aone = 0;
		    atwo = 0;
		    athree = 0;
		    afour = 0;
		    message("Code a is cleared");
		  } else { // select is b
		    bone = 0;
		    btwo = 0;
		    bthree = 0;
		    bfour = 0;
		    message("Code b is cleared");
		  }
		  state = 0;
	    } else { // Task is get - return code.
	      if (select == 'a') {
		    code(aone, atwo, athree, afour);
		  } else { // select is b
		    code(bone, btwo, bthree, bfour);
		  }
		  state = 0;
	    }
      } else {
	    state = 0;
		error(key);
	  }
      break;
	case 7: // First number
	  key = key-48; // De-ASCIIfying
	  if ((key < 10) && (key > 0)) {
	    one = key;
		state++;
	  } else {
	    state = 0;
		error(key+48);
	  }
	  break;
	case 8: // Second number
	  key = key-48; // De-ASCIIfying
	  if ((key < 10) && (key > 0)) {
	    two = key;
		state++;
	  } else {
	    state = 0;
		error(key+48);
	  }
	  break;
	case 9: // Third number
	  key = key-48; // De-ASCIIfying
	  if ((key < 10) && (key > 0)) {
	    three = key;
		state++;
	  } else {
	    state = 0;
		error(key+48);
	  }
	  break;
	case 10: // Fourth number
	  key = key-48; // De-ASCIIfying
	  if ((key < 10) && (key > 0)) {
	    four = key;
		state++;
	  } else {
	    state = 0;
		error(key+48);
	  }
	  break;
	case 11:
	  if (key == 13) { // Enter key
		if (select == 'a') {
		  aone = one;
		  atwo = two;
		  athree = three;
		  afour = four;
		} else  { // Select must be b
		  bone = one;
		  btwo = two;
		  bthree = three;
		  bfour = four;
		}
		state = 0;
		message("Code is loaded");
	  }
	  else {
	    state = 0;
		error(key);
	  }
      break;
    default:
      state = 0;
      break;
  }
}