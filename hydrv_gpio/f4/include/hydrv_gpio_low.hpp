#pragma once

#include "hydrv_gpio_low_base.hpp"
#include "hydrv_gpio_low_ctrl.hpp"

namespace hydrv::gpio
{
template <GPIOPort::Index kPort, int kPin>
class GPIOLowBase<kPort, kPin>::GPIOLow : public GPIOLowCtrl
{
public:
    template <typename T>
    explicit GPIOLow(const T &env);

    GPIOLow(const GPIOLow &) = delete;
    GPIOLow &operator=(const GPIOLow &) = delete;
    GPIOLow &operator=(GPIOLow &&) = delete;
    GPIOLow(GPIOLow &&) = delete;

    ~GPIOLow() = default;

    using GPIOLowCtrl::Reset;
    using GPIOLowCtrl::Set;
};

template <GPIOPort::Index kPort, int kPin>
template <typename T>
GPIOLowBase<kPort, kPin>::GPIOLow::GPIOLow(const T &env)
    : GPIOLowCtrl(env.template GetPeriph<GPIOLowBase<kPort, kPin>>())
{
}

} // namespace hydrv::gpio
