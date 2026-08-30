#include "hydrv_clock.hpp"
#include "hydrv_gpio.hpp"
#include "hydrv_gpio_mapper.hpp"

#include <chrono>

#if defined(STM32F407xx)

constinit hydrv::gpio::GPIOMapper gpio_mapper{
    {.port = hydrv::gpio::GPIOPort::Index::kGPIOD,
     .pin = 12,
     .preset = hydrv::gpio::GPIOPort::kOutput}};

constinit hydrv::gpio::GPIO led_pin_base(hydrv::gpio::GPIOPort::Index::kGPIOD,
                                         12, 0, gpio_mapper);

#elif defined(STM32F103xB)

constexpr hydrv::gpio::GPIOMapper gpio_mapper{
    {.port = hydrv::gpio::GPIOPort::Index::kGPIOC,
     .pin = 13,
     .preset = hydrv::gpio::GPIOPort::kOutput}};

constinit hydrv::gpio::GPIO led_pin_base(hydrv::gpio::GPIOPort::Index::kGPIOC,
                                         13, 0, gpio_mapper);

#endif

hydrv::gpio::GPIOMapper::GPIOPortsHandler gpio_ports_handler(gpio_mapper);

hydrv::gpio::GPIO::GPIOHandler led_pin(led_pin_base, gpio_ports_handler);

int main(void)
{
    NVIC_SetPriorityGrouping(0);

    hydrv::clock::Clock::Init(hydrv::clock::Clock::HSI_DEFAULT);

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