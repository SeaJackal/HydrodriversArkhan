#pragma once

#include <cstdint>

#define HYDRV_NEW_TYPE(Name, type)                                             \
    class Name                                                                 \
    {                                                                          \
    public:                                                                    \
        explicit constexpr Name(type value) : value_(value) {}                 \
        constexpr operator type() const { return value_; }                     \
                                                                               \
    private:                                                                   \
        uint32_t value_;                                                       \
    }

namespace hydrv::gpio
{
class GPIOLowCtrlBase
{
public:
    class GPIOLowCtrl;

    HYDRV_NEW_TYPE(GPIOHandler, uint32_t);
    HYDRV_NEW_TYPE(SetRegMask, uint32_t);
    HYDRV_NEW_TYPE(ResetRegMask, uint32_t);

    consteval GPIOLowCtrlBase(GPIOHandler gpiox, SetRegMask set_reg_mask,
                              ResetRegMask reset_reg_mask);

    GPIOLowCtrlBase(const GPIOLowCtrlBase &) = delete;
    GPIOLowCtrlBase &operator=(const GPIOLowCtrlBase &) = delete;
    GPIOLowCtrlBase &operator=(GPIOLowCtrlBase &&) = delete;

    GPIOLowCtrlBase(GPIOLowCtrlBase &&) = default;

    ~GPIOLowCtrlBase() = default;

private:
    // TODO: SeaJackal - need to be made not movable after creating tuple to
    // store not movable objects
    // NOLINTBEGIN(cppcoreguidelines-avoid-const-or-ref-data-members)
    // Class should be not movable
    const uint32_t gpiox_;

    const uint32_t set_reg_mask_;
    const uint32_t reset_reg_mask_;
    // NOLINTEND(cppcoreguidelines-avoid-const-or-ref-data-members)
};

consteval GPIOLowCtrlBase::GPIOLowCtrlBase(
    GPIOLowCtrlBase::GPIOHandler gpiox,
    GPIOLowCtrlBase::SetRegMask set_reg_mask,
    GPIOLowCtrlBase::ResetRegMask reset_reg_mask)
    : gpiox_(gpiox),
      set_reg_mask_(set_reg_mask),
      reset_reg_mask_(reset_reg_mask)
{
}
} // namespace hydrv::gpio
