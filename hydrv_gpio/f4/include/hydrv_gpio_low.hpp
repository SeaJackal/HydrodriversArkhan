#pragma once

#include <cstdint>

#include "hydrv_gpio_port.hpp"

namespace hydrv::gpio
{
class GPIOPort::GPIOLow
{
public:
    class GPIOLowHandler;

    consteval GPIOLow(GPIOPort &GPIO_port, int pin, int altfunc = 0);

private:
    GPIOPort &GPIO_port_;

    const int altfunc_reg_index_;

    const uint32_t altfunc_reg_mask_;
    const uint32_t altfunc_reg_value_;

    const uint32_t set_reg_mask_;
    const uint32_t reset_reg_mask_;

private:
    static consteval uint32_t CalculateAltfuncRegMask(int pin);
    static consteval uint32_t CalculateAltfuncRegValue(int pin, int altfunc);
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
    auto GPIOx = reinterpret_cast<GPIO_TypeDef *>(GPIO_low_.GPIO_port_.GPIOx_);
    MODIFY_REG(GPIOx->AFR[GPIO_low_.altfunc_reg_index_],
               GPIO_low_.altfunc_reg_mask_, GPIO_low_.altfunc_reg_value_);
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

consteval inline GPIOPort::GPIOLow::GPIOLow(GPIOPort &GPIO_port, int pin,
                                            int altfunc)
    : GPIO_port_(GPIO_port),
      altfunc_reg_index_(pin < 8 ? 0 : 1),
      altfunc_reg_mask_(CalculateAltfuncRegMask(pin)),
      altfunc_reg_value_(CalculateAltfuncRegValue(pin, altfunc)),
      set_reg_mask_(CalculateSetRegValue(pin)),
      reset_reg_mask_(CalculateResetRegValue(pin))
{
}

consteval inline uint32_t GPIOPort::GPIOLow::CalculateAltfuncRegMask(int pin)
{
    return 0xFUL << (4 * (pin % 8));
}

consteval inline uint32_t
GPIOPort::GPIOLow::CalculateAltfuncRegValue(int pin, int altfunc)
{
    return altfunc << (4 * (pin % 8));
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
