#pragma once

#include <cstdint>

extern "C"
{
#include "stm32f4xx.h"
}

namespace hydrv::gpio
{
enum class Mode : uint32_t
{
    kInput = 0,
    kOutput = 1,
    kAlternate = 2,
    kAnalog = 3,
    kMask = 3
};

enum class OutputType : uint32_t
{
    kPushPull = 0,
    kOpenDrain = 1,
    kMask = 1
};

enum class OutputSpeed : uint32_t
{
    kLow = 0,
    kMedium = 1,
    kHigh = 2,
    kVeryHigh = 3,
    kMask = 3
};

enum class PullUpDown : uint32_t
{
    kNo = 0,
    kPullUp = 1,
    kPullDown = 2,
    kMask = 3
};

enum class Altfunc : uint32_t
{
    kAltfunc0 = 0,
    kAltfunc1 = 1,
    kAltfunc2 = 2,
    kAltfunc3 = 3,
    kAltfunc4 = 4,
    kAltfunc5 = 5,
    kAltfunc6 = 6,
    kAltfunc7 = 7,
    kAltfunc8 = 8,
    kAltfunc9 = 9,
    kAltfunc10 = 10,
    kAltfunc11 = 11,
    kAltfunc12 = 12,
    kAltfunc13 = 13,
    kAltfunc14 = 14,
    kAltfunc15 = 15,
    kMask = 0xF,
};

class GPIOPort
{
public:
    enum Index
    {
        kGPIOA = 0,
        kGPIOB,
        kGPIOC,
        kGPIOD,
        kPortsCount
    };

    struct RawConfig
    {
        int pin;
        Index port;
        Mode mode;
        OutputType output_type;
        OutputSpeed output_speed;
        PullUpDown pull_up_down;
        Altfunc altfunc;
    };

    // static constexpr GPIOConfig kOutput = {.pin_function =
    // GPIOFunc::kOutput}; static constexpr GPIOConfig kFastOutput =
    // {.pin_function =
    //                                                GPIOFunc::kFastOutput};
    // static constexpr GPIOConfig kUART = {.pin_function = GPIOFunc::kUART};
    // static constexpr GPIOConfig kTimer = {.pin_function = GPIOFunc::kTimer};
    // static constexpr GPIOConfig kI2C = {.pin_function = GPIOFunc::kI2C};
    // static constexpr GPIOConfig kSPI = {.pin_function = GPIOFunc::kSPI};

    static constexpr int kPinCount = 16;

    static consteval uint32_t GetGPIOx(Index port);

    template <typename T>
    consteval GPIOPort(Index port, T pins);

    void Init() const;

private:
    struct PortInfo
    {
        uint32_t GPIOx;
        uint32_t RCC_AHB1ENR_GPIOxEN;
    };

    struct AltfuncReg
    {
        uint32_t high;
        uint32_t low;
    };

    static consteval PortInfo GetPortInfo(Index port);
    static consteval uint32_t GetRCC_AHB1ENR_GPIOxEN(Index port);

    template <typename T>
    consteval uint32_t CalculateModeRegValue(T &pins);
    template <typename T>
    consteval uint32_t CalculateOutputTypeRegValue(T &pins);
    template <typename T>
    consteval uint32_t CalculateOutputSpeedRegValue(T &pins);
    template <typename T>
    consteval uint32_t CalculatePullUpDownRegValue(T &pins);
    template <typename T>
    consteval AltfuncReg CalculateAltfuncRegValue(T &pins);

    static constexpr uint32_t GetModeRegMask(Mode value, int pin);
    static constexpr uint32_t GetOutputTypeRegMask(OutputType value, int pin);
    static constexpr uint32_t GetOutputSpeedRegMask(OutputSpeed value, int pin);
    static constexpr uint32_t GetPullUpDownRegMask(PullUpDown value, int pin);
    static constexpr uint32_t GetAltfuncRegMask(Altfunc value, int pin);

    static void EnableGPIOxClock(uint32_t RCC_AHB1ENR_GPIOxEN);

