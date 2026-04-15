#include <msp430f6638.h>
#include "tm1638.h"
#include "tm1638.c"
#define delay_us(x) __delay_cycles((long)(CPU_F*(double)x/1000000.0))
#define delay_ms(x) __delay_cycles((long)(CPU_F*(double)x/1000.0))
#define d 0x01
#define c 0x20
#define b 0x40
#define a 0x80
#define dp 0x10
#define g 0x04
#define f 0x08
#define e 0x02
#define keyif1 (P4IN & 0x04)
#define keyif2 (P4IN & 0x08)

int i,j,k,s,p,q,m0,n0,m1,n1,restart,key,direction,overtime,pageLCD,pageLED,scoreA,scoreB,addA,addB,recordnum,recordsel,matek,matenum;

struct Goalrecord
{
	int team,score,num,min,sec;
};
typedef struct Goalrecord GOA;
GOA record[100];

struct Matescore
{
	int team,score;
};
typedef struct Matescore MAS;
MAS mate[20];

const uint8_t tab[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,0x40,0x80};

const char char_gen[]= 
{
 a+b+c+d+e+f, // Displays "0"
 b+c, // Displays "1"
 a+b+d+e+g, // Displays "2"
 a+b+c+d+g, // Displays "3"
 b+c+f+g, // Displays "4"
 a+c+d+f+g, // Displays "5"
 a+c+d+e+f+g, // Displays "6"
 a+b+c, // Displays "7"
 a+b+c+d+e+f+g, // Displays "8"
 a+b+c+d+f+g, // Displays "9"
 a+b+c+e+f+g, // Displays "a"
 c+d+e+f+g, // Displays "b"
 a+d+e+f, // Displays "c"
 b+c+d+e+g, // Displays "d"
 a+d+e+f+g, // Displays "e"
 a+e+f+g, // Displays "f"
 a+b+c+d+f+g, // Displays "g"
 c+e+f+g, // Displays "h"
 b+c, // Displays "i"
 b+c+d, // Displays "j"
 b+c+e+f+g, // Displays "k"
 d+e+f, // Displays "l"
 a+b+c+e+f, // Displays "n"
 a+b+c+d+e+f+g+dp, // Displays "full"
 g// Displays "-"
};

//init function//
void init_port(void)
{
	P3DIR |= BIT6;
	P3DIR |= BIT5;
	P3DIR |= BIT4 + BIT2;
	P1DIR |= BIT0; 
	P1SEL |= BIT0;
}

void Init_lcd(void)
{
	LCDBCTL0 =LCDDIV0 + LCDPRE0 + LCDMX1 + LCDSSEL + LCDMX1 + LCD4MUX ;
	LCDBPCTL0 = LCDS0 + LCDS1 + LCDS2 + LCDS3 + LCDS4 + LCDS5 + LCDS6+ LCDS7 + LCDS8+ LCDS9 + LCDS10 + LCDS11 ;
	P5SEL = 0xfc;
}

void LcdGo(unsigned char Dot)
{
	if(Dot==1)
	{
	LCDBCTL0 |= LCDON;
	}
	else if(Dot==0)
	{
		LCDBCTL0 &= ~LCDON;
	}
}

void LcdBlink(unsigned char doit)
{
	if(doit==0)
	{
		LCDBCTL0 &= ~LCDSON;
	}
	else if(doit==1)
	{
		LCDBCTL0 |= LCDSON;
	}
}

void LCD_Clear(void)
{
	unsigned char index;
	for (index=0;index<12;index++)
	{
		LCDMEM[index] = 0;
	}
}

void Init_TS3A5017DR(void)
{
	P1DIR |= BIT6 + BIT7; 
	P1OUT &= ~BIT7;
	P1OUT |= BIT6;
}

void Backlight_Enable(void)
{
	P8DIR |= BIT0;
	P8OUT |= BIT0;
}

void Timer_a0_init()
{
	TA0CCTL0 = CCIE;
	TA0CCR0 = 32768;
	TA0CTL = TASSEL_1+MC_1+TACLR;
}
//init function//

