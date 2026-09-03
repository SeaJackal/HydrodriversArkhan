#pragma once

#include <array>
#include <cstddef>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "hydrolib_return_codes.hpp"

#include "hydrv_clock.hpp"
#include "hydrv_gpio_port.hpp"

namespace hydrv
{

template <typename... Ts>
class EnvBase
{
public:
    class Env;

    consteval explicit EnvBase(const clock::Clock::ClockPreset &clock_preset,
                               Ts... args);

    EnvBase(const EnvBase &) = delete;
    EnvBase(EnvBase &&) = delete;
    EnvBase &operator=(const EnvBase &) = delete;
    EnvBase &operator=(EnvBase &&) = delete;

    ~EnvBase() = default;

private:
    static consteval bool
    IsAllGPIOsUnique(const std::vector<gpio::GPIOPort::RawConfig> &gpios);

    template <std::size_t... kIndexes>
    consteval std::array<gpio::GPIOPort, gpio::GPIOPort::kPortCount>
    CreateGPIOPorts(Ts... args,
                    [[maybe_unused]] std::index_sequence<kIndexes...> indexes);

    consteval std::vector<gpio::GPIOPort::RawConfig> ExtractGPIOs(Ts... args);

    template <typename T, std::size_t... kGPIOIndexes>
    static consteval void
    AddGPIODataToVector(std::vector<gpio::GPIOPort::RawConfig> &configs, T &arg,
                        std::index_sequence<kGPIOIndexes...>);

    template <typename T, std::size_t... kIndexes>
    static consteval int CalculatePeriphIndex(
        [[maybe_unused]] std::index_sequence<kIndexes...> indexes);

    clock::Clock::ClockPreset clock_preset_;

    clock::Clock clock_;
    std::tuple<typename Ts::Handler...>
        devices_; // TODO: vov-dm-an - make tuple constructible from configs,
                  // not devices, to store not movable objects

    std::array<gpio::GPIOPort, gpio::GPIOPort::kPortCount> gpio_ports_;
};

template <typename... Ts>
consteval EnvBase<Ts...>::EnvBase(const clock::Clock::ClockPreset &clock_preset,
                                  Ts... args)
    : clock_preset_(clock_preset),
      devices_(typename Ts::Handler{args}...),
      gpio_ports_(CreateGPIOPorts(
          args..., std::make_index_sequence<gpio::GPIOPort::kPortCount>()))
{
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
consteval std::array<gpio::GPIOPort, gpio::GPIOPort::kPortCount>
EnvBase<Ts...>::CreateGPIOPorts(
    Ts... args, [[maybe_unused]] std::index_sequence<kIndexes...> indexes)
{
    auto gpios = ExtractGPIOs(args...);
    hydrolib::CompileTimeAssert(IsAllGPIOsUnique(gpios),
                                "GPIOs are not unique");

    return std::array<gpio::GPIOPort, gpio::GPIOPort::kPortCount>{
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
    [[maybe_unused]] std::index_sequence<kGPIOIndexes...> indexes)
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
consteval int EnvBase<Ts...>::CalculatePeriphIndex(
    [[maybe_unused]] std::index_sequence<kIndexes...> indexes)
{
    return ((std::is_same_v<typename Ts::Handler, T> ? kIndexes : 0) + ...);
}

}; // namespace hydrv
