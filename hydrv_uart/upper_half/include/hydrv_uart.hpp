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
        const gpio::GPIOMapper &gpio_mapper,
        UARTLow<kIndex, kSpeed>::GPIORx rx_pin,
        UARTLow<kIndex, kSpeed>::GPIOTx tx_pin, int IRQ_priority,
        CallbackType rx_callback = hydrolib::concepts::func::DummyFunc<void>);

private:
    UARTLow<kIndex, kSpeed> uart_low_;

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
    UARTHandler(
        UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity, CallbackType>
            &uart,
        [[maybe_unused]] const gpio::GPIOMapper::GPIOPortsHandler &gpio_mapper);

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
    UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity, CallbackType>
        &uart_base_;
    UARTLow<kIndex, kSpeed>::UARTLowHandler uart_low_handler_;
};

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
consteval UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity,
               CallbackType>::UART(const gpio::GPIOMapper &gpio_mapper,
                                   UARTLow<kIndex, kSpeed>::GPIORx rx_pin,
                                   UARTLow<kIndex, kSpeed>::GPIOTx tx_pin,
                                   int IRQ_priority, CallbackType rx_callback)
    : uart_low_(gpio_mapper, rx_pin, tx_pin, IRQ_priority),
      tx_in_progress_flag_(false),
      status_(hydrolib::ReturnCode::OK),
      rx_callback_(rx_callback)
{
}

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity, CallbackType>::
    UARTHandler::UARTHandler(
        UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity, CallbackType>
            &uart,
        [[maybe_unused]] const gpio::GPIOMapper::GPIOPortsHandler &gpio_mapper)
    : uart_base_(uart), uart_low_handler_(uart.uart_low_)
{
}

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
bool UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity,
          CallbackType>::UARTHandler::IsTransmiting() const
{
    return uart_base_.tx_in_progress_flag_;
}

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
void UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity,
          CallbackType>::UARTHandler::IRQCallback()
{
    ProcessRx();
    ProcessTx();
}

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
int UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity,
         CallbackType>::UARTHandler::Transmit(std::span<const std::byte> data)
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

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
int UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity,
         CallbackType>::UARTHandler::Read(std::span<std::byte> data)
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

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
void UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity,
          CallbackType>::UARTHandler::ClearRx()
{
    uart_base_.rx_queue_.Clear();
}

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
int UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity,
         CallbackType>::UARTHandler::GetRxLength() const
{
    return uart_base_.rx_queue_.GetLength();
}

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
int UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity,
         CallbackType>::UARTHandler::GetTxLength() const
{
    return uart_base_.tx_queue_.GetLength();
}

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
std::optional<uint8_t>
UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity,
     CallbackType>::UARTHandler::ProcessRx()
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

template <UARTIndex kIndex, Speed kSpeed, int kRxBufferCapacity,
          int kTxBufferCapacity, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
std::optional<uint8_t>
UART<kIndex, kSpeed, kRxBufferCapacity, kTxBufferCapacity,
     CallbackType>::UARTHandler::ProcessTx()
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