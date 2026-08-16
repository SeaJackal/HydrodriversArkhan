#pragma once

#include <cstdint>
#include <cstring>
#include <optional>

#include "hydrolib_func_concepts.hpp"
#include "hydrolib_return_codes.hpp"
#include "hydrolib_ring_queue.hpp"

#include "hydrv_uart_low.hpp"

namespace hydrv::UART
{
template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY,
          typename CallbackType =
              decltype(&hydrolib::concepts::func::DummyFunc<void>)>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
class UART
{
public:
    consteval UART(
        const UARTLow::UARTPreset &UART_preset, hydrv::GPIO::GPIOLow &rx_pin,
        hydrv::GPIO::GPIOLow &tx_pin, unsigned IRQ_priority,
        CallbackType rx_callback = hydrolib::concepts::func::DummyFunc<void>);

    void Init();

    void IRQCallback();

    int Transmit(const void *data, unsigned data_length);

    int Read(void *data, unsigned data_length);
    void ClearRx();

    unsigned GetRxLength() const;
    unsigned GetTxLength() const;

protected:
    bool IsTransmiting_() const;

    std::optional<uint8_t> ProcessRx_();
    std::optional<uint8_t> ProcessTx_();

private:
    UARTLow UART_handler_;

    hydrolib::ring_queue::RingQueue<RX_BUFFER_CAPACITY> rx_queue_;
    hydrolib::ring_queue::RingQueue<TX_BUFFER_CAPACITY> tx_queue_;

    bool tx_in_progress_flag_;

    hydrolib::ReturnCode status_;

    CallbackType rx_callback_;
};

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY, typename CallbackType>
int read(UART<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, CallbackType> &stream,
         void *dest, unsigned length);

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY, typename CallbackType>
int write(UART<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, CallbackType> &stream,
          const void *dest, unsigned length);

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
consteval UART<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, CallbackType>::UART(
    const UARTLow::UARTPreset &UART_preset, hydrv::GPIO::GPIOLow &rx_pin,
    hydrv::GPIO::GPIOLow &tx_pin, unsigned IRQ_priority,
    CallbackType rx_callback)
    : UART_handler_(UART_preset, rx_pin, tx_pin, IRQ_priority),
      tx_in_progress_flag_(false),
      status_(hydrolib::ReturnCode::OK),
      rx_callback_(rx_callback)
{
}

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
void UART<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, CallbackType>::Init()
{
    UART_handler_.Init();
}

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
bool UART<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY,
          CallbackType>::IsTransmiting_() const
{
    return tx_in_progress_flag_;
}

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
void UART<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, CallbackType>::IRQCallback()
{
    ProcessRx_();
    ProcessTx_();
}

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
int UART<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, CallbackType>::Transmit(
    const void *data, unsigned data_length)
{
    tx_in_progress_flag_ = true;
    unsigned length = GetTxLength();
    if (length + data_length > TX_BUFFER_CAPACITY)
    {
        data_length = TX_BUFFER_CAPACITY - length;
    }
    tx_queue_.Push(data, data_length);
    UART_handler_.EnableTxInterruption();
    return data_length;
}

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
int UART<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, CallbackType>::Read(
    void *data, unsigned data_length)
{
    unsigned length = GetRxLength();
    if (data_length > length)
    {
        data_length = length;
    }

    rx_queue_.Pull(data, data_length);
    return data_length;
}

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
void UART<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, CallbackType>::ClearRx()
{
    rx_queue_.Clear();
}

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
unsigned
UART<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, CallbackType>::GetRxLength() const
{
    return rx_queue_.GetLength();
}

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
unsigned
UART<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, CallbackType>::GetTxLength() const
{
    return tx_queue_.GetLength();
}

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
std::optional<uint8_t>
UART<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, CallbackType>::ProcessRx_()
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

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
std::optional<uint8_t>
UART<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, CallbackType>::ProcessTx_()
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

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY, typename CallbackType>
int read(UART<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, CallbackType> &stream,
         void *dest, unsigned length)
{
    return stream.Read(dest, length);
}

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY, typename CallbackType>
int write(UART<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, CallbackType> &stream,
          const void *dest, unsigned length)
{
    return stream.Transmit(dest, length);
}

} // namespace hydrv::UART