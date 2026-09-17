#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stm32f4xx.h>

extern "C"
{
#include "stm32f4xx.h"
}

#include "hydrolib_return_codes.hpp"
#include "hydrv_gpio_low.hpp"

namespace hydrv::i2c
{
enum class I2CIndex
{
    kI2C1,
    kI2C2,
    kI2C3
};

template <I2CIndex kI2CIndex>
class I2CLowBase
{
public:
    class I2CLow;

    enum class Speed
    {
        k100 = 100000
    };

    enum class GPIOSDA;
    enum class GPIOSCL;

    consteval I2CLowBase(Speed speed, int irq_priority);

    static constexpr gpio::GPIOPort::RawConfig
    GetSDAGPIOConfig(GPIOSDA sda_pin);
    static constexpr gpio::GPIOPort::RawConfig
    GetSCLPGPIOConfig(GPIOSCL scl_pin);

private:
    struct Preset
    {
        uint32_t I2Cx;
        uint8_t GPIO_alt_func;
        uint32_t RCC_APBENR_I2CxEN;
        uint32_t RCC_address;
        IRQn_Type I2Cx_EV_IRQn;
        IRQn_Type I2Cx_ER_IRQn;
        int pclk_mhz;
    };

    static constexpr Preset GetPreset();

    static constexpr uint32_t CountCR1Mask_();
    static constexpr uint32_t CountCR2Mask_(int pclk_mhz);
    static constexpr uint32_t CountCCRMask_(int pclk_mhz, Speed speed);
    static constexpr uint32_t CountTRISEMask_(int pclk_mhz);
    static void EnableI2CClock_(uint32_t rcc_address, uint32_t en_bit);

