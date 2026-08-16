#pragma once

#include <cstddef>
#include <cstdint>
#include <stm32f103xb.h>

extern "C"
{
#include "stm32f1xx.h"
}

#include "hydrv_gpio_low.hpp"

namespace hydrv::SPI
{

inline void Error(const char *)
{
    int a = 0;
    [[maybe_unused]] int b = 1 / a;
}

class SPILow
{
public:
    enum class ClockPolarity : uint32_t
    {
        LOW = 0,
        HIGH = 1
    };

    enum class ClockPhase : uint32_t
    {
        FIRST_EDGE = 0,
        SECOND_EDGE = 1
    };

    enum class DataSize : uint32_t
    {
        BITS_8 = 0,
        BITS_16 = 1
    };

    enum class BitOrder : uint32_t
    {
        MSB_FIRST = 0,
        LSB_FIRST = 1
    };

public:
    class BaudratePrescaler
    {
        friend class SPILow;

    public:
        static constexpr int kMaxFrequencyKhz = 18000;

    public:
        consteval BaudratePrescaler(int main_clock_frequency_khz,
                                    int target_frequency_khz);

        consteval BaudratePrescaler(int main_clock_frequency_khz,
                                    int max_frequency_khz,
                                    int min_frequency_khz);

    private:
        uint32_t mask_;
    };

    struct SPIPreset
    {
        const uint32_t SPIx;
        const uint32_t RCC_APBENR_SPIxEN;
        const uint32_t RCC_address;
        const IRQn_Type SPIx_IRQn;
    };

public:
    // Common SPI presets for different peripherals
    static constexpr SPIPreset SPI1_LOW{
        SPI1_BASE, RCC_APB2ENR_SPI1EN,
        RCC_BASE + offsetof(RCC_TypeDef, APB2ENR), SPI1_IRQn};

    static constexpr SPIPreset SPI2_LOW{
        SPI2_BASE, RCC_APB1ENR_SPI2EN,
        RCC_BASE + offsetof(RCC_TypeDef, APB1ENR), SPI2_IRQn};

public:
    consteval SPILow(const SPIPreset &preset, hydrv::GPIO::GPIOLow &sck_pin,
                     hydrv::GPIO::GPIOLow &miso_pin,
                     hydrv::GPIO::GPIOLow &mosi_pin, unsigned IRQ_priority,
                     BaudratePrescaler baudrate_prescaler,
                     ClockPolarity clock_polarity, ClockPhase clock_phase,
                     DataSize data_size, BitOrder bit_order);

public:
    void Init();

    bool IsRxDone();
    bool IsTxDone();
    bool IsBusy();

    uint16_t GetRx();
    void SetTx(uint16_t data);

    void EnableTxInterruption();
    void DisableTxInterruption();

    void EnableRxInterruption();
    void DisableRxInterruption();

    void EnableDMATransmit();
    void EnableDMAReceive();
    void DisableDMATransmit();
    void DisableDMAReceive();

    void Enable();
    void Disable();

private:
    static constexpr uint32_t
    CountCR1Mask_(BaudratePrescaler baudrate_prescaler,
                  ClockPolarity clock_polarity, ClockPhase clock_phase,
                  DataSize data_size, BitOrder bit_order);
    static constexpr uint32_t CountCR2Mask_();

    static void EnableSPIClock_(uint32_t rcc_address, uint32_t en_bit);

private:
    SPIPreset preset_;
    unsigned IRQ_priority_;
    hydrv::GPIO::GPIOLow &sck_pin_;
    hydrv::GPIO::GPIOLow &miso_pin_;
    hydrv::GPIO::GPIOLow &mosi_pin_;

