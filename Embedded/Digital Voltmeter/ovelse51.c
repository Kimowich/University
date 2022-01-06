#include "ovelse51.h"
#include "lcd16216.c"
#include "internal_eeprom.c"
#use rs232 (debugger)

#define SCK PIN_C3 //Ben 18, forbindes til CLK (ben 7) på ADC’en
#define SDO PIN_C5 //Ben 24, forbindes til DIN (ben 5) på ADC’en.
#define SDI PIN_C4 //Ben 23, forbindes til DOUT (ben 6) på ADC’en
#define CS PIN_A5 //Ben 7, forbindes til CS/SHDN (ben1) på ADC’en

//Herunder deffineres de variable der anvendes i programmet
unsigned int16 in_data = 0;
int8 MSB_value=0;
int8 LSB_value=0;
int i;
char buf[16];
int int_flag=0;
int16 tot_data;
int16 avg_data;
float calibrate_fac = 0;
float calibrate_fac_main = 1;
int read_flag = 0;
int calc_flag = 0;


//Funktionerne deffineres
float read_float_eeprom(int16 address);
void write_float_eeprom(int16 address, float data);                 
void adc_read();
void calibrate();

//interruptet oprettes
#int_ext   
void calibrate_int()
{
/*interrupts slås fra for at sikre der ikke interruptes imens interruptet er i gang
dette kunne forekomme ved bounce fra knap/kontakt*/
   disable_interrupts(INT_EXT);
   
   if(int_flag != 0) calc_flag = 1; //if sætningen sikre at mikrokontrolleren ikke kalibrere sig selv ved startup
                                    //calc_flag sættes til 1 så calculate funktionen kaldes i programmet
   
   int_flag = 1;                    //sætter interrupt flaget til 1 igen
   enable_interrupts(INT_EXT);      //interrupts enables igne
}
   
void main()
{

   setup_adc_ports(NO_ANALOGS|VSS_VDD);
   setup_adc(ADC_OFF);
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1);
   setup_timer_1(T1_DISABLED);
   setup_timer_2(T2_DISABLED,0,1);
   setup_comparator(NC_NC_NC_NC);// This device COMP currently not supported by the PICWizard
   //Setup_Oscillator parameter not selected from Intr Oscillator Config tab
   setup_spi ( SPI_MASTER | SPI_L_TO_H | SPI_CLK_DIV_16 ); //initialisering af SPI forbinnelsen
   
   
   
   //interrupts, trigger_egdes og LCD initialiseres
   setup_oscillator(OSC_8MHz, 0);
   port_b_pullups (TRUE);        
   enable_interrupts(global);    
   enable_interrupts(int_ext);   
   ext_int_edge( H_TO_L );       
   int_flag = 0;
   read_flag = 0;
   lcd_init();
   lcd_clear();
   
   while(1)
   {
      delay_ms(500);           
      if(calc_flag == 1) calibrate();        //calibrate kaldes hvis interruptet har været kaldt
      calc_flag = 0;                         //flaget sættes lav til næste interrupt kald
      
      
      
      /*If sætning bestemmer om der er brug for kalibrering ved at  se om eeprom'en er tom
        hvis dette er tilfældet vil "read_flag" sættes til 0 og næste if sætning vil anmode
        om en kalibereing*/
      if(read_float_eeprom(0) != 0 && read_float_eeprom(0) > 0.001)
      {
         calibrate_fac_main = read_float_eeprom(0); //den kalibrede værdi hentes fra eeprom'en
         read_flag = 1;                             //flag sættes for at fortælle mikrokontrolleren er kalibreret
      }
      
      
      if(read_float_eeprom(0) == 0 )
      {
         read_flag = 0;
      }
      
      lcd_clear();        //sletter LCD displayet
      lcd_gotoxy( 1, 1);  //instiller hvor der skrives på LCD displayet
      
      //if sætning der enten anmoder om kalibereing eller tager+udskriver ADC vmålingen
      if(read_flag != 0)
      {
         adc_read();    //fortager ACD måling
         sprintf (buf, "Volt: %.5f  ", (float)avg_data*calibrate_fac_main/1000); //ligger ACD værdien + string i buf
      }         
      else
      {
         adc_read();
         sprintf(buf,"Calibrate device"); //anmoder om kalibrering af mikrokontrolleren
      }
      
      lcd_print(buf);   //udskriver "buf" til LCD displayet
      delay_ms(700);    //delay så man kan se hvad der er på displayet
      tot_data = 0;
   } //while slutter
}  //main program slutter

/*Denne funktion læser ADC målingen fra den eksterne ADC kreds
informationen hentes vha. en SPI forbinnelse. Der fortages 16
målinger og gennemsnittet ligges i avg_data som anvendes til udskrivning*/
void adc_read()
{
   for(i=0;i<=15;i++)
   {
   output_low(CS);               //CS sættes lav så der hentes information fra SPI forbinnelse 
   spi_read(1);                  //De første SPI data forkastes men skal kaldes
   MSB_value = spi_read(0XFF);   //De mest betydene bit ligges is MSB_value og der sendes FF tilbage
   LSB_value = spi_read(0);      //De mindst betydene bit ligges is LSB_value og der sendes 0 tilbage
   output_HIGH(CS);              //CS sættes høj for at stoppe SPI forbinnelsen
   in_data =  ((int16)(MSB_value & 0x0f) << 8) | LSB_value; //de modtagede data ligges nu ned i in_data
   tot_data += in_data;          //Alle dataen ligges ned i tot_data
   } 
   avg_data = (tot_data + 8) >> 4;  //Den gennemsnitlige værdi bestemmes
   printf("avg_data: %lu\n",avg_data); //værdien udskrives til debug monitoren
}

/*funktionen kalibrere mikrokontrolleren ved at finde forskellen på den nuværende
inputværdi og finde en faktor der mangler for at opnå den ønskede. Denne værdi ligges
ned i eeprom'en*/
void calibrate()
{
   calibrate_fac = (3900/(float)avg_data)*1;    //udregner den kalibrerings faktoren
   delay_ms(50);
   write_float_eeprom(0, calibrate_fac);        //skriver til eeprom'en
   printf("Calibration factor: %10.6f\n avg_data: %lu\n",calibrate_fac,avg_data); //udskriver den målte værdi og faktoren til monitoren
   int_flag = 0;
}

//denne funktion skriver til eeprom'en, skal bruge en adresse og den data der skal indlæses.
void write_float_eeprom(int16 address, float data)
   {
   int8 i;
   for( i =0; i < 4; i++) write_eeprom(address + i, *((int8 *)(&data)+i) );
   } //Skrivning til EEPROM færdig.

//denne funktion henter data fra eeprom'en, skal bruge en adresse og retunere data'en på adressen.
float read_float_eeprom(int16 address)
   {
   int8 i;
   float data;
   for(i = 0; i < 4; ++i) *((int8 *)(&data) + i) = read_eeprom(address + i);
   return data;
   }// Læsning af EEPROM stopper