void Show_wholetime()
{
	Write_DATA(4*2,tab[0]);
	Write_DATA(5*2,tab[s]);
	Write_DATA(6*2,tab[16]);
	Write_DATA(7*2,tab[16]);
	Write_DATA(0*2,tab[m1]);
	Write_DATA(1*2,(tab[n1]+0x80));
	Write_DATA(2*2,tab[m0]);
	Write_DATA(3*2,tab[n0]);
}

void Show_goaltime()
{
	Write_DATA(4*2,(tab[record[recordsel].team+10]+0x80));
	Write_DATA(5*2,tab[record[recordsel].score]);
	Write_DATA(6*2,tab[16]);
	Write_DATA(7*2,(tab[record[recordsel].num]+0x80));
	Write_DATA(0*2,tab[(record[recordsel].min)/10]);
	Write_DATA(1*2,(tab[(record[recordsel].min)%10]+0x80));
	Write_DATA(2*2,tab[(record[recordsel].sec)/10]);
	Write_DATA(3*2,tab[(record[recordsel].sec)%10]);
}

void Show_time()
{
	LCDMEM[0] = char_gen[direction+10];
	LCDMEM[1] = char_gen[24];
	LCDMEM[2] = char_gen[24];
	LCDMEM[3] = char_gen[24];
	LCDMEM[4] = char_gen[i];
	LCDMEM[5] = char_gen[j];
}

void Show_score()
{
	LCDMEM[0] = char_gen[scoreA/10];
	LCDMEM[1] = char_gen[scoreA%10];
	LCDMEM[2] = char_gen[24];
	LCDMEM[3] = char_gen[24];
	LCDMEM[4] = char_gen[scoreB/10];
	LCDMEM[5] = char_gen[scoreB%10];
}

void Show_matescore()
{
	LCDMEM[0] = char_gen[mate[matenum].team+10];
	LCDMEM[1] = char_gen[24];
	LCDMEM[2] = char_gen[matenum%10];
	LCDMEM[3] = char_gen[24];
	LCDMEM[4] = char_gen[mate[matenum].score/10];
	LCDMEM[5] = char_gen[mate[matenum].score%10];
}

