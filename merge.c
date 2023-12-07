#define period 3999999
#define cyclestudy 4
#define ON (unsigned char)1
#define OFF (unsigned char)0

// tones = {261, 277, 294, 311, 330, 349, 370, 392, 415, 440} for {C, C#,D,D#,E,F,F#,G,G#,A}
#define C (unsigned char)261
#define sharpC (unsigned char)277
#define D (unsigned char)294
#define sharpD (unsigned char)311
#define E (unsigned char)330
#define F (unsigned char)349
#define sharpF (unsigned char)370
#define G (unsigned char)392
#define sharpG (unsigned char)415
#define A (unsigned char)440
// tones = {261, 277, 294, 311, 330, 349, 370, 392, 415, 440} for {C, C#,D,D#,E,F,F#,G,G#,A}

extern void GPIO_Init();
extern void LCD_SetControlBits(unsigned char, unsigned char);
extern void LCD_Enable();
extern void LCD_Init();
extern void LCD_Clear();
extern void LCD_ReturnHome();
extern void LCD_WriteBits(int, int, int, int, int, int, int, int);
extern void LCD_WriteChar(unsigned char);
extern void LCD_WriteStr(char *, int);
extern void LCD_SetAddress(unsigned char);
extern void print_time(unsigned int time);
extern void print_mode(int mode);
extern int setMode();

void playBuzzer(unsigned int freq, unsigned char on_off);
void playTune(unsigned int f);
void pauseButton();
void TIM2_Init(void);
void TIM3_Init(void);
void TIM2_IRQHandler(void);
void TIM2_Enable(unsigned char a);
static unsigned int mode = 1;
static unsigned int start = 0;
static unsigned int paused = 0;

int main(void){
	GPIO_Init();
	LCD_Init();
	TIM3_Init();
	mode = setMode();
	playTune(C);
	playTune(sharpC);
	playTune(D);
	playTune(sharpD);
	playTune(E);
	playTune(F);
	playTune(sharpF);
	playTune(G);
	playTune(sharpG);
	playTune(A);
//	while(1){}
	
	TIM2_Init();
	LCD_Clear();
	while (1){
		/*
		if(done){
			mode = setMode();
		}
		*/
/* 
		if (paused){
			pauseButton();
			paused = 0;
		}
		 */
	}
}

void TIM2_Init(void){
	volatile unsigned int *RCC_AHB2ENR = (unsigned int *)0x4002104C;
	volatile unsigned int *RCC_APB1ENR1 = (unsigned int *)0x40021058;
	volatile unsigned int *TIM2_ARR = (unsigned int *)0x4000002C;
	volatile unsigned int *TIM2_CR1 = (unsigned int *)0x40000000;
	volatile unsigned int *TIM2_PSC = (unsigned int *)0x40000028;
	volatile unsigned int *TIM2_CNT = (unsigned int *)0x40000024;
	volatile unsigned int *TIM2_DIER = (unsigned int *)0x4000000C;
	volatile unsigned int *NVIC_ISER0 = (unsigned int *)0xE000E100;
	// void PWM_Init(unsigned int duty, unsigned int period)
	// volatile unsigned int* TIM2_CCR1 = (unsigned int*) 0x40000034;
	// volatile unsigned int* GPIOB_MODER = (unsigned int*) 0x48000400; //GPIOB, using PB3: Tim2-CH2
	// volatile unsigned int* GPIOB_AFRL = (unsigned int*) 0x48000020;		//AF1; dont worry using this, for pwm
	// volatile unsigned int* TIM2_CCMR1 = (unsigned int*) 0x40000018;
	// volatile unsigned int* TIM2_CCER = (unsigned int*) 0x40000020;	same with all these above

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

	// set PSC if needed (0 by default)
	*TIM2_PSC = 0;

	//***********
	// Select PWM mode 1 on channel 1 (OC1)
	//*TIM2_CCMR1 &= ~(1 << 16);
	//*TIM2_CCMR1 = (*TIM2_CCMR1 | (3 << 5)) & ~(1 << 4 ); FOR PWM NOT NEEDED
	// Enable output for channel 1
	//*TIM2_CCER |= 1;						 FOR PWM NOT NEEDED

	// enable up-counting mode: clr tim2_cr1(4) AND enable clock tim2_crl(0);
	*TIM2_CR1 |= (1 << 0);

	// Enable capture/compare and update interrupts: TIM2_DIER(1:0) = "11"
	*TIM2_DIER = 3;

	// Enable TIM2 interrupts in NVIC: NVIC
	*NVIC_ISER0 |= (1 << 28);

	return;
}

