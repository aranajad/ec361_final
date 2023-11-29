#define period 3999999
#define cyclestudy 4

void TIM2_Init(void);
void TIM2_IRQHandler(unsigned int);

int main(void){
	//Set timer variables
	static unsigned int mode = 3;
	
	TIM2_Init(); //Tim2 initialization function
	//mode selector
//	if (mode = 1) {
//		tsec = 1500;  //25 min (1500s)
//	}
//	else if (mode = 2) {
//		tsec = 3000; //25 min (1500s)
//	}
//	else if (mode = 3) {
//		tsec = 25;
//	}
//	
//	while(1){} //TIM2 Interrupt Handler
//}
	
void TIM2_Init(void)
{
	volatile unsigned int* RCC_AHB2ENR = (unsigned int*) 0x4002104C;
	volatile unsigned int* RCC_APB1ENR1 = (unsigned int*) 0x40021058;
	volatile unsigned int* TIM2_ARR = (unsigned int*) 0x4000002C;
	volatile unsigned int* TIM2_CR1 = (unsigned int*) 0x40000000;	
	volatile unsigned int* TIM2_PSC = (unsigned int*) 0x40000028;	
	volatile unsigned int* TIM2_CNT = (unsigned int*) 0x40000024;	
	volatile unsigned int* TIM2_DIER = (unsigned int*) 0x4000000C;	
	volatile unsigned int* NVIC_ISER0 = (unsigned int*) 0xE000E100;	
	//void PWM_Init(unsigned int duty, unsigned int period)
	//volatile unsigned int* TIM2_CCR1 = (unsigned int*) 0x40000034;
	//volatile unsigned int* GPIOB_MODER = (unsigned int*) 0x48000400; //GPIOB, using PB3: Tim2-CH2
	//volatile unsigned int* GPIOB_AFRL = (unsigned int*) 0x48000020;		//AF1; dont worry using this, for pwm	
	//volatile unsigned int* TIM2_CCMR1 = (unsigned int*) 0x40000018;
	//volatile unsigned int* TIM2_CCER = (unsigned int*) 0x40000020;	same with all these above
	
	// Enable GPIOA peripheral clock
	*RCC_AHB2ENR |= (0x01);
	
	//***********
	// Configure PA0 to alt. function mode
	//*GPIOB_MODER = (*GPIOB_MODER | (1 << 1)) & ~(1 << 0);
	//*GPIOA_MODER = (*GPIOA_MODER | (1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 17) | (1 << 19) | (1 << 21) | (1 << 23)) & (~(1 << 0) | ~(1 << 2) | ~(1 << 4) | ~(1 << 6) | ~(1 << 16) | ~(1 << 18) | ~(1 << 20) | ~(1 << 22));
	// Select alternate function for PA0
	//*GPIOB_AFRL |= (1 << 0);			FOR PWM NOT NEEDED

	// Enable TIM2 peripheral clock
	*RCC_APB1ENR1 |= 1;

	// Set auto-reload (period)
	*TIM2_ARR = period;
	
	// Set duty cycle
	//*TIM2_CCR1 = duty;
	
	//set PSC if needed (0 by default)
	*TIM2_PSC = 0;
	
	//***********
	// Select PWM mode 1 on channel 1 (OC1)
	//*TIM2_CCMR1 &= ~(1 << 16);
	//*TIM2_CCMR1 = (*TIM2_CCMR1 | (3 << 5)) & ~(1 << 4 ); FOR PWM NOT NEEDED
	// Enable output for channel 1
	//*TIM2_CCER |= 1;						 FOR PWM NOT NEEDED
	
	//enable up-counting mode: clr tim2_cr1(4) AND enable clock tim2_crl(0);
	*TIM2_CR1 |= (1 << 0);
	
	//Enable capture/compare and update interrupts: TIM2_DIER(1:0) = "11"
	*TIM2_DIER = 3;
	
	//Enable TIM2 interrupts in NVIC: NVIC
	*NVIC_ISER0 |= (1 << 28);
	
	return;
}
	
void TIM2_IRQHandler(unsigned int mode)
{
	volatile unsigned int*TIM2_SR = (unsigned int*) 0x40000010;
	static unsigned int tsec; // 25 min (1500s)
	static unsigned int sec;
	static unsigned int min;
	static unsigned int cycle;
	static unsigned int done = 0;
	static unsigned int brk = 0;
	
	if (mode = 1){
		tsec = 1500;
	}
	else if (mode = 2) {
		tsec = 3000;
	}
	else if (mode = 3){
		tsec = 25;
	}
	
	if ((*(TIM2_SR) & 1) && !done){ //check for counter update when cnt == arr
		tsec--;	//decrement total seconds 
		sec = tsec%60; // calc sec
		min = tsec/60; // calc min
		
		if ((cycle < cyclestudy) && !done){  //number of times cycle should repeat; if number of cycles less than expected number of cycles and not done
		
			if ((tsec == 0) && !brk){ //switch to break timer when 5 min left (300s)
				if (mode == 1){
					tsec = 300; // initialize break time 5 min for mode 1
				}
				else if(mode == 2){
					tsec = 600; // initialize break time 10 min for mode 2
				}				
				else if (mode == 3){ 
				tsec = 5; // initialize break time 5 sec for mode 3 demo
				}	
				brk = 1;
			}
			// WHEN BREAK IS FINISHED*				
			else if ((tsec == 0) && brk) { //timer reaches 0 
				if (mode == 1){
					tsec = 1500; // initialize study time 5 min for mode 1
				}
				else if(mode == 2){
					tsec = 3000; // initialize study time 10 min for mode 2
				}				
				else if (mode == 3){ 
				tsec = 25; // initialize study time 5 sec for mode 3 demo
				}
				cycle++; //update cycle count when timer ends at the end of a cycle
				brk = 0;
				}
		}
		else if ((cycle == cyclestudy) && !done){ //move to long break when expected cycle meets cycles and not done with all cycles
			if (mode == 1){
				tsec = 1200; //20 min break (1200s) for mode 1
			}
			else if (mode == 2){
				tsec = 2400; //40 min break (2400s) for mode 2
			}
			else if (mode == 3){
				tsec = 20; //20 sec break for mode 3 demo
			}
		cycle = 0; //reset cycle counter
		done = 1; // sets "completed all cycles" flag
		}
		//update display to show study timer
		*TIM2_SR = *TIM2_SR & (~(1 << 0)); //clear bit
	}
	return;
}

