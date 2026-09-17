#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stm32f4xx.h>

extern "C"
{
#include "stm32f4xx.h"
}

#include "hydrv_i2c_low_base.hpp"

namespace hydrv::i2c
{
template <I2CIndex kI2CIndex>
class I2CLowBase<kI2CIndex>::I2CLow
{
public:
    explicit I2CLow(I2CLowBase &i2c_low_base);

    bool IsStartBit();
    bool IsAddr();
    bool IsTxEmpty();
    bool IsRxNotEmpty();
    bool IsByteTransferFinished();
    bool IsAckFailure();

    void ClearStatusBits();
    void ClearAddr();
    void ClearAckFailure();

    std::byte GetRx();
    void SetTx(std::byte data);

    void SendAddress(std::byte address);

    void GenerateStart();
    void GenerateStop();

    void EnableEventInterrupt();
    void DisableEventInterrupt();
    void EnableErrorInterrupt();
    void DisableErrorInterrupt();

    void EnableAck();
    void DisableAck();

private:
    I2CLowBase &i2c_low_base_;
};

template <I2CIndex kI2CIndex>
I2CLowBase<kI2CIndex>::I2CLow::I2CLow(I2CLowBase &i2c_low_base)
    : i2c_low_base_(i2c_low_base)
{
    auto preset = I2CLowBase<kI2CIndex>::GetPreset();
    EnableI2CClock_(preset.RCC_address, preset.RCC_APBENR_I2CxEN);
    NVIC_SetPriority(preset.I2Cx_EV_IRQn, i2c_low_base_.irq_priority_);
    NVIC_SetPriority(preset.I2Cx_ER_IRQn, i2c_low_base_.irq_priority_);
    NVIC_EnableIRQ(preset.I2Cx_EV_IRQn);
    NVIC_EnableIRQ(preset.I2Cx_ER_IRQn);

    CLEAR_BIT(reinterpret_cast<I2C_TypeDef *>(preset.I2Cx)->CR1, I2C_CR1_PE);
    reinterpret_cast<I2C_TypeDef *>(preset.I2Cx)->CR2 = i2c_low_base_.cr2_;
    reinterpret_cast<I2C_TypeDef *>(preset.I2Cx)->CCR = i2c_low_base_.ccr_;
    reinterpret_cast<I2C_TypeDef *>(preset.I2Cx)->TRISE = i2c_low_base_.trise_;
    reinterpret_cast<I2C_TypeDef *>(preset.I2Cx)->OAR1 = 0x4000;
    reinterpret_cast<I2C_TypeDef *>(preset.I2Cx)->CR1 = i2c_low_base_.cr1_;
    SET_BIT(reinterpret_cast<I2C_TypeDef *>(preset.I2Cx)->CR1, I2C_CR1_PE);
}

template <I2CIndex kI2CIndex>
bool I2CLowBase<kI2CIndex>::I2CLow::IsStartBit()
{
    return READ_BIT(
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->SR1,
        I2C_SR1_SB);
}

template <I2CIndex kI2CIndex>
bool I2CLowBase<kI2CIndex>::I2CLow::IsAddr()
{
    return READ_BIT(
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->SR1,
        I2C_SR1_ADDR);
}

template <I2CIndex kI2CIndex>
bool I2CLowBase<kI2CIndex>::I2CLow::IsTxEmpty()
{
    return READ_BIT(
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->SR1,
        I2C_SR1_TXE);
}

template <I2CIndex kI2CIndex>
bool I2CLowBase<kI2CIndex>::I2CLow::IsRxNotEmpty()
{
    return READ_BIT(
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->SR1,
        I2C_SR1_RXNE);
}

template <I2CIndex kI2CIndex>
bool I2CLowBase<kI2CIndex>::I2CLow::IsByteTransferFinished()
{
    return READ_BIT(
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->SR1,
        I2C_SR1_BTF);
}

template <I2CIndex kI2CIndex>
bool I2CLowBase<kI2CIndex>::I2CLow::IsAckFailure()
{
    return READ_BIT(
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->SR1,
        I2C_SR1_AF);
}

template <I2CIndex kI2CIndex>
void I2CLowBase<kI2CIndex>::I2CLow::ClearStatusBits()
{
    volatile uint32_t tmpreg = 0x00U;
    tmpreg =
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->SR1;
    tmpreg =
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->SR2;
    (void)tmpreg;

    reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
        ->SR1 = 0;
}

template <I2CIndex kI2CIndex>
void I2CLowBase<kI2CIndex>::I2CLow::ClearAddr()
{
    volatile uint32_t tmpreg = 0x00U;
    tmpreg = READ_BIT(
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->SR1,
        I2C_SR1_ADDR);
    tmpreg = READ_BIT(
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->SR2,
        I2C_SR2_MSL);
    (void)tmpreg;
}

template <I2CIndex kI2CIndex>
void I2CLowBase<kI2CIndex>::I2CLow::ClearAckFailure()
{
    CLEAR_BIT(
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->SR1,
        I2C_SR1_AF);
}

template <I2CIndex kI2CIndex>
std::byte I2CLowBase<kI2CIndex>::I2CLow::GetRx()
{
    return static_cast<std::byte>(
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->DR);
}

template <I2CIndex kI2CIndex>
void I2CLowBase<kI2CIndex>::I2CLow::SetTx(std::byte data)
{
    reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
        ->DR = static_cast<uint8_t>(data);
}

template <I2CIndex kI2CIndex>
void I2CLowBase<kI2CIndex>::I2CLow::SendAddress(std::byte address)
{
    reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
        ->DR = static_cast<uint8_t>(address);
}

template <I2CIndex kI2CIndex>
void I2CLowBase<kI2CIndex>::I2CLow::GenerateStart()
{
    SET_BIT(
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->CR1,
        I2C_CR1_START);
}

template <I2CIndex kI2CIndex>
void I2CLowBase<kI2CIndex>::I2CLow::GenerateStop()
{
    SET_BIT(
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->CR1,
        I2C_CR1_STOP);
}

template <I2CIndex kI2CIndex>
void I2CLowBase<kI2CIndex>::I2CLow::EnableEventInterrupt()
{
    SET_BIT(
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->CR2,
        I2C_CR2_ITEVTEN);
    SET_BIT(
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->CR2,
        I2C_CR2_ITBUFEN);
}

template <I2CIndex kI2CIndex>
void I2CLowBase<kI2CIndex>::I2CLow::DisableEventInterrupt()
{
    CLEAR_BIT(
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->CR2,
        I2C_CR2_ITEVTEN);
    CLEAR_BIT(
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->CR2,
        I2C_CR2_ITBUFEN);
}

template <I2CIndex kI2CIndex>
void I2CLowBase<kI2CIndex>::I2CLow::EnableErrorInterrupt()
{
    SET_BIT(
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->CR2,
        I2C_CR2_ITERREN);
}

template <I2CIndex kI2CIndex>
void I2CLowBase<kI2CIndex>::I2CLow::DisableErrorInterrupt()
{
    CLEAR_BIT(
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->CR2,
        I2C_CR2_ITERREN);
}

template <I2CIndex kI2CIndex>
void I2CLowBase<kI2CIndex>::I2CLow::EnableAck()
{
    SET_BIT(
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->CR1,
        I2C_CR1_ACK);
}

template <I2CIndex kI2CIndex>
void I2CLowBase<kI2CIndex>::I2CLow::DisableAck()
{
    CLEAR_BIT(
        reinterpret_cast<I2C_TypeDef *>(I2CLowBase<kI2CIndex>::GetPreset().I2Cx)
            ->CR1,
        I2C_CR1_ACK);
}

} // namespace hydrv::i2c
