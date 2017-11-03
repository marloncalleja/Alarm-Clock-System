//ADVANCED ALARM CLOCK SYSTEM BY MARLON CALLEJA (June 2013)
//ALARM CLOCK BASE STATION

#include <T89C51AC2.h>	 //8051 library
#include <absacc.h>	 
  
//Define ports and data pins of microcontroller
#define dataport P2	 	   
sbit rs = P1^1;
sbit rw = P1^2;
sbit en = P1^3;  
sbit cycle = P1^4;
sbit dec = P1^5;  	
sbit inc = P1^6;     	
sbit busy = P2^7;  	
sbit radio = P1^7;   	  

unsigned char hr,min,sec,num[60]={0X00,0X01,0X02,0X03,0X04,0X05,0X06,0X07,0X08,0X09,0X10,0X11,0X12,0X13,0X14,0X15,0X16,0X17,0X18,0X19,0X20,0X21,0X22,0X23,0X24,0X25,0X26,0X27,0X28,0X29,0X30,0X31,0X32,0X33,0X34,0X35,0X36,0X37,0X38,0X39,0X40,0X41,0X42,0X43,0X44,0X45,0X46,0X47,0X48,0X49,0X50,0X51,0X52,0X53,0X54,0X55,0X56,0X57,0X58,0X59}; 
int int_hr, int_min;

void delay(int time) 
{
	//Time delay function
	int i,j;
	for(i=0;i<time;i++)
	for(j=0;j<1275;j++);	 	
}
										
void lcdready()		 
{
	//Check if LCD is busy processing data
	busy = 1;            //make the busy pin an input
	rs = 0;
	rw = 1;
	while (busy==1)      //wait here for busy flag
	 {
		en = 0;          //strobe the enable pin
		delay(1);
		en = 1;
     }
	return;
} 	  


void lcdcmd (unsigned char value)	 
{
	//LCD command input
	lcdready();             //check the LCD busy flag
	dataport = value;	    //put the value on the pins
	rs = 0;
	rw = 0;
	en = 1;			        //strobe the enable pin
	delay(1);
	en = 0;
	return;
}

void lcddata (unsigned char value)	 
{
	//LCD character input
	lcdready();    		   //check the LCD busy flag 
	dataport = value;	   //put the value on the pins	 	 
	rs = 1;
	rw = 0;
	en = 1;		           //strobe the enable pin
	delay(1);
	en = 0;
	return;				   
}  		
 
void bcdconv(unsigned time)  
{	
	//conver BCD to ASCII and send it to lcddata function for display
	unsigned char x,y; 		    
	x=time&0x0F;
	x=x|0x30;
	y=time&0xF0;
	y=y>>4;
	y=y|0x30;
	lcddata(y);		
	lcddata(x);	   			
}    

void lcd_string (unsigned char x[])
{
	// data string array, every single char in the array is sent to lcddata function for display 
	int pnt=0;				 //array pointer
							  
	while(x[pnt]!= '\0') 	 
	{
	lcddata(x[pnt]); 	   	
	pnt++;					// increment array pointer
	}   	

}   		

void display_time(void)
{
 	//Display Time. Read the time, convert it to ASCII, and send it to the LCD 
	lcdcmd(0x01);		 	//Clear LCD	   	
	lcdcmd(0x81);			//position cursor  		    
    hr=XBYTE[4];			//get hour 
	bcdconv(hr);			//convert and display
	lcddata(':');	
	min=XBYTE[2];			//get minute 
	bcdconv(min);			//convert and display  	 
}

void display_alarm(void)
{
 	//Display Alarm Time. Read the time, convert it to ASCII, and send it to the LCD 
	lcdcmd(0x01);		 	//Clear LCD	   	
	lcdcmd(0x81);			//position cursor  		    
    hr=XBYTE[5];			//get alarm hour 
	bcdconv(hr);			//convert and display
	lcddata(':');	
	min=XBYTE[3];			//get alarm minute 
	bcdconv(min);			//convert and display  	 
}	   

void convert_time(void)
{
	//Convert time from BCD format to decimal
	int x;

	for(x=0;x<60;x++)	  	//acquire min and save it as integer
	{
	  	if(min==num[x])
	  	int_min=x;
	}	 

	for(x=0;x<24;x++)		//acquire hr and save it as integer
	{
	    if(hr==num[x])
	  	int_hr=x;
	}	   
		 
}


