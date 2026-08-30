#pragma once

#include <cstdint>

extern "C"
{
#include "stm32f1xx.h"
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

    struct Mode
    {
        enum Value : uint32_t
        {
            kInput = 0x00,
            kOutput10MHz = 0x01,
            kOutput2MHz = 0x02,
            kOutput50MHz = 0x03
        };
    };

    struct Configure
    {
        enum Value : uint32_t // TODO: Make check Input-Output from mode
        {
            kAnalogInput = 0x00,
            kFloatingInput = 0x01,
            kPullUpDownInput = 0x02,
            kGeneralPurposePushPullOutput = 0x00,
            kGeneralPurposeOpenDrainOutput = 0x01,
            kAlternateFunctionPushPullOutput = 0x02,
            kAlternateFunctionOpenDrainOutput = 0x03
        };
    };

    struct GPIOPreset
    {
        Mode::Value mode;
        Configure::Value configure;
    };

    static constexpr GPIOPreset kOutput{
        Mode::Value::kOutput2MHz,
        Configure::Value::kGeneralPurposePushPullOutput};
    static constexpr GPIOPreset kFastOutput{
        Mode::kOutput50MHz, Configure::kGeneralPurposePushPullOutput};
    static constexpr GPIOPreset kUART_TX{
        Mode::kOutput10MHz, Configure::kAlternateFunctionPushPullOutput};
    static constexpr GPIOPreset kUART_RX{Mode::kInput,
                                         Configure::kFloatingInput};
    static constexpr GPIOPreset kSPIInput{Mode::kInput,
                                          Configure::kFloatingInput};
    static constexpr GPIOPreset kSPIOutput{
        Mode::kOutput50MHz, Configure::kAlternateFunctionPushPullOutput};

    static constexpr int kPinCount = 16;

    template <typename T>
    consteval GPIOPort(Index port, T pins);

    void Init() const;

private:
    struct PortInfo
    {
        uint32_t GPIOx;
        uint32_t RCC_APB2ENR_IOPxEN;
    };

    struct ControlReg
    {
        uint32_t high;
        uint32_t low;
    };

    static consteval PortInfo GetPortInfo(Index port);
    static consteval uint32_t GetRCC_APB2ENR_IOPxEN(Index port);
    static consteval uint32_t GetGPIOx(Index port);

    template <typename T>
    consteval ControlReg CalculateControlRegValue(T pins);
    template <typename T>
    consteval uint32_t CalculateOutputTypeRegValue(T pins);
    template <typename T>
    consteval uint32_t CalculateOutputSpeedRegValue(T pins);
    template <typename T>
    consteval uint32_t CalculatePullUpDownRegValue(T pins);

    static constexpr uint32_t GetModeInControlRegMask(Mode::Value value,
                                                      int pin);
    static constexpr uint32_t
    GetConfigureInControlRegMask(Configure::Value value, int pin);

    static void EnableGPIOxClock(uint32_t RCC_APB2ENR_IOPxEN);

    uint32_t RCC_APB2ENR_IOPxEN_ = 0;
    uint32_t GPIOx_ = 0;

    ControlReg control_reg_;
};

template <typename T>
consteval GPIOPort::GPIOPort(Index port, T pins)
    : RCC_APB2ENR_IOPxEN_(GetRCC_APB2ENR_IOPxEN(port)),
      GPIOx_(GetGPIOx(port)),
      control_reg_(CalculateControlRegValue(pins))
{
}

inline void GPIOPort::Init() const
{
    EnableGPIOxClock(RCC_APB2ENR_IOPxEN_);

    auto GPIOx = reinterpret_cast<GPIO_TypeDef *>(GPIOx_);

    GPIOx->CRH = control_reg_.high;
    GPIOx->CRL = control_reg_.low;
}

consteval GPIOPort::PortInfo GPIOPort::GetPortInfo(Index port)
{
    switch (port)
    {
    case kGPIOA:
        return {.GPIOx = GPIOA_BASE, .RCC_APB2ENR_IOPxEN = RCC_APB2ENR_IOPAEN};
    case kGPIOB:
        return {.GPIOx = GPIOB_BASE, .RCC_APB2ENR_IOPxEN = RCC_APB2ENR_IOPBEN};
    case kGPIOC:
        return {.GPIOx = GPIOC_BASE, .RCC_APB2ENR_IOPxEN = RCC_APB2ENR_IOPCEN};
    case kGPIOD:
        return {.GPIOx = GPIOD_BASE, .RCC_APB2ENR_IOPxEN = RCC_APB2ENR_IOPDEN};
    default:
        int a = 1 / 0;
        break;
    }
}

consteval uint32_t GPIOPort::GetGPIOx(Index port)
{
    return GetPortInfo(port).GPIOx;
}

consteval uint32_t GPIOPort::GetRCC_APB2ENR_IOPxEN(Index port)
{
    return GetPortInfo(port).RCC_APB2ENR_IOPxEN;
}

template <typename T>
consteval GPIOPort::ControlReg GPIOPort::CalculateControlRegValue(T pins)
{
    ControlReg result{.high = 0x44444444, .low = 0x44444444};

    for (const auto &pin : pins)
    {
        if (pin.pin > 7)
        {
            result.high |=
                GetModeInControlRegMask(pin.preset.mode, pin.pin) |
                GetConfigureInControlRegMask(pin.preset.configure, pin.pin);
        }
        else
        {
            result.low |=
                GetModeInControlRegMask(pin.preset.mode, pin.pin) |
                GetConfigureInControlRegMask(pin.preset.configure, pin.pin);
        }
    }
    return result;
}

constexpr uint32_t GPIOPort::GetModeInControlRegMask(Mode::Value value, int pin)
{
    return value << ((pin % 8) * 4);
}

constexpr uint32_t
GPIOPort::GetConfigureInControlRegMask(Configure::Value value, int pin)
{
    return value << ((pin % 8) * 4 + 2);
}

inline void GPIOPort::EnableGPIOxClock(uint32_t RCC_APB2ENR_IOPxEN)
{
    __IO uint32_t tmpreg = 0x00U;
    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPxEN); /* Delay after an RCC peripheral
                                                   clock enabling */
    tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPxEN);
    (void)tmpreg;
}
} // namespace hydrv::gpio
