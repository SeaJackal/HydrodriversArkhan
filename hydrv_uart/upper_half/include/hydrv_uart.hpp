#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <optional>
#include <span>
#include <tuple>

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

    struct Config
    {
        using Handler = UARTBase;

        static constexpr int kGPIOCount = 2;

        // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
        // Struct with no invariants
        UARTLowBase<kIndex>::Speed speed;
        UARTLowBase<kIndex>::GPIORx rx_pin;
        UARTLowBase<kIndex>::GPIOTx tx_pin;
        int irq_priority = 7; // NOLINT(cppcoreguidelines-avoid-magic-numbers,
        //  readability-magic-numbers)

        CallbackType rx_callback = hydrolib::concepts::func::DummyFunc<void>;
        // NOLINTEND(misc-non-private-member-variables-in-classes)

        [[nodiscard]] consteval std::tuple<gpio::GPIOPort::RawConfig,
                                           gpio::GPIOPort::RawConfig>
        GetGPIOConfigs() const;
    };

    consteval explicit UARTBase(const Config &config);

private:
    UARTLowBase<kIndex> uart_low_;

    hydrolib::ring_queue::RingQueue<kRxBufferCapacity> rx_queue_;
    hydrolib::ring_queue::RingQueue<kTxBufferCapacity> tx_queue_;

    bool tx_in_progress_flag_ = false;

    hydrolib::ReturnCode status_ = hydrolib::ReturnCode::kOk;

    CallbackType rx_callback_;
};

template <UARTIndex kIndex, int kRxBufferCapacity, int kTxBufferCapacity,
          typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
consteval std::tuple<gpio::GPIOPort::RawConfig, gpio::GPIOPort::RawConfig>
UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity,
         CallbackType>::Config::GetGPIOConfigs() const
{
    return std::make_tuple(UARTLowBase<kIndex>::GetRxGPIOConfig(rx_pin),
                           UARTLowBase<kIndex>::GetTxGPIOConfig(tx_pin));
}

template <UARTIndex kIndex, int kRxBufferCapacity, int kTxBufferCapacity,
          typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
class UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity, CallbackType>::UART
{
public:
    template <typename T>
    explicit UART(T &env);

    UART(const UART &) = delete;
    UART &operator=(const UART &) = delete;
    UART &operator=(UART &&) = delete;

    UART(UART &&) = default;

    ~UART() = default;

    void IRQCallback();

    int Transmit(std::span<const std::byte> data);

    int Read(std::span<std::byte> data);
    void ClearRx();

    [[nodiscard]] int GetRxLength() const;
    [[nodiscard]] int GetTxLength() const;

    int write(const void *source, unsigned length);
    int read(void *dest, unsigned length);

protected:
    [[nodiscard]] bool IsTransmiting() const;

    std::optional<uint8_t> ProcessRx();
    std::optional<uint8_t> ProcessTx();

private:
    // TODO: SeaJackal - need to be made not movable after creating tuple to
    // store not movable objects
    // NOLINTBEGIN(cppcoreguidelines-avoid-const-or-ref-data-members)
    // Class should be not movable
    UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity, CallbackType>
        &uart_base_;
    UARTLowBase<kIndex>::UARTLow uart_low_handler_;
    // NOLINTEND(cppcoreguidelines-avoid-const-or-ref-data-members)
};

template <UARTIndex kIndex, int kRxBufferCapacity, int kTxBufferCapacity,
          typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
consteval UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity,
                   CallbackType>::UARTBase(const Config &config)
    : uart_low_(config.speed, config.irq_priority),
      rx_callback_(config.rx_callback)
{
}

template <UARTIndex kIndex, int kRxBufferCapacity, int kTxBufferCapacity,
          typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
template <typename T>
UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity,
         CallbackType>::UART::UART(T &env)
    : uart_base_(
          env.template GetPeriph<UARTBase<kIndex, kRxBufferCapacity,
                                          kTxBufferCapacity, CallbackType>>()),
      uart_low_handler_(uart_base_.uart_low_)
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
    int data_length = static_cast<int>(data.size());
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
    int data_length = std::min(static_cast<int>(data.size()), length);

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
        uart_base_.status_ = hydrolib::ReturnCode::kFail;
        uart_base_.rx_callback_();
        return std::nullopt;
    }

    uint8_t rx_data = uart_low_handler_.GetRx();
    uart_base_.rx_queue_.PushByte(rx_data);

    uart_base_.rx_callback_();
    return rx_data;
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

    uint8_t tx_data = 0;
    uart_base_.tx_queue_.PullByte(&tx_data);
    uart_low_handler_.SetTx(tx_data);
    return tx_data;
}

template <UARTIndex kIndex, int kRxBufferCapacity, int kTxBufferCapacity,
          typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
int UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity,
             CallbackType>::UART::write(const void *source, unsigned length)
{
    return Transmit(std::span<const std::byte>(
        static_cast<const std::byte *>(source), length));
}

template <UARTIndex kIndex, int kRxBufferCapacity, int kTxBufferCapacity,
          typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
int UARTBase<kIndex, kRxBufferCapacity, kTxBufferCapacity,
             CallbackType>::UART::read(void *dest, unsigned length)
{
    return Read(std::span<std::byte>(static_cast<std::byte *>(dest), length));
}
} // namespace hydrv::uart