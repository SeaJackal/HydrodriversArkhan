#pragma once

#include "hydrolib_func_concepts.hpp"
#include "hydrv_gpio_port.hpp"
#include "hydrv_i2c_low_base.hpp"
#include "hydrv_i2c_low_fsm.hpp"
#include <cstring>
#include <span>
#include <tuple>

namespace hydrv::i2c
{

template <I2CIndex kIndex,
          typename CallbackType =
              decltype(&hydrolib::concepts::func::DummyFunc<void>),
          int kMaxTxCapacity = 255>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
class I2CBase
{
public:
    class I2C;

    struct Config
    {
        using Handler = I2CBase;

        static constexpr int kGPIOCount = 2;

        // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
        // Struct with no invariants
        I2CLowBase<kIndex>::GPIOSDA sda_pin;
        I2CLowBase<kIndex>::GPIOSCL scl_pin;
        I2CLowBase<kIndex>::Speed speed;
        int irq_priority = 7; // NOLINT(cppcoreguidelines-avoid-magic-numbers,
        //  readability-magic-numbers)

        CallbackType transaction_complete_callback =
            hydrolib::concepts::func::DummyFunc<void>;
        // NOLINTEND(misc-non-private-member-variables-in-classes)

        [[nodiscard]] consteval std::tuple<gpio::GPIOPort::RawConfig,
                                           gpio::GPIOPort::RawConfig>
        GetGPIOConfigs() const;
    };

    consteval explicit I2CBase(const Config &config);

private:
    enum class Direction
    {
        kWriting,
        kReading
    };

    I2CLowBase<kIndex> i2c_low_;
    std::array<std::byte, kMaxTxCapacity> tx_buffer_{};
    decltype(tx_buffer_)::iterator tx_iterator_{};
    std::span<std::byte>::iterator rx_iterator_;

    Direction direction_ = Direction::kWriting;
    WriteTransaction<kIndex> write_transaction_;
    ReadTransaction<kIndex> read_transaction_;

    CallbackType transaction_complete_callback_;
};

template <I2CIndex kIndex, typename CallbackType, int kMaxTxCapacity>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
consteval std::tuple<gpio::GPIOPort::RawConfig, gpio::GPIOPort::RawConfig>
I2CBase<kIndex, CallbackType, kMaxTxCapacity>::Config::GetGPIOConfigs() const
{
    return std::make_tuple(I2CLowBase<kIndex>::GetSDAGPIOConfig(sda_pin),
                           I2CLowBase<kIndex>::GetSCLPGPIOConfig(scl_pin));
}

template <I2CIndex kIndex, typename CallbackType, int kMaxTxCapacity>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
consteval I2CBase<kIndex, CallbackType, kMaxTxCapacity>::I2CBase(
    const Config &config)
    : i2c_low_(config.speed, config.irq_priority),
      transaction_complete_callback_(config.transaction_complete_callback)
{
}

} // namespace hydrv::i2c