    uint32_t RCC_AHB1ENR_GPIOxEN_ = 0;
    uint32_t GPIOx_ = 0;

    uint32_t mode_reg_value_ = 0;
    uint32_t output_type_reg_value_ = 0;
    uint32_t output_speed_reg_value_ = 0;
    uint32_t pull_up_down_reg_value_ = 0;

    AltfuncReg altfunc_reg_value_ = {.high = 0, .low = 0};
};

template <typename T>
consteval GPIOPort::GPIOPort(Index port, T pins)
    : RCC_AHB1ENR_GPIOxEN_(GetRCC_AHB1ENR_GPIOxEN(port)),
      GPIOx_(GetGPIOx(port)),
      mode_reg_value_(CalculateModeRegValue(pins)),
      output_type_reg_value_(CalculateOutputTypeRegValue(pins)),
      output_speed_reg_value_(CalculateOutputSpeedRegValue(pins)),
      pull_up_down_reg_value_(CalculatePullUpDownRegValue(pins)),
      altfunc_reg_value_(CalculateAltfuncRegValue(pins))
{
}

inline void GPIOPort::Init() const
{
    EnableGPIOxClock(RCC_AHB1ENR_GPIOxEN_);

    auto GPIOx = reinterpret_cast<GPIO_TypeDef *>(GPIOx_);

    GPIOx->MODER = mode_reg_value_;
    GPIOx->OTYPER = output_type_reg_value_;
    GPIOx->OSPEEDR = output_speed_reg_value_;
    GPIOx->PUPDR = pull_up_down_reg_value_;
    GPIOx->AFR[0] = altfunc_reg_value_.low;
    GPIOx->AFR[1] = altfunc_reg_value_.high;
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
consteval uint32_t GPIOPort::CalculateModeRegValue(T &pins)
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
        MODIFY_REG(result, GetModeRegMask(Mode::kMask, pin.pin),
                   GetModeRegMask(pin.mode, pin.pin));
        // switch (pin.preset.pin_function)
        // {
        // case GPIOFunc::kSPI:
        // case GPIOFunc::kUART:
        // case GPIOFunc::kTimer:
        // case GPIOFunc::kI2C:
        //     MODIFY_REG(result, GetModeRegMask(ModeReg::kMask, pin.pin),
        //                GetModeRegMask(ModeReg::kAlternate, pin.pin));

        //     break;
        // case GPIOFunc::kOutput:
        // case GPIOFunc::kFastOutput:
        //     MODIFY_REG(result, GetModeRegMask(ModeReg::kMask, pin.pin),
        //                GetModeRegMask(ModeReg::kOutput, pin.pin));
        //     break;
        // default:
        //     int a = 1 / 0;
        //     break;
        // }
    }
    return result;
}

template <typename T>
consteval uint32_t GPIOPort::CalculateOutputTypeRegValue(T &pins)
{
    uint32_t result = 0;
    for (const auto &pin : pins)
    {
        MODIFY_REG(result, GetOutputTypeRegMask(OutputType::kMask, pin.pin),
                   GetOutputTypeRegMask(pin.output_type, pin.pin));
        // switch (pin.preset.pin_function)
        // {
        // case GPIOFunc::kI2C:
        //     MODIFY_REG(
        //         result, GetOutputTypeRegMask(OutputTypeReg::kMask, pin.pin),
        //         GetOutputTypeRegMask(OutputTypeReg::kOpenDrain, pin.pin));
        //     break;
        // case GPIOFunc::kSPI:
        // case GPIOFunc::kUART:
        // case GPIOFunc::kTimer:
        // case GPIOFunc::kOutput:
        // case GPIOFunc::kFastOutput:
        //     MODIFY_REG(result,
        //                GetOutputTypeRegMask(OutputTypeReg::kMask, pin.pin),
        //                GetOutputTypeRegMask(OutputTypeReg::kPushPull,
        //                pin.pin));
        //     break;
        // default:
        //     int a = 1 / 0;
        //     break;
        // }
    }
    return result;
}

