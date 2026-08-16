#include "hydrv_clock.hpp"
#include "hydrv_gpio_low.hpp"

#include <chrono>

#if defined(STM32F407xx)
constinit hydrv::GPIO::GPIOLow led_pin(hydrv::GPIO::GPIOLow::GPIOD_port, 12,
                                       hydrv::GPIO::GPIOLow::GPIO_Output);
#elif defined(STM32F103xB)
constinit hydrv::GPIO::GPIOLow led_pin(hydrv::GPIO::GPIOLow::GPIOC_port, 13,
                                       hydrv::GPIO::GPIOLow::GPIO_Output);
#endif

int main(void)
{
    NVIC_SetPriorityGrouping(0);

    hydrv::clock::Clock::Init(hydrv::clock::Clock::HSI_DEFAULT);

    led_pin.Init();

    while (1)
    {
        led_pin.Set();
        auto start_time = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start_time <
               std::chrono::seconds(1))
        {
        }
        led_pin.Reset();
        start_time = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start_time <
               std::chrono::seconds(1))
        {
        }
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
    void SysTick_Handler(void) { hydrv::clock::Clock::SysTickHandler(); }
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