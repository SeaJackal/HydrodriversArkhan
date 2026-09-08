#pragma once

extern "C"
{
#include "stm32f4xx.h"
}

#include "hydrv_gpio_low_ctrl_base.hpp"

namespace hydrv::gpio
{
class GPIOLowCtrlBase::GPIOLowCtrl
{
public:
    explicit GPIOLowCtrl(const GPIOLowCtrlBase &gpio);

    GPIOLowCtrl(const GPIOLowCtrl &) = delete;
    GPIOLowCtrl &operator=(const GPIOLowCtrl &) = delete;
    GPIOLowCtrl &operator=(GPIOLowCtrl &&) = delete;
    GPIOLowCtrl(GPIOLowCtrl &&) = delete;

    ~GPIOLowCtrl() = default;

    void Set() const;
    void Reset() const;

private:
    const GPIOLowCtrlBase &gpio_ctrl_low_;
};

inline GPIOLowCtrlBase::GPIOLowCtrl::GPIOLowCtrl(const GPIOLowCtrlBase &gpio)
    : gpio_ctrl_low_(gpio)
{
}

inline void GPIOLowCtrlBase::GPIOLowCtrl::Set() const
{
    auto *gpiox =
        reinterpret_cast // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast,
        // performance-no-int-to-ptr): Registers access :(
        <GPIO_TypeDef *>(gpio_ctrl_low_.gpiox_);
    gpiox->BSRR = gpio_ctrl_low_.set_reg_mask_;
}

inline void GPIOLowCtrlBase::GPIOLowCtrl::Reset() const
{
    auto *gpiox =
        reinterpret_cast // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast,
        // performance-no-int-to-ptr): Registers access :(
        <GPIO_TypeDef *>(gpio_ctrl_low_.gpiox_);
    gpiox->BSRR = gpio_ctrl_low_.reset_reg_mask_;
}
} // namespace hydrv::gpio
