#pragma once

#include <cstdint>
#include <cstring>
#include <optional>
#include <span>

#include "hydrolib_func_concepts.hpp"
#include "hydrolib_return_codes.hpp"
#include "hydrolib_ring_queue.hpp"

#include "hydrv_uart_low.hpp"

namespace hydrv::uart
{
template <UARTIndex kIndex, int kRxBufferCapacity, int kTxBufferCapacity,
          typename CallbackType =
              decltype(&hydrolib::concepts::func::DummyFunc<void>)>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
class UARTBase
{
public:
    class UART;

    template <typename T>
    UARTBase(const T &env, CallbackType rx_callback =
                               hydrolib::concepts::func::DummyFunc<void>);

private:
    UARTLowBase<kIndex>::UARTLow uart_low_;

    hydrolib::ring_queue::RingQueue<kRxBufferCapacity> rx_queue_;
    hydrolib::ring_queue::RingQueue<kTxBufferCapacity> tx_queue_;

    bool tx_in_progress_flag_;

    hydrolib::ReturnCode status_;

    CallbackType rx_callback_;
};

template <UARTIndex kIndex, int kRxBufferCapacity, int kTxBufferCapacity,
          typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
class UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity, CallbackType>::UART
{
public:
    UART(UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity, CallbackType>
             &uart);

    void IRQCallback();

    int Transmit(std::span<const std::byte> data);

    int Read(std::span<std::byte> data);
    void ClearRx();

    int GetRxLength() const;
    int GetTxLength() const;

protected:
    bool IsTransmiting() const;

    std::optional<uint8_t> ProcessRx();
    std::optional<uint8_t> ProcessTx();

private:
    UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity, CallbackType>
        &uart_base_;
    UARTLowBase<kIndex>::UARTLow uart_low_handler_;
};

template <UARTIndex kIndex, int kRxBufferCapacity, int kTxBufferCapacity,
          typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
template <typename T>
UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity, CallbackType>::UARTBase(
    const T &env, CallbackType rx_callback)
    : uart_low_(env),
      tx_in_progress_flag_(false),
      status_(hydrolib::ReturnCode::OK),
      rx_callback_(rx_callback)
{
}

template <UARTIndex kIndex, int kRxBufferCapacity, int kTxBufferCapacity,
          typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity, CallbackType>::UART::
    UART(UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity, CallbackType>
             &uart)
    : uart_base_(uart), uart_low_handler_(uart.uart_low_)
{
}

template <UARTIndex kIndex, int kRxBufferCapacity, int kTxBufferCapacity,
          typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
bool UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity,
              CallbackType>::UART::IsTransmiting() const
{
    return uart_base_.tx_in_progress_flag_;
}

template <UARTIndex kIndex, int kRxBufferCapacity, int kTxBufferCapacity,
          typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
void UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity,
              CallbackType>::UART::IRQCallback()
{
    ProcessRx();
    ProcessTx();
}

template <UARTIndex kIndex, int kRxBufferCapacity, int kTxBufferCapacity,
          typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
int UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity,
             CallbackType>::UART::Transmit(std::span<const std::byte> data)
{
    uart_base_.tx_in_progress_flag_ = true;
    int length = GetTxLength();
    auto data_length = data.size();
    if (length + data_length > kTxBufferCapacity)
    {
        data_length = kTxBufferCapacity - length;
    }
    uart_base_.tx_queue_.Push(data.data(), data_length);
    uart_low_handler_.EnableTxInterruption();
    return data_length;
}

template <UARTIndex kIndex, int kRxBufferCapacity, int kTxBufferCapacity,
          typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
int UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity,
             CallbackType>::UART::Read(std::span<std::byte> data)
{
    int length = GetRxLength();
    auto data_length = data.size();
    if (data_length > length)
    {
        data_length = length;
    }

    uart_base_.rx_queue_.Pull(data.data(), data_length);
    return data_length;
}

template <UARTIndex kIndex, int kRxBufferCapacity, int kTxBufferCapacity,
          typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
void UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity,
              CallbackType>::UART::ClearRx()
{
    uart_base_.rx_queue_.Clear();
}

template <UARTIndex kIndex, int kRxBufferCapacity, int kTxBufferCapacity,
          typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
int UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity,
             CallbackType>::UART::GetRxLength() const
{
    return uart_base_.rx_queue_.GetLength();
}

template <UARTIndex kIndex, int kRxBufferCapacity, int kTxBufferCapacity,
          typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
int UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity,
             CallbackType>::UART::GetTxLength() const
{
    return uart_base_.tx_queue_.GetLength();
}

template <UARTIndex kIndex, int kRxBufferCapacity, int kTxBufferCapacity,
          typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
std::optional<uint8_t> UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity,
                                CallbackType>::UART::ProcessRx()
{
    if (!uart_low_handler_.IsRxDone())
    {
        return std::nullopt;
    }

    if (uart_base_.rx_queue_.IsFull())
    {
        uart_low_handler_.GetRx();
        uart_base_.status_ = hydrolib::ReturnCode::FAIL;
        uart_base_.rx_callback_();
        return std::nullopt;
    }

    uint8_t rx = uart_low_handler_.GetRx();
    uart_base_.rx_queue_.PushByte(rx);

    uart_base_.rx_callback_();
    return rx;
}

template <UARTIndex kIndex, int kRxBufferCapacity, int kTxBufferCapacity,
          typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
std::optional<uint8_t> UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity,
                                CallbackType>::UART::ProcessTx()
{
    if (!uart_low_handler_.IsTxDone())
    {
        return std::nullopt;
    }

    if (uart_base_.tx_queue_.IsEmpty())
    {
        uart_base_.tx_in_progress_flag_ = false;
        uart_low_handler_.DisableTxInterruption();
        return std::nullopt;
    }

    uint8_t tx = 0;
    uart_base_.tx_queue_.PullByte(&tx);
    uart_low_handler_.SetTx(tx);
    return tx;
}

} // namespace hydrv::uart