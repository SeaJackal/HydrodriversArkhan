#include "hydrv_clock.hpp"


#include "hydrv_tim_low.hpp"

extern "C"
{
    void SysTick_Handler();
}
constinit hydrv::GPIO::GPIOLow tim_pin(hydrv::GPIO::GPIOLow::GPIOA_port, 0,
                             hydrv::GPIO::GPIOLow::GPIO_Timer);
constinit hydrv::timer::TimerLow tim(hydrv::timer::TimerLow::TIM5_low, 168, 10000);

int main(void)
{
     NVIC_SetPriorityGrouping(0);
    hydrv::clock::Clock::Init(hydrv::clock::Clock::HSI_DEFAULT);

    tim.Init();
    tim.ConfigurePWM(0, tim_pin);
    tim.SetCaptureCompare(0, 7500);
    tim.StartTimer();

    while (1)
    {
        tim.SetCaptureCompare(0, 1);
        hydrv::clock::Clock::Delay(500);
        tim.SetCaptureCompare(0, 2500);
        hydrv::clock::Clock::Delay(500);
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}

extern "C"
{
    void SysTick_Handler() { hydrv::clock::Clock::SysTickHandler(); }
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line
       number, ex: printf("Wrong parameters value: file %s on line %d\r\n",
       file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
