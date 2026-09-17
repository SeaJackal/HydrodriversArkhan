#pragma once

#include <cstdint>
#include <stdbool.h>
#include <stdint.h>

#include "hydrolib_return_codes.hpp"

extern "C"
{
#include "stm32f407xx.h"
#include "stm32f4xx.h"
}

namespace hydrv::clock
{

class ClockBase
{
public:
    static void SysTickHandler();
    static unsigned GetSystemTime();
    static void Delay(int time_ms);

private:
    static inline volatile unsigned systick_counter = 0;
};

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz = 0>
class Clock : public ClockBase
{
private:
    static constexpr int CalculateAPB1();
    static constexpr int CalculateAPB2();

public:
    static constexpr int kMaxSysclkFreqMhz = 168;

    static_assert(kTemplateSysclkFreqMhz == kMaxSysclkFreqMhz,
                  "Sysclk frequency not supported, only supported frequency - "
                  "kMaxSysclkFreqMhz");
    static_assert(kHSEFreqMhz == 0 || kHSEFreqMhz == 8, // NOLINT
                  "HSE frequency not supported, only supported frequency - 8 "
                  "Mhz"); // TODO: SeaJackal - Add pll config counter

    static constexpr int kSysclkFreqMhz = kTemplateSysclkFreqMhz;
    static constexpr int kAPB1FreqMhz = CalculateAPB1();
    static constexpr int kAPB2FreqMhz = CalculateAPB2();

    static constexpr int kTimeoutMs = 1000;

    static hydrolib::ReturnCode Init();

    static bool IsDefaultTickFailed();
    static bool IsHSIFailed();
    static bool IsHSEFailed();
    static bool IsPLLFailed();
    static bool IsSysTickFailed();

private:
    enum class PLLsource : uint32_t
    {
        kHSE = RCC_PLLCFGR_PLLSRC_HSE,
        kHSI = RCC_PLLCFGR_PLLSRC_HSI
    };

    struct ClockPreset
    {
        PLLsource source;
        uint32_t m;
        uint32_t n;
        uint32_t p;
        unsigned frequency_hse_mhz;
    };

    static constexpr ClockPreset GetClockPreset();

    static void EnablePowerClock();
    static void SetPowerVoltageScale();

    static hydrolib::ReturnCode EnableHSI();
    static hydrolib::ReturnCode EnableHSE();
    static void ConfigureSystemClock();
    static hydrolib::ReturnCode ConfigurePLL(uint32_t pllcfgr_value);

    static bool IsHSIReady();
    static bool IsHSEReady();
    static bool IsPLLReady();

    static constexpr uint32_t
    CalculatePLLCFGRRegValue(const ClockPreset &preset);
    static constexpr uint32_t CalculateLowFreqCFGRRegValue();
    static constexpr uint32_t CalculateCFGRRegValue();

    static constexpr int MhzToKhz(int freq);

    static constexpr uint32_t kPLLCFGRRegDefaultValue = 0x24003010;

    static constexpr uint32_t kPwrRegulatorVoltageScale1 = PWR_CR_VOS;
    static constexpr uint32_t kPwrRegulatorVoltageScale2 = 0;
    static constexpr int kFrequencyHSIMhz = 16;

    static constexpr uint32_t kPLLCFGRValue =
        CalculatePLLCFGRRegValue(GetClockPreset());
    static constexpr uint32_t kLowFreqCFGRRegValue =
        CalculateLowFreqCFGRRegValue();
    static constexpr uint32_t kCFGRRegValue = CalculateCFGRRegValue();

