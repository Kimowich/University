#include "RFID_pwm.h"
#use rs232(debugger)

unsigned int8 periode = 16;
unsigned int16 duty = 0;

int array[129] = 0;
int flag = 0;


void main()
{
   int i = 0;
   set_tris_a (0x0f);
   setup_adc_ports(NO_ANALOGS);
   setup_comparator(NC_NC_A1_A2_OUT_ON_A5);
   setup_ccp1(CCP_PWM);

   setup_timer_2(T2_DIV_BY_1,15,1); // (prescale,period,postscale)
   duty = 32;
   set_pwm1_duty(duty);
   enable_interrupts(global);

   while(1)
   {
      if(flag == 1)
      {
         flag = 0;
         for(i=0;i<128;i++)
         array[i] = pulsewidth;
         printf("Array %d = %d",i,array[i]);
      }
      
   }
}
