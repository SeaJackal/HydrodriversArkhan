#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "hydrv_clock.hpp"
#include "hydrv_gpio_port.hpp"

namespace hydrv
{

constexpr void CompileTimeAssert(bool assertion,
                                 [[maybe_unused]] std::string_view message)
{
    [[maybe_unused]] int a = 1 / static_cast<int>(assertion);
}

template <typename... Ts>
class EnvBase
{
public:
    class Env;

    consteval EnvBase(const clock::Clock::ClockPreset &clock_preset,
                      Ts... args);

private:
    static consteval bool
    IsAllGPIOsUnique(const std::vector<gpio::GPIOPort::RawConfig> &gpios);

    template <std::size_t... kIndexes>
    consteval std::array<gpio::GPIOPort, gpio::GPIOPort::kPortsCount>
    CreateGPIOPorts(Ts... args, std::index_sequence<kIndexes...>);

    consteval std::vector<gpio::GPIOPort::RawConfig> ExtractGPIOs(Ts... args);

    template <typename T, std::size_t... kGPIOIndexes>
    static consteval void
    AddGPIODataToVector(std::vector<gpio::GPIOPort::RawConfig> &configs, T &arg,
                        std::index_sequence<kGPIOIndexes...>);

    template <typename T, std::size_t... kIndexes>
    static consteval int CalculatePeriphIndex(std::index_sequence<kIndexes...>);

    clock::Clock::ClockPreset clock_preset_;

    clock::Clock clock_;
    std::tuple<typename Ts::Handler...> devices_;

    std::array<gpio::GPIOPort, gpio::GPIOPort::kPortsCount> gpio_ports_;
};

template <typename... Ts>
class EnvBase<Ts...>::Env
{
public:
    Env(EnvBase<Ts...> &env_base);

    template <typename T>
    auto &GetPeriph();
    template <typename T>
    const auto &GetPeriph() const;

private:
    EnvBase<Ts...> &env_base_;
};

template <typename... Ts>
consteval EnvBase<Ts...>::EnvBase(const clock::Clock::ClockPreset &clock_preset,
                                  Ts... args)
    : clock_preset_(clock_preset),
      devices_(typename Ts::Handler(args)...),
      gpio_ports_(CreateGPIOPorts(
          args..., std::make_index_sequence<gpio::GPIOPort::kPortsCount>()))
{
}

template <typename... Ts>
EnvBase<Ts...>::Env::Env(EnvBase<Ts...> &env_base) : env_base_(env_base)
{
    env_base_.clock_.Init(env_base_.clock_preset_);
    for (const auto &gpio_port : env_base_.gpio_ports_)
    {
        gpio_port.Init();
    }
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

template <typename... Ts>
consteval bool EnvBase<Ts...>::IsAllGPIOsUnique(
    const std::vector<gpio::GPIOPort::RawConfig> &gpios)
{
    for (auto i = gpios.begin(); i != gpios.end(); ++i)
    {
        for (auto j = i + 1; j != gpios.end(); ++j)
        {
            if (i->pin == j->pin && i->port == j->port)
            {
                return false;
            }
        }
    }
    return true;
}

template <typename... Ts>
template <std::size_t... kIndexes>
consteval std::array<gpio::GPIOPort, gpio::GPIOPort::kPortsCount>
EnvBase<Ts...>::CreateGPIOPorts(Ts... args, std::index_sequence<kIndexes...>)
{
    auto gpios = ExtractGPIOs(args...);
    CompileTimeAssert(IsAllGPIOsUnique(gpios), "GPIOs are not unique");

    return std::array<gpio::GPIOPort, gpio::GPIOPort::kPortsCount>{
        gpio::GPIOPort(
            static_cast<gpio::GPIOPort::Index>(kIndexes),
            gpios | std::ranges::views::filter(
                        [](const gpio::GPIOPort::RawConfig &gpio)
                        {
                            return gpio.port ==
                                   static_cast<gpio::GPIOPort::Index>(kIndexes);
                        }))...};
}

template <typename... Ts>
consteval std::vector<gpio::GPIOPort::RawConfig>
EnvBase<Ts...>::ExtractGPIOs(Ts... args)
{
    std::vector<gpio::GPIOPort::RawConfig> configs;
    (AddGPIODataToVector(configs, args,
                         std::make_index_sequence<Ts::kGPIOCount>()),
     ...);
    return configs;
}

template <typename... Ts>
template <typename T, std::size_t... kGPIOIndexes>
consteval void EnvBase<Ts...>::AddGPIODataToVector(
    std::vector<gpio::GPIOPort::RawConfig> &configs, T &arg,
    std::index_sequence<kGPIOIndexes...>)
{
    if constexpr (sizeof...(kGPIOIndexes) > 1)
    {
        (configs.push_back(get<kGPIOIndexes>(arg.GetGPIOConfigs())), ...);
    }
    else
    {
        configs.push_back(arg.GetGPIOConfigs());
    }
}

template <typename... Ts>
template <typename T, std::size_t... kIndexes>
consteval int
EnvBase<Ts...>::CalculatePeriphIndex(std::index_sequence<kIndexes...>)
{
    return ((std::is_same_v<typename Ts::Handler, T> ? kIndexes : 0) + ...);
}

}; // namespace hydrv
