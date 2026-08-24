#pragma once

#include <cstdint>

extern "C"
{
#include "stm32f4xx.h"
}

namespace hydrv::gpio
{
class GPIOPort
{
public:
    class GPIOLow;

    enum Index
    {
        kGPIOA = 0,
        kGPIOB,
        kGPIOC,
        kGPIOD,
        kPortsCount
    };

    enum class GPIOFunc
    {
        kOutput = 0,
        kFastOutput,
        kUART,
        kTimer,
        kI2C,
        kSPI
    };

    struct GPIOPreset
    {
        GPIOFunc pin_function;
    };

    static constexpr GPIOPreset kOutput = {.pin_function = GPIOFunc::kOutput};
    static constexpr GPIOPreset kFastOutput = {.pin_function =
                                                   GPIOFunc::kFastOutput};
    static constexpr GPIOPreset kUART = {.pin_function = GPIOFunc::kUART};
    static constexpr GPIOPreset kTimer = {.pin_function = GPIOFunc::kTimer};
    static constexpr GPIOPreset kI2C = {.pin_function = GPIOFunc::kI2C};
    static constexpr GPIOPreset kSPI = {.pin_function = GPIOFunc::kSPI};

    static constexpr int kPinCount = 16;

    consteval GPIOPort() = default;

    template <typename T>
    consteval GPIOPort(Index port, T pins);

    void Init();

private:
    struct PortInfo
    {
        uint32_t GPIOx;
        uint32_t RCC_AHB1ENR_GPIOxEN;
    };

    struct ModeReg
    {
        enum Value : uint32_t
        {
            kInput = 0,
            kOutput = 1,
            kAlternate = 2,
            kAnalog = 3,
            kMask = 3
        };
    };

    struct OutputTypeReg
    {
        enum Value : uint32_t
        {
            kPushPull = 0,
            kOpenDrain = 1,
            kMask = 1
        };
    };

    struct OutputSpeedReg
    {
        enum Value : uint32_t
        {
            kLow = 0,
            kMedium = 1,
            kHigh = 2,
            kVeryHigh = 3,
            kMask = 3
        };
    };

    struct PullUpDownReg
    {
        enum Value : uint32_t
        {
            kNo = 0,
            kPullUp = 1,
            kPullDown = 2,
            kMask = 3
        };
    };

    static consteval PortInfo GetPortInfo(Index port);
    static consteval uint32_t GetRCC_AHB1ENR_GPIOxEN(Index port);
    static consteval uint32_t GetGPIOx(Index port);

    template <typename T>
    consteval uint32_t CalculateModeRegValue(T pins);
    template <typename T>
    consteval uint32_t CalculateOutputTypeRegValue(T pins);
    template <typename T>
    consteval uint32_t CalculateOutputSpeedRegValue(T pins);
    template <typename T>
    consteval uint32_t CalculatePullUpDownRegValue(T pins);

    static constexpr uint32_t GetModeRegMask(ModeReg::Value value, int pin);
    static constexpr uint32_t GetOutputTypeRegMask(OutputTypeReg::Value value,
                                                   int pin);
    static constexpr uint32_t GetOutputSpeedRegMask(OutputSpeedReg::Value value,
                                                    int pin);
    static constexpr uint32_t GetPullUpDownRegMask(PullUpDownReg::Value value,
                                                   int pin);

    static void EnableGPIOxClock(uint32_t RCC_AHB1ENR_GPIOxEN);

    uint32_t RCC_AHB1ENR_GPIOxEN_ = 0;
    uint32_t GPIOx_ = 0;

    uint32_t mode_reg_value_ = 0;
    uint32_t output_type_reg_value_ = 0;
    uint32_t output_speed_reg_value_ = 0;
    uint32_t pull_up_down_reg_value_ = 0;