    const uint32_t cr1_;
    const uint32_t cr2_;
};

consteval SPILow::SPILow(const SPIPreset &preset, hydrv::GPIO::GPIOLow &sck_pin,
                         hydrv::GPIO::GPIOLow &miso_pin,
                         hydrv::GPIO::GPIOLow &mosi_pin, unsigned IRQ_priority,
                         BaudratePrescaler baudrate_prescaler,
                         ClockPolarity clock_polarity, ClockPhase clock_phase,
                         DataSize data_size, BitOrder bit_order)
    : preset_(preset),
      IRQ_priority_(IRQ_priority),
      sck_pin_(sck_pin),
      miso_pin_(miso_pin),
      mosi_pin_(mosi_pin),
      cr1_(CountCR1Mask_(baudrate_prescaler, clock_polarity, clock_phase,
                         data_size, bit_order)),
      cr2_(CountCR2Mask_())
{
}

inline void SPILow::Init()
{
    EnableSPIClock_(preset_.RCC_address, preset_.RCC_APBENR_SPIxEN);
    NVIC_SetPriority(preset_.SPIx_IRQn, IRQ_priority_);
    NVIC_EnableIRQ(preset_.SPIx_IRQn);

    CLEAR_BIT(reinterpret_cast<SPI_TypeDef *>(preset_.SPIx)->CR1, SPI_CR1_SPE);

    reinterpret_cast<SPI_TypeDef *>(preset_.SPIx)->CR1 = cr1_;
    reinterpret_cast<SPI_TypeDef *>(preset_.SPIx)->CR2 = cr2_;

    SET_BIT(reinterpret_cast<SPI_TypeDef *>(preset_.SPIx)->CR1, SPI_CR1_SPE);

    sck_pin_.Init(0);
    miso_pin_.Init(0);
    mosi_pin_.Init(0);
}

inline bool SPILow::IsRxDone()
{
    return READ_BIT(reinterpret_cast<SPI_TypeDef *>(preset_.SPIx)->SR,
                    SPI_SR_RXNE);
}

inline bool SPILow::IsTxDone()
{
    return READ_BIT(reinterpret_cast<SPI_TypeDef *>(preset_.SPIx)->SR,
                    SPI_SR_TXE);
}

inline bool SPILow::IsBusy()
{
    return READ_BIT(reinterpret_cast<SPI_TypeDef *>(preset_.SPIx)->SR,
                    SPI_SR_BSY);
}

inline uint16_t SPILow::GetRx()
{
    return reinterpret_cast<SPI_TypeDef *>(preset_.SPIx)->DR;
}

inline void SPILow::SetTx(uint16_t data)
{
    reinterpret_cast<SPI_TypeDef *>(preset_.SPIx)->DR = data;
}

inline void SPILow::EnableTxInterruption()
{
    SET_BIT(reinterpret_cast<SPI_TypeDef *>(preset_.SPIx)->CR2, SPI_CR2_TXEIE);
}

inline void SPILow::DisableTxInterruption()
{
    CLEAR_BIT(reinterpret_cast<SPI_TypeDef *>(preset_.SPIx)->CR2,
              SPI_CR2_TXEIE);
}

inline void SPILow::EnableRxInterruption()
{
    SET_BIT(reinterpret_cast<SPI_TypeDef *>(preset_.SPIx)->CR2, SPI_CR2_RXNEIE);
}

inline void SPILow::DisableRxInterruption()
{
    CLEAR_BIT(reinterpret_cast<SPI_TypeDef *>(preset_.SPIx)->CR2,
              SPI_CR2_RXNEIE);
}

inline void SPILow::EnableDMATransmit()
{
    SET_BIT(reinterpret_cast<SPI_TypeDef *>(preset_.SPIx)->CR2,
            SPI_CR2_TXDMAEN);
}

inline void SPILow::EnableDMAReceive()
{
    SET_BIT(reinterpret_cast<SPI_TypeDef *>(preset_.SPIx)->CR2,
            SPI_CR2_RXDMAEN);
}

inline void SPILow::DisableDMATransmit()
{
    CLEAR_BIT(reinterpret_cast<SPI_TypeDef *>(preset_.SPIx)->CR2,
              SPI_CR2_TXDMAEN);
}

inline void SPILow::DisableDMAReceive()
{
    CLEAR_BIT(reinterpret_cast<SPI_TypeDef *>(preset_.SPIx)->CR2,
              SPI_CR2_RXDMAEN);
}

inline void SPILow::Enable()
{
    SET_BIT(reinterpret_cast<SPI_TypeDef *>(preset_.SPIx)->CR1, SPI_CR1_SPE);
}

inline void SPILow::Disable()
{
    CLEAR_BIT(reinterpret_cast<SPI_TypeDef *>(preset_.SPIx)->CR1, SPI_CR1_SPE);
}

constexpr uint32_t SPILow::CountCR1Mask_(BaudratePrescaler baudrate_prescaler,
                                         ClockPolarity clock_polarity,
                                         ClockPhase clock_phase,
                                         DataSize data_size, BitOrder bit_order)
{
    uint32_t cr1 = 0;

    SET_BIT(cr1, SPI_CR1_MSTR);
    MODIFY_REG(cr1, SPI_CR1_BR, baudrate_prescaler.mask_);

    switch (clock_polarity)
    {
    case ClockPolarity::LOW:
        CLEAR_BIT(cr1, SPI_CR1_CPOL);
        break;
    case ClockPolarity::HIGH:
        SET_BIT(cr1, SPI_CR1_CPOL);
        break;
    }

    switch (clock_phase)
    {
    case ClockPhase::FIRST_EDGE:
        CLEAR_BIT(cr1, SPI_CR1_CPHA);
        break;
    case ClockPhase::SECOND_EDGE:
        SET_BIT(cr1, SPI_CR1_CPHA);
        break;
    }

    switch (data_size)
    {
    case DataSize::BITS_8:
        CLEAR_BIT(cr1, SPI_CR1_DFF);
        break;
    case DataSize::BITS_16:
        SET_BIT(cr1, SPI_CR1_DFF);
        break;
    }

    switch (bit_order)
    {
    case BitOrder::MSB_FIRST:
        CLEAR_BIT(cr1, SPI_CR1_LSBFIRST);
        break;
    case BitOrder::LSB_FIRST:
        SET_BIT(cr1, SPI_CR1_LSBFIRST);
        break;
    }

    // CLEAR_BIT(cr1, SPI_CR1_SSM);
    SET_BIT(cr1, SPI_CR1_SSM);
    SET_BIT(cr1, SPI_CR1_SSI);
    CLEAR_BIT(cr1, SPI_CR1_CRCEN);

    return cr1;
}

constexpr uint32_t SPILow::CountCR2Mask_()
{
    uint32_t cr2 = 0;
    // SET_BIT(cr2, SPI_CR2_RXNEIE);
    // SET_BIT(cr2, SPI_CR2_ERRIE);
    // SET_BIT(cr2, SPI_CR2_SSOE);
    return cr2;
}

inline void SPILow::EnableSPIClock_(uint32_t rcc_address, uint32_t en_bit)
{
    volatile uint32_t *rcc_reg =
        reinterpret_cast<volatile uint32_t *>(rcc_address);
    __IO uint32_t tmpreg = 0x00U;
    SET_BIT(*rcc_reg, en_bit);
    tmpreg = READ_BIT(*rcc_reg, en_bit);
    (void)tmpreg;
}

consteval SPILow::BaudratePrescaler::BaudratePrescaler(
    int main_clock_frequency_khz, int target_frequency_khz)
    : mask_(0)
{
    int divided_frequency = main_clock_frequency_khz / 2;
    while (target_frequency_khz < divided_frequency)
    {
        divided_frequency /= 2;
        mask_++;
    }
    if (divided_frequency != target_frequency_khz ||
        mask_ > (SPI_CR1_BR_Msk >> SPI_CR1_BR_Pos))
    {
        Error("Target frequency is not reachable");
    }
    mask_ = mask_ << SPI_CR1_BR_Pos;
}

consteval SPILow::BaudratePrescaler::BaudratePrescaler(
    int main_clock_frequency_khz, int max_frequency_khz, int min_frequency_khz)
    : mask_(0)
{
    int divided_frequency = main_clock_frequency_khz / 2;
    while (max_frequency_khz < divided_frequency)
    {
        divided_frequency /= 2;
        mask_++;
    }
    if (divided_frequency < min_frequency_khz ||
        mask_ > (SPI_CR1_BR_Msk >> SPI_CR1_BR_Pos))
    {
        Error("Frequency is not reachable");
    }
    mask_ = mask_ << SPI_CR1_BR_Pos;
}

} // namespace hydrv::SPI
