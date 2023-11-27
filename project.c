/* LCD Wiring:
 * 1 - GND									2 - Pin 15 &  NUCLEO 5V Source
 * 3 - Potentiometer for Contrast Adjust	4 - A4 / PC_1 (RS)
 * 5 - A5 / PC_0 (R/W)						6 - A0/PA_0 (Enable)
 * 7 - A0 / PA_0 (NOT USED)					8 - A1 / PA_1 (NOT USED)
 * 9 - D1 / PA_2 (NOT USED)					10 - D0 /PA_3 (NOT USED)
 * 11 - A2 / PA_4 (d4)						12 - D13 / PA_5 (d5)
 * 13 - D12 / PA_6 (d6)						14 - D11 / PA_7 (d7)
 * 15 - Pin 2 (VDO)							16 - Nucleo GND
 */

/* LCD Instruction Set:
 * OPCODE = (RS)(R/W)(DB7:0)  - 8 bit length
 * OPCODE = (RS)(R/W) | (DB7:4) | (DB7:4) - 4 bit length (D7-D4 are used twice)
 * OPCODE = PC_1 | PC_0 | PA_7:4 | PA_7:4
 * Clear  = 0b0000000001
 * Return  = 0b000000001X
 */
#define ON (unsigned char) 1
#define OFF (unsigned char) 0
//unsigned char on = 1;
//unsigned char off = 0;

volatile unsigned int* GPIOA_MODER =  (unsigned int*) 0x48000000;
volatile unsigned int* GPIOC_MODER =  (unsigned int*) 0x48000800;
volatile unsigned int* GPIOA_ODR =    (unsigned int*) 0x48000014;
volatile unsigned int* GPIOC_ODR =    (unsigned int*) 0x48000814;
volatile unsigned int* RCC_AHB2ENR 	= (unsigned int*) 0x4002104C;
void GPIO_Init();
void LCD_SetControlBits(unsigned char, unsigned char);
void LCD_Enable();
void LCD_Init();
void LCD_Clear();
void LCD_ReturnHome();
void LCD_WriteBits(int, int,int,int,int,int,int,int);
void LCD_WriteChar(unsigned char);
void LCD_WriteStr(char*, int);
void LCD_SetAddress(unsigned char);
void LCD_Display(unsigned char D, unsigned char C, unsigned char B);

int main(void){
	GPIO_Init();
	LCD_Init();
	LCD_Clear();
	LCD_WriteBits(1,1,1,1,1,1,1,1);
	LCD_WriteChar((unsigned char) 0xF);
	LCD_WriteChar((unsigned char) 0xF);
	LCD_WriteChar((unsigned char) 0xF);
 	LCD_WriteBits(0,1,0,0,1,0,0,0); //Write decimal 72 (H)
	LCD_WriteChar('I');
	LCD_WriteChar(' ');
	LCD_WriteChar((unsigned char) 0xF);
	LCD_WriteStr("WORLD",5);
	LCD_WriteChar((unsigned char) 0xF);
	LCD_WriteChar((unsigned char) 0xF);
	LCD_WriteChar((unsigned char) 0xF);
	LCD_WriteStr("aaaa",4);
	LCD_SetAddress(0x40);
	LCD_WriteStr("DDDDDDDD",8);
	while(1){}
}

/* Enables Pins 0 - 16 as O/P
 * Control Sgnals for the LCD are PC_1, PC_0, PA_0 (RS, R/W, ENABLE)
 * D7-D4 are PA_7-PA_4
 */
void GPIO_Init(){
	// Enable GPIOA peripheral clock
	*RCC_AHB2ENR |= (1|1<<2); //Turns off GPIOA & GPIOC
	// Set GPIOA PA9-0 as output
	*GPIOA_MODER = (*GPIOA_MODER & ~(0xFFFFF)) | 0x5555;
	*GPIOC_MODER = (*GPIOA_MODER & ~(0xF)) | 0x5;
}