    bool is_inited_ = false;
};

template <typename T>
consteval GPIOPort::GPIOPort(Index port, T pins)
    : RCC_AHB1ENR_GPIOxEN_(GetRCC_AHB1ENR_GPIOxEN(port)),
      GPIOx_(GetGPIOx(port)),
      mode_reg_value_(CalculateModeRegValue(pins)),
      output_type_reg_value_(CalculateOutputTypeRegValue(pins)),
      output_speed_reg_value_(CalculateOutputSpeedRegValue(pins)),
      pull_up_down_reg_value_(CalculatePullUpDownRegValue(pins))
{
}

inline void GPIOPort::Init()
{
    if (is_inited_)
    {
        return;
    }

    EnableGPIOxClock(RCC_AHB1ENR_GPIOxEN_);

    auto GPIOx = reinterpret_cast<GPIO_TypeDef *>(GPIOx_);

    GPIOx->MODER = mode_reg_value_;
    GPIOx->OTYPER = output_type_reg_value_;
    GPIOx->OSPEEDR = output_speed_reg_value_;
    GPIOx->PUPDR = pull_up_down_reg_value_;

    is_inited_ = true;
}

consteval GPIOPort::PortInfo GPIOPort::GetPortInfo(Index port)
{
    switch (port)
    {
    case kGPIOA:
        return {.GPIOx = GPIOA_BASE,
                .RCC_AHB1ENR_GPIOxEN = RCC_AHB1ENR_GPIOAEN};
    case kGPIOB:
        return {.GPIOx = GPIOB_BASE,
                .RCC_AHB1ENR_GPIOxEN = RCC_AHB1ENR_GPIOBEN};
    case kGPIOC:
        return {.GPIOx = GPIOC_BASE,
                .RCC_AHB1ENR_GPIOxEN = RCC_AHB1ENR_GPIOCEN};
    case kGPIOD:
        return {.GPIOx = GPIOD_BASE,
                .RCC_AHB1ENR_GPIOxEN = RCC_AHB1ENR_GPIODEN};
    default:
        int a = 1 / 0;
        break;
    }
}

consteval uint32_t GPIOPort::GetGPIOx(Index port)
{
    return GetPortInfo(port).GPIOx;
}

consteval uint32_t GPIOPort::GetRCC_AHB1ENR_GPIOxEN(Index port)
{
    return GetPortInfo(port).RCC_AHB1ENR_GPIOxEN;
}

template <typename T>
consteval uint32_t GPIOPort::CalculateModeRegValue(T pins)
{
    uint32_t result = 0;
    if (GPIOx_ == GPIOA_BASE)
    {
        result = 0xA8000000;
    }
    else if (GPIOx_ == GPIOB_BASE)
    {
        result = 0x00000280;
    }

    for (const auto &pin : pins)
    {
        switch (pin.preset.pin_function)
        {
        case GPIOFunc::kSPI:
        case GPIOFunc::kUART:
        case GPIOFunc::kTimer:
        case GPIOFunc::kI2C:
            MODIFY_REG(result, GetModeRegMask(ModeReg::kMask, pin.pin),
                       GetModeRegMask(ModeReg::kAlternate, pin.pin));

            break;
        case GPIOFunc::kOutput:
        case GPIOFunc::kFastOutput:
            MODIFY_REG(result, GetModeRegMask(ModeReg::kMask, pin.pin),
                       GetModeRegMask(ModeReg::kOutput, pin.pin));
            break;
        default:
            int a = 1 / 0;
            break;
        }
    }
    return result;
}

template <typename T>
consteval uint32_t GPIOPort::CalculateOutputTypeRegValue(T pins)
{
    uint32_t result = 0;
    for (const auto &pin : pins)
    {
        switch (pin.preset.pin_function)
        {
        case GPIOFunc::kI2C:
            MODIFY_REG(
                result, GetOutputTypeRegMask(OutputTypeReg::kMask, pin.pin),
                GetOutputTypeRegMask(OutputTypeReg::kOpenDrain, pin.pin));
            break;
        case GPIOFunc::kSPI:
        case GPIOFunc::kUART:
        case GPIOFunc::kTimer:
        case GPIOFunc::kOutput:
        case GPIOFunc::kFastOutput:
            MODIFY_REG(result,
                       GetOutputTypeRegMask(OutputTypeReg::kMask, pin.pin),
                       GetOutputTypeRegMask(OutputTypeReg::kPushPull, pin.pin));
            break;
        default:
            int a = 1 / 0;
            break;
        }
    }
    return result;
}

template <typename T>
consteval uint32_t GPIOPort::CalculateOutputSpeedRegValue(T pins)
{
    uint32_t result = 0;
    if (GPIOx_ == GPIOA_BASE)
    {
        result = 0x0C000000;
    }
    else if (GPIOx_ == GPIOB_BASE)
    {
        result = 0x000000C0;
    }

    for (const auto &pin : pins)
    {
        switch (pin.preset.pin_function)
        {
        case GPIOFunc::kSPI:
        case GPIOFunc::kUART:
        case GPIOFunc::kFastOutput:
        case GPIOFunc::kI2C:
            MODIFY_REG(
                result, GetOutputSpeedRegMask(OutputSpeedReg::kMask, pin.pin),
                GetOutputSpeedRegMask(OutputSpeedReg::kVeryHigh, pin.pin));
            break;
        case GPIOFunc::kTimer:
        case GPIOFunc::kOutput:
            MODIFY_REG(result,
                       GetOutputSpeedRegMask(OutputSpeedReg::kMask, pin.pin),
                       GetOutputSpeedRegMask(OutputSpeedReg::kLow, pin.pin));
            break;
        default:
            int a = 1 / 0;
            break;
        }
    }
    return result;
}

template <typename T>
consteval uint32_t GPIOPort::CalculatePullUpDownRegValue(T pins)
{
    uint32_t result = 0;
    if (GPIOx_ == GPIOA_BASE)
    {
        result = 0x64000000;
    }
    else if (GPIOx_ == GPIOB_BASE)
    {
        result = 0x00000100;
    }

    for (const auto &pin : pins)
    {
        switch (pin.preset.pin_function)
        {
        case GPIOFunc::kI2C:
            MODIFY_REG(result,
                       GetPullUpDownRegMask(PullUpDownReg::kMask, pin.pin),
                       GetPullUpDownRegMask(PullUpDownReg::kPullUp, pin.pin));
            break;
        case GPIOFunc::kOutput:
        case GPIOFunc::kFastOutput:
        case GPIOFunc::kSPI:
        case GPIOFunc::kUART:
        case GPIOFunc::kTimer:
            MODIFY_REG(result,
                       GetPullUpDownRegMask(PullUpDownReg::kMask, pin.pin),
                       GetPullUpDownRegMask(PullUpDownReg::kNo, pin.pin));
            break;
        default:
            int a = 1 / 0;
            break;
        }
    }
    return result;
}

constexpr uint32_t GPIOPort::GetModeRegMask(ModeReg::Value value, int pin)
{
    return value << (pin * 2);
}

constexpr uint32_t GPIOPort::GetOutputTypeRegMask(OutputTypeReg::Value value,
                                                  int pin)
{
    return value << pin;
}

constexpr uint32_t GPIOPort::GetOutputSpeedRegMask(OutputSpeedReg::Value value,
                                                   int pin)
{
    return value << (pin * 2);
}

constexpr uint32_t GPIOPort::GetPullUpDownRegMask(PullUpDownReg::Value value,
                                                  int pin)
{
    return value << (pin * 2);
}

inline void GPIOPort::EnableGPIOxClock(uint32_t RCC_AHB1ENR_GPIOxEN)
{
    __IO uint32_t tmpreg = 0x00U;
    SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOxEN); /* Delay after an RCC peripheral
                                                   clock enabling */
    tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOxEN);
    (void)tmpreg;
}
} // namespace hydrv::gpio