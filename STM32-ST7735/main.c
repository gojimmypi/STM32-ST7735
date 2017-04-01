#include <stm32f10x_gpio.h>
#include <stm32f10x_tim.h> // timer
#include <stm32f10x_rcc.h>
#include <stm32f10x_spi.h>
#include <misc.h> // interrupts

#include <delay.h>
#include <st7735.h>
#include <garmin-digits.h>

// ST7735 Display Driver from https://github.com/LonelyWolf/stm32/tree/master/ST7735

void ST7735_BigDig(uint8_t digit, uint16_t X, uint16_t Y, uint16_t color) {
	uint8_t i,j;
    uint8_t CH = color >> 8;
    uint8_t CL = (uint8_t)color;

	CS_L();
	ST7735_AddrSet(X,Y,X + 15,Y + 43);
	A0_H();
	for (j = 0; j < 44; j++) {
		for (i = 0; i < 16; i++) {
			if ((garmin_big_digits[(digit * 96) + i + (j / 8) * 16] >> (j % 8)) & 0x01) {
    			ST7735_write(CH);
    			ST7735_write(CL);
			} else {
    			ST7735_write(0x00);
    			ST7735_write(0x00);
			}
		}
	}
	CS_H();
}

void ST7735_MidDig(uint8_t digit, uint16_t X, uint16_t Y, uint16_t color) {
	uint8_t i,j;
    uint8_t CH = color >> 8;
    uint8_t CL = (uint8_t)color;

	CS_L();
	ST7735_AddrSet(X,Y,X + 11,Y + 23);
	A0_H();
	for (j = 0; j < 24; j++) {
		for (i = 0; i < 12; i++) {
			if ((garmin_mid_digits[(digit * 36) + i + (j / 8) * 12] >> (j % 8)) & 0x01) {
    			ST7735_write(CH);
    			ST7735_write(CL);
			} else {
    			ST7735_write(0x00);
    			ST7735_write(0x00);
			}
		}
	}
	CS_H();
}

void ST7735_SmallDig(uint8_t digit, uint16_t X, uint16_t Y, uint16_t color) {
	uint8_t i,j;
    uint8_t CH = color >> 8;
    uint8_t CL = (uint8_t)color;

	CS_L();
	ST7735_AddrSet(X,Y,X + 10,Y + 20);
	A0_H();
	for (j = 0; j < 21; j++) {
		for (i = 0; i < 11; i++) {
			if ((garmin_small_digits[(digit * 33) + i + (j / 8) * 11] >> (j % 8)) & 0x01) {
    			ST7735_write(CH);
    			ST7735_write(CL);
			} else {
    			ST7735_write(0x00);
    			ST7735_write(0x00);
			}
		}
	}
	CS_H();
}


void InitializeTimer()
{
	// see ARM Timer tutorial https://visualgdb.com/tutorials/arm/stm32/timers/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
 
	TIM_TimeBaseInitTypeDef timerInitStructure; 
	timerInitStructure.TIM_Prescaler = 12000; // the main clock is 72,000,000 so we'll prescale to 72,000,000 / 12,000 = 6,000 Hz
	timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStructure.TIM_Period = 6000;     // of the 6,000 Hz scaled clock, we actually only want 1/6000th of that (once per second)
	timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &timerInitStructure);
	TIM_Cmd(TIM2, ENABLE);
	
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); // enable timter interrupt updates

}

void InitializeLED()
{
	GPIO_InitTypeDef GPIO_InitStructure;
  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void EnableTimerInterrupt()
{
	NVIC_InitTypeDef nvicStructure;
	nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
	nvicStructure.NVIC_IRQChannelSubPriority = 1;
	nvicStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStructure);
}

int hours = 5, minutes = 48, seconds = 0;



