#include "hydrv_clock.hpp"
#include "hydrv_gpio.hpp"
#include "hydrv_gpio_low.hpp"
#include "hydrv_uart.hpp"

constexpr hydrv::gpio::GPIOMapper gpio_mapper{
    {.port = hydrv::gpio::GPIOPort::Index::kGPIOB,
     .pin = 11,
     .preset = hydrv::gpio::GPIOPort::kUART_RX},
    {.port = hydrv::gpio::GPIOPort::Index::kGPIOB,
     .pin = 10,
     .preset = hydrv::gpio::GPIOPort::kUART_TX}};

hydrv::gpio::GPIOMapper::GPIOPortsHandler gpio_ports_handler(gpio_mapper);

constinit hydrv::uart::UART<hydrv::uart::UARTIndex::kUSART3,
                            hydrv::uart::Speed::k115200, 255, 255>
    uart_base(gpio_mapper,
              hydrv::uart::UARTLow<hydrv::uart::UARTIndex::kUSART3,
                                   hydrv::uart::Speed::k115200>::GPIORx::kB11,
              hydrv::uart::UARTLow<hydrv::uart::UARTIndex::kUSART3,
                                   hydrv::uart::Speed::k115200>::GPIOTx::kB10,
              7);

decltype(uart_base)::UARTHandler uart(uart_base, gpio_ports_handler);

constexpr int kBufferSize = 5;

std::array<std::byte, kBufferSize> buffer;

int main(void)
{
    hydrv::clock::Clock::Init(hydrv::clock::Clock::HSI_DEFAULT);
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
