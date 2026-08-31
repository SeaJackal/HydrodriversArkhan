#pragma once

#include "hydrv_gpio_port.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <initializer_list>
#include <ranges>

namespace hydrv::gpio
{
class GPIOMapper
{
public:
    class GPIOPortsHandler;

    struct GPIOData
    {
        GPIOPort::Index port;
        int pin;
        GPIOPort::GPIOConfig preset;
    };

    consteval GPIOMapper(std::initializer_list<GPIOData> gpio_data);

    consteval bool IsGPIOMapped(GPIOPort::Index port, int pin) const;
    consteval const GPIOPort &GetPort(GPIOPort::Index port) const;

private:
    static consteval bool
    IsDuplicated(const std::initializer_list<GPIOData> &gpio_data);
    static consteval bool
    IsPinsValid(const std::initializer_list<GPIOData> &gpio_data);

    static consteval GPIOPort
    MakePort(GPIOPort::Index port,
             const std::initializer_list<GPIOData> &gpio_data);

    std::array<GPIOData, GPIOPort::kPortsCount * GPIOPort::kPinCount>
        gpio_data_buffer_ = {};
    std::span<GPIOData> gpio_data_;
    std::array<GPIOPort, GPIOPort::kPortsCount> gpio_ports_;
};

class GPIOMapper::GPIOPortsHandler
{
public:
    GPIOPortsHandler(const GPIOMapper &gpio_mapper);
};

inline GPIOMapper::GPIOPortsHandler::GPIOPortsHandler(
    const GPIOMapper &gpio_mapper)
{
    for (const auto &port : gpio_mapper.gpio_ports_)
    {
        port.Init();
    }
}

consteval GPIOMapper::GPIOMapper(std::initializer_list<GPIOData> gpio_data)
    : gpio_data_(std::span(gpio_data_buffer_).subspan(0, gpio_data.size()))
{
    std::ranges::copy(gpio_data, gpio_data_.begin());
    if (!IsDuplicated(gpio_data))
    {
        int a = 1 / 0;
    }
    if (!IsPinsValid(gpio_data))
    {
        int a = 1 / 0;
    }

    for (int i = 0; i < GPIOPort::kPortsCount; i++)
    {
        gpio_ports_[i] = MakePort(static_cast<GPIOPort::Index>(i), gpio_data);
    }
}

consteval bool GPIOMapper::IsGPIOMapped(GPIOPort::Index port, int pin) const
{
    return std::ranges::any_of(
        gpio_data_, [port, pin](const GPIOData &gpio)
        { return gpio.port == port && gpio.pin == pin; });
}

consteval const GPIOPort &GPIOMapper::GetPort(GPIOPort::Index port) const
{
    return gpio_ports_[port];
}

consteval bool
GPIOMapper::IsDuplicated(const std::initializer_list<GPIOData> &gpio_data)
{
    for (auto i = gpio_data.begin(); i != gpio_data.end(); ++i)
    {
        for (auto j = i + 1; j != gpio_data.end(); ++j)
        {
            if (i->pin == j->pin && i->port == j->port)
            {
                return false;
            }
        }
    }
    return true;
}

consteval bool
GPIOMapper::IsPinsValid(const std::initializer_list<GPIOData> &gpio_data)
{
    return std::ranges::all_of(
        gpio_data, [](const GPIOData &gpio)
        { return gpio.pin >= 0 && gpio.pin < GPIOPort::kPinCount; });
}

consteval GPIOPort
GPIOMapper::MakePort(GPIOPort::Index port,
                     const std::initializer_list<GPIOData> &gpio_data)
{
    return GPIOPort(
        port, gpio_data | std::views::filter([port](const GPIOData &gpio)
                                             { return gpio.port == port; }));
}
} // namespace hydrv::gpio
