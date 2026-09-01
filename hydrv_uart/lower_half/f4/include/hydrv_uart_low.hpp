#pragma once

#include <cstddef>
#include <cstdint>
#include <tuple>

extern "C"
{
#include "stm32f4xx.h"
}

#include "hydrv_gpio_port.hpp"

namespace hydrv::uart
{
enum class UARTIndex
{
    kUSART3
};

template <UARTIndex kIndex>
class UARTLowBase
{
public:
    enum class GPIORx;
    enum class GPIOTx;

    enum class Speed
    {
        k115200 = 115200
    };

    class UARTLow;

    // static constexpr UARTPreset USART1_115200_LOW{
    //     USART1_BASE,
    //     7,
    //     RCC_APB2ENR_USART1EN,
    //     RCC_BASE + offsetof(RCC_TypeDef, APB2ENR),
    //     USART1_IRQn,
    //     45,
    //     9};

    // static constexpr UARTPreset USART1_921600_LOW{
    //     USART1_BASE,
    //     7,
    //     RCC_APB2ENR_USART1EN,
    //     RCC_BASE + offsetof(RCC_TypeDef, APB2ENR),
    //     USART1_IRQn,
    //     5,
    //     11};

    // static constexpr UARTPreset USART2_115200_LOW{
    //     USART2_BASE,
    //     7,
    //     RCC_APB1ENR_USART2EN,
    //     RCC_BASE + offsetof(RCC_TypeDef, APB1ENR),
    //     USART2_IRQn,
    //     22,
    //     13};

    // static constexpr UARTPreset USART3_115200_LOW{
    //     USART3_BASE,
    //     7,
    //     RCC_APB1ENR_USART3EN,
    //     RCC_BASE + offsetof(RCC_TypeDef, APB1ENR),
    //     USART3_IRQn,
    //     22,
    //     13};

    // static constexpr UARTPreset USART3_921600_LOW{
    //     USART3_BASE,
    //     7,
    //     RCC_APB1ENR_USART3EN,
    //     RCC_BASE + offsetof(RCC_TypeDef, APB1ENR),
    //     USART3_IRQn,
    //     2,
    //     14};

    static constexpr gpio::GPIOPort::RawConfig GetRxGPIOConfig(GPIORx rx_pin);
    static constexpr gpio::GPIOPort::RawConfig GetTxGPIOConfig(GPIOTx tx_pin);

    consteval UARTLowBase(UARTLowBase<kIndex>::Speed speed, int IRQ_priority);

private:
    struct UARTPreset
    {
        uint32_t USARTx;

        uint32_t RCC_APBENR_UARTxEN;
        uint32_t RCC_address;

        IRQn_Type USARTx_IRQn;

        gpio::Altfunc GPIO_alt_func;
    };

    struct GPIOData
    {
        gpio::GPIOPort::Index port;
        int pin;
    };

    static constexpr GPIOData GetRxGPIOData(GPIORx rx_pin);
    static constexpr GPIOData GetTxGPIOData(GPIOTx tx_pin);
    static constexpr UARTPreset GetUARTPreset();

    static constexpr uint32_t CountCR1Mask();
    static constexpr uint32_t CountCR2Mask();
    static constexpr uint32_t CountBRRMask(Speed speed);

    static void EnableUARTClock(uint32_t rcc_address, uint32_t en_bit);
    static constexpr uint32_t USARTBRRDIVFractionVal(uint32_t val);
    static constexpr uint32_t USARTBRRDIVMantissaVal(uint32_t val);
    static constexpr uint32_t USARTCR2Stop1bit();

    int IRQ_priority_;

    const uint32_t cr1_;
    const uint32_t cr2_;
    const uint32_t brr_;
};

template <>
enum class UARTLowBase<UARTIndex::kUSART3>::GPIORx {
    kB11,
    kC11,
    kD9
};

template <>
enum class UARTLowBase<UARTIndex::kUSART3>::GPIOTx {
    kB10,
    kC10,
    kD8
};

template <UARTIndex kIndex>
class UARTLowBase<kIndex>::UARTLow
{
public:
    UARTLow(UARTLowBase<kIndex> &uart_low_base);

    bool IsRxDone();
    bool IsTxDone();

    uint8_t GetRx();
    void SetTx(uint8_t byte);

    void EnableTxInterruption();
    void DisableTxInterruption();
    void EnableRxInterruption();
    void DisableRxInterruption();

