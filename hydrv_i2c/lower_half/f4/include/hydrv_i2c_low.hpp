#pragma once

#include <cstddef>
#include <cstdint>
#include <stm32f4xx.h>

extern "C"
{
#include "stm32f4xx.h"
}

#include "hydrv_gpio_low.hpp"

namespace hydrv::I2C
{

inline void Error(const char *)
{
    int a = 0;
    [[maybe_unused]] int b = 1 / a;
}

class I2CLow
{
public:
    struct I2CPreset
    {
        const uint32_t I2Cx;
        const uint8_t GPIO_alt_func;
        const uint32_t RCC_APBENR_I2CxEN;
        const uint32_t RCC_address;
        const IRQn_Type I2Cx_EV_IRQn;
        const IRQn_Type I2Cx_ER_IRQn;
        const int pclk_mhz;
        const int bus_khz;
    };

public:
    static constexpr I2CPreset I2C1_100KHZ_LOW{
        I2C1_BASE,
        4,
        RCC_APB1ENR_I2C1EN,
        RCC_BASE + offsetof(RCC_TypeDef, APB1ENR),
        I2C1_EV_IRQn,
        I2C1_ER_IRQn,
        42,
        100};

    static constexpr I2CPreset I2C2_100KHZ_LOW{
        I2C2_BASE,
        4,
        RCC_APB1ENR_I2C2EN,
        RCC_BASE + offsetof(RCC_TypeDef, APB1ENR),
        I2C2_EV_IRQn,
        I2C2_ER_IRQn,
        42,
        100};

    static constexpr I2CPreset I2C3_100KHZ_LOW{
        I2C3_BASE,
        4,
        RCC_APB1ENR_I2C3EN,
        RCC_BASE + offsetof(RCC_TypeDef, APB1ENR),
        I2C3_EV_IRQn,
        I2C3_ER_IRQn,
        42,
        100};

public:
    consteval I2CLow(const I2CPreset &preset, hydrv::GPIO::GPIOLow &scl_pin,
                     hydrv::GPIO::GPIOLow &sda_pin, unsigned IRQ_priority);

public:
    void Init();

    bool IsStartBit();
    bool IsAddr();
    bool IsTxEmpty();
    bool IsRxNotEmpty();
    bool IsByteTransferFinished();
    bool IsAckFailure();

    void ClearStatusBits();
    void ClearAddr();
    void ClearAckFailure();

    uint8_t GetRx();
    void SetTx(uint8_t data);

    void SendAddress(uint8_t address);

    void GenerateStart();
    void GenerateStop();

    void EnableEventInterrupt();
    void DisableEventInterrupt();
    void EnableErrorInterrupt();
    void DisableErrorInterrupt();

    void EnableAck();
    void DisableAck();

private:
    static constexpr uint32_t CountCR1Mask_();
    static constexpr uint32_t CountCR2Mask_(int pclk_mhz);
    static constexpr uint32_t CountCCRMask_(int pclk_mhz, int bus_khz);
    static constexpr uint32_t CountTRISEMask_(int pclk_mhz, int bus_khz);

    static void EnableI2CClock_(uint32_t rcc_address, uint32_t en_bit);

private:
    I2CPreset preset_;
    unsigned IRQ_priority_;
    hydrv::GPIO::GPIOLow &scl_pin_;
    hydrv::GPIO::GPIOLow &sda_pin_;

    const uint32_t cr1_;
    const uint32_t cr2_;
    const uint32_t ccr_;
    const uint32_t trise_;
};

consteval I2CLow::I2CLow(const I2CPreset &preset, hydrv::GPIO::GPIOLow &scl_pin,
                         hydrv::GPIO::GPIOLow &sda_pin, unsigned IRQ_priority)
    : preset_(preset),
      IRQ_priority_(IRQ_priority),
      scl_pin_(scl_pin),
      sda_pin_(sda_pin),
      cr1_(CountCR1Mask_()),
      cr2_(CountCR2Mask_(preset.pclk_mhz)),
      ccr_(CountCCRMask_(preset.pclk_mhz, preset.bus_khz)),
      trise_(CountTRISEMask_(preset.pclk_mhz, preset.bus_khz))
{
}

inline void I2CLow::Init()
{
    EnableI2CClock_(preset_.RCC_address, preset_.RCC_APBENR_I2CxEN);
    NVIC_SetPriority(preset_.I2Cx_EV_IRQn, IRQ_priority_);
    NVIC_SetPriority(preset_.I2Cx_ER_IRQn, IRQ_priority_);
    NVIC_EnableIRQ(preset_.I2Cx_EV_IRQn);
    NVIC_EnableIRQ(preset_.I2Cx_ER_IRQn);

    CLEAR_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->CR1, I2C_CR1_PE);
    reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->CR2 = cr2_;
    reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->CCR = ccr_;
    reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->TRISE = trise_;
    reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->OAR1 = 0x4000;
    reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->CR1 = cr1_;
    SET_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->CR1, I2C_CR1_PE);

    scl_pin_.Init(preset_.GPIO_alt_func);
    sda_pin_.Init(preset_.GPIO_alt_func);
}

