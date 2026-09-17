#pragma once

#include "hydrv_i2c_low.hpp"
#include <optional>

namespace hydrv::i2c
{
template <I2CIndex kI2CIndex>
class Transaction
{
public:
    constexpr Transaction() = default;
    Transaction(I2CLowBase<kI2CIndex>::I2CLow &i2c_low, std::byte address);

    bool Process();
    bool IsFinished();
    void Start();

protected:
    void Finish();

    I2CLowBase<kI2CIndex>::I2CLow *i2c_low_{};

private:
    enum class State
    {
        kStartCondition,
        kAddressWrite,
        kTransiving,
        kFinished
    };

    std::byte address_{};
    State state_ = State::kFinished;
};

template <I2CIndex kI2CIndex>
class WriteTransaction : public Transaction<kI2CIndex>
{
public:
    constexpr WriteTransaction() = default;
    WriteTransaction(I2CLowBase<kI2CIndex>::I2CLow &i2c_low, std::byte address,
                     int size);

    using Transaction<kI2CIndex>::Process;
    using Transaction<kI2CIndex>::IsFinished;
    bool Transmit(std::byte data);

private:
    int size_ = 0;
    int current_counter_ = 0;
};

template <I2CIndex kI2CIndex>
class ReadTransaction : public Transaction<kI2CIndex>
{
public:
    constexpr ReadTransaction() = default;
    ReadTransaction(I2CLowBase<kI2CIndex>::I2CLow &i2c_low, std::byte address,
                    int size);

    using Transaction<kI2CIndex>::Process;
    using Transaction<kI2CIndex>::IsFinished;
    std::optional<std::byte> Receive();

private:
    int size_ = 0;
    int current_counter_ = 0;
};

template <I2CIndex kI2CIndex>
Transaction<kI2CIndex>::Transaction(I2CLowBase<kI2CIndex>::I2CLow &i2c_low,
                                    std::byte address)
    : i2c_low_(&i2c_low), address_(address), state_(State::kStartCondition)
{
}

template <I2CIndex kI2CIndex>
bool Transaction<kI2CIndex>::Process()
{
    if (i2c_low_->IsAckFailure())
    {
        i2c_low_->ClearAckFailure();
        Finish();
        return false;
    }

    switch (state_)
    {
    case State::kStartCondition:
        if (i2c_low_->IsStartBit())
        {
            i2c_low_->SendAddress(address_);
            state_ = State::kAddressWrite;
        }
        return false;
    case State::kAddressWrite:
        if (i2c_low_->IsAddr())
        {
            i2c_low_->ClearAddr();
            state_ = State::kTransiving;
        }
        return true;
    case State::kTransiving:
        return true;
    case State::kFinished:
        return false;
    }
}

template <I2CIndex kI2CIndex>
bool Transaction<kI2CIndex>::IsFinished()
{
    return state_ == State::kFinished;
}

template <I2CIndex kI2CIndex>
void Transaction<kI2CIndex>::Start()
{
    i2c_low_->ClearStatusBits();
    i2c_low_->EnableEventInterrupt();
    i2c_low_->EnableErrorInterrupt();
    i2c_low_->GenerateStart();
}

template <I2CIndex kI2CIndex>
void Transaction<kI2CIndex>::Finish()
{
    i2c_low_->GenerateStop();
    i2c_low_->DisableEventInterrupt();
    i2c_low_->DisableErrorInterrupt();
    state_ = State::kFinished;
}

template <I2CIndex kI2CIndex>
WriteTransaction<kI2CIndex>::WriteTransaction(
    I2CLowBase<kI2CIndex>::I2CLow &i2c_low, std::byte address, int size)
    : Transaction<kI2CIndex>(i2c_low, address), size_(size)
{
}

template <I2CIndex kI2CIndex>
bool WriteTransaction<kI2CIndex>::Transmit(std::byte data)
{
    if (!Transaction<kI2CIndex>::i2c_low_->IsTxEmpty())
    {
        return false;
    }
    Transaction<kI2CIndex>::i2c_low_->SetTx(data);
    current_counter_++;
    if (current_counter_ == size_)
    {
        Transaction<kI2CIndex>::Finish();
    }
    return true;
}

template <I2CIndex kI2CIndex>
ReadTransaction<kI2CIndex>::ReadTransaction(
    I2CLowBase<kI2CIndex>::I2CLow &i2c_low, std::byte address, int size)
    : Transaction<kI2CIndex>(
          i2c_low, static_cast<std::byte>(static_cast<uint8_t>(address) | 0x1)),
      size_(size)
{
    Transaction<kI2CIndex>::i2c_low_->EnableAck();
}

template <I2CIndex kI2CIndex>
std::optional<std::byte> ReadTransaction<kI2CIndex>::Receive()
{
    if (!Transaction<kI2CIndex>::i2c_low_->IsRxNotEmpty())
    {
        return std::nullopt;
    }
    current_counter_++;
    if (current_counter_ == size_ - 1)
    {
        Transaction<kI2CIndex>::i2c_low_->DisableAck();
    }
    if (current_counter_ == size_)
    {
        Transaction<kI2CIndex>::Finish();
    }
    return Transaction<kI2CIndex>::i2c_low_->GetRx();
}

} // namespace hydrv::i2c