    void EnableDMATransmit();
    void EnableDMAReceive();

private:
    const UARTLowBase &uart_low_base_;
};

template <UARTIndex kIndex>
constexpr gpio::GPIOPort::RawConfig
UARTLowBase<kIndex>::GetRxGPIOConfig(GPIORx rx_pin)
{
    return {.pin = GetRxGPIOData(rx_pin).pin,
            .port = GetRxGPIOData(rx_pin).port,
            .mode = gpio::Mode::kAlternate,
            .output_type = gpio::OutputType::kPushPull,
            .output_speed = gpio::OutputSpeed::kVeryHigh,
            .pull_up_down = gpio::PullUpDown::kNo,
            .altfunc = GetUARTPreset().GPIO_alt_func};
}

template <UARTIndex kIndex>
constexpr gpio::GPIOPort::RawConfig
UARTLowBase<kIndex>::GetTxGPIOConfig(GPIOTx tx_pin)
{
    return {.pin = GetTxGPIOData(tx_pin).pin,
            .port = GetTxGPIOData(tx_pin).port,
            .mode = gpio::Mode::kAlternate,
            .output_type = gpio::OutputType::kPushPull,
            .output_speed = gpio::OutputSpeed::kVeryHigh,
            .pull_up_down = gpio::PullUpDown::kNo,
            .altfunc = GetUARTPreset().GPIO_alt_func};
}

template <>
constexpr UARTLowBase<UARTIndex::kUSART3>::GPIOData
UARTLowBase<UARTIndex::kUSART3>::GetRxGPIOData(GPIORx rx_pin)
{
    switch (rx_pin)
    {
    case GPIORx::kB11:
        return GPIOData{.port = gpio::GPIOPort::Index::kGPIOB, .pin = 10};
    case GPIORx::kC11:
        return GPIOData{.port = gpio::GPIOPort::Index::kGPIOC, .pin = 11};
    case GPIORx::kD9:
        return GPIOData{.port = gpio::GPIOPort::Index::kGPIOD, .pin = 9};
    default:
        int a = 1 / 0;
    }
}

template <>
constexpr UARTLowBase<UARTIndex::kUSART3>::GPIOData
UARTLowBase<UARTIndex::kUSART3>::GetTxGPIOData(GPIOTx tx_pin)
{
    switch (tx_pin)
    {
    case GPIOTx::kB10:
        return GPIOData{.port = gpio::GPIOPort::Index::kGPIOB, .pin = 10};
    case GPIOTx::kC10:
        return GPIOData{.port = gpio::GPIOPort::Index::kGPIOC, .pin = 10};
    case GPIOTx::kD8:
        return GPIOData{.port = gpio::GPIOPort::Index::kGPIOD, .pin = 8};
    default:
        int a = 1 / 0;
    }
}

template <>
constexpr UARTLowBase<UARTIndex::kUSART3>::UARTPreset
UARTLowBase<UARTIndex::kUSART3>::GetUARTPreset()
{
    return UARTPreset{.USARTx = USART3_BASE,
                      .RCC_APBENR_UARTxEN = RCC_APB1ENR_USART3EN,
                      .RCC_address = RCC_BASE + offsetof(RCC_TypeDef, APB1ENR),
                      .USARTx_IRQn = USART3_IRQn,
                      .GPIO_alt_func = gpio::Altfunc::kAltfunc7};
}

template <UARTIndex kIndex>
constexpr uint32_t UARTLowBase<kIndex>::USARTBRRDIVFractionVal(uint32_t val)
{
    return val << USART_BRR_DIV_Fraction_Pos;
}

template <UARTIndex kIndex>
constexpr uint32_t UARTLowBase<kIndex>::USARTBRRDIVMantissaVal(uint32_t val)
{
    return val << USART_BRR_DIV_Mantissa_Pos;
}

template <UARTIndex kIndex>
constexpr uint32_t UARTLowBase<kIndex>::USARTCR2Stop1bit()
{
    return 0x0UL << USART_CR2_STOP_Pos;
}

template <UARTIndex kIndex>
void UARTLowBase<kIndex>::EnableUARTClock(uint32_t rcc_address, uint32_t en_bit)
{
    volatile uint32_t *rcc_reg =
        reinterpret_cast<volatile uint32_t *>(rcc_address);
    __IO uint32_t tmpreg = 0x00U;
    SET_BIT(*rcc_reg,
            en_bit); /* Delay after an RCC peripheral clock enabling */
    tmpreg = READ_BIT(*rcc_reg, en_bit);
    (void)tmpreg;
}

template <UARTIndex kIndex>
consteval UARTLowBase<kIndex>::UARTLowBase(Speed speed, int IRQ_priority)
    : IRQ_priority_(IRQ_priority),
      cr1_(CountCR1Mask()),
      cr2_(CountCR2Mask()),
      brr_(CountBRRMask(speed))
{
}

template <UARTIndex kIndex>
UARTLowBase<kIndex>::UARTLow::UARTLow(UARTLowBase<kIndex> &uart_low_base)
    : uart_low_base_(uart_low_base)
{
    auto preset = uart_low_base_.GetUARTPreset();
    EnableUARTClock(preset.RCC_address, preset.RCC_APBENR_UARTxEN);
    NVIC_SetPriority(preset.USARTx_IRQn, uart_low_base_.IRQ_priority_);
    NVIC_EnableIRQ(preset.USARTx_IRQn);

    auto USARTx = reinterpret_cast<USART_TypeDef *>(preset.USARTx);

    CLEAR_BIT(USARTx->CR1, USART_CR1_UE);

    USARTx->CR1 = uart_low_base_.cr1_;
    USARTx->CR2 = uart_low_base_.cr2_;
    USARTx->BRR = uart_low_base_.brr_;

    SET_BIT(USARTx->CR1, USART_CR1_UE);
}

template <UARTIndex kIndex>
bool UARTLowBase<kIndex>::UARTLow::IsRxDone()
{
    auto preset = uart_low_base_.GetUARTPreset();
    return READ_BIT(reinterpret_cast<USART_TypeDef *>(preset.USARTx)->SR,
                    USART_SR_RXNE);
}

template <UARTIndex kIndex>
bool UARTLowBase<kIndex>::UARTLow::IsTxDone()
{
    auto preset = uart_low_base_.GetUARTPreset();
    return READ_BIT(reinterpret_cast<USART_TypeDef *>(preset.USARTx)->SR,
                    USART_SR_TC);
}

template <UARTIndex kIndex>
uint8_t UARTLowBase<kIndex>::UARTLow::GetRx()
{
    auto preset = uart_low_base_.GetUARTPreset();
    return reinterpret_cast<USART_TypeDef *>(preset.USARTx)->DR;
}

template <UARTIndex kIndex>
void UARTLowBase<kIndex>::UARTLow::SetTx(uint8_t byte)
{
    auto preset = uart_low_base_.GetUARTPreset();
    reinterpret_cast<USART_TypeDef *>(preset.USARTx)->DR = byte;
}

template <UARTIndex kIndex>
void UARTLowBase<kIndex>::UARTLow::EnableTxInterruption()
{
    auto preset = uart_low_base_.GetUARTPreset();
    SET_BIT(reinterpret_cast<USART_TypeDef *>(preset.USARTx)->CR1,
            USART_CR1_TCIE);
}

template <UARTIndex kIndex>
void UARTLowBase<kIndex>::UARTLow::DisableTxInterruption()
{
    auto preset = uart_low_base_.GetUARTPreset();
    CLEAR_BIT(reinterpret_cast<USART_TypeDef *>(preset.USARTx)->CR1,
              USART_CR1_TCIE);
}

template <UARTIndex kIndex>
void UARTLowBase<kIndex>::UARTLow::EnableRxInterruption()
{
    auto preset = uart_low_base_.GetUARTPreset();
    SET_BIT(reinterpret_cast<USART_TypeDef *>(preset.USARTx)->CR1,
            USART_CR1_RXNEIE);
}

template <UARTIndex kIndex>
void UARTLowBase<kIndex>::UARTLow::DisableRxInterruption()
{
    auto preset = uart_low_base_.GetUARTPreset();
    CLEAR_BIT(reinterpret_cast<USART_TypeDef *>(preset.USARTx)->CR1,
              USART_CR1_RXNEIE);
}

template <UARTIndex kIndex>
void UARTLowBase<kIndex>::UARTLow::EnableDMATransmit()
{
    auto preset = uart_low_base_.GetUARTPreset();
    SET_BIT(reinterpret_cast<USART_TypeDef *>(preset.USARTx)->CR3,
            USART_CR3_DMAT);
}

template <UARTIndex kIndex>
void UARTLowBase<kIndex>::UARTLow::EnableDMAReceive()
{
    auto preset = uart_low_base_.GetUARTPreset();
    SET_BIT(reinterpret_cast<USART_TypeDef *>(preset.USARTx)->CR3,
            USART_CR3_DMAR);
}

template <UARTIndex kIndex>
constexpr uint32_t UARTLowBase<kIndex>::CountCR1Mask()
{
    uint32_t cr1 = 0;
    CLEAR_BIT(cr1, USART_CR1_M);     // 8 bits including parity
    CLEAR_BIT(cr1, USART_CR1_PCE);   // parity disable
    SET_BIT(cr1, USART_CR1_PS);      // odd parity
    CLEAR_BIT(cr1, USART_CR1_OVER8); // 16-bit oversampling
    SET_BIT(cr1, USART_CR1_TE);
    SET_BIT(cr1, USART_CR1_RE);
    SET_BIT(cr1, USART_CR1_RXNEIE);

    return cr1;
}

template <UARTIndex kIndex>
constexpr uint32_t UARTLowBase<kIndex>::CountCR2Mask()
{
    uint32_t cr2 = 0;
    MODIFY_REG(cr2, USART_CR2_STOP, USARTCR2Stop1bit());
    return cr2;
}

template <>
constexpr uint32_t UARTLowBase<UARTIndex::kUSART3>::CountBRRMask(Speed speed)
{
    int fraction = 0;
    int mantissa = 0;
    switch (speed)
    {
    case Speed::k115200:
        mantissa = 22;
        fraction = 13;
        break;
    default:
        int a = 1 / 0;
    }

    uint32_t brr = 0;
    MODIFY_REG(brr, USART_BRR_DIV_Fraction, USARTBRRDIVFractionVal(fraction));
    MODIFY_REG(brr, USART_BRR_DIV_Mantissa, USARTBRRDIVMantissaVal(mantissa));
    return brr;
}

} // namespace hydrv::uart