inline bool I2CLow::IsStartBit()
{
    return READ_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->SR1,
                    I2C_SR1_SB);
}

inline bool I2CLow::IsAddr()
{
    return READ_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->SR1,
                    I2C_SR1_ADDR);
}

inline bool I2CLow::IsTxEmpty()
{
    return READ_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->SR1,
                    I2C_SR1_TXE);
}

inline bool I2CLow::IsRxNotEmpty()
{
    return READ_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->SR1,
                    I2C_SR1_RXNE);
}

inline bool I2CLow::IsByteTransferFinished()
{
    return READ_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->SR1,
                    I2C_SR1_BTF);
}

inline bool I2CLow::IsAckFailure()
{
    return READ_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->SR1,
                    I2C_SR1_AF);
}

inline void I2CLow::ClearStatusBits()
{
    volatile uint32_t tmpreg = 0x00U;
    tmpreg = reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->SR1;
    tmpreg = reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->SR2;
    (void)tmpreg;

    reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->SR1 = 0;
}

inline void I2CLow::ClearAddr()
{
    volatile uint32_t tmpreg = 0x00U;
    tmpreg = READ_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->SR1,
                      I2C_SR1_ADDR);
    tmpreg = READ_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->SR2,
                      I2C_SR2_MSL);
    (void)tmpreg;
}

inline void I2CLow::ClearAckFailure()
{
    CLEAR_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->SR1, I2C_SR1_AF);
}

inline uint8_t I2CLow::GetRx()
{
    return reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->DR;
}

inline void I2CLow::SetTx(uint8_t data)
{
    reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->DR = data;
}

inline void I2CLow::SendAddress(uint8_t address)
{
    reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->DR = address;
}

inline void I2CLow::GenerateStart()
{
    SET_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->CR1, I2C_CR1_START);
}

inline void I2CLow::GenerateStop()
{
    SET_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->CR1, I2C_CR1_STOP);
}

inline void I2CLow::EnableEventInterrupt()
{
    SET_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->CR2,
            I2C_CR2_ITEVTEN);
    SET_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->CR2,
            I2C_CR2_ITBUFEN);
}

inline void I2CLow::DisableEventInterrupt()
{
    CLEAR_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->CR2,
              I2C_CR2_ITEVTEN);
    CLEAR_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->CR2,
              I2C_CR2_ITBUFEN);
}

inline void I2CLow::EnableErrorInterrupt()
{
    SET_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->CR2,
            I2C_CR2_ITERREN);
}

inline void I2CLow::DisableErrorInterrupt()
{
    CLEAR_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->CR2,
              I2C_CR2_ITERREN);
}

inline void I2CLow::EnableAck()
{
    SET_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->CR1, I2C_CR1_ACK);
}

inline void I2CLow::DisableAck()
{
    CLEAR_BIT(reinterpret_cast<I2C_TypeDef *>(preset_.I2Cx)->CR1, I2C_CR1_ACK);
}

constexpr uint32_t I2CLow::CountCR1Mask_()
{
    uint32_t cr1 = 0;
    SET_BIT(cr1, I2C_CR1_ACK);
    return cr1;
}

constexpr uint32_t I2CLow::CountCR2Mask_(int pclk_mhz)
{
    if (pclk_mhz < 2 || pclk_mhz > 50)
    {
        Error("Invalid PCLK1 frequency");
    }
    return static_cast<uint32_t>(pclk_mhz) << I2C_CR2_FREQ_Pos;
}

constexpr uint32_t I2CLow::CountCCRMask_(int pclk_mhz, int bus_khz)
{
    if (bus_khz != 100)
    {
        Error("Only 100 kHz is supported");
    }
    int ccr_val = (pclk_mhz * 1000) / (bus_khz * 2);
    if (ccr_val < 4)
    {
        ccr_val = 4;
    }
    return ccr_val;
}

constexpr uint32_t I2CLow::CountTRISEMask_(int pclk_mhz, int bus_khz)
{
    if (bus_khz != 100)
    {
        Error("Only 100 kHz is supported");
    }
    return static_cast<uint32_t>(pclk_mhz + 1);
}

inline void I2CLow::EnableI2CClock_(uint32_t rcc_address, uint32_t en_bit)
{
    volatile uint32_t *rcc_reg =
        reinterpret_cast<volatile uint32_t *>(rcc_address);
    __IO uint32_t tmpreg = 0x00U;
    SET_BIT(*rcc_reg, en_bit);
    tmpreg = READ_BIT(*rcc_reg, en_bit);
    (void)tmpreg;
}

} // namespace hydrv::I2C