void timeToDigits()
{
	int hour10, hour1, minute10, minute1, second10, second1;

	if (hours >= 10)
	{
		hour10 = 1;
		hour1 = hours - 10;
	}
	else
	{
		hour10 = 0;
		hour1 = hours;
	}
	minute10 = 0;
	minute1 = 0;

	int tmp_min = minutes;
	while (tmp_min >= 10)
	{
		minute10++;
		tmp_min = tmp_min - 10;
	}
	minute1 = tmp_min;

	second10 = 0;
	second1 = 0;
	int tmp_sec = seconds;
	while (tmp_sec >= 10)
	{
		second10++;
		tmp_sec = tmp_sec - 10;
	}
	second1 = tmp_sec;
	
	ST7735_BigDig(hour10, 0, 35, RGB565(212, 246, 190));
	ST7735_BigDig(hour1, 16, 35, RGB565(212, 246, 190));
	ST7735_FillRect(33, 42, 35, 44, RGB565(177, 211, 190));
	ST7735_FillRect(33, 67, 35, 69, RGB565(177, 211, 190));
	ST7735_BigDig(minute10, 37, 35, RGB565(212, 246, 190));
	ST7735_BigDig(minute1, 53, 35, RGB565(212, 246, 190));
	ST7735_FillRect(70, 42, 72, 44, RGB565(177, 211, 190));
	ST7735_FillRect(70, 67, 72, 69, RGB565(177, 211, 190));
	ST7735_BigDig(second10, 75, 35, RGB565(212, 246, 190));
	ST7735_BigDig(second1, 91, 35, RGB565(212, 246, 190));
}

void timeIncrement()
{
	seconds++;
	if (seconds >= 60)
	{
		seconds = 0; // the 60'th second is actually 0
		minutes++;
		if (minutes >= 60) 
		{
			minutes = 0; // the 60'th minute is actually 0
			hours++;
			if (hours >= 13) 
			{
				hours = 1; // the 13'th hour is actually 1
			}
		}
	}
}


volatile int ledStatus = 0;

//extern "C" void TIM2_IRQHandler() //Note that you only need extern “C” if you are building a C++ program.
void TIM2_IRQHandler() //Note that you only need extern “C” if you are building a C++ program.
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		// GPIO_ToggleBits(GPIOC, GPIO_Pin_13); // appears to be no ToggleBits for stm32f10x, so we'll do this a bit more manually with ledStatus
		if (ledStatus == 0)
		{
			ledStatus = 1;
			GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
		}
		else
		{
			ledStatus = 0;
			GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
		}
	}
	timeIncrement();
	timeToDigits();
}
int main(void)
{

	// Screen connection
	// SCK  -> PB3
	// A0   -> PB4
	// SDA  -> PB5
	// RST  -> PB6
	// CS   -> PB7

	ST7735_Init();
	ST7735_AddrSet(0,0,159,127);
	ST7735_Clear(0x0000);

	ST7735_PutStr5x7(0,0,"Hello world!",RGB565(255,0,0));
	ST7735_PutStr5x7(0,10,"This is 5x7 font",RGB565(0,255,0));
	ST7735_PutStr5x7(0,20,"Screen 128x160 pixels",RGB565(0,0,255));

	ST7735_Orientation(scr_normal);

	InitializeLED();
	InitializeTimer();
	EnableTimerInterrupt(); 

	

	while(1) {
//		ST7735_BigDig(2,0,35,RGB565(212,246,190));
//		ST7735_BigDig(3,16,35,RGB565(212,246,190));
//		ST7735_FillRect(33,42,35,44,RGB565(177,211,190));
//		ST7735_FillRect(33,67,35,69,RGB565(177,211,190));
//		ST7735_BigDig(4,37,35,RGB565(212,246,190));
//		ST7735_BigDig(8,53,35,RGB565(212,246,190));
//		ST7735_FillRect(70,42,72,44,RGB565(177,211,190));
//		ST7735_FillRect(70,67,72,69,RGB565(177,211,190));
//		ST7735_BigDig(5,75,35,RGB565(212,246,190));
//		ST7735_BigDig(6,91,35,RGB565(212,246,190));
//		ST7735_PutStr5x7(0,80,"Time:",RGB565(255,255,255));

//		int timerValue = TIM_GetCounter(TIM2);
//		if (timerValue == 400)
//			GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
//		else if (timerValue == 500)
//			GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);

		
	}

    while(1) {
    }
}


//#include <stm32f10x_gpio.h>
//#include <stm32f10x_rcc.h>
//
//void Delay()
//{
//	int i;
//	for (i = 0; i < 1000000; i++)
//		asm("nop");
//}

//int main()
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
//  
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
//
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
//  
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOC, &GPIO_InitStructure);
//
//	for (;;)
//	{
//		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
//		Delay();
//		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
//		Delay();
//	}
//}
