/*
Initializing the LCD will require the following steps; 
1. Set 8-bit mode (Rs-D0) = 00001110xx
2. Clear (Rs-D0) = 0000000001
3. Display on (Rs-D0) = 0000001100
*/

/*
Write Relax or Study
1. Set DDRAM(Cursor) Address
	-(Rs-D0) = 0011000101
2. Write "Focus" or "Relax"
  -Focus = Set 1001000110, then enable bit
					 Set 1001101111, then enable bit
					 Set 1001100011, then enable bit
					 Set 1001110101, then enable bit
					 Set 1001110011, then enable bit
	
	-Break = Set 1001000010, then enable bit
					 Set 1001110010, then enable bit
					 Set 1001100101, then enable bit
					 Set 1001100001, then enable bit
					 Set 1001101011, then enable bit
					
Writing the current time 
1. Select 05 position (Rs-D0) = 0010000101
2. Write time
	-If two digits, write 10's Minute Place (Rs-D0) = 10(
	-Write
*/
/*
void LCD_init(void);
void LCD_print(int,int,int,int);
*/

int main(void)
{
	volatile unsigned int* RCC_AHB2ENR = (unsigned int *) 0x4002104C;
	volatile unsigned int* GPIOA_BASE = (unsigned int *) 0x48000000;
	volatile unsigned int* GPIOA_MODER = (unsigned int *) 0x48000000;
	volatile unsigned int* GPIOA_ODR = (unsigned int *)0x48000014;
	// Enable GPIOA peripheral clock
	*RCC_AHB2ENR |= 1; // *RCC_AHB2ENR = *RCC_AHB2ENR | (1<<0);
	// Set GPIOA PA9-0 as output
	*GPIOA_MODER = (*GPIOA_MODER & ~(0xFFFFF)) | 0x55555;
	
	
	// Initialize LCD ------------------------
	//1. Set 8-bit mode (Rs-D0) = 00001110xx
	*GPIOA_ODR = (*GPIOA_ODR & ~(0x3FF)) | (7 << 3);
  //2. Clear (Rs-D0) = 0000000001
	*GPIOA_ODR = (*GPIOA_ODR & ~(7 << 3)) | 1;
  //3. Display on (Rs-D0) = 0000001100
	*GPIOA_ODR = (*GPIOA_ODR & ~(1)) | (3 << 2);
	
	// Enter 25 
	*GPIOA_ODR = (*GPIOA_ODR & ~(3 << 2)) | (1 << 9) | 50; // 1000110010
	*GPIOA_ODR = (*GPIOA_ODR & ~(1 << 1)) | 5 | 53; // 1000110101
	*GPIOA_ODR = *GPIOA_ODR | 53; // 1000110101
	
}
/*
void LCD_init(void)
{
	
}
*/















/*
ASCII Number Values
0 = 00110000
1 = 00110001
2 = 00110010
3 = 00110011
4 = 00110100
5 = 00110101
6 = 00110110
7 = 00110111
8 = 00111000
9 = 00111001
: = 00111010
*/