void TIM2_IRQHandler(){
	volatile unsigned int *TIM2_SR = (unsigned int *)0x40000010;
	static unsigned int tsec; // 25 min (1500s)
	static unsigned int sec;
	static unsigned int min;
	static unsigned int cycle;
	static unsigned int done = 0;
	static unsigned int brk;
	static unsigned int finlb;

	// deal with start refresh**
	if (!start){
		if (mode == 1){
			tsec = 1500;
		}
		else if (mode == 2){
			tsec = 3000;
		}
		else if (mode == 3){
			tsec = 10;
		}
		start = 1; // set start flag so tsec is not initiated again
		paused = 0;
	}
	if (!done){
		if ((*(TIM2_SR) & 1)){ // check for counter update when cnt == arr //flip done and have everything else in else**
			tsec--;
			sec = tsec % 60; // calc sec
			min = tsec / 60; // calc min

			if ((cycle < cyclestudy)){ // number of times cycle should repeat; if number of cycles less than expected number of cycles and not done

				if ((tsec == 0) && !brk){ // switch to break timer when 5 min left (300s)
					if (mode == 1){
						tsec = 300; // initialize break time 5 min for mode 1
					}
					else if (mode == 2){
						tsec = 600; // initialize break time 10 min for mode 2
					}
					else if (mode == 3){
						tsec = 5; // initialize break time 5 sec for mode 3 demo
					}
					brk = 1;
					TIM2_Enable(OFF);
					playTune(F);
				}
				// WHEN BREAK IS FINISHED*
				else if ((tsec == 0) && brk){ // timer reaches 0 and we are taking a break
					if (mode == 1){
						tsec = 1500; // initialize study time 5 min for mode 1
					}
					else if (mode == 2){
						tsec = 3000; // initialize study time 10 min for mode 2
					}
					else if (mode == 3){
						tsec = 10; // initialize study time 5 sec for mode 3 demo
					}
					cycle++; // update cycle count when timer ends at the end of a cycle
					if (cycle == cyclestudy) {
						finlb = 1;
					}
					brk = 0;
					//TIM2_Enable(OFF);
					//playTune(F);
				}
				print_mode(!brk);
			}
			else if ((cycle == cyclestudy) && finlb){ // move to long break when expected cycle meets cycles and not done with all cycles
				if (mode == 1){
					tsec = 1200; // 20 min break (1200s) for mode 1
				}
				else if (mode == 2){
					tsec = 2400; // 40 min break (2400s) for mode 2
				}
				else if (mode == 3){
					tsec = 20; // 20 sec break for mode 3 demo
				}
				finlb = 0;
			}
			else if ((tsec == 0) && !finlb){
				done = 1;  // sets "completed all cycles" flag
				cycle = 0; // reset cycle counter
				start = 0; // reset start flag
			}

			// update display to show study timer
			print_time(tsec);
			*TIM2_SR = *TIM2_SR & (~(1 << 0)); // clear bit (Acknowledge the interrupt flag)
		}
	}
	return;
}

/* Passing ON (1) -> TIM2 is Enabled
 * Passing OFF (0) -> TIM2 is Disabled
 */
void TIM2_Enable(unsigned char on_off){
	volatile unsigned int *TIM2_CR1 = (unsigned int *)0x40000000;
	if (on_off){
		*TIM2_CR1 |= (on_off << 0);
	}
	else{
		*TIM2_CR1 &= (on_off << 0);
	}
}

/* Generates are square wave (50% duty cycle PWM) on TIM3 Channel 2 (PWM3/2)
 * PWM3/2 maps to D4 (PB_5)
 * Attached to a buzzer on the board. Frequency of square wave determines pitch of buzzer
 * Desired frequency can range from 60Hz to 4MHz (w/ PSC = 0)
 * tones = {261, 277, 294, 311, 330, 349, 370, 392, 415, 440} for {C,C#,D,D#,E,F,F#,G,G#,A}
 * C - 261
 * sharpC - 277
 * D - 294
 * sharpD - 311
 * E - 330
 * F - 349
 * sharpF - 370
 * G - 392
 * sharpG - 415
 * A - 440
 */
void playBuzzer(unsigned int desired_freq, unsigned char on_off){
	unsigned int calcARR = (unsigned int)((4e6) / desired_freq) - 1;
	unsigned int calcDuty = (unsigned int)calcARR / 2;
	volatile unsigned int *TIM3_ARR = (unsigned int *)0x4000042C;
	volatile unsigned int *TIM3_CCR1 = (unsigned int *)0x40000434;
	volatile unsigned int *TIM3_CCER = (unsigned int *)0x40000420; // same with all these above

	*TIM3_CCER &= ~(1 << 4); // Sets CC2E (Capture/Computer 2 Enable) to 0 (to change TIM3 settings)

	// EQN for determining ARR
	// ARR = (CLK_Freq/Desire_Freq) - 1
	// ARR = (4e6/Desired_Freq)-1
	// TIM3_ARR = (unsigned int) ((4e6)/[something])- 1; //261 is a Middle C
	*TIM3_ARR = calcARR;

	// Set duty cycle
	*TIM3_CCR1 = calcDuty;

	// Enable output for channel 2
	*TIM3_CCER &= ~(1 << 4);   // Sets CC2E (Capture/Compare 2 Enable) to 0
	*TIM3_CCER |= on_off << 4; // Sets CC2E (Capture/Compare 2 Enable) to 1
}

