#pragma once

#include <cstdint>

#include "hydrv_gpio_low_ctrl_base.hpp"
#include "hydrv_gpio_port.hpp"

namespace hydrv::gpio
{
template <GPIOPort::Index kPort, int kPin>
class GPIOLowBase : public GPIOLowCtrlBase
{
public:
    struct Config
    {
        using Handler = GPIOLowBase;

        static constexpr int kGPIOCount = 1;

        // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
        // Struct with no invariants
        OutputType output_type;
        OutputSpeed output_speed;
        PullUpDown pull_up_down;
        // NOLINTEND(misc-non-private-member-variables-in-classes)

        [[nodiscard]] consteval GPIOPort::RawConfig GetGPIOConfigs() const
        {
            return GPIOPort::RawConfig{.pin = kPin,
                                       .port = kPort,
                                       .mode = Mode::kOutput,
                                       .output_type = output_type,
                                       .output_speed = output_speed,
                                       .pull_up_down = pull_up_down,
                                       .altfunc = Altfunc::kAltfunc0};
        }
    };

    class GPIOLow;

    explicit consteval GPIOLowBase(const Config &config);

    GPIOLowBase(const GPIOLowBase &) = delete;
    GPIOLowBase &operator=(const GPIOLowBase &) = delete;
    GPIOLowBase &operator=(GPIOLowBase &&) = delete;

    GPIOLowBase(GPIOLowBase &&) = default;

    ~GPIOLowBase() = default;

private:
    static consteval uint32_t CalculateSetRegValue(int pin);
    static consteval uint32_t CalculateResetRegValue(int pin);
};

template <GPIOPort::Index kPort, int kPin>
consteval GPIOLowBase<kPort, kPin>::GPIOLowBase([[maybe_unused]] const Config &config)
    : GPIOLowCtrlBase(
          GPIOLowCtrlBase::GPIOHandler(GPIOPort::GetGPIOx(kPort)),
          GPIOLowCtrlBase::SetRegMask(CalculateSetRegValue(kPin)),
          GPIOLowCtrlBase::ResetRegMask(CalculateResetRegValue(kPin)))
{
}

template <GPIOPort::Index kPort, int kPin>
consteval uint32_t GPIOLowBase<kPort, kPin>::CalculateSetRegValue(int pin)
{
    return 0x1UL << pin;
}

template <GPIOPort::Index kPort, int kPin>
consteval uint32_t GPIOLowBase<kPort, kPin>::CalculateResetRegValue(int pin)
{
    return 0x1UL << (pin + GPIO_BSRR_BR0_Pos);
}

} // namespace hydrv::gpio
