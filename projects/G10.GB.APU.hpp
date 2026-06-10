/**
 * @file    G10.GB.APU.hpp
 * @brief   Contains declarations for the G10.Boy's audio processing unit
 *          (APU) component, and related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.GB.Common.hpp>

// Types ***********************************************************************

namespace G10::GB
{
    class System;
    class Timer;

    using SampleCallback = std::function<void (const System&, float, float)>;
    using RawSampleCallback = std::function<void (const System&, std::int16_t, std::int16_t)>;
}

// Constants & Enumeartions ****************************************************

namespace G10::GB
{
    constexpr std::uint8_t
        kWaveMemorySize         = 16;

    constexpr std::uint32_t
        kDefaultSampleRate      = 44100,
        kMinimumSampleRate      = 8000,
        kMaximumSampleRate      = 192000;

    constexpr float
        kCapacitorChargeRate    = 0.01f,
        kCapacitorDischargeRate = 0.001f,
        kHighPassAlphaDMG       = 0.999958f,
        kHighPassAlphaCGB       = 0.9995f;

    static constexpr std::array<std::uint8_t, 4>
        kWaveDutyPatterns = {
            0b00000001,
            0b10000001,
            0b10000111,
            0b01111110
        };

    enum class AudioChannel : std::uint8_t
    {
        Channel1,
        Channel2,
        Channel3,
        Channel4,

        Pulse1      = Channel1,
        Pulse2      = Channel2,
        Wave        = Channel3,
        Noise       = Channel4
    };

    enum class WaveOutputLevel : std::uint8_t
    {
        Mute,
        Full,
        Half,
        Quarter
    };
}

// Unions & Structures *********************************************************

namespace G10::GB
{
    union AudioControlRegister final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t    mChannel1On     : 1;
            std::uint8_t    mChannel2On     : 1;
            std::uint8_t    mChannel3On     : 1;
            std::uint8_t    mChannel4On     : 1;
            std::uint8_t                    : 2;
            std::uint8_t                    : 1;
            std::uint8_t    mMasterOn       : 1;
        };
    };

    union AudioPanningRegister final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t    mChannel1Right  : 1;
            std::uint8_t    mChannel2Right  : 1;
            std::uint8_t    mChannel3Right  : 1;
            std::uint8_t    mChannel4Right  : 1;
            std::uint8_t    mChannel1Left   : 1;
            std::uint8_t    mChannel2Left   : 1;
            std::uint8_t    mChannel3Left   : 1;
            std::uint8_t    mChannel4Left   : 1;
        };
    };

    union AudioVolumeRegister final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t    mVolumeRight    : 3;
            std::uint8_t    mExternalRight  : 1;
            std::uint8_t    mVolumeLeft     : 3;
            std::uint8_t    mExternalLeft   : 1;
        };
    };

    union PulseSweepRegister final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t    mIndividualStep         : 3;
            std::uint8_t    mIsSweepingDownwards    : 1;
            std::uint8_t    mPace                   : 3;
            std::uint8_t                            : 1;
        };
    };

    union LengthRegister final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t    mLength     : 6;
            std::uint8_t    mDutyCycle  : 2;
        };
    };

    union EnvelopeRegister final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t    mPace               : 3;
            std::uint8_t    mIsSweepingUpwards  : 1;
            std::uint8_t    mInitialVolume      : 4;
        };
    };

    union WaveEnableRegister final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t                        : 7;
            std::uint8_t    mDacEnable          : 1;
        };
    };

    union WaveVolumeRegister final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t                        : 5;
            std::uint8_t    mOutputLevel        : 1;
            std::uint8_t                        : 1;
        };
    };

    union NoisePolynomialRegister final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t    mClockDivider       : 3;
            std::uint8_t    mIsShortWidth       : 1;
            std::uint8_t    mClockShift         : 4;
        };
    };

    union ChannelControlRegister final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t    mPeriodHigh         : 3;
            std::uint8_t                        : 3;
            std::uint8_t    mLengthEnable       : 1;
            std::uint8_t    mTrigger            : 1;
        };
    };

    struct DigitalAudioConverter final
    {
        bool            mEnable { false };
        std::uint8_t    mDigital { 0 };
        float           mAnalog { 0.0f };
        float           mCharge { 1.0f };
    };

    struct PulseChannel final
    {
        // DAC
        DigitalAudioConverter   mDAC;

        // Sweep Unit (CH1 only)
        bool                    mSweepEnabled { false };
        std::uint16_t           mSweepShadow { 0 };
        std::uint8_t            mSweepTimer { 0 };
        bool                    mSweepNegate { false };

        // Length Timer
        bool                    mLengthEnable { false };
        std::uint16_t           mLengthTimer { 0 };

        // Envelope Unit
        std::uint8_t            mVolume { 0 };
        std::uint8_t            mEnvelopeTimer { 0 };
        bool                    mEnvelopeRunning { false };

        // Period Divider
        std::uint16_t           mPeriodTimer { 0 };
        std::uint8_t            mDutyPosition { 0 };

        // Trigger Handling
        bool                    mTriggeredSincePowerOn { false };
        bool                    mFirstSampleSupressed { false };
    };

    struct WaveChannel final
    {
        // DAC
        DigitalAudioConverter   mDAC;

        // Length Timer
        bool                    mLengthEnable { false };
        std::uint16_t           mLengthTimer { 0 };

        // Period Divider
        std::uint16_t           mPeriodTimer { 0 };
        std::uint8_t            mWavePosition { 0 };
        std::uint8_t            mLastWaveByte { 0 };
    };

    struct NoiseChannel final
    {
        // DAC
        DigitalAudioConverter   mDAC;

        // Length Timer
        bool                    mLengthEnable { false };
        std::uint16_t           mLengthTimer { 0 };

        // Envelope Unit
        std::uint8_t            mVolume { 0 };
        std::uint8_t            mEnvelopeTimer { 0 };
        bool                    mEnvelopeRunning { false };

        // Period Divider
        std::uint16_t           mPeriodTimer { 0 };
        std::uint16_t           mLFSR { 0x7FFF };
    };
}

// Classes *********************************************************************

namespace G10::GB
{
    class G10_API APU final
    {   
        friend class Timer;

    public: // Constructors & Destructor ***************************************

        explicit APU (System& pSystem);

    public: // Methods *********************************************************

        auto Reset () -> void;
        auto Clock (const std::uint64_t& pCycle) -> bool;

    public: // Methods - Callbacks *********************************************

        auto SetSampleCallback (const SampleCallback& pCallback) -> void;
        auto SetRawSampleCallback (const RawSampleCallback& pCallback) -> void;

    public: // Methods - Bus Access ********************************************

        auto ReadWaveRAM (std::uint32_t pRelAddress, std::uint8_t& pDataOut) -> bool;
        auto WriteWaveRAM (std::uint32_t pRelAddress, std::uint8_t pDataIn) -> bool;

    public: // Methods - Port Registers ****************************************

        auto ReadNR10 (std::uint8_t& pDataOut) -> bool;
        auto ReadNR11 (std::uint8_t& pDataOut) -> bool;
        auto ReadNR12 (std::uint8_t& pDataOut) -> bool;
        auto ReadNR14 (std::uint8_t& pDataOut) -> bool;
        auto ReadNR21 (std::uint8_t& pDataOut) -> bool;
        auto ReadNR22 (std::uint8_t& pDataOut) -> bool;
        auto ReadNR24 (std::uint8_t& pDataOut) -> bool;
        auto ReadNR30 (std::uint8_t& pDataOut) -> bool;
        auto ReadNR32 (std::uint8_t& pDataOut) -> bool;
        auto ReadNR34 (std::uint8_t& pDataOut) -> bool;
        auto ReadNR42 (std::uint8_t& pDataOut) -> bool;
        auto ReadNR43 (std::uint8_t& pDataOut) -> bool;
        auto ReadNR44 (std::uint8_t& pDataOut) -> bool;
        auto ReadNR50 (std::uint8_t& pDataOut) -> bool;
        auto ReadNR51 (std::uint8_t& pDataOut) -> bool;
        auto ReadNR52 (std::uint8_t& pDataOut) -> bool;
        auto ReadPCM12 (std::uint8_t& pDataOut) -> bool;
        auto ReadPCM34 (std::uint8_t& pDataOut) -> bool;

        auto WriteNR10 (std::uint8_t pDataIn) -> bool;
        auto WriteNR11 (std::uint8_t pDataIn) -> bool;
        auto WriteNR12 (std::uint8_t pDataIn) -> bool;
        auto WriteNR13 (std::uint8_t pDataIn) -> bool;
        auto WriteNR14 (std::uint8_t pDataIn) -> bool;
        auto WriteNR21 (std::uint8_t pDataIn) -> bool;
        auto WriteNR22 (std::uint8_t pDataIn) -> bool;
        auto WriteNR23 (std::uint8_t pDataIn) -> bool;
        auto WriteNR24 (std::uint8_t pDataIn) -> bool;
        auto WriteNR30 (std::uint8_t pDataIn) -> bool;
        auto WriteNR31 (std::uint8_t pDataIn) -> bool;
        auto WriteNR32 (std::uint8_t pDataIn) -> bool;
        auto WriteNR33 (std::uint8_t pDataIn) -> bool;
        auto WriteNR34 (std::uint8_t pDataIn) -> bool;
        auto WriteNR41 (std::uint8_t pDataIn) -> bool;
        auto WriteNR42 (std::uint8_t pDataIn) -> bool;
        auto WriteNR43 (std::uint8_t pDataIn) -> bool;
        auto WriteNR44 (std::uint8_t pDataIn) -> bool;
        auto WriteNR50 (std::uint8_t pDataIn) -> bool;
        auto WriteNR51 (std::uint8_t pDataIn) -> bool;
        auto WriteNR52 (std::uint8_t pDataIn) -> bool;

    public: // Methods - Accessors *********************************************

        inline auto GetChannel1 () const -> const PulseChannel&
            { return mChannel1; }
        inline auto GetChannel2 () const -> const PulseChannel&
            { return mChannel2; }
        inline auto GetChannel3 () const -> const WaveChannel&
            { return mChannel3; }
        inline auto GetChannel4 () const -> const NoiseChannel&
            { return mChannel4; }
        inline auto GetSampleRate () const -> std::uint32_t
            { return mSampleRate; }
        auto SetSampleRate (const std::uint32_t pSampleRate) -> void;

    private: // Methods - Helpers **********************************************

        auto CalculatePulseSweep () -> std::uint16_t;
        auto CalculateCyclesPerSample () -> std::uint64_t;
        auto DischargeCapacitor (DigitalAudioConverter& pDAC) -> void;
        auto ChargeCapacitor (DigitalAudioConverter& pDAC) -> void;
        auto UpdateCapacitorCharge (DigitalAudioConverter& pDAC) -> void;
        auto GenerateSample () -> void;

    private: // Methods - Channel Initialiation & Triggering *******************

        auto InitializeChannel (AudioChannel pChannel) -> void;
        auto TriggerChannel (AudioChannel pChannel) -> void;
        
    private: // Methods - Period Dividers **************************************

        auto ClockPulseChannels () -> void;
        auto ClockWaveChannel () -> void;
        auto ClockNoiseChannel () -> void;

    private: // Methods - Frame Sequencer **************************************

        auto ClockFrameSequencer () -> void;
        auto ClockEnvelopeSweepUnits () -> void;
        auto ClockLengthTimers () -> void;
        auto ClockPulseSweepUnit () -> void;

    private: // Members ********************************************************

        System& mSystem;

        // Callbacks
        SampleCallback      mSampleCallback { nullptr };
        RawSampleCallback   mRawSampleCallback { nullptr };

        // Bus Access
        std::array<std::uint8_t, kWaveMemorySize>   mWaveRAM {};

        // Port Registers
        PulseSweepRegister      mNR10 {};
        WaveEnableRegister                              mNR30 {};
        LengthRegister          mNR11 {},   mNR21 {},   mNR31 {},   mNR41 {};
        EnvelopeRegister        mNR12 {},   mNR22 {},               mNR42 {};
        WaveVolumeRegister                              mNR32 {};
        std::uint8_t            mNR13 {},   mNR23 {},   mNR33 {};
        NoisePolynomialRegister                                     mNR43 {};
        ChannelControlRegister  mNR14 {},   mNR24 {},   mNR34 {},   mNR44 {};
        AudioVolumeRegister     mNR50 {};
        AudioPanningRegister    mNR51 {};
        AudioControlRegister    mNR52 {};

        // Internal State - Channels
        PulseChannel    mChannel1 {};
        PulseChannel    mChannel2 {};
        WaveChannel     mChannel3 {};
        NoiseChannel    mChannel4 {};

        // Internal State - Sample Rate
        std::uint32_t   mSampleRate { kDefaultSampleRate };
        std::uint32_t   mMixClockFrequency { 0 };

        // Internal State - Timing
        std::uint64_t   mCycles { 0 };
        std::uint64_t   mCyclesPerSample { 0 };
        std::uint64_t   mCycleAccumulator { 0 };
        std::uint8_t    mFrameSequencerStep { 0 };

        // Internal State - High Pass Filter
        float   mHighPassLeft { 0.0f };
        float   mHighPassRight { 0.0f };

    };
}
