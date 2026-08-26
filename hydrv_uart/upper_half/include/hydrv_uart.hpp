#pragma once

#include <cstdint>
#include <cstring>
#include <optional>

#include "hydrolib_func_concepts.hpp"
#include "hydrolib_return_codes.hpp"
#include "hydrolib_ring_queue.hpp"

#include "hydrv_uart_low.hpp"

namespace hydrv::uart
{
template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity,
          typename CallbackType =
              decltype(&hydrolib::concepts::func::DummyFunc<void>)>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
class UART
{
public:
    class UARTHandler;

    consteval UART(
        const gpio::GPIOMapper &gpio_mapper, UARTLow<kIndex, kSpeed>::GPIORx rx_pin,
        UARTLow<kIndex, kSpeed>::GPIOTx tx_pin, int IRQ_priority,
        CallbackType rx_callback = hydrolib::concepts::func::DummyFunc<void>);

protected:
    bool IsTransmiting() const;

    std::optional<uint8_t> ProcessRx();
    std::optional<uint8_t> ProcessTx();

private:
    UARTLow<kIndex, kSpeed> UART_handler_;

    hydrolib::ring_queue::RingQueue<kRxBufferCapacity> rx_queue_;
    hydrolib::ring_queue::RingQueue<kTxBufferCapacity> tx_queue_;

    bool tx_in_progress_flag_;

    hydrolib::ReturnCode status_;

    CallbackType rx_callback_;
};

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
class UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity,
           CallbackType>::UARTHandler
{
public:
    UARTHandler(UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity,
                     CallbackType> &uart);

    void IRQCallback();

    int Transmit(std::span<const std::byte> data);

    int Read(std::span<std::byte> data);
    void ClearRx();

    int GetRxLength() const;
    int GetTxLength() const;

private:
    UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity, CallbackType>
        &uart_;
};

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
consteval UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity,
               CallbackType>::UART(gpio::GPIOMapper &gpio_mapper,
                                   UARTLow<kIndex, kSpeed>::GPIORx rx_pin,
                                   UARTLow<kIndex, kSpeed>::GPIOTx tx_pin,
                                   int IRQ_priority, CallbackType rx_callback)
    : UART_handler_(gpio_mapper, rx_pin, tx_pin, IRQ_priority),
      tx_in_progress_flag_(false),
      status_(hydrolib::ReturnCode::OK),
      rx_callback_(rx_callback)
{
}

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
bool UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity,
          CallbackType>::IsTransmiting() const
{
    return tx_in_progress_flag_;
}

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
void UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity,
          CallbackType>::UARTHandler::IRQCallback()
{
    uart_.ProcessRx();
    uart_.ProcessTx();
}

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
int UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity,
         CallbackType>::UARTHandler::Transmit(std::span<const std::byte> data)
{
    uart_.tx_in_progress_flag_ = true;
    unsigned length = GetTxLength();
    auto data_length = data.size();
    if (length + data_length > kTxBufferCapacity)
    {
        data_length = kTxBufferCapacity - length;
    }
    uart_.tx_queue_.Push(data.data(), data_length);
    uart_.UART_handler_.EnableTxInterruption();
    return data_length;
}

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
int UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity,
         CallbackType>::UARTHandler::Read(std::span<std::byte> data)
{
    unsigned length = GetRxLength();
    auto data_length = data.size();
    if (data_length > length)
    {
        data_length = length;
    }

    uart_.rx_queue_.Pull(data.data(), data_length);
    return data_length;
}

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
void UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity,
          CallbackType>::UARTHandler::ClearRx()
{
    uart_.rx_queue_.Clear();
}

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
int UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity,
         CallbackType>::UARTHandler::GetRxLength() const
{
    return uart_.rx_queue_.GetLength();
}

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
int UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity,
         CallbackType>::UARTHandler::GetTxLength() const
{
    return uart_.tx_queue_.GetLength();
}

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
std::optional<uint8_t> UART<kIndex, kSpeed, kRxBufferCapacity,
                            kTxBufferCapacity, CallbackType>::ProcessRx()
{
    if (!UART_handler_.IsRxDone())
    {
        return std::nullopt;
    }

    if (rx_queue_.IsFull())
    {
        UART_handler_.GetRx();
        status_ = hydrolib::ReturnCode::FAIL;
        rx_callback_();
        return std::nullopt;
    }

    uint8_t rx = UART_handler_.GetRx();
    rx_queue_.PushByte(rx);

    rx_callback_();
    return rx;
}

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
std::optional<uint8_t> UART<kIndex, kSpeed, kRxBufferCapacity,
                            kTxBufferCapacity, CallbackType>::ProcessTx()
{
    if (!UART_handler_.IsTxDone())
    {
        return std::nullopt;
    }

    if (tx_queue_.IsEmpty())
    {
        tx_in_progress_flag_ = false;
        UART_handler_.DisableTxInterruption();
        return std::nullopt;
    }

    uint8_t tx = 0;
    tx_queue_.PullByte(&tx);
    UART_handler_.SetTx(tx);
    return tx;
}

} // namespace hydrv::uart