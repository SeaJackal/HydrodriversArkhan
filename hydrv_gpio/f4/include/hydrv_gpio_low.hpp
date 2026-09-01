#pragma once

#include <cstdint>

#include "hydrv_gpio_port.hpp"

namespace hydrv::gpio
{
template <GPIOPort::Index kPort, int kPin>
class GPIOLow
{
public:
    struct Config
    {
        using Handler = GPIOLow;

        static constexpr int kGPIOCount = 1;

        OutputType output_type;
        OutputSpeed output_speed;
        PullUpDown pull_up_down;

        consteval GPIOPort::RawConfig GetGPIOConfigs() const
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

    class GPIOLowHandler;

    consteval GPIOLow(const Config &config);

private:
    const uint32_t GPIOx_;

    const uint32_t set_reg_mask_;
    const uint32_t reset_reg_mask_;

    static consteval uint32_t CalculateSetRegValue(int pin);
    static consteval uint32_t CalculateResetRegValue(int pin);
};

template <GPIOPort::Index kPort, int kPin>
class GPIOLow<kPort, kPin>::GPIOLowHandler
{
public:
    template <typename T>
    GPIOLowHandler(const T &env);

    void Set();
    void Reset();

private:
    const GPIOLow &GPIO_low_;
};

template <GPIOPort::Index kPort, int kPin>
template <typename T>
GPIOLow<kPort, kPin>::GPIOLowHandler::GPIOLowHandler(const T &env)
    : GPIO_low_(env.template GetPeriph<GPIOLow<kPort, kPin>>())
{
}

template <GPIOPort::Index kPort, int kPin>
void GPIOLow<kPort, kPin>::GPIOLowHandler::Set()
{
    auto GPIOx = reinterpret_cast<GPIO_TypeDef *>(GPIO_low_.GPIOx_);
    GPIOx->BSRR = GPIO_low_.set_reg_mask_;
}

template <GPIOPort::Index kPort, int kPin>
void GPIOLow<kPort, kPin>::GPIOLowHandler::Reset()
{
    auto GPIOx = reinterpret_cast<GPIO_TypeDef *>(GPIO_low_.GPIOx_);
    GPIOx->BSRR = GPIO_low_.reset_reg_mask_;
}

template <GPIOPort::Index kPort, int kPin>
consteval GPIOLow<kPort, kPin>::GPIOLow([[maybe_unused]] const Config &config)
    : GPIOx_(GPIOPort::GetGPIOx(kPort)),
      set_reg_mask_(CalculateSetRegValue(kPin)),
      reset_reg_mask_(CalculateResetRegValue(kPin))
{
}

template <GPIOPort::Index kPort, int kPin>
consteval uint32_t GPIOLow<kPort, kPin>::CalculateSetRegValue(int pin)
{
    return 0x1UL << pin;
}

template <GPIOPort::Index kPort, int kPin>
consteval uint32_t GPIOLow<kPort, kPin>::CalculateResetRegValue(int pin)
{
    return 0x1UL << (pin + GPIO_BSRR_BR0_Pos);
}

} // namespace hydrv::gpio