void tx_data (unsigned char x[])		
{
	//Transfer serially a string of data
	int pnt=0;				  //array pointer		
							  
	while(x[pnt]!= '\0') 	 
	{
	SBUF = x[pnt]; 	    	// load SBUF with char data	for transmitting
	while(TI==0);	 		// Wait for byte to be transmitted 	 
	TI=0;			 		// Reset the transfer interrupt flag  
	pnt++;					// increment array pointer
	}   

} 

void send(void)	 //transmit data to RX
{  	 
	tx_data("ATD55\r");	   //AT command for sending data   
    tx_data("ON\r");  	   //data to be transmitted				  
} 

void alarm() interrupt 0   // ALARM INTERRUPT  ---- Clear Alarm Interrupt Flag(AF) from Register C of RTC, turn on external device and transmit data to RX
{   
   
   	unsigned char creg;	 	 
		    	
	creg = XBYTE[12];	 // read value of address location 12 to clear AF	 
	TR2=1;	  	         // start timer 2	 	
		
	delay(50);
	send();	   

	TR2=0; 				 // stop timer 2
    radio = 1; 	      	 // turn on radio
}	

void radio_on(void)
{
	//turn on external device
	delay(50);
	radio = 1;

}

void radio_off(void)
{
	//turn off external device
	delay(50);
	radio = 0;

}  
  	  
