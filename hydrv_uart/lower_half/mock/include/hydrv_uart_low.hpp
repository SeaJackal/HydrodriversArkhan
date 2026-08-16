#ifndef HYDRV_UART_LOW_H_
#define HYDRV_UART_LOW_H_

#include <cstdint>

#include <cstring>

namespace hydrv::GPIO
{
struct GPIOLow
{
};
} // namespace hydrv::GPIO

typedef uint32_t hydrv_GPIOpinNumber;

namespace hydrv::UART
{
namespace Test
{
typedef void (*IRQHandlerFunc)(void);

class UARTMockEntry
{
public:
    UARTMockEntry(IRQHandlerFunc IRQ_handler)
        : rx_byte_(0),
          rx_flag_(false),
          tx_counter_(0),
          tx_IRQ_enabled_(false),
          IRQ_handler_(IRQ_handler)
    {
    }

public:
    bool IsTransmitted() { return tx_counter_ != 0; }

    bool IsReceived() { return rx_flag_; }

    void SetTx(uint8_t byte)
    {
        tx_buffer_[tx_counter_] = byte;
        tx_counter_++;
    }

    uint32_t GetTx(void *data)
    {
        memcpy(data, tx_buffer_, tx_counter_);
        uint32_t return_count = tx_counter_;
        tx_counter_ = 0;
        return return_count;
    }

    void SetRx(uint8_t byte)
    {
        rx_byte_ = byte;
        rx_flag_ = true;
        IRQ_handler_();
    }

    uint8_t GetRx()
    {
        rx_flag_ = false;
        return rx_byte_;
    }

    void EnableTxIRQ()
    {
        tx_IRQ_enabled_ = true;
        IRQ_handler_();
    }

    void DisableTxIRQ() { tx_IRQ_enabled_ = false; }

    bool IsTxIRQEnabled() { return tx_IRQ_enabled_; }

private:
    uint8_t rx_byte_;
    bool rx_flag_;

    uint8_t tx_buffer_[256];
    uint32_t tx_counter_;

    bool tx_IRQ_enabled_;

    IRQHandlerFunc IRQ_handler_;
};
} // namespace Test

class UARTLow
{
public:
    struct UARTPreset
    {
        Test::UARTMockEntry &mock;
    };

public:
    UARTLow(const UARTPreset &preset, hydrv::GPIO::GPIOLow &rx_pin,
            hydrv::GPIO::GPIOLow &tx_pin, unsigned IRQ_priority)
        : mock_(preset.mock)
    {
        (void)IRQ_priority;
        (void)rx_pin;
        (void)tx_pin;
    }

public:
    void Init() {}
    bool IsRxDone() { return mock_.IsReceived(); }
    bool IsTxDone() { return true; }

    uint8_t GetRx() { return mock_.GetRx(); }
    void SetTx(uint8_t byte) { mock_.SetTx(byte); }

    void EnableTxInterruption() { mock_.EnableTxIRQ(); }
    void DisableTxInterruption() { mock_.DisableTxIRQ(); }

private:
    Test::UARTMockEntry &mock_;
};
} // namespace hydrv::UART

#endif
