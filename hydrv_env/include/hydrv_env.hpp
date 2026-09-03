#pragma once

#include "core_cm4.h"
#include "hydrv_env_base.hpp"

namespace hydrv
{

template <typename ClockInfo, typename... Ts>
class EnvBase<ClockInfo, Ts...>::Env
{
public:
    explicit Env(EnvBase<ClockInfo, Ts...> &env_base);

    Env(const Env &) = delete;
    Env(Env &&) = delete;
    Env &operator=(const Env &) = delete;
    Env &operator=(Env &&) = delete;

    ~Env() = default;

    template <typename T>
    auto &GetPeriph();
    template <typename T>
    const auto &GetPeriph() const;

private:
    EnvBase<ClockInfo, Ts...> &env_base_;
};

template <typename ClockInfo, typename... Ts>
EnvBase<ClockInfo, Ts...>::Env::Env(EnvBase<ClockInfo, Ts...> &env_base)
    : env_base_(env_base)
{
    ClockInfo::Init();
    for (const auto &gpio_port : env_base_.gpio_ports_)
    {
        gpio_port.Init();
    }
    NVIC_SetPriorityGrouping(0);
}

template <typename ClockInfo, typename... Ts>
template <typename T>
auto &EnvBase<ClockInfo, Ts...>::Env::GetPeriph()
{
    return std::get<CalculatePeriphIndex<T>(
        std::make_index_sequence<sizeof...(Ts)>())>(env_base_.devices_);
}

template <typename ClockInfo, typename... Ts>
template <typename T>
const auto &EnvBase<ClockInfo, Ts...>::Env::GetPeriph() const
{
    return std::get<CalculatePeriphIndex<T>(
        std::make_index_sequence<sizeof...(Ts)>())>(env_base_.devices_);
}

}; // namespace hydrv
