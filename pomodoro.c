int main(void){
	
	static int period = 0;
	static int duty = 1500;
	
	//void PWM_Init(unsigned int duty, unsigned int period)
	volatile unsigned int* RCC_AHB2ENR = (unsigned int*) 0x4002104C;
	volatile unsigned int* RCC_APB1ENR1 = (unsigned int*) 0x40021058;
	volatile unsigned int* GPIOB_MODER = (unsigned int*) 0x48000400; //GPIOB, using PB3: Tim2-CH2
	volatile unsigned int* TIM2_ARR = (unsigned int*) 0x4000002C;
	volatile unsigned int* TIM2_CCR1 = (unsigned int*) 0x40000034;
	//volatile unsigned int* GPIOB_AFRL = (unsigned int*) 0x48000020;		//AF1; dont worry using this, for pwm
	//volatile unsigned int* TIM2_CR1 = (unsigned int*) 0x40000000;			
	//volatile unsigned int* TIM2_CCMR1 = (unsigned int*) 0x40000018;
	//volatile unsigned int* TIM2_CCER = (unsigned int*) 0x40000020;	same with all these above
	
	
	// Enable GPIOA peripheral clock
	*RCC_AHB2ENR |= (0x01);

	// Configure PA0 to alt. function mode
	//*GPIOB_MODER = (*GPIOB_MODER | (1 << 1)) & ~(1 << 0);
	//*GPIOA_MODER = (*GPIOA_MODER | (1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 17) | (1 << 19) | (1 << 21) | (1 << 23)) & (~(1 << 0) | ~(1 << 2) | ~(1 << 4) | ~(1 << 6) | ~(1 << 16) | ~(1 << 18) | ~(1 << 20) | ~(1 << 22));
	// Select alternate function for PA0
	//*GPIOB_AFRL |= (1 << 0);

	// Enable TIM2 peripheral clock
	*RCC_APB1ENR1 |= 1;

	// Set auto-reload (period)
	*TIM2_ARR = period;

	// Select PWM mode 1 on channel 1 (OC1)
	//*TIM2_CCMR1 &= ~(1 << 16);
	//*TIM2_CCMR1 = (*TIM2_CCMR1 | (3 << 5)) & ~(1 << 4 );

	// Set duty cycle
	*TIM2_CCR1 = duty;

	// Enable output for channel 1
	//*TIM2_CCER |= 1;

	// Enable counter
	*TIM2_CR1 |= 1;

	
	}
}
