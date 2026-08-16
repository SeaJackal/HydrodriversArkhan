#pragma once

#include "hydrolib_func_concepts.hpp"
#include "hydrv_gpio_low.hpp"
#include "hydrv_spi_low.hpp"
#include <cstdint>
#include <cstring>

namespace hydrv::SPI
{

template <uint8_t IDLE_TX = 0x00,
          typename CallbackType =
              decltype(&hydrolib::concepts::func::DummyFunc<void>),
          int MAX_TX_LENGTH = 255>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
class SPI
{
public:
    consteval SPI(const SPILow::SPIPreset &preset,
                  hydrv::GPIO::GPIOLow &sck_pin, hydrv::GPIO::GPIOLow &miso_pin,
                  hydrv::GPIO::GPIOLow &mosi_pin, unsigned IRQ_priority,
                  SPILow::BaudratePrescaler baudrate_prescaler,
                  SPILow::ClockPolarity clock_polarity,
                  SPILow::ClockPhase clock_phase, SPILow::DataSize data_size,
                  SPILow::BitOrder bit_order, hydrv::GPIO::GPIOLow &cs_pin,
                  CallbackType transaction_complete_callback =
                      hydrolib::concepts::func::DummyFunc<void>);

    void Init();

    void MakeTransaction(const void *tx_buffer, int tx_length, void *rx_buffer,
                         int rx_length);

    void IRQCallback();

private:
    SPILow spi_low_;
    hydrv::GPIO::GPIOLow &cs_pin_;
    int transaction_counter_;
    int tx_length_;
    int transaction_length_;
    uint8_t tx_buffer_[MAX_TX_LENGTH] = {};
    uint8_t *rx_buffer_;
    CallbackType transaction_complete_callback_;
};

template <uint8_t IDLE_TX, typename CallbackType, int MAX_TX_LENGTH>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
consteval SPI<IDLE_TX, CallbackType, MAX_TX_LENGTH>::SPI(
    const SPILow::SPIPreset &preset, hydrv::GPIO::GPIOLow &sck_pin,
    hydrv::GPIO::GPIOLow &miso_pin, hydrv::GPIO::GPIOLow &mosi_pin,
    unsigned IRQ_priority, SPILow::BaudratePrescaler baudrate_prescaler,
    SPILow::ClockPolarity clock_polarity, SPILow::ClockPhase clock_phase,
    SPILow::DataSize data_size, SPILow::BitOrder bit_order,
    hydrv::GPIO::GPIOLow &cs_pin, CallbackType transaction_complete_callback)
    : spi_low_(preset, sck_pin, miso_pin, mosi_pin, IRQ_priority,
               baudrate_prescaler, clock_polarity, clock_phase, data_size,
               bit_order),
      cs_pin_(cs_pin),
      transaction_counter_(0),
      tx_length_(0),
      transaction_length_(0),
      rx_buffer_(nullptr),
      transaction_complete_callback_(transaction_complete_callback)
{
}

template <uint8_t IDLE_TX, typename CallbackType, int MAX_TX_LENGTH>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
inline void SPI<IDLE_TX, CallbackType, MAX_TX_LENGTH>::Init()
{
    cs_pin_.Init();
    cs_pin_.Set();
    spi_low_.Init();
}

template <uint8_t IDLE_TX, typename CallbackType, int MAX_TX_LENGTH>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
inline void SPI<IDLE_TX, CallbackType, MAX_TX_LENGTH>::MakeTransaction(const void *tx_buffer,
                                                        int tx_length,
                                                        void *rx_buffer,
                                                        int rx_length)
{
    cs_pin_.Reset();
    tx_length_ = tx_length;
    transaction_length_ = tx_length + rx_length;
    if (tx_length)
    {
        memcpy(tx_buffer_, tx_buffer, tx_length);
        rx_buffer_ = static_cast<uint8_t *>(rx_buffer);
        spi_low_.SetTx(tx_buffer_[0]);
    }
    else
    {
        rx_buffer_ = static_cast<uint8_t *>(rx_buffer);
        spi_low_.SetTx(IDLE_TX);
    }
    spi_low_.EnableRxInterruption();
}

template <uint8_t IDLE_TX, typename CallbackType, int MAX_TX_LENGTH>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
inline void SPI<IDLE_TX, CallbackType, MAX_TX_LENGTH>::IRQCallback()
{
    if (spi_low_.IsRxDone())
    {
        transaction_counter_++;
        if (transaction_counter_ > tx_length_)
        {
            rx_buffer_[transaction_counter_ - tx_length_ - 1] =
                spi_low_.GetRx();
        }
        else
        {
            [[maybe_unused]] int pass = spi_low_.GetRx();
        }
        if (transaction_counter_ < tx_length_)
        {
            spi_low_.SetTx(tx_buffer_[transaction_counter_]);
        }
        else if (transaction_counter_ < transaction_length_)
        {
            spi_low_.SetTx(IDLE_TX);
        }
        else
        {
            cs_pin_.Set();
            spi_low_.DisableRxInterruption();
            transaction_counter_ = 0;
            transaction_complete_callback_();
        }
    }
}

} // namespace hydrv::SPI
