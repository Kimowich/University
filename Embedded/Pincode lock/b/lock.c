#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h> /* interrupt flags e.g. TIMSK */

#define true 1
#define false 0

void initialize (void);
void changeState(void);
void message(char* msg);
void error(char key);
void code(int cone, int ctwo, int cthree, int cfour);
int usart_getchar(char c, FILE *stream);
int usart_putchar(char c, FILE *stream);

/* global variables */
unsigned char task='0', select='0', key='0';
unsigned int state=0, one, two, three, four;
unsigned int aone=0, atwo=0, athree=0, afour=0;
unsigned int bone=0, btwo=0, bthree=0, bfour=0;
FILE uart_str_in  = FDEV_SETUP_STREAM(usart_getchar, NULL, _FDEV_SETUP_READ);
FILE uart_str_out = FDEV_SETUP_STREAM(usart_putchar, NULL, _FDEV_SETUP_WRITE);

int main(int argn, char * argv[]) { /* initialize */
  UCSRB = (1<<TXEN)|(1<<RXEN);
  UBRRL = 1;
  stdout = &uart_str_out;
  stdin  = &uart_str_in;
  message("Ready...");
  while (true){
    while((UCSRA & (1<<RXC))) {
	  key = UDR;
	  printf("%c", key);
	  changeState();
	}
  }
  return 0;
};

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