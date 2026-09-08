#include "hydrv_clock.hpp"
#include "hydrv_env.hpp"
#include "hydrv_env_base.hpp"
#include "hydrv_gpio_low.hpp"

#include <chrono>

namespace
{

using Clock = hydrv::clock::Clock<168>;

#ifdef STM32F407xx
using LedGPIO = hydrv::gpio::GPIOLowBase<hydrv::gpio::GPIOPort::Index::kGPIOD,
                                         12>; // NOLINT
#elifdef STM32F103xB
using LedGPIO = hydrv::gpio::GPIOLow<hydrv::gpio::GPIOPort::Index::kGPIOC, 13>;
#endif

constinit hydrv::EnvBase
    env_base(Clock(),
             LedGPIO::Config{.output_type = hydrv::gpio::OutputType::kPushPull,
                             .output_speed = hydrv::gpio::OutputSpeed::kLow,
                             .pull_up_down = hydrv::gpio::PullUpDown::kNo});

decltype(env_base)::Env env(env_base);

LedGPIO::GPIOLow led_pin(env);

} // namespace

int main()
{
    while (true)
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

extern "C"
{
    void SysTick_Handler(void) // NOLINT
    {
        Clock::SysTickHandler();
    }
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