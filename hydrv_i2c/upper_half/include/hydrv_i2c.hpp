#pragma once

#include "hydrolib_func_concepts.hpp"
#include "hydrv_gpio_low.hpp"
#include "hydrv_i2c_low.hpp"
#include <cstdint>
#include <cstring>

namespace hydrv::I2C
{

template <typename CallbackType =
              decltype(&hydrolib::concepts::func::DummyFunc<void>),
          int MAX_TX_LENGTH = 255>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
class I2C
{
public:
    consteval I2C(const I2CLow::I2CPreset &preset,
                  hydrv::GPIO::GPIOLow &scl_pin, hydrv::GPIO::GPIOLow &sda_pin,
                  unsigned IRQ_priority,
                  CallbackType transaction_complete_callback =
                      hydrolib::concepts::func::DummyFunc<void>);

    void Init();
    void Write(uint8_t address, const void *tx_buffer, int tx_length);
    void Read(uint8_t address, void *rx_buffer, int rx_length);

    void IRQCallback();

private:
    enum class State
    {
        IDLE,
        START_CONDITION,
        ADDRESS_WRITE,
        WRITING,
        READING
    };

private:
    I2CLow i2c_low_;
    uint8_t address_;
    int tx_length_;
    int rx_length_;
    int current_counter_;
    uint8_t tx_buffer_[MAX_TX_LENGTH] = {};
    uint8_t *rx_buffer_;
    State state_;
    CallbackType transaction_complete_callback_;

private:
    void Finish_();
};

template <typename CallbackType, int MAX_TX_LENGTH>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
consteval I2C<CallbackType, MAX_TX_LENGTH>::I2C(
    const I2CLow::I2CPreset &preset, hydrv::GPIO::GPIOLow &scl_pin,
    hydrv::GPIO::GPIOLow &sda_pin, unsigned IRQ_priority,
    CallbackType transaction_complete_callback)
    : i2c_low_(preset, scl_pin, sda_pin, IRQ_priority),
      address_(0),
      tx_length_(0),
      rx_length_(0),
      current_counter_(0),
      rx_buffer_(nullptr),
      state_(State::IDLE),
      transaction_complete_callback_(transaction_complete_callback)
{
}

template <typename CallbackType, int MAX_TX_LENGTH>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
inline void I2C<CallbackType, MAX_TX_LENGTH>::Init()
{
    i2c_low_.Init();
}

template <typename CallbackType, int MAX_TX_LENGTH>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
inline void I2C<CallbackType, MAX_TX_LENGTH>::Write(uint8_t address,
                                                    const void *tx_buffer,
                                                    int tx_length)
{
    i2c_low_.ClearStatusBits();
    address_ = address;
    tx_length_ = tx_length;
    memcpy(tx_buffer_, tx_buffer, tx_length);
    state_ = State::START_CONDITION;
    i2c_low_.EnableEventInterrupt();
    i2c_low_.EnableErrorInterrupt();
    i2c_low_.GenerateStart();
}

template <typename CallbackType, int MAX_TX_LENGTH>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
inline void I2C<CallbackType, MAX_TX_LENGTH>::Read(uint8_t address,
                                                   void *rx_buffer,
                                                   int rx_length)
{
    i2c_low_.ClearStatusBits();
    address_ = address | 0x1;
    rx_length_ = rx_length;
    rx_buffer_ = static_cast<uint8_t *>(rx_buffer);
    state_ = State::START_CONDITION;
    i2c_low_.EnableAck();
    i2c_low_.EnableEventInterrupt();
    i2c_low_.EnableErrorInterrupt();
    i2c_low_.GenerateStart();
}

template <typename CallbackType, int MAX_TX_LENGTH>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
inline void I2C<CallbackType, MAX_TX_LENGTH>::IRQCallback()
{
    if (i2c_low_.IsAckFailure())
    {
        i2c_low_.ClearAckFailure();
        Finish_();
        return;
    }

    switch (state_)
    {
    case State::IDLE:
        return;
    case State::START_CONDITION:
        if (i2c_low_.IsStartBit())
        {
            i2c_low_.SendAddress(address_);
            state_ = State::ADDRESS_WRITE;
        }
        return;
    case State::ADDRESS_WRITE:
        if (i2c_low_.IsAddr())
        {
            i2c_low_.ClearAddr();
            if (tx_length_ == 0)
            {
                state_ = State::READING;
                if (rx_length_ == 1)
                {
                    i2c_low_.DisableAck();
                }
            }
            else
            {
                state_ = State::WRITING;
            }
        }
        return;
    case State::WRITING:
        if (i2c_low_.IsTxEmpty())
        {
            i2c_low_.SetTx(tx_buffer_[current_counter_]);
            current_counter_++;
            if (current_counter_ == tx_length_)
            {
                Finish_();
            }
        }
        return;
    case State::READING:
        if (i2c_low_.IsRxNotEmpty())
        {
            rx_buffer_[current_counter_] = i2c_low_.GetRx();
            current_counter_++;
            if (current_counter_ == rx_length_ - 1)
            {
                i2c_low_.DisableAck();
            }
            if (current_counter_ == rx_length_)
            {
                Finish_();
            }
        }
        return;
    }
}

template <typename CallbackType, int MAX_TX_LENGTH>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
inline void I2C<CallbackType, MAX_TX_LENGTH>::Finish_()
{
    i2c_low_.GenerateStop();
    i2c_low_.DisableEventInterrupt();
    i2c_low_.DisableErrorInterrupt();
    state_ = State::IDLE;
    current_counter_ = 0;
    tx_length_ = 0;
    rx_length_ = 0;
    transaction_complete_callback_();
}

} // namespace hydrv::I2C
