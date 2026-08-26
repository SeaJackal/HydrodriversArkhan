#pragma once

#include "hydrv_gpio_port.hpp"
#include <cstddef>
#include <cstdint>

extern "C"
{
#include "stm32f1xx.h"
}

#include "hydrv_gpio_low.hpp"
#include "hydrv_gpio_mapper.hpp"

namespace hydrv::uart
{

enum class UARTIndex
{
    kUSART3
};

enum class Speed
{
    k115200
};

template <UARTIndex kIndex, Speed kSpeed>
class UARTLow
{
public:
    enum class GPIORx;
    enum class GPIOTx;

    class UARTLowHandler;

    consteval UARTLow(gpio::GPIOMapper &gpio_mapper, GPIORx rx_pin,
                      GPIOTx tx_pin, int IRQ_priority);

private:
    struct UARTPreset
    {
        uint32_t USARTx;

        uint8_t GPIO_alt_func;

        uint32_t RCC_APBENR_UARTxEN;
        uint32_t RCC_address;

        IRQn_Type USARTx_IRQn;

        unsigned mantissa;
        unsigned fraction;
    };

    static constexpr UARTPreset GetPreset();
    static consteval gpio::GPIOPort::GPIOLow
    GetRxPin(gpio::GPIOMapper &gpio_mapper, GPIORx rx_pin);
    static consteval gpio::GPIOPort::GPIOLow
    GetTxPin(gpio::GPIOMapper &gpio_mapper, GPIOTx tx_pin);

    static constexpr uint32_t CountCR1Mask();
    static constexpr uint32_t CountCR2Mask();
    static constexpr uint32_t CountBRRMask(const UARTPreset &preset);

    static void EnableUARTClock(uint32_t rcc_address, uint32_t en_bit);
    static constexpr uint32_t USARTBRRDIVFractionVal(uint32_t val);
    static constexpr uint32_t USARTBRRDIVMantissaVal(uint32_t val);
    static constexpr uint32_t USARTCR2Stop1bit();

    const int IRQ_priority_;
    const gpio::GPIOPort::GPIOLow rx_pin_;
    gpio::GPIOPort::GPIOLow tx_pin_;

