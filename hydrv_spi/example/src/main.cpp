#include "hydrv_clock.hpp"
#include "hydrv_gpio_low.hpp"
#include "hydrv_spi.hpp"

#define BUFFER_LENGTH 4

#ifdef STM32F407xx
constinit hydrv::GPIO::GPIOLow cs_pin(hydrv::GPIO::GPIOLow::GPIOA_port, 4,
                                      hydrv::GPIO::GPIOLow::GPIO_Fast_Output);
constinit hydrv::GPIO::GPIOLow sck_pin(hydrv::GPIO::GPIOLow::GPIOA_port, 5,
                                       hydrv::GPIO::GPIOLow::GPIO_SPI_OUTPUT);
constinit hydrv::GPIO::GPIOLow miso_pin(hydrv::GPIO::GPIOLow::GPIOA_port, 6,
                                        hydrv::GPIO::GPIOLow::GPIO_SPI_INPUT);
constinit hydrv::GPIO::GPIOLow mosi_pin(hydrv::GPIO::GPIOLow::GPIOA_port, 7,
                                        hydrv::GPIO::GPIOLow::GPIO_SPI_OUTPUT);
constexpr auto kPrescaler = hydrv::SPI::SPILow::BaudratePrescaler(84000, 5250);
#endif
#ifdef STM32F103xB
constinit hydrv::GPIO::GPIOLow cs_pin(hydrv::GPIO::GPIOLow::GPIOA_port, 4,
                                      hydrv::GPIO::GPIOLow::GPIO_Fast_Output);
constinit hydrv::GPIO::GPIOLow sck_pin(hydrv::GPIO::GPIOLow::GPIOA_port, 5,
                                       hydrv::GPIO::GPIOLow::GPIO_SPI_OUTPUT);
constinit hydrv::GPIO::GPIOLow miso_pin(hydrv::GPIO::GPIOLow::GPIOA_port, 6,
                                        hydrv::GPIO::GPIOLow::GPIO_SPI_INPUT);
constinit hydrv::GPIO::GPIOLow mosi_pin(hydrv::GPIO::GPIOLow::GPIOA_port, 7,
                                        hydrv::GPIO::GPIOLow::GPIO_SPI_OUTPUT);
constexpr auto kPrescaler = hydrv::SPI::SPILow::BaudratePrescaler(64000, 16000);
#endif

constinit hydrv::SPI::SPI spi(hydrv::SPI::SPILow::SPI1_LOW, sck_pin, miso_pin,
                              mosi_pin, 5, kPrescaler,
                              hydrv::SPI::SPILow::ClockPolarity::HIGH,
                              hydrv::SPI::SPILow::ClockPhase::SECOND_EDGE,
                              hydrv::SPI::SPILow::DataSize::BITS_8,
                              hydrv::SPI::SPILow::BitOrder::MSB_FIRST, cs_pin);

uint8_t tx_buffer[BUFFER_LENGTH] = {0x76, 0x00, 0x80 | 0x75, 0x00};
uint8_t rx_buffer[BUFFER_LENGTH];

int count = 0;

int main(void)
{
    hydrv::clock::Clock::Init(hydrv::clock::Clock::HSI_DEFAULT);
    NVIC_SetPriorityGrouping(0);
    spi.Init();

    while (1)
    {
        spi.MakeTransaction(tx_buffer, 3, rx_buffer, 1);
        hydrv::clock::Clock::Delay(500);
    }
}

extern "C"
{
    void SysTick_Handler(void) { hydrv::clock::Clock::SysTickHandler(); }

    void SPI1_IRQHandler(void) { spi.IRQCallback(); }
}