//Assumes RS is PA_2, R/W Select is PA_1, and d7-d4 are PA_7 to PA_4
void LCD_Init(){
	//_____Initilize the LCD to 4 bit length, 2 line, 5x8 Font_________________
	//OPCODE = 0b00001(DL)(N)(F)XX; DL = 0 (4 bits), N = 1 (2 lines), F = 0 (5x8)
	//OPCODE = 0b00 0010 10XX

	//***FIRST INSTRUCTION IS 8 BITS
	LCD_SetControlBits(OFF,OFF);

	//D7-D4 = 0010 & D3-D0 = 0000 (Shorted on board) 
	*GPIOA_ODR &= ~(1<<7|1<<6|1<<4);
	*GPIOA_ODR |= 1<<5;
	
	//OPCODE = 00 0010 0000 (4 bits, 1 line, 5x8 Font)
	LCD_Enable();
	
	//Must send OPCODE 4 bits at a time
	//D7-D4 = 0010
	*GPIOA_ODR &= ~(1<<7|1<<6|1<<4);
	*GPIOA_ODR |= 1<<5;
	LCD_Enable();

	//D7-D4 = 11XX
	//*GPIOA_ODR &= ~(1<<6);
	*GPIOA_ODR |= 1<<7;
	*GPIOA_ODR |= 1<<6;

	//OPCODE = 00 0010 11XX (4 bits, 2 line, 5x11 font)
	LCD_Enable();

	LCD_Clear();
	LCD_ReturnHome();
/* 
	//__________Clear the LCD Display_________________________________
	//OPCODE = 0b 00 0000 0001

	//RS & R/W = 0
	*GPIOC_ODR &= ~(1<<1|1<<0);

	//D7-D4 = 0000
	*GPIOA_ODR &= ~((1<<7|1<<6|1<<5|1<<4));
	LCD_Enable();

	//D7-D4 = 0001
	*GPIOA_ODR |= 1<<4;
	LCD_Enable();

	//___________________Return______Home______________________
	//OPCODE = 0b 00 0000 0010

	//RS & R/W = 0
	*GPIOC_ODR &= ~(1<<1|1<<0);

	//D7-D4 = 0000
	*GPIOA_ODR &= ~((1<<7|1<<6|1<<5|1<<4));
	LCD_Enable();

	//D7-D4 = 0010
	*GPIOA_ODR |= 1<<5;
	LCD_Enable();
 */
	//____________________Turn the entire display on___________________
	//OPCODE = 00 0000 1100

	//RS & R/W = 0
	LCD_SetControlBits(OFF, OFF);

	//D7-D4 = 0000
	*GPIOA_ODR &= ~((1<<7|1<<6|1<<5|1<<4));
	LCD_Enable();

	//D7-D4 = 1100
	*GPIOA_ODR |= (1<<7|1<<6);
	LCD_Enable();
}

void LCD_Enable(){
	//Set PA_0 = 1
	*GPIOA_ODR |= 1; 

	//Busy wait (1mS)
	for(int i = 0; i < 4003;i++){}

	//Set PA_0 = 0
	*GPIOA_ODR &= ~1; 
	
}

void LCD_ReturnHome(){
	//OPCODE = 0b 00 0000 0010

	//RS & R/W = 00
	LCD_SetControlBits(OFF, OFF);

	//D7-D4 = 0000
	*GPIOA_ODR &= ~((1<<7|1<<6|1<<5|1<<4));
	LCD_Enable();

	//D7-D4 = 0010
	*GPIOA_ODR |= 1<<5;
	LCD_Enable();
}

void LCD_Clear(){
	//Clear the display (Send OPCODE = 0b0000000001)
	//OPCODE = 0b 00 0000 0001

	//RS & R/W = 00
	LCD_SetControlBits(OFF, OFF);

	//D7-D4 = 0000
	*GPIOA_ODR &= ~((1<<7|1<<6|1<<5|1<<4));
	LCD_Enable();

	//D7-D4 = 0001
	*GPIOA_ODR |= 1<<4;
	LCD_Enable();
}

/* Generates an OPCODE where:
 * RS = 1, R/W = 0
 * d7-d4 are the first 4 bits set for the OPCODE
 * l7-l4 are the lower 4 bits set for the OPCODE
 * 
 * OPCODE = 10 | (d7-d4) | (l7-l4)
 * Assumes RS is PA_2, R/W Select is PA_1, and d7-d4 are PA_7 to PA_4
 */
