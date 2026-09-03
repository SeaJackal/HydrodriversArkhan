#pragma once

#include <cstdint>

#include "hydrv_gpio_port.hpp"

namespace hydrv::gpio
{
template <GPIOPort::Index kPort, int kPin>
class GPIOLow
{
public:
    struct Config
    {
        using Handler = GPIOLow;

        static constexpr int kGPIOCount = 1;

        // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
        // Struct with no invariants
        OutputType output_type;
        OutputSpeed output_speed;
        PullUpDown pull_up_down;
        // NOLINTEND(misc-non-private-member-variables-in-classes)

        [[nodiscard]] consteval GPIOPort::RawConfig GetGPIOConfigs() const
        {
            return GPIOPort::RawConfig{.pin = kPin,
                                       .port = kPort,
                                       .mode = Mode::kOutput,
                                       .output_type = output_type,
                                       .output_speed = output_speed,
                                       .pull_up_down = pull_up_down,
                                       .altfunc = Altfunc::kAltfunc0};
        }
    };

    class GPIOLowHandler;

    explicit consteval GPIOLow(const Config &config);

    GPIOLow(const GPIOLow &) = delete;
    GPIOLow &operator=(const GPIOLow &) = delete;
    GPIOLow &operator=(GPIOLow &&) = delete;

    GPIOLow(GPIOLow &&) = default;

    ~GPIOLow() = default;

private:
    // TODO: SeaJackal - need to be made not movable after creating tuple to
    // store not movable objects
    // NOLINTBEGIN(cppcoreguidelines-avoid-const-or-ref-data-members)
    // Class should be not movable
    const uint32_t gpiox_;

    const uint32_t set_reg_mask_;
    const uint32_t reset_reg_mask_;
    // NOLINTEND(cppcoreguidelines-avoid-const-or-ref-data-members)

    static consteval uint32_t CalculateSetRegValue(int pin);
    static consteval uint32_t CalculateResetRegValue(int pin);
};

template <GPIOPort::Index kPort, int kPin>
class GPIOLow<kPort, kPin>::GPIOLowHandler
{
public:
    template <typename T>
    explicit GPIOLowHandler(const T &env);

    GPIOLowHandler(const GPIOLowHandler &) = delete;
    GPIOLowHandler &operator=(const GPIOLowHandler &) = delete;
    GPIOLowHandler &operator=(GPIOLowHandler &&) = delete;
    GPIOLowHandler(GPIOLowHandler &&) = delete;

    ~GPIOLowHandler() = default;

    void Set();
    void Reset();

private:
    const GPIOLow &GPIO_low_;
};

template <GPIOPort::Index kPort, int kPin>
template <typename T>
GPIOLow<kPort, kPin>::GPIOLowHandler::GPIOLowHandler(const T &env)
    : GPIO_low_(env.template GetPeriph<GPIOLow<kPort, kPin>>())
{
}

template <GPIOPort::Index kPort, int kPin>
void GPIOLow<kPort, kPin>::GPIOLowHandler::Set()
{
    auto *gpiox =
        reinterpret_cast // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast,
        // performance-no-int-to-ptr): Registers access :(
        <GPIO_TypeDef *>(GPIO_low_.gpiox_);
    gpiox->BSRR = GPIO_low_.set_reg_mask_;
}

template <GPIOPort::Index kPort, int kPin>
void GPIOLow<kPort, kPin>::GPIOLowHandler::Reset()
{
    auto *gpiox =
        reinterpret_cast // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast,
        // performance-no-int-to-ptr): Registers access :(
        <GPIO_TypeDef *>(GPIO_low_.gpiox_);
    gpiox->BSRR = GPIO_low_.reset_reg_mask_;
}

template <GPIOPort::Index kPort, int kPin>
consteval GPIOLow<kPort, kPin>::GPIOLow([[maybe_unused]] const Config &config)
    : gpiox_(GPIOPort::GetGPIOx(kPort)),
      set_reg_mask_(CalculateSetRegValue(kPin)),
      reset_reg_mask_(CalculateResetRegValue(kPin))
{
}

template <GPIOPort::Index kPort, int kPin>
consteval uint32_t GPIOLow<kPort, kPin>::CalculateSetRegValue(int pin)
{
    return 0x1UL << pin;
}

template <GPIOPort::Index kPort, int kPin>
consteval uint32_t GPIOLow<kPort, kPin>::CalculateResetRegValue(int pin)
{
    return 0x1UL << (pin + GPIO_BSRR_BR0_Pos);
}

} // namespace hydrv::gpio
