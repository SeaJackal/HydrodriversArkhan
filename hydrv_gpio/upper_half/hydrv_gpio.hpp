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
                   GPIOMapper &gpio_mapper);

private:
    GPIOPort::GPIOLow gpio_;
};

class GPIO::GPIOHandler
{
public:
    GPIOHandler(GPIO &gpio);

    void Set();
    void Reset();

private:
    GPIOPort::GPIOLow::GPIOLowHandler gpio_low_handler_;
};

consteval GPIO::GPIO(GPIOPort::Index port, int pin, int altfunc,
                     GPIOMapper &gpio_mapper)
    : gpio_(gpio_mapper.GetPort(port), pin, altfunc)
{
}

inline GPIO::GPIOHandler::GPIOHandler(GPIO &gpio)
    : gpio_low_handler_(gpio.gpio_)
{
}

inline void GPIO::GPIOHandler::Set() { gpio_low_handler_.Set(); }

inline void GPIO::GPIOHandler::Reset() { gpio_low_handler_.Reset(); }
} // namespace hydrv::gpio
