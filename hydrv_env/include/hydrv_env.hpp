#pragma once

#include "core_cm4.h"
#include "hydrv_env_base.hpp"

namespace hydrv
{

template <typename... Ts>
class EnvBase<Ts...>::Env
{
public:
    explicit Env(EnvBase<Ts...> &env_base);

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
    EnvBase<Ts...> &env_base_;
};

template <typename... Ts>
EnvBase<Ts...>::Env::Env(EnvBase<Ts...> &env_base) : env_base_(env_base)
{
    hydrv::clock::Clock::Init(env_base_.clock_preset_);
    for (const auto &gpio_port : env_base_.gpio_ports_)
    {
        gpio_port.Init();
    }
    NVIC_SetPriorityGrouping(0);
}

template <typename... Ts>
template <typename T>
auto &EnvBase<Ts...>::Env::GetPeriph()
{
    return std::get<CalculatePeriphIndex<T>(
        std::make_index_sequence<sizeof...(Ts)>())>(env_base_.devices_);
}

template <typename... Ts>
template <typename T>
const auto &EnvBase<Ts...>::Env::GetPeriph() const
{
    return std::get<CalculatePeriphIndex<T>(
        std::make_index_sequence<sizeof...(Ts)>())>(env_base_.devices_);
}

}; // namespace hydrv
