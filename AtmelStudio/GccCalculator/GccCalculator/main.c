#include <stdio.h>
#include <avr/io.h>
#define  F_CPU 1000000
#include <util/delay.h>
#include <math.h>
#include <avr/interrupt.h>
#include <string.h>
#include "check_parser.c";

unsigned char plus_minus_state;
unsigned char ishyper;
unsigned char resultShowing;
unsigned char isEnd[18];
char expStr[18];
char message[36];
char msg_char;
int currpos;
#define MAIN_DELAY 38
#define INIT_DELAY 80    //pre 100
#define PER_CHAR_DELAY 30  //pre 80
#define PMAX 1000000000000000
#define NMAX -100000000000000
unsigned char cport,dport,cdata,ddata;
void command(unsigned char cmd)
{
	/*PORTC=0X02;
	PORTD=cmd;
	PORTC=0X00;*/
	//Implementing previous PORTC=0X02;
	cport=PORTC&(0b11001111);
	cport=cport|(2<<4);
	PORTC=cport;
	//Implementing previous PORTD=cmd;
	dport=PORTD&(0b00000011);
	ddata=cmd&(0b11111100);
	cdata=cmd&(0b00000011);
	cdata=cdata<<6;
	cport=PORTC&(0b00111111);
	dport=dport|ddata;
	cport=cport|cdata;
	PORTD=dport;
	PORTC=cport;
	//Implementing previous PORTC=0X00;
	cport=PORTC&(0b11001111);
	cport=cport|(0<<4);
	PORTC=cport;
	_delay_ms(5);
}
void lcd_data(unsigned char data)
{
	/*PORTC=0X03;
	PORTD=data;
	PORTC=0X01;*/
	//Implementing previous PORTC=0X03;
	cport=PORTC&(0b11001111);
	cport=cport|(3<<4);
	PORTC=cport;
	//Implementing previous PORTD=data;
	dport=PORTD&(0b00000011);
	ddata=data&(0b11111100);
	cdata=data&(0b00000011);
	cdata=cdata<<6;
	cport=PORTC&(0b00111111);
	dport=dport|ddata;
	cport=cport|cdata;
	PORTD=dport;
	PORTC=cport;
	//Implementing previous PORTC=0X01;
	cport=PORTC&(0b11001111);
	cport=cport|(1<<4);
	PORTC=cport;
	_delay_ms(5);
}
void uart_init()
{
	UCSRA = 0b00000010;
	UCSRB = 0b10011000;
	UCSRC = 0b10000110;
	
	UBRRH = 0;
	UBRRL = 12;
}


void uart_send(unsigned char data){
	while ((UCSRA & (1<<UDRE)) == 0x00);
	UDR = data;
}

unsigned char uart_receive(void){
	while ((UCSRA & (1<<RXC)) == 0x00);
	return UDR;
}
void clrScr()
{
	for(int i=0; i<18; i++) {
		isEnd[i] = 0;
		expStr[i] = rans[i] = '\0';
	}
	currpos = 0;
	command(0x01);
}
void printExp()
{
	command(0x01);
	for(int i=0;i<currpos;i++)
	{
		lcd_data(expStr[i]);
		command(0x06);
	}
}
void printRes()
{
	double d=parse_exp(expStr);
	if(d<=NMAX||d>=PMAX)
	{
		sprintf(rans,"Range_Error");
	}
	else
	{
		long long denominator=1;
		int i,sign_bit=0;
		if(d<0)
		{
			sign_bit=1;
			d=-d;
		}
		long long rng;
		if(sign_bit)
		{
			rans[0]='-';
			rng=-NMAX;
		}
		else rng=PMAX;
		int pos=strlen(rans);
		while(d>0 && d*10<rng && denominator<1000)
		{
			d=d*10;
			denominator=denominator*10;
		}
		long long f=(long long) d;
		long long s=f%denominator;
		f=f/denominator;
		long long rev_num=0;
		int digits=0;
		do
		{
			rev_num=rev_num*10+(f%10);
			f=f/10;
			digits++;
		}while(f>0);
		for(i=0; i<digits; i++)
		{
			long long current_digit=rev_num%10;
			rev_num=rev_num/10;
			int cans=(int) current_digit;
			rans[pos++]=(cans+'0');
		}
		if(!(s==0||f>=(rng/100)))
		{
			rans[pos++]='.';
			while(pos<15&&s>0)
			{
				s=s*10;
				long long curr=s/denominator;
				int cans=(int) curr;
				rans[pos++]=(cans+'0');
				s=s%denominator;
			}
		}
	}
	//Lcd4_Write_String(rans);
	command(0xC0);			// line 2 of lcd.
	for(int i=0;i<16;i++)
	{
		if(rans[i]=='\0') break;
		lcd_data(rans[i]);
		command(0x06);
	}
	resultShowing=1;
}

