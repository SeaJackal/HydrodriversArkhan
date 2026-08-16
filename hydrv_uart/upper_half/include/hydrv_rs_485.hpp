#pragma once

#include <cstring>

#include "hydrolib_func_concepts.hpp"

#include "hydrv_uart.hpp"
#include "hydrv_uart_low.hpp"

namespace hydrv::UART
{

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY,
          bool TRANSMIT_ON_HIGHT = true,
          typename CallbackType =
              decltype(&hydrolib::concepts::func::DummyFunc<void>)>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
class RS485 : public UART<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY>
{
private:
    using Parent =
        hydrv::UART::UART<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, CallbackType>;
    hydrv::GPIO::GPIOLow &direction_pin_;
    void SetTransmitMode();

public:
    consteval RS485(
        const UARTLow::UARTPreset &preset, hydrv::GPIO::GPIOLow &rx_pin,
        hydrv::GPIO::GPIOLow &tx_pin, hydrv::GPIO::GPIOLow &direction_pin,
        unsigned irq_priority,
        CallbackType rx_callback = hydrolib::concepts::func::DummyFunc<void>);

    void SetReceiveMode();

    int Transmit(const void *data, unsigned data_length);

    int Read(void *data, unsigned data_length);

    void Init();

    void IRQCallback();
};

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY,
          bool TRANSMIT_ON_HIGHT, typename CallbackType>
int read(RS485<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, TRANSMIT_ON_HIGHT,
               CallbackType> &stream,
         void *dest, unsigned length);

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY,
          bool TRANSMIT_ON_HIGHT, typename CallbackType>
int write(RS485<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, TRANSMIT_ON_HIGHT,
                CallbackType> &stream,
          const void *dest, unsigned length);

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY,
          bool TRANSMIT_ON_HIGHT, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
consteval RS485<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, TRANSMIT_ON_HIGHT,
                CallbackType>::RS485(const UARTLow::UARTPreset
                                         &UART_preset,
                                     hydrv::GPIO::GPIOLow &rx_pin,
                                     hydrv::GPIO::GPIOLow &tx_pin,
                                     hydrv::GPIO::GPIOLow &direction_pin,
                                     unsigned IRQ_priority,
                                     CallbackType rx_callback)
    : Parent(UART_preset, rx_pin, tx_pin, IRQ_priority, rx_callback),
      direction_pin_(direction_pin)
{
}

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY,
          bool TRANSMIT_ON_HIGHT, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
void RS485<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, TRANSMIT_ON_HIGHT,
           CallbackType>::IRQCallback()
{
    Parent::IRQCallback();
    if (!Parent::IsTransmiting_())
    {
        SetReceiveMode();
    }
}

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY,
          bool TRANSMIT_ON_HIGHT, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
void RS485<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, TRANSMIT_ON_HIGHT,
           CallbackType>::Init()
{
    Parent::Init();
    direction_pin_.Init();
    SetReceiveMode();
}

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY,
          bool TRANSMIT_ON_HIGHT, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
int RS485<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, TRANSMIT_ON_HIGHT,
          CallbackType>::Transmit(const void *data, unsigned data_length)
{
    SetTransmitMode();
    int result = Parent::Transmit(data, data_length);
    return result;
}

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY,
          bool TRANSMIT_ON_HIGHT, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
int RS485<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, TRANSMIT_ON_HIGHT,
          CallbackType>::Read(void *data, unsigned data_length)
{
    int result = Parent::Read(data, data_length);
    return result;
}

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY,
          bool TRANSMIT_ON_HIGHT, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
void RS485<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, TRANSMIT_ON_HIGHT,
           CallbackType>::SetTransmitMode()
{
    if constexpr (TRANSMIT_ON_HIGHT)
    {
        direction_pin_.Set();
    }
    else
    {
        direction_pin_.Reset();
    }
}

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY,
          bool TRANSMIT_ON_HIGHT, typename CallbackType>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
void RS485<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, TRANSMIT_ON_HIGHT,
           CallbackType>::SetReceiveMode()
{
    if constexpr (TRANSMIT_ON_HIGHT)
    {
        direction_pin_.Reset();
    }
    else
    {
        direction_pin_.Set();
    }
}

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY,
          bool TRANSMIT_ON_HIGHT, typename CallbackType>
int read(RS485<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, TRANSMIT_ON_HIGHT,
               CallbackType> &stream,
         void *dest, unsigned length)
{
    return stream.Read(dest, length);
}

template <int RX_BUFFER_CAPACITY, int TX_BUFFER_CAPACITY,
          bool TRANSMIT_ON_HIGHT, typename CallbackType>
int write(RS485<RX_BUFFER_CAPACITY, TX_BUFFER_CAPACITY, TRANSMIT_ON_HIGHT,
                CallbackType> &stream,
          const void *dest, unsigned length)
{
    return stream.Transmit(dest, length);
}

} // namespace hydrv::RS485