void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;
	Init_TS3A5017DR();
	Init_lcd();
	Backlight_Enable();
	LcdGo(1);
	LCD_Clear();
	init_port();
	init_TM1638();
	TA0CTL=(TA0CTL&0xCF);
	
	P4DIR &= ~(BIT2); 
	P4DIR &= ~(BIT3); 
	P4DIR |= BIT4;
	__bis_SR_register(GIE);
	
	key=0;
	overtime=0;
	direction=0;
	m1=1;
	n1=2;
	m0=n0=0;
	s=1;
	pageLCD=pageLED=0;
	scoreA=scoreB=0;
	addA=addB=0;
	recordnum=0;
	matek=0;
	matenum=0;
	for(p=0;p<2;p++)
	{
		for(q=0;q<10;q++)
		{
			mate[q+10*p].team=p;
			mate[q+10*p].score=0;
		}
	}
	
	while(1)
	{
		k=Read_key();
		if(k==15)
		{
			delay_ms(1);
			if(k==15)
			{
				key++;
				switch(key)
				{
				case 1:
					if(overtime==1)
					{
						direction++;
						direction=direction%2;
					}
					restart=1;
					overtime=0;
					P4OUT&=~(BIT4);
					Timer_a0_init();
					break;
				case 2:
					TA0CTL=(TA0CTL&0xCF);
					break;
				case 3:
					Timer_a0_init();
					key=1;
					break;
				}
				while(Read_key()==k);
			}
		}
		if(k==10)
		{
			delay_ms(1);
			if(k==10)
			{
				addA=1;
				addB=0;
			}
			while(Read_key()==k);
		}
		if(k==11)
		{
			delay_ms(1);
			if(k==11)
			{
				addB=1;
				addA=0;
			}
			while(Read_key()==k);
		}
		if(addA==1&&(k==1||k==2||k==3||k==4||k==5||k==6||k==7||k==8||k==9||k==0))
		{
			delay_ms(1);
			if(addA==1&&(k==1||k==2||k==3||k==4||k==5||k==6||k==7||k==8||k==9||k==0))
			{
			addA=2;
			matek=k;
			}
			while(Read_key()==k);
			k=0;
		}
		if(addA==2&&(k==1||k==2||k==3))
		{
			delay_ms(1);
			if(addA==2&&(k==1||k==2||k==3))
			{
				addA=0;
				scoreA+=k;
				record[recordnum].min=10*m1+n1;
				record[recordnum].sec=10*m0+n0;
				record[recordnum].num=s;
				record[recordnum].score=k;
				record[recordnum].team=0;
				recordnum++;
				mate[matek].score+=k;
			}
			while(Read_key()==k);
		}
		if(addB==1&&(k==1||k==2||k==3||k==4||k==5||k==6||k==7||k==8||k==9||k==0))
		{
			delay_ms(1);
			if(addB==1&&(k==1||k==2||k==3||k==4||k==5||k==6||k==7||k==8||k==9||k==0))
			{
				addB=2;
				matek=k;
			}
			while(Read_key()==k);
			k=0;
		}
		if(addB==2&&(k==1||k==2||k==3))
		{
			delay_ms(1);
			if(addB==2&&(k==1||k==2||k==3))
			{
				addB=0;
				scoreB+=k;
				record[recordnum].min=10*m1+n1;
				record[recordnum].sec=10*m0+n0;
				record[recordnum].num=s;
				record[recordnum].score=k;
				record[recordnum].team=1;
				recordnum++;
				mate[matek+10].score+=k;
			}
			while(Read_key()==k);
		}
		if(k==14)
		{
			delay_ms(1);
			if(k==14)
			{
				pageLED++;
				pageLED=pageLED%2;
			}
			while(Read_key()==k);
		}
		if(pageLED==1&&k==12)
		{
			delay_ms(1);
			if(pageLED==1&&k==12)
			{
				recordsel++;
				recordsel=recordsel%recordnum;
			}
			while(Read_key()==k);
		}
		if(pageLED==1&&k==13)
		{
			delay_ms(1);
			if(pageLED==1&&k==13)
			{
				recordsel--;
				if(recordsel<0)
				{
					recordsel=recordnum-1;
				}
			}
			while(Read_key()==k);
		}
		if(keyif1==0)
		{
			delay_ms(1);
			if(keyif1==0)
			{
				pageLCD++;
				pageLCD=pageLCD%3;
				while(keyif1==0);
			}
		}
		if(keyif2==0&&pageLCD!=2)
		{
			delay_ms(1);
			if(keyif2==0&&pageLCD!=2)
			{
				direction++;
				direction=direction%2;
				restart=1;
				overtime=0;
				P4OUT&=~(BIT4);
				Timer_a0_init();
				key=1;
				while(keyif2==0&&pageLCD!=2);
			}
		}
		if(keyif2==0&&pageLCD==2)
		{
			delay_ms(1);
			if(keyif2==0&&pageLCD==2)
			{
				matenum++;
				matenum=matenum%20;
				while(keyif2==0&&pageLCD==2);
			}
		}
		switch(pageLCD)
		{
			case 0:
				Show_time();
				break;
			case 1:
				Show_score();
				break;
			case 2:
				Show_matescore();
				break;
		}
		switch(pageLED)
		{
			case 0:
 				Show_wholetime();
				break;
			case 1:
				Show_goaltime();
				break;
		}
		if(overtime==1)
		{
			P4OUT ^= BIT4;
			delay_ms(5);
		}
		if(restart==1)
		{
			restart=0;
			i=2,j=4;
		}
	}
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
	j--;
	if(j<0)
	{
		j=9;
	}
	if(j==9)
	{
		i--;
	}
	n0--;
	if(n0<0)
	{
		n0=9;
	}
	if(n0==9)
	{
		m0--;
		if(m0<0)
		{
			m0=5;
		}
		if(m0==5)
		{
			n1--;
			if(n1<0)
			{
				n1=9;
			}
			if(n1==9)
			{
				m1--;
				if(m1<0)
				{
					TA0CTL=(TA0CTL&0xCF);
					s++;
					m1=1;
					n1=2;
					m0=n0=0;
				}
			}
		}
	}
	if(i==0&&j==0)
	{
		TA0CTL=(TA0CTL&0xCF);
		overtime=1;
		key=0;
	}
}