template <typename T>
consteval uint32_t GPIOPort::CalculateOutputSpeedRegValue(T &pins)
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
        MODIFY_REG(result, GetOutputSpeedRegMask(OutputSpeed::kMask, pin.pin),
                   GetOutputSpeedRegMask(pin.output_speed, pin.pin));
        // switch (pin.preset.pin_function)
        // {
        // case GPIOFunc::kSPI:
        // case GPIOFunc::kUART:
        // case GPIOFunc::kFastOutput:
        // case GPIOFunc::kI2C:
        //     MODIFY_REG(
        //         result, GetOutputSpeedRegMask(OutputSpeedReg::kMask,
        //         pin.pin), GetOutputSpeedRegMask(OutputSpeedReg::kVeryHigh,
        //         pin.pin));
        //     break;
        // case GPIOFunc::kTimer:
        // case GPIOFunc::kOutput:
        //     MODIFY_REG(result,
        //                GetOutputSpeedRegMask(OutputSpeedReg::kMask, pin.pin),
        //                GetOutputSpeedRegMask(OutputSpeedReg::kLow, pin.pin));
        //     break;
        // default:
        //     int a = 1 / 0;
        //     break;
        // }
    }
    return result;
}

template <typename T>
consteval uint32_t GPIOPort::CalculatePullUpDownRegValue(T &pins)
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
        MODIFY_REG(result, GetPullUpDownRegMask(PullUpDown::kMask, pin.pin),
                   GetPullUpDownRegMask(pin.pull_up_down, pin.pin));
        // switch (pin.preset.pin_function)
        // {
        // case GPIOFunc::kI2C:
        //     MODIFY_REG(result,
        //                GetPullUpDownRegMask(PullUpDownReg::kMask, pin.pin),
        //                GetPullUpDownRegMask(PullUpDownReg::kPullUp,
        //                pin.pin));
        //     break;
        // case GPIOFunc::kOutput:
        // case GPIOFunc::kFastOutput:
        // case GPIOFunc::kSPI:
        // case GPIOFunc::kUART:
        // case GPIOFunc::kTimer:
        //     MODIFY_REG(result,
        //                GetPullUpDownRegMask(PullUpDownReg::kMask, pin.pin),
        //                GetPullUpDownRegMask(PullUpDownReg::kNo, pin.pin));
        //     break;
        // default:
        //     int a = 1 / 0;
        //     break;
        // }
    }
    return result;
}

template <typename T>
consteval GPIOPort::AltfuncReg GPIOPort::CalculateAltfuncRegValue(T &pins)
{
    AltfuncReg result = {.high = 0, .low = 0};
    for (const auto &pin : pins)
    {
        if (pin.pin < 8)
        {
            MODIFY_REG(result.low, GetAltfuncRegMask(Altfunc::kMask, pin.pin),
                       GetAltfuncRegMask(pin.altfunc, pin.pin));
        }
        else
        {
            MODIFY_REG(result.high, GetAltfuncRegMask(Altfunc::kMask, pin.pin),
                       GetAltfuncRegMask(pin.altfunc, pin.pin));
        }
    }
    return result;
}

constexpr uint32_t GPIOPort::GetModeRegMask(Mode value, int pin)
{
    return static_cast<uint32_t>(value) << (pin * 2);
}

constexpr uint32_t GPIOPort::GetOutputTypeRegMask(OutputType value, int pin)
{
    return static_cast<uint32_t>(value) << pin;
}

constexpr uint32_t GPIOPort::GetOutputSpeedRegMask(OutputSpeed value, int pin)
{
    return static_cast<uint32_t>(value) << (pin * 2);
}

constexpr uint32_t GPIOPort::GetPullUpDownRegMask(PullUpDown value, int pin)
{
    return static_cast<uint32_t>(value) << (pin * 2);
}

constexpr uint32_t GPIOPort::GetAltfuncRegMask(Altfunc value, int pin)
{
    return static_cast<uint32_t>(value) << (4 * (pin % 8));
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