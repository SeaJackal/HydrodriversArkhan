#include "hydrv_clock.hpp"
#include "hydrv_env.hpp"
#include "hydrv_env_base.hpp"
#include "hydrv_gpio_low.hpp"
#include "hydrv_gpio_port.hpp"
#include "hydrv_uart.hpp"

#include <array>
#include <cstddef>

namespace
{

constexpr int kUARTBufferSize = 255;
constexpr int kSysClkFreq = 168;

using Clock = hydrv::clock::Clock<kSysClkFreq>;
using UART = hydrv::uart::UARTBase<hydrv::uart::UARTIndex::kUSART3,
                                   kUARTBufferSize, kUARTBufferSize>;
using Pin =
    hydrv::gpio::GPIOLow<hydrv::gpio::GPIOPort::Index::kGPIOD,
                         12>; // NOLINT(cppcoreguidelines-avoid-magic-numbers,
                              // readability-magic-numbers)

constinit hydrv::EnvBase
    env_base(Clock(),
             UART::Config{.speed = hydrv::uart::UARTLowBase<
                              hydrv::uart::UARTIndex::kUSART3>::Speed::k115200,
                          .rx_pin = hydrv::uart::UARTLowBase<
                              hydrv::uart::UARTIndex::kUSART3>::GPIORx::kB11,
                          .tx_pin = hydrv::uart::UARTLowBase<
                              hydrv::uart::UARTIndex::kUSART3>::GPIOTx::kB10},
             Pin::Config{.output_type = hydrv::gpio::OutputType::kPushPull,
                         .output_speed = hydrv::gpio::OutputSpeed::kLow,
                         .pull_up_down = hydrv::gpio::PullUpDown::kNo});

decltype(env_base)::Env env(env_base);

UART::UART uart(env);

Pin::GPIOLowHandler led_pin(env);

constexpr int kEchoBufferSize = 5;

std::array<std::byte, kEchoBufferSize> buffer;

}; // namespace

int main()
{
    while (true)
    {
        int rx_length = uart.GetRxLength();
        if (rx_length >= kEchoBufferSize)
        {
            uart.Read(buffer);
            uart.Transmit(buffer);
        }
    }
}

extern "C"
{
    // NOLINTBEGIN(readability-identifier-naming)
    void SysTick_Handler(void) { Clock::SysTickHandler(); }
    void USART3_IRQHandler(void) { uart.IRQCallback(); }
    void HardFault_Handler(void)
    {
        while (true)
        {
        }
    }
    // NOLINTEND(readability-identifier-naming)
}
