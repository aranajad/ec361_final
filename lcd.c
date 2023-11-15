/* LCD Wiring:
 * 1 - GND									2 - Pin 15 &  NUCLEO 5V Source
 * 3 - Potentiometer for Contrast Adjust	4 - D8 / PA_9 (RS)
 * 5 - D7 / PA_8 (R/W)						6 - RC Enable Ckt
 * 7 - A0 / PA_0 (d0)						8 - A1 / PA_1 (d1)
 * 9 - D1 / PA_2 (d2)						10 - D0 /PA_3 (d3)
 * 11 - A2 / PA_4 (d4)						12 - D13 / PA_5 (d5)
 * 13 - D12 / PA_6 (d6)						14 - D11 / PA_7 (d7)
 * 15 - Pin 2 (VDO)							16 - Nucleo GND
 */

/* LCD Instruction Set:
 * OPCODE = (RS)(R/W)(DB7:0)  
 * Clear  = 0b0000000001
 * Return  = 0b000000001X
 */

volatile unsigned int* GPIOA_MODER =  (unsigned int*) 0x48000000;
volatile unsigned int* GPIOA_ODR =    (unsigned int*) 0x48000014;
volatile unsigned int* RCC_AHB2ENR 	= (unsigned int*) 0x4002104C;
void GPIOA_Init();
void LCD_Init();
void LCD_Clear();
void LCD_Display(unsigned char D, unsigned char C, unsigned char B);

int main(void){
	while(1){}
}

void GPIOA_Init(){
	*RCC_AHB2ENR |= 1; //Enable GPIOA Clock

}


void LCD_Init(){
	//Initilize the LCD to 8 bit length, 2 line, 5x8 Font 
	//OPCODE = 0b00001(DL)(N)(F)XX; DL = 1 (8 bits), N = 1 (2 lines), F = 0 (5x8)
	//OPCODE = 0b00001110XX
	*GPIOA_ODR &= ~(1<<9|1<<8|1<<7|1<<6|1<<2); //RS, R/W, d7-d6, d2 = 0
	*GPIOA_ODR |= (1<<3|1<<4|1<<5); //d5-d3 = 1

	//Turn the entire display on
	*GPIOA_ODR &= ~(1<<9|1<<8|1<<7|1<<6|1<<5|1<<4); //RS, R/W, d7-d4 = 0
	*GPIOA_ODR |= (1<<3|1<<2);//|finish)
	LCD_Clear();
}

void LCD_Display(unsigned char D, unsigned char C, unsigned char B){
	*GPIOA_ODR &= ~(1<<9|1<<8|1<<7|1<<6|1<<5|1<<4); //RS, R/W, d7-d4 = 0
	*GPIOA_ODR |= (1<<3);


}

void LCD_Clear(){
	//Clear the display (Write 0b0000000001)
	volatile unsigned int* GPIOA_ODR = GPIOA_MODER + 0x14;
	*GPIOA_ODR &= ~(0b1111111 << 1); //Sets d7-d1 to 0
	*GPIOA_ODR |= 1; //Sets d0 to 1
	*GPIOA_ODR &= ~(1 << 9 | 1 << 8); //Sets RS (PA_9) and R/W (PA_8) to 0
}
