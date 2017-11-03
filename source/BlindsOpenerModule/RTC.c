//ADVANCED ALARM CLOCK SYSTEM BY MARLON CALLEJA (June 2013)
//BLINDS OPENER MODULE
#include <T89C51AC2.h>	  //8051 library
#include <absacc.h>	 

//set pins for H-bridge 
sbit in1 = P2^0;  
sbit in2 = P2^1;  
sbit en1 = P2^2; 
//set pins for photo-int sensors
sbit opto_bot = P1^1; 
sbit opto_top =	P1^2;  
//set pins for blind's control buttons
sbit sw_down = P1^3; 	
sbit sw_up = P1^4; 	 
//set pins for motor activity status leds	  
sbit led_busy = P1^5;
sbit led_idle = P1^6;  	

unsigned char tmp, txt[3];
int semafor, i, state;
int alarm_state=0;	    

void delay(int time)		
{
	int i,j;
	for(i=0;i<time;i++)
	for(j=0;j<1275;j++);
} 

unsigned char rx_data()			
{  
	//Function for reading data from SBUF
	unsigned char rx;

		while(RI==0);  //wait for RI flag 
   	 	rx = SBUF;     //read SBUF
   	 	RI=0; 		   //clear RI flag
	 	return rx;  	 
}

void serial_it(void) interrupt 4	//SERIAL INTERRUPT	 
{       
	// Checks if data 'ON' has been recieved

 		 if(RI==1) tmp = rx_data();

 			 switch(state)
			 {	  
       			 case 0: 
				 {
                 	if (tmp == 'D')
                  	  state=1;						  
                 	else
                   	 state=0;
                 	break;
               	 }

        		case 1:
				 {
               		if (tmp == 'A')
                   		 state=2;
                	else
                   		 state=0;
                 	break;
                 }

       			case 2: 
				 {
                	if (tmp == 'T')
                  		 state = 3;
                	else
                   		 state=0;
               		break;
                 }

        		case 3: 
				 {
                	if (tmp == 'A')
                   		state = 4;
                	else
                   		state = 0;
                	break;
                 }

        		case 4: 
				 {
                	if (tmp == ':')
                   		state = 5;
                	break;
              	 }

        		case 5: 
				 {
                 	semafor = 1;
				 	i = 0;
                	state = 6;
                	break;
                 }

        		case 6:
				 {
                	if (tmp == 13) 	  //check for end of data string
					 {	                    	                    	
						if (txt[0]=='O' && txt[1]=='N')	
						{
							alarm_state=1;						
						}  
						semafor = 0; 
						state = 0;	        
                   	 }
                	break;
               	 }
     
      			default: 
				 {
                    state=0;
                    break;
                 }
  			 }

    if (semafor)   // if semafor = 1 (true) perform action
	{
        txt[i] = tmp;                   // move the data received into array txt[]
        i++;							// increment array pointer
    }  	
	RI=0; 	   //clear RI flag   
}	

void rotate_f(void)
{
        in1 = 1;             //Make positive of motor 1
        in2 = 0;             //Make negative of motor 0
        en1 = 1;             //Enable L293D
		led_idle = 1;
		led_busy = 0;
}

void rotate_b(void)
{
        in1 = 0;             //Make positive of motor 0
        in2 = 1;             //Make negative of motor 1
        en1 = 1;             //Enable L293D
		led_idle = 1;
		led_busy = 0;
}

void stop(void)
{
        in1 = 0;             //Make positive of motor 0
        in2 = 0;             //Make negative of motor 0
        en1 = 0;             //Disable L293D
		led_idle = 0;
		led_busy = 1;
		
} 

void sw_alarm(void)
{	
	
	rotate_b();
	while(opto_top==0 && sw_up==1 && sw_down==1)   // Switch & sensor polling
	{
 	  delay(5);
  	}
	stop();	
	alarm_state=0;
	delay(50); 
		
}



void up(void)
{  
  delay(50); 	   
  rotate_b();	 
  while(sw_up==1 && opto_top==0)	  // Switch & sensor polling
  {
  	delay(5);
  }
  stop();
  delay(50);	
}


void down(void)
{  
  delay(50); 	   
  rotate_f();
  while(sw_down==1 && opto_bot==0)	   // Switch & sensor polling
  {
  	delay(5);
  }
  stop();
  delay(50);
}  


void main(void)	
{  
delay(200); 
sw_up = 1;
sw_down = 1;
opto_top = 1;
opto_bot = 1;		
led_idle = 0;
led_busy = 1;	

IEN0=0x90;			//enable interrupts, enable serial interrupt	   

SCON=0x50; 			//enables serial reception in 8-bit mode
T2MOD=0;			//Timer2
T2CON=0x30;			//use Timer 2 as receive/transmit clock for serial port
RCAP2H=0xff	;
RCAP2L=0xf1;
TR2=1;	 			//turn on timer 2

delay(1000);   		//Power-up delay  

	while(1)   		// Switch polling
	{  
		 if(sw_up==0) up();
		 else if(sw_down==0) down();
		 else if (alarm_state==1) sw_alarm();		
	}




}