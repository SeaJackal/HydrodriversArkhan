#pragma once

#include <cstdint>

#include "hydrv_gpio_port.hpp"

namespace hydrv::gpio
{
class GPIOPort::GPIOLow
{
public:
    class GPIOLowHandler;

    consteval GPIOLow(const GPIOPort &GPIO_port, int pin, int altfunc);

private:
    const GPIOPort &GPIO_port_;

    const uint32_t set_reg_mask_;
    const uint32_t reset_reg_mask_;

private:
    static consteval uint32_t CalculateSetRegValue(int pin);
    static consteval uint32_t CalculateResetRegValue(int pin);
};

class GPIOPort::GPIOLow::GPIOLowHandler
{
public:
    GPIOLowHandler(GPIOLow &GPIO_low);

    void Set();
    void Reset();

private:
    GPIOLow &GPIO_low_;
};

inline GPIOPort::GPIOLow::GPIOLowHandler::GPIOLowHandler(GPIOLow &GPIO_low)
    : GPIO_low_(GPIO_low)
{
    GPIO_low_.GPIO_port_.Init();
}

inline void GPIOPort::GPIOLow::GPIOLowHandler::Set()
{
    auto GPIOx = reinterpret_cast<GPIO_TypeDef *>(GPIO_low_.GPIO_port_.GPIOx_);
    GPIOx->BSRR = GPIO_low_.set_reg_mask_;
}

inline void GPIOPort::GPIOLow::GPIOLowHandler::Reset()
{
    auto GPIOx = reinterpret_cast<GPIO_TypeDef *>(GPIO_low_.GPIO_port_.GPIOx_);
    GPIOx->BSRR = GPIO_low_.reset_reg_mask_;
}

consteval inline GPIOPort::GPIOLow::GPIOLow(const GPIOPort &GPIO_port, int pin,
                                            [[maybe_unused]] int altfunc)
    : GPIO_port_(GPIO_port),
      set_reg_mask_(CalculateSetRegValue(pin)),
      reset_reg_mask_(CalculateResetRegValue(pin))
{
}

consteval inline uint32_t GPIOPort::GPIOLow::CalculateSetRegValue(int pin)
{
    return 0x1UL << pin;
}

consteval inline uint32_t GPIOPort::GPIOLow::CalculateResetRegValue(int pin)
{
    return 0x1UL << (pin + GPIO_BSRR_BR0_Pos);
}

} // namespace hydrv::gpio