void makeExp(char ch,unsigned char commandOn)
{
	if(resultShowing)
	{
		resultShowing=!resultShowing;
		clrScr();
	}
	expStr[currpos++]=ch;
	expStr[currpos]='\0';
	if(commandOn)
	{
		printExp();
		isEnd[currpos-1]=1;
	}
}

void delExp()
{
	if(resultShowing)
	{
		resultShowing=!resultShowing;
		clrScr();
	}
	if(currpos==0) return;
	isEnd[currpos-1]=0;
	for(int i=currpos-1;i>0;i--)
	{
		expStr[i]='\0';
		if(isEnd[i-1])
		{
			currpos=i;
			printExp();
			return;
		}
	}
	currpos=0;
	printExp();
}

void sendToMem()
{
	if(!resultShowing) return;
	_delay_ms(INIT_DELAY);
	for(int i=0;i<currpos;i++)
	{
		_delay_ms(PER_CHAR_DELAY);
		if(expStr[i]>='A' && expStr[i]<='Z') 
			printf("%c",expStr[i]+32);
		else
			printf("%c",expStr[i]);
	}
	_delay_ms(PER_CHAR_DELAY);
	printf("%c",'#');
	for(int i=0;i<18;i++)
	{
		if(rans[i]=='\0') break;
		_delay_ms(PER_CHAR_DELAY);
		printf("%c",rans[i]);
	}
	_delay_ms(PER_CHAR_DELAY);
	printf("%c",'@');
}
void checkError()
{
	if((PORTB&32))
	{
		clrScr();
		unsigned char dummy=UDR;
		dummy=UDR;
		dummy=UDR;
		dummy=UDR;
		dummy=UDR;
		//UCSRA=UCSRA&(0b11100011);
		UCSRA = 0b00000010;
		if((PORTB&32)) PORTB=PORTB^32;
		lcd_data('T');
		command(0x06);
		lcd_data('r');
		command(0x06);
		lcd_data('a');
		command(0x06);
		lcd_data('n');
		command(0x06);
		lcd_data('s');
		command(0x06);
		lcd_data('m');
		command(0x06);
		lcd_data('i');
		command(0x06);
		lcd_data('s');
		command(0x06);
		lcd_data('s');
		command(0x06);
		lcd_data('i');
		command(0x06);
		lcd_data('o');
		command(0x06);
		lcd_data('n');
		command(0x06);
		command(0xC0);
		lcd_data('E');
		command(0x06);
		lcd_data('r');
		command(0x06);
		lcd_data('r');
		command(0x06);
		lcd_data('o');
		command(0x06);
		lcd_data('r');
		command(0x06);
		resultShowing=1;
	}
}
void receiveFromMem()
{
	scanf("%s", message);
	int i;
	/*for(i=0;i<34;i++)
	{
		PORTB=PORTB|((UCSRA&(1<<3))<<2);
		scanf("%c",&message[i]);
		if(message[i]=='@') break;
		//expStr[i]=msg_char;
	}*/
	//PORTB=PORTB|((UCSRA&(1<<3))<<2);
	//currpos=i;
	/*for(i=0;i<18;i++)
	{
		scanf("%c",&msg_char);
		if(msg_char=='@') break;
		rans[i]=msg_char;
	}*/
	
	clrScr();
	for(i=0; i<34; i++)
	{
		if(message[i]=='#') {
			command(0xC0);
			break;
		}
		else if (message[i]=='\0') break;
		//expStr[i]=message[i];
		lcd_data(message[i]);
		command(0x06);
	}
	//printExp();
	for(i=i+1;i<34;i++)
	{
		if(message[i]=='\0') break;
		rans[i]=message[i];
		lcd_data(rans[i]);
		command(0x06);
	}
	if(message[2]!='\0' && (PORTB&32)) PORTB=PORTB^32;
	for(i=0; i<36; i++)
	{
		message[i] = '\0';
	}
	resultShowing=1;
}
void row1()
{
	PORTB=0X01;
	if(PINA==0X01)
	{
		clrScr();	//clear display
	}
	else if(PINA==0X02)
	{
		makeExp('7',1);
	}
	else if(PINA==0X04)
	{
		makeExp('8',1);
	}
	else if(PINA==0X08)
	{
		makeExp('9',1);
	}
	else if(PINA==0X10)
	{
		makeExp('x',1);
	}
	else if(PINA==0X20)
	{
		makeExp('/',1);
	}
	else if(PINA==0X40)
	{
		//have to implement clear function of LCD
		delExp();
	}
	else if(PINA==0X80)
	{
		makeExp('s', 0);
		makeExp('i', 0);

		if(ishyper)
		{
			makeExp('n', 0);
			makeExp('h',1);
		}
		else makeExp('n',1);
	}
	else
	{
		unsigned char mask = 0x0C;
		unsigned char var = PINC;

		var = var&mask;
		if(var == 0x04)
		{
			makeExp('c', 0);
			makeExp('o', 0);

			if(ishyper)
			{
				makeExp('s', 0);
				makeExp('h',1);
			}
			else makeExp('s',1);
		}
		else if(var == 0x08)
		{
			makeExp('t', 0);
			makeExp('a', 0);

			if(ishyper)
			{
				makeExp('n', 0);
				makeExp('h',1);
			}
			else makeExp('n',1);
		}
	}
}