void TIM3_Init(){

	volatile unsigned int *RCC_AHB2ENR = (unsigned int *)0x4002104C;
	volatile unsigned int *RCC_APB1ENR1 = (unsigned int *)0x40021058;
	volatile unsigned int *TIM3_ARR = (unsigned int *)0x4000042C; //
	volatile unsigned int *TIM3_CR1 = (unsigned int *)0x40000400; // GOod
	volatile unsigned int *TIM3_PSC = (unsigned int *)0x40000428; //
	volatile unsigned int *TIM3_CNT = (unsigned int *)0x40000424; //
	// void PWM_Init(unsigned int duty, unsigned int period)
	volatile unsigned int *TIM3_CCR2 = (unsigned int *)0x40000438;
	volatile unsigned int *GPIOB_MODER = (unsigned int *)0x48000400; // GPIOB, using PB5: Tim3-CH2
	volatile unsigned int *GPIOB_AFRL = (unsigned int *)0x48000420;	 // AF2;
	volatile unsigned int *TIM3_CCMR1 = (unsigned int *)0x40000418;
	volatile unsigned int *TIM3_CCER = (unsigned int *)0x40000420; // same with all these above

	// Enable GPIOB peripheral clock
	*RCC_AHB2ENR |= (1 << 1);

	// Enable TIM3 peripheral clock
	*RCC_APB1ENR1 |= 1 << 1;

	//***********
	// Configure PB_5 to alt. function mode
	*GPIOB_MODER &= ~(0b11 << 10); // Zero out MODE5
	*GPIOB_MODER |= (0b10 << 10);  // Mode 5 = 10 (Alt Func Mode)

	*GPIOB_AFRL &= ~(0b1111 << 20); // Zero AFSEL5 (AF2 = TIM3/2)
	*GPIOB_AFRL |= (0b0010 << 20);	// Set AFSEL5 to 0010 (AF2 = TIM3/2)

	// Enable OC2PE bit
	*TIM3_CCMR1 |= (1 << 11);

	// Set ARR to upcounting via ARPE in TIM3_CR1
	*TIM3_CR1 |= (1 << 7);

	// Generate PWM via CMS bits in TIM3_CR1
	*TIM3_CR1 &= ~(1 << 6 | 1 << 5);

	// 1 for Upcount & 0 for downcount (DIR bit)
	//*TIM3_CR1 &= ~(1<<4);  //Upcount
	*TIM3_CR1 |= (1 << 4); // Downcount

	// Set OC2 Polarity w/ CC2P bit
	*TIM3_CCER &= ~(1 << 5);

	// set PSC if needed (0 by default)
	*TIM3_PSC = 0;

	// EQN for determining ARR
	// ARR = (CLK_Freq/Desire_Freq) - 1
	// ARR = (4e6/Desired_Freq)-1
	// TIM3_ARR = (unsigned int) ((4e6)/[something])- 1; //261 is a Middle C
	*TIM3_ARR |= 15000;

	// Set duty cycle
	*TIM3_CCR2 |= 15000 / 2;

	// Select PWM mode 1 on channel 2 (OC2M = 0110)
	*TIM3_CCMR1 &= ~(1 << 24);
	*TIM3_CCMR1 &= ~(0b111 << 12);
	*TIM3_CCMR1 |= (0b110 << 12);

	//*TIM3_CCER |= 1<<4;
	// enable up-counting mode: clr tim2_cr1(4) AND enable clock tim2_crl(0);
	*TIM3_CR1 |= (1 << 0);
	return;
}

/*tones = {261, 277, 294, 311, 330, 349, 370, 392, 415, 440} for {C,C#,D,D#,E,F,F#,G,G#,A}
 * C - 261
 * sharpC - 277
 * D - 294
 * sharpD - 311
 * E - 330
 * F - 349
 * sharpF - 370
 * G - 392
 * sharpG - 415
 * A - 440
 */
void playTune(unsigned int freq){
	playBuzzer(freq, ON);
	for (int i = 0; i < 2e4; i++){}
	playBuzzer(freq, OFF);
	return;
}
void pauseButton(){
	volatile unsigned int *GPIOC_IDR = (unsigned int *)0x48000810;
	while (!(*GPIOC_IDR & (1 << 12))){
		playTune(F);
	}
	TIM2_Enable(ON);
}
