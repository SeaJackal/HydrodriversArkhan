#include "hydrolib_log_distributor.hpp"
#include "hydrolib_log_macro.hpp"
#include "hydrolib_logger.hpp"
#include "hydrv_clock.hpp"
#include "hydrv_env.hpp"
#include "hydrv_env_base.hpp"
#include "hydrv_gpio_low.hpp"
#include "hydrv_gpio_port.hpp"
#include "hydrv_i2c.hpp"
#include "hydrv_uart.hpp"

#include <array>
#include <chrono>
#include <cstddef>

namespace
{

constexpr std::byte I2C_ADDRESS = std::byte(0x36) << 1;
constexpr int READ_LENGTH = 2;
constexpr int kUARTBufferSize = 255;
constexpr int kSysClkFreq = 168;

volatile bool i2c_done = false;

void I2CTransactionComplete() { i2c_done = true; }

using Clock = hydrv::clock::Clock<kSysClkFreq>;
using UART = hydrv::uart::UARTBase<hydrv::uart::UARTIndex::kUSART3,
                                   kUARTBufferSize, kUARTBufferSize>;
using I2C = hydrv::i2c::I2CBase<hydrv::i2c::I2CIndex::kI2C1,
                                decltype(&I2CTransactionComplete), 255>;

constinit hydrv::EnvBase env_base(
    Clock(),
    UART::Config{.speed = hydrv::uart::UARTLowBase<
                     hydrv::uart::UARTIndex::kUSART3>::Speed::k115200,
                 .rx_pin = hydrv::uart::UARTLowBase<
                     hydrv::uart::UARTIndex::kUSART3>::GPIORx::kB11,
                 .tx_pin = hydrv::uart::UARTLowBase<
                     hydrv::uart::UARTIndex::kUSART3>::GPIOTx::kB10},
    I2C::Config{
        .sda_pin =
            hydrv::i2c::I2CLowBase<hydrv::i2c::I2CIndex::kI2C1>::GPIOSDA::kB7,
        .scl_pin =
            hydrv::i2c::I2CLowBase<hydrv::i2c::I2CIndex::kI2C1>::GPIOSCL::kB6,
        .speed =
            hydrv::i2c::I2CLowBase<hydrv::i2c::I2CIndex::kI2C1>::Speed::k100,
        .irq_priority = 7,
        .transaction_complete_callback = &I2CTransactionComplete,
    });

decltype(env_base)::Env env(env_base);

UART::UART uart(env);

I2C::I2C i2c(env);
} // namespace

constinit hydrolib::logger::LogDistributor distributor("[%s] [%l] %m\n\r",
                                                       uart);
constinit hydrolib::logger::Logger logger1("I2CExample", 0, distributor);

std::byte tx_value{0x0C};
std::array<std::byte, READ_LENGTH> rx_buffer;
int raw_angle = 0;

int main(void)
{
    distributor.SetAllFilters(0, hydrolib::logger::LogLevel::INFO);

    i2c_done = false;
    i2c.Write(I2C_ADDRESS, std::span<const std::byte>{&tx_value, 1});

    while (!i2c_done)
    {
    }

    auto last_request = std::chrono::steady_clock::now();

    while (1)
    {
        if (std::chrono::steady_clock::now() - last_request <
            std::chrono::milliseconds(100))
        {
            continue;
        }
        i2c_done = false;
        i2c.Read(I2C_ADDRESS, rx_buffer);
        // while (!i2c_done && std::chrono::steady_clock::now() - last_request <
        //                         std::chrono::milliseconds(1000))
        while (!i2c_done)
        {
            continue;
        }
        raw_angle = static_cast<int>(rx_buffer[0]) << 8 |
                    static_cast<int>(rx_buffer[1]);
        LOG(logger1, hydrolib::logger::LogLevel::INFO, "angle: {}", raw_angle);
        last_request = std::chrono::steady_clock::now();
    }
}

extern "C"
{
    void SysTick_Handler(void) { Clock::SysTickHandler(); }
    void I2C1_EV_IRQHandler(void) { i2c.IRQCallback(); }
    void I2C1_ER_IRQHandler(void) { i2c.IRQCallback(); }
    void USART3_IRQHandler(void) { uart.IRQCallback(); }
}
