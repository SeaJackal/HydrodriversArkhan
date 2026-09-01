#include "hydrv_clock.hpp"
#include "hydrv_env.hpp"
#include "hydrv_gpio_low.hpp"
#include "hydrv_uart.hpp"

constinit hydrv::EnvBase env_base(
    hydrv::clock::Clock::HSI_DEFAULT,
    hydrv::uart::UARTBase<hydrv::uart::UARTIndex::kUSART3, 255, 255>::Config{
        .speed = hydrv::uart::UARTLowBase<
            hydrv::uart::UARTIndex::kUSART3>::Speed::k115200,
        .rx_pin = hydrv::uart::UARTLowBase<
            hydrv::uart::UARTIndex::kUSART3>::GPIORx::kB11,
        .tx_pin = hydrv::uart::UARTLowBase<
            hydrv::uart::UARTIndex::kUSART3>::GPIOTx::kB10,
        .IRQ_priority = 7},
    hydrv::gpio::GPIOLow<hydrv::gpio::GPIOPort::Index::kGPIOD, 12>::Config{
        .output_type = hydrv::gpio::OutputType::kPushPull,
        .output_speed = hydrv::gpio::OutputSpeed::kLow,
        .pull_up_down = hydrv::gpio::PullUpDown::kNo});

decltype(env_base)::Env env(env_base);

hydrv::uart::UARTBase<hydrv::uart::UARTIndex::kUSART3, 255, 255>::UART
    uart(env);

hydrv::gpio::GPIOLow<hydrv::gpio::GPIOPort::Index::kGPIOD, 12>::GPIOLowHandler
    led_pin(env);

constexpr int kBufferSize = 5;

std::array<std::byte, kBufferSize> buffer;

int main(void)
{
    NVIC_SetPriorityGrouping(0);

    while (1)
    {
        unsigned rx_length = uart.GetRxLength();
        if (rx_length >= 5)
        {
            uart.Read(buffer);
            uart.Transmit(buffer);
        }
    }
}

extern "C"
{
    void SysTick_Handler(void) { hydrv::clock::Clock::SysTickHandler(); }
    void USART3_IRQHandler(void) { uart.IRQCallback(); }
    void HardFault_Handler(void)
    {
        while (1)
        {
        }
    }
}
