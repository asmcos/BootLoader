
#include <stdio.h>
#include <string.h>
#include <stm32f4xx_hal.h>
static void Error_Handler(void);

static void uart_init(void);
static void gpio_init(void);

static GPIO_InitTypeDef GPIO_InitStruct;
UART_HandleTypeDef UartHandle;

void Led_Display(int n)
{
	for (;n>0;n--)
	{

        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_12);     
        HAL_Delay(1000);
	}
}



int main(void)
{

    unsigned int iter = 0;

    uart_init();
    gpio_init();

	boot();

	printf("Not Found Uart data ...\nBootLoader is Sleep.......\n");
	
	//void (*startApp)() = (void(*)())(0x8004000); 
    //startApp();


    while (1)
    {
        iter++;
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_12);     
        HAL_Delay(1000);
        printf("[%5d] \r\n", iter);

    }

}
static void uart_init(void)
{
    UartHandle.Instance        = USART2;
    UartHandle.Init.BaudRate   = 115200;
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits   = UART_STOPBITS_1;
    UartHandle.Init.Parity     = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode       = UART_MODE_TX_RX;

    if(HAL_UART_Init(&UartHandle) != HAL_OK) {
        Error_Handler(); 
    }
}

static void gpio_init(void) 
{
    __GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); 
}

static void Error_Handler(void)
{
    while(1) {

    }
}