void LCD_WriteBits(int d7, int d6, int d5, int d4, int l7, int l6, int l5, int l4){
	//OPCODE = 10 BBBB BBBB

	//[RS, R/W] = 10
	LCD_SetControlBits(ON,OFF);

	//D7-D4 = d7-d4

	//if d7 = 1 then set 7th bit in ODR to 1 else set it to 0
	if(d7){
		*GPIOA_ODR |= (d7<<7);
	} else {
		*GPIOA_ODR &= ~(1<<7);
	}

	if(d6){
		*GPIOA_ODR |= (d6<<6);
	} else {
		*GPIOA_ODR &= ~(1<<6);
	}

	if(d5){
		*GPIOA_ODR |= (d5<<5);
	} else {
		*GPIOA_ODR &= ~(1<<5);
	}

	if(d4){
		*GPIOA_ODR |= (d4<<4);
	} else {
		*GPIOA_ODR &= ~(1<<4);
	}

	LCD_Enable();

	//D7-D4 = l7-l4
	if(l7){
		*GPIOA_ODR |= (l7<<7);
	} else {
		*GPIOA_ODR &= ~(1<<7);
	}

	if(l6){
		*GPIOA_ODR |= (l6<<6);
	} else {
		*GPIOA_ODR &= ~(1<<6);
	}

	if(l5){
		*GPIOA_ODR |= (l5<<5);
	} else {
		*GPIOA_ODR &= ~(1<<5);
	}

	if(l4){
		*GPIOA_ODR |= (l4<<4);
	} else {
		*GPIOA_ODR &= ~(1<<4);
	}

	LCD_Enable();
}

/* Generates an OPCODE where:
 * RS = 1, R/W = 0
 * d7-d4 are the upper 4 bits of the ASCII value
 * l7-l4 are the lower 4 bits of the ASCII value
 * 
 * OPCODE = 10 | (d7-d4) | (l7-l4)
 * Assumes RS is PA_2, R/W Select is PA_1, and d7-d4 are PA_7 to PA_4
 */
void LCD_WriteChar(unsigned char asc){
	//OPCODE = 10 BBBB BBBB
	//To get upper 4 bits do c >> 4
	//To get lower 4 bits do c & 0XF
	//To enter into ODR move RS & R/W are in PA_3 and PA_2 

	//[RS, R/W] = 10
	LCD_SetControlBits(ON,OFF);

	//D7-D4 = d7-d4
	*GPIOA_ODR &= ~(15<<4); //0s out D7-D4
	*GPIOA_ODR |= (asc>>4)<<4; //Takes upper 4 bits of asc

	LCD_Enable();

	*GPIOA_ODR &= ~(15<<4); //0s out D7-D4
	*GPIOA_ODR |= ((asc & 0xF)<<4); // Takes lower 4 bits of asc

	LCD_Enable();
}

void LCD_WriteStr(char* str,int len){
	for(int i=0; i<len;i++){
		LCD_WriteChar(str[i]);
	}
}

void LCD_SetAddress(unsigned char addr){
	//OPCODE = 00 1BBB BBBB
	//To get upper 1 bits do addr >> 4
	//To get lower 4 bits do addr & 0XF

	//[RS, R/W] = 00
	LCD_SetControlBits(OFF,OFF);

	//D7-D4 = d7-d4
	*GPIOA_ODR &= ~(15<<4); //0s out D7-D4
	*GPIOA_ODR |= (addr>>4)<<4; //Takes upper 4 bits of asc
	*GPIOA_ODR |= 1<<7; //Sets D7 to 1

	LCD_Enable();

	*GPIOA_ODR &= ~(15<<4); //0s out D7-D4
	*GPIOA_ODR |= ((addr & 0xF)<<4); // Takes lower 4 bits of asc

	LCD_Enable();
}
/* Sets the control bits RS and R_W
 * First argument - RS (PC_1)
 * Second argument - R_W (PC_0)
 */
void LCD_SetControlBits(unsigned char RS, unsigned char R_W){
	*GPIOC_ODR &= ~(1<<0|1<<1); //0s out the 2 bits

	*GPIOC_ODR |= (RS & 1) << 1; //PC_1
	*GPIOC_ODR |= ((R_W & 1)) << 0; //PC_0
}