void row2()
{
	PORTB=0X02;
	if(PINA==0X01)
	{
		// + or - don't know, have to impelement
		makeExp('-',1);
	}
	else if(PINA==0X02)
	{
		makeExp('4',1);
	}
	else if(PINA==0X04)
	{
		makeExp('5',1);
	}
	else if(PINA==0X08)
	{
		makeExp('6',1);
	}
	else if(PINA==0X10)
	{
		makeExp('-',1);
	}
	/*else if(PINA==0X20)
	{
		//MRC implementation
	}*/
	else if(PINA==0X40)
	{
		makeExp('l', 0);
		makeExp('o', 0);
		makeExp('g',1);
	}
	else if(PINA==0X80)
	{
		makeExp('l', 0);
		makeExp('n',1);
	}
	else
	{
		unsigned char mask = 0x0C;
		unsigned char var = PINC;

		var = var&mask;
		if(var == 0x04)
		{
			makeExp('e',1);
		}
		else if(var == 0x08)
		{
			ishyper = !ishyper;
		}
	}
}

void row3()
{
	PORTB=0x10;
	if(PINA==0X01)
	{
		makeExp('/', 0);
		makeExp('1', 0);
		makeExp('0', 0);
		makeExp('0', 1);
	}
	else if(PINA==0X02)
	{
		makeExp('1',1);
	}
	else if(PINA==0X04)
	{
		makeExp('2',1);
	}
	else if(PINA==0X08)
	{
		makeExp('3',1);
	}
	else if(PINA==0X10)
	{
		//Not sure since the "+" button occupies space from both row3 and row4
		makeExp('+',1);
	}
	/*else if(PINA==0X20)
	{
		//M- implementation
		receiveFromMem();
	}*/
	else if(PINA==0X40)
	{
		// 10^x
		makeExp('1', 0);
		makeExp('0', 0);
		makeExp('^',1);
	}
	else if(PINA==0X80)
	{
		makeExp('(',1);
	}
	else
	{
		unsigned char mask = 0x0C;
		unsigned char var = PINC;

		var = var&mask;
		if(var == 0x04)
		{
			makeExp(')',1);
		}
		else if(var == 0x08)
		{
			//x^y
			makeExp('^',1);
		}
	}
}

