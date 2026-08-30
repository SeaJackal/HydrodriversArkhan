#pragma once

#include "hydrv_gpio_low.hpp"
#include "hydrv_gpio_mapper.hpp"
#include "hydrv_gpio_port.hpp"

namespace hydrv::gpio
{

class GPIO
{
public:
    class GPIOHandler;

    consteval GPIO(GPIOPort::Index port, int pin, int altfunc,
                   const GPIOMapper &gpio_mapper);

private:
    GPIOPort::GPIOLow gpio_;
};

class GPIO::GPIOHandler
{
public:
    GPIOHandler(
        GPIO &gpio,
        [[maybe_unused]] const GPIOMapper::GPIOPortsHandler &gpio_mapper);

    void Set();
    void Reset();

private:
    GPIOPort::GPIOLow::GPIOLowHandler gpio_low_handler_;
};

consteval GPIO::GPIO(GPIOPort::Index port, int pin, int altfunc,
                     const GPIOMapper &gpio_mapper)
    : gpio_(gpio_mapper.GetPort(port), pin, altfunc)
{
    if (!gpio_mapper.IsGPIOMapped(port, pin))
    {
        int a = 1 / 0;
    }
}

inline GPIO::GPIOHandler::GPIOHandler(
    GPIO &gpio,
    [[maybe_unused]] const GPIOMapper::GPIOPortsHandler &gpio_mapper)
    : gpio_low_handler_(gpio.gpio_)
{
}

inline void GPIO::GPIOHandler::Set() { gpio_low_handler_.Set(); }

inline void GPIO::GPIOHandler::Reset() { gpio_low_handler_.Reset(); }
} // namespace hydrv::gpio