    static inline bool default_tick_failed = false;
    static inline bool hsi_failed = false;
    static inline bool hse_failed = false;
    static inline bool pll_failed = false;
    static inline bool sys_tick_failed = false;
};

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
constexpr int Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::MhzToKhz(int freq)
{
    return freq * 1000; // NOLINT
    // TODO: SeaJackal - Use special types for freqs (see chrono)
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
hydrolib::ReturnCode Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::Init()
{
    default_tick_failed = SysTick_Config(MhzToKhz(kFrequencyHSIMhz)) != 0;
    if (default_tick_failed)
    {
        return hydrolib::ReturnCode::kError;
    }

    EnablePowerClock();
    SetPowerVoltageScale();

    if constexpr (kHSEFreqMhz == 0)
    {
        hsi_failed = EnableHSI() != hydrolib::ReturnCode::kOk;
        if (hsi_failed)
        {
            return hydrolib::ReturnCode::kError;
        }
    }
    else
    {
        hse_failed = EnableHSE() != hydrolib::ReturnCode::kOk;
        if (hse_failed)
        {
            return hydrolib::ReturnCode::kError;
        }
    }

    pll_failed = ConfigurePLL(kPLLCFGRValue) != hydrolib::ReturnCode::kOk;
    if (pll_failed)
    {
        return hydrolib::ReturnCode::kError;
    }

    ConfigureSystemClock();

    sys_tick_failed = SysTick_Config(MhzToKhz(kSysclkFreqMhz)) != 0;
    if (sys_tick_failed)
    {
        return hydrolib::ReturnCode::kError;
    }

    return hydrolib::ReturnCode::kOk;
}

inline void ClockBase::SysTickHandler() { systick_counter++; }

inline unsigned ClockBase::GetSystemTime() { return systick_counter; }

inline void ClockBase::Delay(int time_ms)
{
    auto start_counter = GetSystemTime();
    auto current_counter = GetSystemTime();
    while (current_counter - start_counter < static_cast<unsigned>(time_ms))
    {
        current_counter = GetSystemTime();
    }
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
bool Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::IsDefaultTickFailed()
{
    return default_tick_failed;
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
bool Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::IsHSIFailed()
{
    return hsi_failed;
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
bool Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::IsHSEFailed()
{
    return hse_failed;
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
bool Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::IsPLLFailed()
{
    return pll_failed;
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
bool Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::IsSysTickFailed()
{
    return sys_tick_failed;
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
constexpr Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::ClockPreset
Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::GetClockPreset()
{
    if constexpr (kHSEFreqMhz == 0)
    {
        constexpr ClockPreset kHSIPreset{
            .source = PLLsource::kHSI,
            .m = 8,
            .n = 168,
            .p = 2,
            .frequency_hse_mhz = 0,
        };
        return kHSIPreset;
    }
    else
    {
        constexpr ClockPreset kHSEPreset{
            .source = PLLsource::kHSE,
            .m = 4,
            .n = 168,
            .p = 2,
            .frequency_hse_mhz = 8,
        };
        return kHSEPreset;
    }
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
void Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::EnablePowerClock()
{
    volatile uint32_t tmpreg = 0x00U;
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);
    tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);
    (void)tmpreg;
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
void Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::SetPowerVoltageScale()
{
    volatile uint32_t tmpreg = 0x00U;
    MODIFY_REG(PWR->CR, PWR_CR_VOS, kPwrRegulatorVoltageScale1);
    tmpreg = READ_BIT(PWR->CR, PWR_CR_VOS);
    (void)tmpreg;
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
hydrolib::ReturnCode Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::EnableHSI()
{
    SET_BIT(RCC->CR, RCC_CR_HSION);

    uint32_t start = GetSystemTime();
    while (!IsHSIReady())
    {
        if (GetSystemTime() - start > kTimeoutMs)
        {
            return hydrolib::ReturnCode::kFail;
        }
    }
    return hydrolib::ReturnCode::kOk;
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
hydrolib::ReturnCode Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::EnableHSE()
{
    SET_BIT(RCC->CR, RCC_CR_HSEON);

    uint32_t start = GetSystemTime();
    while (!IsHSEReady())
    {
        if (GetSystemTime() - start > kTimeoutMs)
        {
            return hydrolib::ReturnCode::kFail;
        }
    }
    return hydrolib::ReturnCode::kOk;
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
void Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::ConfigureSystemClock()
{
    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_5WS);

    RCC->CFGR = kLowFreqCFGRRegValue;

    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL);

    RCC->CFGR = kCFGRRegValue;
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
hydrolib::ReturnCode
Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::ConfigurePLL(uint32_t pllcfgr_value)
{
    CLEAR_BIT(RCC->CR, RCC_CR_PLLON);
    RCC->PLLCFGR = pllcfgr_value;
    SET_BIT(RCC->CR, RCC_CR_PLLON);

    uint32_t start = GetSystemTime();
    while (!IsPLLReady())
    {
        if (GetSystemTime() - start > kTimeoutMs)
        {
            return hydrolib::ReturnCode::kFail;
        }
    }

    return hydrolib::ReturnCode::kOk;
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
bool Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::IsHSIReady()
{
    return READ_BIT(RCC->CR, RCC_CR_HSIRDY);
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
bool Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::IsHSEReady()
{
    return READ_BIT(RCC->CR, RCC_CR_HSERDY);
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
bool Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::IsPLLReady()
{
    return READ_BIT(RCC->CR, RCC_CR_PLLRDY);
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
constexpr uint32_t
Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::CalculatePLLCFGRRegValue(
    const ClockPreset &preset)
{
    uint32_t result = kPLLCFGRRegDefaultValue;
    MODIFY_REG(result, RCC_PLLCFGR_PLLSRC,
               static_cast<uint32_t>(preset.source));
    MODIFY_REG(result, RCC_PLLCFGR_PLLM, preset.m << RCC_PLLCFGR_PLLM_Pos);
    MODIFY_REG(result, RCC_PLLCFGR_PLLN, preset.n << RCC_PLLCFGR_PLLN_Pos);
    MODIFY_REG(result, RCC_PLLCFGR_PLLP,
               ((preset.p / 2) - 1) << RCC_PLLCFGR_PLLP_Pos);
    return result;
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
constexpr uint32_t
Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::CalculateLowFreqCFGRRegValue()
{
    uint32_t result = 0;
    MODIFY_REG(result, RCC_CFGR_PPRE1, RCC_CFGR_PPRE1_DIV16);
    MODIFY_REG(result, RCC_CFGR_PPRE2, RCC_CFGR_PPRE2_DIV16);
    MODIFY_REG(result, RCC_CFGR_HPRE, RCC_CFGR_HPRE_DIV1);
    return result;
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
constexpr uint32_t
Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::CalculateCFGRRegValue()
{
    uint32_t result = 0;

    MODIFY_REG(result, RCC_CFGR_SW, RCC_CFGR_SW_PLL);

    MODIFY_REG(result, RCC_CFGR_PPRE1, RCC_CFGR_PPRE1_DIV4);
    MODIFY_REG(result, RCC_CFGR_PPRE2, RCC_CFGR_PPRE2_DIV2);
    return result;
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
constexpr int Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::CalculateAPB1()
{
    return kTemplateSysclkFreqMhz / 4;
}

template <int kTemplateSysclkFreqMhz, int kHSEFreqMhz>
constexpr int Clock<kTemplateSysclkFreqMhz, kHSEFreqMhz>::CalculateAPB2()
{
    return kTemplateSysclkFreqMhz / 2;
}

} // namespace hydrv::clock