void row4()
{
	PORTB=0X08;
	if(PINA==0X01)
	{
		makeExp('s', 0);
		makeExp('q', 0);
		makeExp('r', 0);
		makeExp('t',1);
	}
	else if(PINA==0X02)
	{
		makeExp('0',1);
	}
	else if(PINA==0X04)
	{
		makeExp('.',1);
	}
	else if(PINA==0X08)
	{
		//Have to implement equals to '='.
		printRes();
		sendToMem();
	}
	else if(PINA==0X10)
	{
		//Not sure since the "+" button occupies space from both row3 and row4
		makeExp('+',1);
	}
	/*else if(PINA==0X20)
	{
		//M+ implementation
		receiveFromMem();
		
	}*/
	else if(PINA==0X40)
	{
		makeExp('a', 0);
		makeExp('s', 0);
		makeExp('i', 0);
		if(ishyper)
		{
			makeExp('n', 0);
			makeExp('h',1);
		}
		else makeExp('n',1);

	}
	else if(PINA==0X80)
	{
		makeExp('a', 0);
		makeExp('c', 0);
		makeExp('o', 0);
		if(ishyper)
		{
			makeExp('s', 0);
			makeExp('h',1);
		}
		else makeExp('s',1);
	}
	else
	{
		unsigned char mask = 0x0C;
		unsigned char var = PINC;

		var = var&mask;
		if(var == 0x04)
		{
			makeExp('a', 0);
			makeExp('t', 0);
			makeExp('a', 0);
			if(ishyper)
			{
				makeExp('n', 0);
				makeExp('h',1);
			}
			else makeExp('n',1);
		}
		else if(var == 0x08)
		{
			//nCr, not sure whether to implement or not.
		}
	}
}
ISR(USART_RXC_vect)
{
	if(UCSRA&28) PORTB=PORTB|32;
}

ISR(INT2_vect)
{
	receiveFromMem();
}

int main(void)
{
    /* Replace with your application code */
    DDRA = 0x00;
	DDRB = 0xFF;
	//DDRB=0b11110000;
	DDRC = 0b11110011;
	DDRD = 0xFE; //Previously was 0xFF
	
	GICR = 1<<INT2;		//Enabling Interrupt 2
	MCUCSR |= (1 << ISC2);
	
	uart_init();
	stdout = fdevopen(uart_send, NULL);
	stdin = fdevopen(NULL, uart_receive);
	command(0x38);
	command(0x0F);
	command(0x01);
	command(0x80);
	sei();
	while (1)
    {
		row1();
		checkError();
		_delay_ms(MAIN_DELAY);
		checkError();
		row2();
		checkError();
		_delay_ms(MAIN_DELAY);
		checkError();
		row3();
		checkError();
		_delay_ms(MAIN_DELAY);
		checkError();
		row4();
		checkError();
		_delay_ms(MAIN_DELAY);
    }
}

/*
Replace the connections in this way:
(X -> Y means move the connection from pin X to pin Y)
C0 -> C4
C1 -> C5
D0 -> D6
D1 -> D7
*/