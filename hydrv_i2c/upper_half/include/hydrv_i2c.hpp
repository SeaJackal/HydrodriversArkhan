#pragma once

#include "hydrv_i2c_base.hpp"

namespace hydrv::i2c
{

template <I2CIndex kIndex, typename CallbackType, int kMaxTxCapacity>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
class I2CBase<kIndex, CallbackType, kMaxTxCapacity>::I2C
{
public:
    template <typename T>
    explicit I2C(T &env);

    void Write(std::byte address, std::span<const std::byte> data);
    void Read(std::byte address, std::span<std::byte> data);

    void IRQCallback();

private:
    I2CBase<kIndex, CallbackType, kMaxTxCapacity> &i2c_base_;
    I2CLowBase<kIndex>::I2CLow i2c_low_;
};

template <I2CIndex kIndex, typename CallbackType, int kMaxTxCapacity>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
template <typename T>
I2CBase<kIndex, CallbackType, kMaxTxCapacity>::I2C::I2C(T &env)
    : i2c_base_(env.template GetPeriph<
                I2CBase<kIndex, CallbackType, kMaxTxCapacity>>()),
      i2c_low_(i2c_base_.i2c_low_)

{
}

template <I2CIndex kIndex, typename CallbackType, int kMaxTxCapacity>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
void I2CBase<kIndex, CallbackType, kMaxTxCapacity>::I2C::Write(
    std::byte address, std::span<const std::byte> data)
{
    std::ranges::copy(data, i2c_base_.tx_buffer_.begin());
    i2c_base_.direction_ = Direction::kWriting;
    i2c_base_.tx_iterator_ = i2c_base_.tx_buffer_.begin();
    i2c_base_.write_transaction_ =
        WriteTransaction<kIndex>(i2c_low_, address, data.size());
    i2c_base_.write_transaction_.Start();
}

template <I2CIndex kIndex, typename CallbackType, int kMaxTxCapacity>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
void I2CBase<kIndex, CallbackType, kMaxTxCapacity>::I2C::Read(
    std::byte address, std::span<std::byte> data)
{
    i2c_base_.rx_iterator_ = data.begin();
    i2c_base_.direction_ = Direction::kReading;
    i2c_base_.read_transaction_ =
        ReadTransaction<kIndex>(i2c_low_, address, data.size());
    i2c_base_.read_transaction_.Start();
}

template <I2CIndex kIndex, typename CallbackType, int kMaxTxCapacity>
requires hydrolib::concepts::func::FuncConcept<CallbackType, void>
void I2CBase<kIndex, CallbackType, kMaxTxCapacity>::I2C::IRQCallback()
{
    switch (i2c_base_.direction_)
    {
    case Direction::kWriting:
        if (i2c_base_.write_transaction_.Process())
        {
            if (i2c_base_.write_transaction_.Transmit(*i2c_base_.tx_iterator_))
            {
                i2c_base_.tx_iterator_++;
            }
        }
        if (i2c_base_.write_transaction_.IsFinished())
        {
            i2c_base_.transaction_complete_callback_();
        }
        break;
    case Direction::kReading:
        if (i2c_base_.read_transaction_.Process())
        {
            auto received = i2c_base_.read_transaction_.Receive();
            if (received)
            {
                *i2c_base_.rx_iterator_ = *received;
                i2c_base_.rx_iterator_++;
            }
        }
        if (i2c_base_.read_transaction_.IsFinished())
        {
            i2c_base_.transaction_complete_callback_();
        }
        break;
    }
}

} // namespace hydrv::i2c