    const int irq_priority_;
    const uint32_t cr1_;
    const uint32_t cr2_;
    const uint32_t ccr_;
    const uint32_t trise_;
};

template <>
enum class I2CLowBase<I2CIndex::kI2C1>::GPIOSDA {
    kB7,
    kB9
};

template <>
enum class I2CLowBase<I2CIndex::kI2C1>::GPIOSCL {
    kB6,
    kB8
};

template <>
enum class I2CLowBase<I2CIndex::kI2C2>::GPIOSDA {
    kB11
};

template <>
enum class I2CLowBase<I2CIndex::kI2C2>::GPIOSCL {
    kB10
};

template <>
enum class I2CLowBase<I2CIndex::kI2C3>::GPIOSDA {
    kC9
};

template <>
enum class I2CLowBase<I2CIndex::kI2C3>::GPIOSCL {
    kA8
};

template <I2CIndex kI2CIndex>
consteval I2CLowBase<kI2CIndex>::I2CLowBase(Speed speed, int irq_priority)
    : irq_priority_(irq_priority),
      cr1_(CountCR1Mask_()),
      cr2_(CountCR2Mask_(GetPreset().pclk_mhz)),
      ccr_(CountCCRMask_(GetPreset().pclk_mhz, speed)),
      trise_(CountTRISEMask_(GetPreset().pclk_mhz))
{
}

template <>
constexpr gpio::GPIOPort::RawConfig
I2CLowBase<I2CIndex::kI2C1>::GetSDAGPIOConfig(GPIOSDA sda_pin)
{
    switch (sda_pin)
    {
    case GPIOSDA::kB7:
        return {.pin = 7,
                .port = gpio::GPIOPort::Index::kGPIOB,
                .mode = gpio::Mode::kAlternate,
                .output_type = gpio::OutputType::kOpenDrain,
                .output_speed = gpio::OutputSpeed::kVeryHigh,
                .pull_up_down = gpio::PullUpDown::kPullUp,
                .altfunc = gpio::Altfunc::kAltfunc4};
    case GPIOSDA::kB9:
        return {.pin = 9,
                .port = gpio::GPIOPort::Index::kGPIOB,
                .mode = gpio::Mode::kAlternate,
                .output_type = gpio::OutputType::kOpenDrain,
                .output_speed = gpio::OutputSpeed::kVeryHigh,
                .pull_up_down = gpio::PullUpDown::kPullUp,
                .altfunc = gpio::Altfunc::kAltfunc4};
    default:
        hydrolib::CompileTimeAssert(false, "Invalid SDA pin");
    }
}

template <>
constexpr gpio::GPIOPort::RawConfig
I2CLowBase<I2CIndex::kI2C1>::GetSCLPGPIOConfig(GPIOSCL scl_pin)
{
    switch (scl_pin)
    {
    case GPIOSCL::kB6:
        return {.pin = 6,
                .port = gpio::GPIOPort::Index::kGPIOB,
                .mode = gpio::Mode::kAlternate,
                .output_type = gpio::OutputType::kOpenDrain,
                .output_speed = gpio::OutputSpeed::kVeryHigh,
                .pull_up_down = gpio::PullUpDown::kPullUp,
                .altfunc = gpio::Altfunc::kAltfunc4};
    case GPIOSCL::kB8:
        return {.pin = 8,
                .port = gpio::GPIOPort::Index::kGPIOB,
                .mode = gpio::Mode::kAlternate,
                .output_type = gpio::OutputType::kOpenDrain,
                .output_speed = gpio::OutputSpeed::kVeryHigh,
                .pull_up_down = gpio::PullUpDown::kPullUp,
                .altfunc = gpio::Altfunc::kAltfunc4};
    default:
        hydrolib::CompileTimeAssert(false, "Invalid SCL pin");
    }
}

template <>
constexpr gpio::GPIOPort::RawConfig
I2CLowBase<I2CIndex::kI2C2>::GetSDAGPIOConfig(GPIOSDA sda_pin)
{
    switch (sda_pin)
    {
    case GPIOSDA::kB11:
        return {.pin = 11,
                .port = gpio::GPIOPort::Index::kGPIOB,
                .mode = gpio::Mode::kAlternate,
                .output_type = gpio::OutputType::kOpenDrain,
                .output_speed = gpio::OutputSpeed::kVeryHigh,
                .pull_up_down = gpio::PullUpDown::kPullUp,
                .altfunc = gpio::Altfunc::kAltfunc4};
    default:
        hydrolib::CompileTimeAssert(false, "Invalid SDA pin");
    }
}

template <>
constexpr gpio::GPIOPort::RawConfig
I2CLowBase<I2CIndex::kI2C2>::GetSCLPGPIOConfig(GPIOSCL scl_pin)
{
    switch (scl_pin)
    {
    case GPIOSCL::kB10:
        return {.pin = 10,
                .port = gpio::GPIOPort::Index::kGPIOB,
                .mode = gpio::Mode::kAlternate,
                .output_type = gpio::OutputType::kOpenDrain,
                .output_speed = gpio::OutputSpeed::kVeryHigh,
                .pull_up_down = gpio::PullUpDown::kPullUp,
                .altfunc = gpio::Altfunc::kAltfunc4};
    default:
        hydrolib::CompileTimeAssert(false, "Invalid SCL pin");
    }
}

template <>
constexpr gpio::GPIOPort::RawConfig
I2CLowBase<I2CIndex::kI2C3>::GetSDAGPIOConfig(GPIOSDA sda_pin)
{
    switch (sda_pin)
    {
    case GPIOSDA::kC9:
        return {.pin = 9,
                .port = gpio::GPIOPort::Index::kGPIOC,
                .mode = gpio::Mode::kAlternate,
                .output_type = gpio::OutputType::kOpenDrain,
                .output_speed = gpio::OutputSpeed::kVeryHigh,
                .pull_up_down = gpio::PullUpDown::kPullUp,
                .altfunc = gpio::Altfunc::kAltfunc4};
    default:
        hydrolib::CompileTimeAssert(false, "Invalid SDA pin");
    }
}

template <>
constexpr gpio::GPIOPort::RawConfig
I2CLowBase<I2CIndex::kI2C3>::GetSCLPGPIOConfig(GPIOSCL scl_pin)
{
    switch (scl_pin)
    {
    case GPIOSCL::kA8:
        return {.pin = 8,
                .port = gpio::GPIOPort::Index::kGPIOA,
                .mode = gpio::Mode::kAlternate,
                .output_type = gpio::OutputType::kOpenDrain,
                .output_speed = gpio::OutputSpeed::kVeryHigh,
                .pull_up_down = gpio::PullUpDown::kPullUp,
                .altfunc = gpio::Altfunc::kAltfunc4};
    default:
        hydrolib::CompileTimeAssert(false, "Invalid SCL pin");
    }
}
template <I2CIndex kI2CIndex>
constexpr I2CLowBase<kI2CIndex>::Preset I2CLowBase<kI2CIndex>::GetPreset()
{
    if constexpr (kI2CIndex == I2CIndex::kI2C1)
    {
        return {.I2Cx = I2C1_BASE,
                .GPIO_alt_func = 4,
                .RCC_APBENR_I2CxEN = RCC_APB1ENR_I2C1EN,
                .RCC_address = RCC_BASE + offsetof(RCC_TypeDef, APB1ENR),
                .I2Cx_EV_IRQn = I2C1_EV_IRQn,
                .I2Cx_ER_IRQn = I2C1_ER_IRQn,
                .pclk_mhz = 42};
    }
    else if (kI2CIndex == I2CIndex::kI2C2)
    {
        return {.I2Cx = I2C2_BASE,
                .GPIO_alt_func = 4,
                .RCC_APBENR_I2CxEN = RCC_APB1ENR_I2C2EN,
                .RCC_address = RCC_BASE + offsetof(RCC_TypeDef, APB1ENR),
                .I2Cx_EV_IRQn = I2C2_EV_IRQn,
                .I2Cx_ER_IRQn = I2C2_ER_IRQn,
                .pclk_mhz = 42};
    }
    else if (kI2CIndex == I2CIndex::kI2C3)
    {
        return {.I2Cx = I2C3_BASE,
                .GPIO_alt_func = 4,
                .RCC_APBENR_I2CxEN = RCC_APB1ENR_I2C3EN,
                .RCC_address = RCC_BASE + offsetof(RCC_TypeDef, APB1ENR),
                .I2Cx_EV_IRQn = I2C3_EV_IRQn,
                .I2Cx_ER_IRQn = I2C3_ER_IRQn,
                .pclk_mhz = 42};
    }
    else
    {
        hydrolib::CompileTimeAssert(false, "Unknown I2C index");
    }
}

template <I2CIndex kI2CIndex>
constexpr uint32_t I2CLowBase<kI2CIndex>::CountCR1Mask_()
{
    uint32_t cr1 = 0;
    SET_BIT(cr1, I2C_CR1_ACK);
    return cr1;
}

template <I2CIndex kI2CIndex>
constexpr uint32_t I2CLowBase<kI2CIndex>::CountCR2Mask_(int pclk_mhz)
{
    hydrolib::CompileTimeAssert(pclk_mhz > 2 && pclk_mhz < 50,
                                "Invalid PCLK1 frequency");
    return static_cast<uint32_t>(pclk_mhz) << I2C_CR2_FREQ_Pos;
}

template <I2CIndex kI2CIndex>
constexpr uint32_t I2CLowBase<kI2CIndex>::CountCCRMask_(int pclk_mhz,
                                                        Speed speed)
{
    int ccr_val = (pclk_mhz * 1000) / (static_cast<int>(speed) * 2);
    ccr_val = std::max(ccr_val, 4);
    return ccr_val;
}

template <I2CIndex kI2CIndex>
constexpr uint32_t I2CLowBase<kI2CIndex>::CountTRISEMask_(int pclk_mhz)
{
    return static_cast<uint32_t>(pclk_mhz + 1);
}

template <I2CIndex kI2CIndex>
void I2CLowBase<kI2CIndex>::EnableI2CClock_(uint32_t rcc_address,
                                            uint32_t en_bit)
{
    volatile uint32_t *rcc_reg =
        reinterpret_cast<volatile uint32_t *>(rcc_address);
    __IO uint32_t tmpreg = 0x00U;
    SET_BIT(*rcc_reg, en_bit);
    tmpreg = READ_BIT(*rcc_reg, en_bit);
    (void)tmpreg;
}

} // namespace hydrv::i2c