void main(void)	
{  
   delay(200); 
   AUXR = 0x0A; 	//initialise AUXR to enable ALE and external memory access (RTC memory)
   IEN0=0x81;		//enable interrupts, enable external interrupt 0            
   cycle = 1; 		//set pin as input
   inc = 1;			//set pin as input
   dec = 1;			//set pin as input	    
   radio = 0;	   	  
   
   //Set timing and baud rate for serial communication with RF module
    
   SCON=0x50; 			  //enables serial reception in 8-bit mode
   T2MOD=0;				  //Timer2
   T2CON=0x30;			  //use Timer 2 as receive/transmit clock for serial port
   RCAP2H=0xff;			  
   RCAP2L=0xf1;	  
   TR2=0;   	  		  //turn off timer 2
   	  	  
	  //RTC initialization. Turn on oscillator and set initial time	

	  delay(500);		//  Power-up delay - the RTC is accessible after 200ms 	   	
	  XBYTE[11]=0x22;   //  set as 24-hour clock. Also permits the Alarm Flag(AF) bit in register C to assert IRQ for interrupt 0				  
      XBYTE[10]=0x20;   //  turn on osc  
/*	  XBYTE[0]=0x00;		SECOND
	  XBYTE[2]=0x00;		MINUTE 
	  XBYTE[4]=0x00;		HOUR   	 
	  XBYTE[1]=0x00;		Alarm SECOND
	  XBYTE[3]=0x00;		Alarm MINUTE
	  XBYTE[5]=0x00;      	Alarm HOUR    */	   	  		  
	  	  	  

	  //LCD initialization in 8-bit mode
	  lcdcmd(0x30);
	  delay(10);
	  lcdcmd(0x30);
	  delay(10);
	  lcdcmd(0x30);
	  delay(10);
	  lcdcmd(0x38);		 	//Set: 2 Line, 8-bit, 5x7 dots	  
	  lcdcmd(0x0C);		    //Display on, Cursor off
	  lcdcmd(0x01);		 	//Clear LCD	   	
	  lcdcmd(0x06);		 	//Entry mode, auto increment with no shift		  		   
    

		   while(1)
		   { 		   		    	    		
			display_time(); 
			lcddata(':');
					
			while(cycle==1)		   //display time
	 		{  	
				if (dec==0)	        radio_on();
				else if (inc==0)	radio_off(); 	 												
										
				lcdcmd(0x87); 		//position cursor   				
				sec=XBYTE[0];   	//get second   							  							
				bcdconv(sec);   	//convert and display 
				delay(20);
									
					if(sec!=0X00) continue;
									
						  min=XBYTE[2];			//get minute 
								
							if(min==0X00)
							{												
								 hr=XBYTE[4];	//get hour 								 										 												
								 lcdcmd(0x81);	//position cursor   
								 delay(100);										
								 bcdconv(hr); 	//convert and display 
								 lcdcmd(0x84);	//position cursor 
								 bcdconv(min);	//convert and display 								
							} 
							else
							{ 	
								lcdcmd(0x84); 	//position cursor  
								delay(200);	 
								bcdconv(min);	//convert and display 												
							}

							
			}
			delay(60);
			lcdcmd(0x01);
			lcdcmd(0x80);			//position cursor  	 
			lcd_string("1.Set Time");
			lcdcmd(0xC0);			//position cursor  	 
			lcd_string("2.Set Alarm");		


					while(1)
					{
						if(dec==0)			    //SET TIME----------------------------
						{
							delay(60);								
							display_time();	
							lcdcmd(0x8A);
							lcd_string("TIME");
							lcdcmd(0xCA);
							lcd_string("MIN");	
							convert_time();  

							while(cycle==1) 
	  						{	   
	  							if(inc==0)
	   							{	
							   		delay(2);  //wait for switch bounce time
	  	 							int_min++;

									if(int_min>59) int_min=0;	

	   	 							lcdcmd(0x84);	
	  	 							min=num[int_min];  	
									delay(30);	 							
	 	 							bcdconv(min);					 
								}
								else if(dec==0)
	   							{	
							   		delay(2);  //wait for switch bounce time
	  	 							int_min--;

									if(int_min<1) int_min=59;	

	   	 							lcdcmd(0x84);	
	  	 							min=num[int_min];  	
									delay(30);	 							
	 	 							bcdconv(min);					 
								}
		
		
	  						}

	  						delay(60);
							lcdcmd(0xCA);
							lcd_string("HOUR");	 


	  						while(cycle==1)
	  						{
	  							if(inc==0)
	   		 					{	 
							  		delay(2);  //wait for switch bounce time
	  								int_hr++;

									if(int_hr>23) int_hr=0;	

	  								lcdcmd(0x81);
	  								hr=num[int_hr];	  
									delay(30);	 									
	 								bcdconv(hr);   		  				  			
								}
								else if(dec==0)
	   		 					{	 
							  		delay(2);  //wait for switch bounce time
	  								int_hr--;

									if(int_hr<1) int_hr=23;	

	  								lcdcmd(0x81);
	  								hr=num[int_hr];	  
									delay(30);	 									
	 								bcdconv(hr);   		  				  			
								}		
		
	 						 }

							XBYTE[4]=hr;
							XBYTE[2]=min;
					 		delay(60);													
							break;	 				
						}

						else if(inc==0)			//SET ALARM-------------------------

						{	 					
							delay(60);
							display_alarm();	
							lcdcmd(0x8A);
							lcd_string("ALARM"); 								
							lcdcmd(0xCA);
							lcd_string("MIN");																
							convert_time();  							 

							while(cycle==1) 
	  						{	   
	  							if(inc==0)
	   							{	
							   		delay(2);  //wait for switch bounce time
	  	 							int_min++;

									if(int_min>59) int_min=0;	

	   	 							lcdcmd(0x84);	
	  	 							min=num[int_min];  	
									delay(30);	 							
	 	 							bcdconv(min);					 
								}
								else if(dec==0)
	   							{	
							   		delay(2);  //wait for switch bounce time
	  	 							int_min--;

									if(int_min<1) int_min=59;	

	   	 							lcdcmd(0x84);	
	  	 							min=num[int_min];  	
									delay(30);	 							
	 	 							bcdconv(min);					 
								}
		
		
	  						}

	  						delay(60);
							lcdcmd(0xCA);
							lcd_string("HOUR");	

	  						while(cycle==1)
	  						{
	  							if(inc==0)
	   		 					{	 
							  		delay(2);  //wait for switch bounce time
	  								int_hr++;

									if(int_hr>23) int_hr=0;	

	  								lcdcmd(0x81);
	  								hr=num[int_hr];	  
									delay(30);	 									
	 								bcdconv(hr);   		  				  			
								}
								else if(dec==0)
	   		 					{	 
							  		delay(2);  //wait for switch bounce time
	  								int_hr--;

									if(int_hr<1) int_hr=23;	

	  								lcdcmd(0x81);
	  								hr=num[int_hr];	  
									delay(30);	 									
	 								bcdconv(hr);   		  				  			
								}		
		
	 						 }

							XBYTE[5]=hr;
							XBYTE[3]=min;
					 		delay(60);													
							break;	 
									
						} 
						else if(cycle==0)	    //EXIT MENU------------------------------
						{	
						 	delay(50);						
							break;
						}		

					}	 			
		   }  	  
}	  	    