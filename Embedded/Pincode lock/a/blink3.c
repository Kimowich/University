/* filename */
#include <avr/io.h>

#include <avr/interrupt.h> /* interrupt flags e.g. TIMSK */
/* Make boolean more readable */
#define true 1
#define false 0
/* Prototype of used functions */
void initialize (void);
void changeState(void);

/* global variables */
/**
Delay value used by timer 1 interrupt */
unsigned char delay;
/**
Main state of lamp */
unsigned char state;

/**
Main function */
int main(int argn, char * argv[])
{ /* initialize */
  initialize();
  while (true)
    ; /* loop forever */
  return 0;
};

/**
Timer 0 interrupt function */
ISR (TIMER0_OVF_vect)
{ /* timer 0 interrupt */
  changeState();
}

void changeState(void)
{ /* Blink led 0 */
  switch (state)
  {
    case 0: /* lamp is off */
      if (delay >= 40) /* long off */
      { /* turn led 0 on */
        PORTB &= 0xfe;
        delay = 0; /* no initial delay */
        state = 1;
      }
      break;
    case 1: /* lamp is on */
      if (delay >= 3) /* short on */
      { /* turn led 0 off */
        PORTB |= 0x01; 
        delay = 0;
        state = 2;
      }
      break;
    case 2: /* lamp is off */
      if (delay >= 5) /* short off */
      { /* turn led 0 off */
        PORTB &= 0xfe; /* turn led 0 on */
        delay = 0;
        state = 3;
      }
      break;
    case 3: /* lamp is on */
      if (delay >= 9) /* long on */
      { /* turn led 0 off */
        PORTB |= 0x01; 
        delay = 0;
        state = 0; /* back to start */
      }
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
