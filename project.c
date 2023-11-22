/* LCD Wiring:
 * 1 - GND									2 - Pin 15 &  NUCLEO 5V Source
 * 3 - Potentiometer for Contrast Adjust	4 - D1 / PA_2 (RS)
 * 5 - A1 / PA_1 (R/W)						6 - A0/PA_0 (Enable)
 * 7 - A0 / PA_0 (NOT USED)					8 - A1 / PA_1 (NOT USED)
 * 9 - D1 / PA_2 (NOT USED)					10 - D0 /PA_3 (NOT USED)
 * 11 - A2 / PA_4 (d4)						12 - D13 / PA_5 (d5)
 * 13 - D12 / PA_6 (d6)						14 - D11 / PA_7 (d7)
 * 15 - Pin 2 (VDO)							16 - Nucleo GND
 */

/* LCD Instruction Set:
 * OPCODE = (RS)(R/W)(DB7:0)  - 8 bit length
 * OPCODE = (RS)(R/W) | (DB7:4) | (DB7:4) - 4 bit length (D7-D4 are used twice)
 * Clear  = 0b0000000001
 * Return  = 0b000000001X
 */

volatile unsigned int* GPIOA_MODER =  (unsigned int*) 0x48000000;
volatile unsigned int* GPIOA_ODR =    (unsigned int*) 0x48000014;
volatile unsigned int* RCC_AHB2ENR 	= (unsigned int*) 0x4002104C;
void GPIOA_Init();
void LCD_Enable();
void LCD_Init();
void LCD_Clear();
void LCD_ReturnHome();
void LCD_Write(int, int,int,int,int,int,int,int);
void LCD_Write2(unsigned int);
void LCD_Display(unsigned int);

int main(void){
	GPIOA_Init();
	LCD_Init();
	LCD_Write(0,1,0,0,1,0,0,0); //Write decimal 72 (H)
	LCD_Write(0,1,0,0,1,0,0,1); //Write decimal 73(I)
	LCD_Write2(' ');
	LCD_Write2('W');
	LCD_Write2('O');
	LCD_Write2('R');
	LCD_Write2('L');
	LCD_Write2('D');
	while(1){}
}

/* Enables Pins 0 - 16 as O/P
 * Control Sgnals for the LCD are PA_2, PA_1, PA_0 (RS, R/W, ENABLE)
 * D7-D4 are PA_7-PA_4
 */
void GPIOA_Init(){
	// Enable GPIOA peripheral clock
	*RCC_AHB2ENR |= 1; // *RCC_AHB2ENR = *RCC_AHB2ENR | (1<<0);
	// Set GPIOA PA9-0 as output
	*GPIOA_MODER = (*GPIOA_MODER & ~(0xFFFFF)) | 0x5555;
}

//Assumes RS is PA_2, R/W Select is PA_1, and d7-d4 are PA_7 to PA_4
void LCD_Init(){
	//_____Initilize the LCD to 4 bit length, 2 line, 5x8 Font_________________
	//OPCODE = 0b00001(DL)(N)(F)XX; DL = 0 (4 bits), N = 1 (2 lines), F = 0 (5x8)
	//OPCODE = 0b00 0010 10XX
	//Must send OPCODE 4 bits at a time
	*GPIOA_ODR &= ~(1<<2|1<<1); //RS & R/W = 0

	//D7-D4 = 0010
	*GPIOA_ODR &= ~(1<<7|1<<6|1<<4);
	*GPIOA_ODR |= 1<<5;
	
	LCD_Enable();
	
	//D7-D4 = 0010
	*GPIOA_ODR &= ~(1<<7|1<<6|1<<4);
	*GPIOA_ODR |= 1<<5;
	
	LCD_Enable();

	//D7-D4 = 10XX
	*GPIOA_ODR &= ~(1<<6);
	*GPIOA_ODR |= 1<<7;

	LCD_Enable();

	LCD_Clear();

	LCD_ReturnHome();
	//____________________Turn the entire display on___________________
	//OPCODE = 00 0000 1100

	//RS & R/W = 0
	*GPIOA_ODR &= ~(1<<2|1<<1);

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

//Assumes RS is PA_2, R/W Select is PA_1, and d7-d4 are PA_7 to PA_4
void LCD_Clear(){
	//Clear the display (Send OPCODE = 0b0000000001)
	//OPCODE = 0b 00 0000 0001

	//RS & R/W = 0
	*GPIOA_ODR &= ~(1<<2|1<<1);

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
void LCD_Write(int d7, int d6, int d5, int d4, int l7, int l6, int l5, int l4){
	//OPCODE = 10 BBBB BBBB

	//[RS, R/W] = 10
	*GPIOA_ODR |= (1<<2);
	*GPIOA_ODR &= ~(1<<1);

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
void LCD_Write2(unsigned int asc){
	//OPCODE = 10 BBBB BBBB
	//To get upper 4 bits do c >> 4
	//To get lower 4 bits do c & 0XF
	//To enter into ODR move RS & R/W are in PA_3 and PA_2 

	//[RS, R/W] = 10
	*GPIOA_ODR |= (1<<2);
	*GPIOA_ODR &= ~(1<<1);

	//D7-D4 = d7-d4
	*GPIOA_ODR &= ~(0b1111<<4); //D7-D4 = 0000
	*GPIOA_ODR |= ((asc >> 4)& 0xF); //Takes upper 4 bits of asc

	LCD_Enable();

	*GPIOA_ODR &= ~(0b1111<<4); //0s out D7-D4
	*GPIOA_ODR |= asc & 0xF; // Takes lower 4 bits of asc

	LCD_Enable();
}

void LCD_ReturnHome(){
	//OPCODE = 0b 00 0000 0010

	//RS & R/W = 00
	*GPIOA_ODR &= ~(1<<2|1<<1);

	//D7-D4 = 0000
	*GPIOA_ODR &= ~((1<<7|1<<6|1<<5|1<<4));
	LCD_Enable();

	//D7-D4 = 0010
	*GPIOA_ODR |= 1<<5;
	LCD_Enable();
}

void LCD_Display(unsigned int mode){
	//OPCODE = 00 0000 1100

	//RS & R/W = 0
	*GPIOA_ODR &= ~(1<<2|1<<1);

	//D7-D4 = 0000
	*GPIOA_ODR &= ~((1<<7|1<<6|1<<5|1<<4));
	LCD_Enable();

	//D7-D4 = 1100
	*GPIOA_ODR &= ~(15<<4); //Set D7-D4 to 0000
	*GPIOA_ODR |= (1<<7|(mode & 1)); // Sets D7-D4 to 1[mode]00
	LCD_Enable();
}