    const uint32_t cr1_;
    const uint32_t cr2_;
    const uint32_t brr_;
};

template <>
enum class UARTLow<UARTIndex::kUSART3, Speed::k115200>::GPIORx {
    kB11
};

template <>
enum class UARTLow<UARTIndex::kUSART3, Speed::k115200>::GPIOTx {
    kB10
};

template <UARTIndex kIndex, Speed kSpeed>
class UARTLow<kIndex, kSpeed>::UARTLowHandler
{
public:
    UARTLowHandler(UARTLow<kIndex, kSpeed> &uart_low);

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
    UARTLow<kIndex, kSpeed> &uart_low_;
};

template <UARTIndex kIndex, Speed kSpeed>
consteval UARTLow<kIndex, kSpeed>::UARTLow(gpio::GPIOMapper &gpio_mapper,
                                           GPIORx rx_pin, GPIOTx tx_pin,
                                           int IRQ_priority)
    : IRQ_priority_(IRQ_priority),
      rx_pin_(GetRxPin(gpio_mapper, rx_pin)),
      tx_pin_(GetTxPin(gpio_mapper, tx_pin)),
      cr1_(CountCR1Mask()),
      cr2_(CountCR2Mask()),
      brr_(CountBRRMask(GetPreset()))
{
}

template <>
constexpr UARTLow<UARTIndex::kUSART3, Speed::k115200>::UARTPreset
UARTLow<UARTIndex::kUSART3, Speed::k115200>::GetPreset()
{
    return {.USARTx = USART3_BASE,
            .GPIO_alt_func = 7,
            .RCC_APBENR_UARTxEN = RCC_APB1ENR_USART3EN,
            .RCC_address = RCC_BASE + offsetof(RCC_TypeDef, APB1ENR),
            .USARTx_IRQn = USART3_IRQn,
            .mantissa = 17,
            .fraction = 6};
}

template <>
consteval gpio::GPIOPort::GPIOLow
UARTLow<UARTIndex::kUSART3, Speed::k115200>::GetRxPin(
    gpio::GPIOMapper &gpio_mapper,
    UARTLow<UARTIndex::kUSART3, Speed::k115200>::GPIORx rx_pin)
{
    switch (rx_pin)
    {
    case UARTLow<UARTIndex::kUSART3, Speed::k115200>::GPIORx::kB11:
        if (!gpio_mapper.IsGPIOMapped(gpio::GPIOPort::Index::kGPIOB, 11))
        {
            int a = 1 / 0;
        }
        return gpio::GPIOPort::GPIOLow(
            gpio_mapper.GetPort(gpio::GPIOPort::Index::kGPIOB), 11,
            GetPreset().GPIO_alt_func);
    default:
        int a = 1 / 0;
    }
}

template <>
consteval gpio::GPIOPort::GPIOLow
UARTLow<UARTIndex::kUSART3, Speed::k115200>::GetTxPin(
    gpio::GPIOMapper &gpio_mapper,
    UARTLow<UARTIndex::kUSART3, Speed::k115200>::GPIOTx tx_pin)
{
    switch (tx_pin)
    {
    case UARTLow<UARTIndex::kUSART3, Speed::k115200>::GPIOTx::kB10:
        if (!gpio_mapper.IsGPIOMapped(gpio::GPIOPort::Index::kGPIOB, 10))
        {
            int a = 1 / 0;
        }
        return gpio::GPIOPort::GPIOLow(
            gpio_mapper.GetPort(gpio::GPIOPort::Index::kGPIOB), 10,
            GetPreset().GPIO_alt_func);
    default:
        int a = 1 / 0;
    }
}

template <UARTIndex kIndex, Speed kSpeed>
UARTLow<kIndex, kSpeed>::UARTLowHandler::UARTLowHandler(
    UARTLow<kIndex, kSpeed> &uart_low)
    : uart_low_(uart_low)
{
    auto preset = UARTLow<kIndex, kSpeed>::GetPreset();
    EnableUARTClock_(preset.RCC_address, preset.RCC_APBENR_UARTxEN);
    NVIC_SetPriority(preset.USARTx_IRQn, uart_low_.IRQ_priority_);
    NVIC_EnableIRQ(preset.USARTx_IRQn);

    auto USARTx = reinterpret_cast<USART_TypeDef *>(preset.USARTx);

    CLEAR_BIT(USARTx->CR1, USART_CR1_UE);

    USARTx->CR1 = uart_low_.cr1_;
    USARTx->CR2 = uart_low_.cr2_;
    USARTx->BRR = uart_low_.brr_;

    SET_BIT(USARTx->CR1, USART_CR1_UE);
}

template <UARTIndex kIndex, Speed kSpeed>
bool UARTLow<kIndex, kSpeed>::UARTLowHandler::IsRxDone()
{
    return READ_BIT(reinterpret_cast<USART_TypeDef *>(
                        UARTLow<kIndex, kSpeed>::GetPreset().USARTx)
                        ->SR,
                    USART_SR_RXNE);
}

template <UARTIndex kIndex, Speed kSpeed>
bool UARTLow<kIndex, kSpeed>::UARTLowHandler::IsTxDone()
{
    return READ_BIT(reinterpret_cast<USART_TypeDef *>(
                        UARTLow<kIndex, kSpeed>::GetPreset().USARTx)
                        ->SR,
                    USART_SR_TC);
}

template <UARTIndex kIndex, Speed kSpeed>
uint8_t UARTLow<kIndex, kSpeed>::UARTLowHandler::GetRx()
{
    return reinterpret_cast<USART_TypeDef *>(
               UARTLow<kIndex, kSpeed>::GetPreset().USARTx)
        ->DR;
}

template <UARTIndex kIndex, Speed kSpeed>
void UARTLow<kIndex, kSpeed>::UARTLowHandler::SetTx(uint8_t byte)
{
    reinterpret_cast<USART_TypeDef *>(
        UARTLow<kIndex, kSpeed>::GetPreset().USARTx)
        ->DR = byte;
}

template <UARTIndex kIndex, Speed kSpeed>
void UARTLow<kIndex, kSpeed>::UARTLowHandler::EnableRxInterruption()
{
    SET_BIT(reinterpret_cast<USART_TypeDef *>(
                UARTLow<kIndex, kSpeed>::GetPreset().USARTx)
                ->CR1,
            USART_CR1_RXNEIE);
}

template <UARTIndex kIndex, Speed kSpeed>
void UARTLow<kIndex, kSpeed>::UARTLowHandler::DisableRxInterruption()
{
    CLEAR_BIT(reinterpret_cast<USART_TypeDef *>(
                  UARTLow<kIndex, kSpeed>::GetPreset().USARTx)
                  ->CR1,
              USART_CR1_RXNEIE);
}

template <UARTIndex kIndex, Speed kSpeed>
void UARTLow<kIndex, kSpeed>::UARTLowHandler::EnableDMATransmit()
{
    SET_BIT(reinterpret_cast<USART_TypeDef *>(
                UARTLow<kIndex, kSpeed>::GetPreset().USARTx)
                ->CR3,
            USART_CR3_DMAT);
}

template <UARTIndex kIndex, Speed kSpeed>
void UARTLow<kIndex, kSpeed>::UARTLowHandler::EnableDMAReceive()
{
    SET_BIT(reinterpret_cast<USART_TypeDef *>(
                UARTLow<kIndex, kSpeed>::GetPreset().USARTx)
                ->CR3,
            USART_CR3_DMAR);
}

template <UARTIndex kIndex, Speed kSpeed>
constexpr uint32_t UARTLow<kIndex, kSpeed>::CountCR1Mask()
{
    uint32_t cr1 = 0;
    CLEAR_BIT(cr1, USART_CR1_M);   // 8 bits including parity
    CLEAR_BIT(cr1, USART_CR1_PCE); // parity disable
    SET_BIT(cr1, USART_CR1_PS);    // odd parity
    SET_BIT(cr1, USART_CR1_TE);
    SET_BIT(cr1, USART_CR1_RE);
    SET_BIT(cr1, USART_CR1_RXNEIE);

    return cr1;
}

template <UARTIndex kIndex, Speed kSpeed>
constexpr uint32_t UARTLow<kIndex, kSpeed>::CountCR2Mask()
{
    uint32_t cr2 = 0;
    MODIFY_REG(cr2, USART_CR2_STOP, USARTCR2Stop1bit());
    return cr2;
}

template <UARTIndex kIndex, Speed kSpeed>
constexpr uint32_t
UARTLow<kIndex, kSpeed>::CountBRRMask(const UARTPreset &preset)
{
    uint32_t brr = 0;
    MODIFY_REG(brr, USART_BRR_DIV_Fraction,
               USARTBRRDIVFractionVal(preset.fraction));
    MODIFY_REG(brr, USART_BRR_DIV_Mantissa,
               USARTBRRDIVMantissaVal(preset.mantissa));
    return brr;
}

template <UARTIndex kIndex, Speed kSpeed>
void UARTLow<kIndex, kSpeed>::EnableUARTClock(uint32_t rcc_address,
                                              uint32_t en_bit)
{
    volatile uint32_t *rcc_reg =
        reinterpret_cast<volatile uint32_t *>(rcc_address);
    __IO uint32_t tmpreg = 0x00U;
    SET_BIT(*rcc_reg,
            en_bit); /* Delay after an RCC peripheral clock enabling */
    tmpreg = READ_BIT(*rcc_reg, en_bit);
    (void)tmpreg;
}

template <UARTIndex kIndex, Speed kSpeed>
constexpr uint32_t UARTLow<kIndex, kSpeed>::USARTBRRDIVFractionVal(uint32_t val)
{
    return val << USART_BRR_DIV_Fraction_Pos;
}

template <UARTIndex kIndex, Speed kSpeed>
constexpr uint32_t UARTLow<kIndex, kSpeed>::USARTBRRDIVMantissaVal(uint32_t val)
{
    return val << USART_BRR_DIV_Mantissa_Pos;
}

template <UARTIndex kIndex, Speed kSpeed>
constexpr uint32_t UARTLow<kIndex, kSpeed>::USARTCR2Stop1bit()
{
    return 0x0UL << USART_CR2_STOP_Pos;
}

} // namespace hydrv::uart
