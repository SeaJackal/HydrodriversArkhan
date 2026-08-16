#include "hydrolib_log_distributor.hpp"
#include "hydrolib_log_macro.hpp"
#include "hydrolib_logger.hpp"
#include "hydrv_clock.hpp"
#include "hydrv_gpio_low.hpp"
#include "hydrv_i2c.hpp"
#include "hydrv_uart.hpp"
#include <chrono>

constexpr uint8_t I2C_ADDRESS = 0x36 << 1;
constexpr int READ_LENGTH = 2;

volatile bool i2c_done = false;

enum class Stage : uint8_t
{
    Write,
    Read
};

void I2CTransactionComplete() { i2c_done = true; }

constinit hydrv::GPIO::GPIOLow scl_pin(hydrv::GPIO::GPIOLow::GPIOB_port, 6,
                                       hydrv::GPIO::GPIOLow::GPIO_I2C_SCL);
constinit hydrv::GPIO::GPIOLow sda_pin(hydrv::GPIO::GPIOLow::GPIOB_port, 7,
                                       hydrv::GPIO::GPIOLow::GPIO_I2C_SDA);

constinit hydrv::GPIO::GPIOLow rx_pin3(hydrv::GPIO::GPIOLow::GPIOB_port, 11,
                                       hydrv::GPIO::GPIOLow::GPIO_UART_RX);
constinit hydrv::GPIO::GPIOLow tx_pin3(hydrv::GPIO::GPIOLow::GPIOB_port, 10,
                                       hydrv::GPIO::GPIOLow::GPIO_UART_TX);
constinit hydrv::UART::UART<255, 255>
    uart3(hydrv::UART::UARTLow::USART3_115200_LOW, rx_pin3, tx_pin3, 7);

constinit hydrolib::logger::LogDistributor distributor("[%s] [%l] %m\n\r",
                                                       uart3);
constinit hydrolib::logger::Logger logger1("I2CExample", 0, distributor);

constinit hydrv::I2C::I2C<decltype(&I2CTransactionComplete)>
    i2c(hydrv::I2C::I2CLow::I2C1_100KHZ_LOW, scl_pin, sda_pin, 5,
        I2CTransactionComplete);

uint8_t tx_value = 0x0C;
uint8_t rx_buffer[READ_LENGTH] = {};
int raw_angle = 0;

int main(void)
{
    hydrv::clock::Clock::Init(hydrv::clock::Clock::HSI_DEFAULT);
    NVIC_SetPriorityGrouping(0);
    uart3.Init();
    i2c.Init();

    distributor.SetAllFilters(0, hydrolib::logger::LogLevel::INFO);

    i2c_done = false;
    i2c.Write(I2C_ADDRESS, &tx_value, 1);

    auto last_request = std::chrono::steady_clock::now();

    while (1)
    {
        if (std::chrono::steady_clock::now() - last_request <
            std::chrono::milliseconds(100))
        {
            continue;
        }
        i2c_done = false;
        i2c.Read(I2C_ADDRESS, rx_buffer, READ_LENGTH);
        while (!i2c_done && std::chrono::steady_clock::now() - last_request <
                                std::chrono::milliseconds(1000))
        {
            continue;
        }
        raw_angle = (rx_buffer[0] << 8) | rx_buffer[1];
        LOG(logger1, hydrolib::logger::LogLevel::INFO, "angle: {}", raw_angle);
        last_request = std::chrono::steady_clock::now();
    }
}

extern "C"
{
    void SysTick_Handler(void) { hydrv::clock::Clock::SysTickHandler(); }
    void I2C1_EV_IRQHandler(void) { i2c.IRQCallback(); }
    void I2C1_ER_IRQHandler(void) { i2c.IRQCallback(); }
    void USART3_IRQHandler(void) { uart3.IRQCallback(); }
}
