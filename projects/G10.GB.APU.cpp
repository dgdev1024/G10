/**
 * @file    G10.GB.APU.cpp
 * @brief   Contains implementations for the G10.Boy's audio processing unit
 *          (APU) component, and related definitions.
 */

// Includes ********************************************************************

#include <G10.GB.System.hpp>

// Public - Constructors & Destructor ******************************************

namespace G10::GB
{
    APU::APU (System& pSystem) :
        mSystem     { pSystem }
    {
    }
}

// Public Methods **************************************************************

namespace G10::GB
{
    auto APU::Reset () -> void
    {
        // Initialize Buffers
        std::ranges::fill(mWaveRAM, 0);

        // Initialize Hardware Registers
        mNR10.mValue    = 0x80;
        mNR11.mValue    = 0xBF;
        mNR12.mValue    = 0xF3;
        mNR13           = 0xFF;
        mNR14.mValue    = 0xBF;
        mNR21.mValue    = 0x3F;
        mNR22.mValue    = 0x00;
        mNR23           = 0xFF;
        mNR24.mValue    = 0xBF;
        mNR30.mValue    = 0x7F;
        mNR31.mValue    = 0xFF;
        mNR32.mValue    = 0x9F;
        mNR33           = 0xFF;
        mNR34.mValue    = 0xBF;
        mNR41.mValue    = 0xFF;
        mNR42.mValue    = 0x00;
        mNR43.mValue    = 0x00;
        mNR44.mValue    = 0xBF;
        mNR50.mValue    = 0x77;
        mNR51.mValue    = 0xF3;
        mNR52.mValue    = 0xF1;

        // Initialize Channels
        InitializeChannel(AudioChannel::Channel1);
        InitializeChannel(AudioChannel::Channel2);
        InitializeChannel(AudioChannel::Channel3);
        InitializeChannel(AudioChannel::Channel4);

        // Initialize Internal State
        mCycles = 0;
        mCyclesPerSample = 0;
        mCycleAccumulator = 0;
        mFrameSequencerStep = 0;
        mHighPassLeft = 0.0f;
        mHighPassRight = 0.0f;
        CalculateCyclesPerSample();
    }

    auto APU::Clock (const std::uint64_t& pCycle) -> bool
    {
        // Component Clock Rate: 1 Dot per 2 CPU Cycles
        // if ((pCycle & 1) != 1)
        //     { return true; }

        // If APU is off, don't process, but still generate silence.
        if (mNR52.mMasterOn == false)
        {
            mCycles++;
            if (++mCycleAccumulator >= mCyclesPerSample)
            {
                mCycleAccumulator -= mCyclesPerSample;
                GenerateSample();
            }

            return true;
        }

        // Clock Channels
        // - Tick pulse channels every 4 dots.
        // - Tick wave channel every 2 dots.
        // - Tick noise channel.
        mCycles++;
        if ((mCycles & 3) == 0)
            { ClockPulseChannels(); }
        if ((mCycles & 1) == 0)
            { ClockWaveChannel(); }
        ClockNoiseChannel();            // Timing handled based on `NR43`.

        // Generate audio sample at configured sample rate.
        if (++mCycleAccumulator >= mCyclesPerSample)
        {
            mCycleAccumulator -= mCyclesPerSample;
            GenerateSample();
        }

        return true;
    }
}

// Public Methods - Callbacks **************************************************

namespace G10::GB
{
    auto APU::SetSampleCallback (const SampleCallback& pCallback) -> void
    {
        mSampleCallback = pCallback;
    }

    auto APU::SetRawSampleCallback (const RawSampleCallback& pCallback) -> void
    {
        mRawSampleCallback = pCallback;
    }
}

// Public Methods - Bus Access *************************************************

namespace G10::GB
{
    auto APU::ReadWaveRAM (std::uint32_t pRelAddress, std::uint8_t& pDataOut) -> bool
    {
        if (mNR52.mChannel3On == true)
        {
            pDataOut = mWaveRAM[(mChannel3.mWavePosition >> 1) & 0xF];
            return true;
        }

        pDataOut = mWaveRAM[pRelAddress & 0x0F];
        return true;
    }

    auto APU::WriteWaveRAM (std::uint32_t pRelAddress, std::uint8_t pDataIn) -> bool
    {
        if (mNR52.mChannel3On == true)
        {
            mWaveRAM[(mChannel3.mWavePosition >> 1) & 0x0F] = pDataIn;
            return true;
        }

        mWaveRAM[pRelAddress & 0x0F] = pDataIn;
        return true;
    }
}

// Public Methods - Port Registers *********************************************

namespace G10::GB
{
    auto APU::ReadNR10 (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - Bit 7 is unused; read `1`.
        // - Bits 0 through 6 are readable.
        pDataOut = 0b10000000 | (mNR10.mValue & 0b01111111);
        return true;
    }

    auto APU::ReadNR11 (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - Bits 0 through 5 are write-only; read `1`.
        // - Bits 6 and 7 are readable.
        pDataOut = 0b00111111 | (mNR11.mValue & 0b11000000);
        return true;
    }

    auto APU::ReadNR12 (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        pDataOut = mNR12.mValue;
        return true;
    }

    auto APU::ReadNR14 (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - Bits 0 through 2 and 7 are write-only; read `1`.
        // - Bits 3 through 5 are unused; read `1`.
        // - Bit 6 is readable.
        pDataOut = 0b10111111 | (mNR14.mValue & 0b01000000);
        return true;
    }

    auto APU::ReadNR21 (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - Bits 0 through 5 are write-only; read `1`.
        // - Bits 6 and 7 are readable.
        pDataOut = 0b00111111 | (mNR21.mValue & 0b11000000);
        return true;
    }

    auto APU::ReadNR22 (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        pDataOut = mNR22.mValue;
        return true;
    }

    auto APU::ReadNR24 (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - Bits 0 through 2 and 7 are write-only; read `1`.
        // - Bits 3 through 5 are unused; read `1`.
        // - Bit 6 is readable.
        pDataOut = 0b10111111 | (mNR24.mValue & 0b01000000);
        return true;
    }

    auto APU::ReadNR30 (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - Bits 0 through 6 are unused; read `1`.
        // - Bit 7 is readable.
        pDataOut = 0b01111111 | (mNR30.mValue & 0b10000000);
        return true;
    }

    auto APU::ReadNR32 (std::uint8_t& pDataOut) -> bool
    {
        // Read 
        // - Bits 0 through 4 and 7 are unused; read `1`.
        // - Bits 5 and 6 are readable.
        pDataOut = 0b10011111 | (mNR32.mValue & 0b01100000);
        return true;
    }

    auto APU::ReadNR34 (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - Bits 0 through 2 and 7 are write-only; read `1`.
        // - Bits 3 through 5 are unused; read `1`.
        // - Bit 6 is readable.
        pDataOut = 0b10111111 | (mNR34.mValue & 0b01000000);
        return true;
    }

    auto APU::ReadNR42 (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        pDataOut = mNR42.mValue;
        return true;
    }

    auto APU::ReadNR43 (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        pDataOut = mNR43.mValue;
        return true;
    }

    auto APU::ReadNR44 (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - Bit 7 is write-only; read `1`.
        // - Bits 0 through 5 are unused; read `1`.
        // - Bit 6 is readable.
        pDataOut = 0b10111111 | (mNR44.mValue & 0b01000000);
        return true;
    }

    auto APU::ReadNR50 (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        pDataOut = mNR50.mValue;
        return true;
    }

    auto APU::ReadNR51 (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        pDataOut = mNR51.mValue;
        return true;
    }

    auto APU::ReadNR52 (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - Bits 4 through 6 are unused; read `1`.
        // - Bits 0 through 3 and 7 are readable.
        pDataOut = 0b01110000 | (mNR52.mValue & 0b10001111);
        return true;
    }

    auto APU::ReadPCM12 (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        // - Bits 0 through 3 contain Channel 1's DAC digital output.
        // - Bits 4 through 7 contain Channel 2's DAC digital output.
        pDataOut = 
            ((mChannel2.mDAC.mDigital & 0xF) << 4) |
            (mChannel1.mDAC.mDigital & 0xF);
        return true;
    }

    auto APU::ReadPCM34 (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        // - Bits 0 through 3 contain Channel 3's DAC digital output.
        // - Bits 4 through 7 contain Channel 4's DAC digital output.
        pDataOut = 
            ((mChannel4.mDAC.mDigital & 0xF) << 4) |
            (mChannel3.mDAC.mDigital & 0xF);
        return true;
    }

    auto APU::WriteNR10 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore if `NR52.7` is clear.
        // - If switching from downwards to upwards sweep after have used
        //   downwards sweep since last trigger, then disable this channel.
        if (mNR52.mMasterOn == false)
            { return true; }

        PulseSweepRegister
            oldSweep = mNR10,
            newSweep = { .mValue = static_cast<std::uint8_t>(0b10000000 | (pDataIn & 0b01111111)) };
        if (oldSweep.mIsSweepingDownwards == true &&
            newSweep.mIsSweepingDownwards == false &&
            mChannel1.mSweepNegate == true)
            { mNR52.mChannel1On = false; }

        // Write
        // - Bit 7 is unused; write `1`.
        // - Bits 0 through 6 are writable.
        mNR10.mValue = newSweep.mValue;
        return true;
    }

    auto APU::WriteNR11 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore if `NR52.7` is clear.
        if (mNR52.mMasterOn == false)
            { return true; }

        // Write
        // - All bits writable
        mNR11.mValue = pDataIn;

        // After Write
        // - Reload length timer with `(64 - (NR11.0-5))`
        // - If `NR11.0-5` is zero, then load with 64 instead.
        mChannel1.mLengthTimer = (mNR11.mLength == 0) ? 
            64 : (64 - mNR11.mLength);
        return true;
    }

    auto APU::WriteNR12 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore if `NR52.7` is clear.
        // - Writes while the channel is active can affect its current volume.
        if (mNR52.mMasterOn == false)
            { return true; }
        
        EnvelopeRegister newNR12 = { .mValue = pDataIn };
        if (mNR52.mChannel1On == true)
        {
            if (mNR12.mPace == 0 && mChannel1.mEnvelopeRunning == true)
                { mChannel1.mVolume = (mChannel1.mVolume + 1) & 0xF; }
            else if (mNR12.mIsSweepingUpwards == false)
                { mChannel1.mVolume = (mChannel1.mVolume + 2) & 0xF; }

            if (mNR12.mIsSweepingUpwards != newNR12.mIsSweepingUpwards)
                { mChannel1.mVolume = (16 - mChannel1.mVolume) & 0xF; }
        }

        // Write
        // - All bits writable.
        mNR12.mValue = pDataIn;

        // After Write
        // - If `NR12.3-7` are all clear, then disable the channel's DAC.
        // - If the channel's DAC is disabled, then disable the channel, too.
        mChannel1.mDAC.mEnable = ((mNR12.mValue & 0b11111000) != 0);
        if (mChannel1.mDAC.mEnable == false)
            { mNR52.mChannel1On = false; }

        return true;
    }

    auto APU::WriteNR13 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore if `NR52.7` is clear.
        if (mNR52.mMasterOn == false)
            { return true; }

        // Write
        // - All bits writable.
        mNR13 = pDataIn;
        return true;
    }

    auto APU::WriteNR14 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore if `NR52.7` is clear.
        // - Keep track of the previous state of the length timer's enable state.
        if (mNR52.mMasterOn == false)
            { return true; }
        bool wasLengthEnabled = mChannel1.mLengthEnable;

        // Write
        // - Bits 3 through 5 are unused; write `1`.
        // - Bits 0 through 3, 6 and 7 are writable.
        mNR14.mValue = 0b00111000 | (pDataIn & 0b11000111);

        // After Write
        // - Update the channel's length enable flag.
        // - If the length timer is enabled during the first half of the length
        //   period, then clock the length timer now.
        // - If clocking length here causes the length timer to reach zero, and
        //   the channel is not being triggered, then disable the channel.
        // - If `NR14.7` is now set, then trigger the channel.
        mChannel1.mLengthEnable = mNR14.mLengthEnable;
        if (wasLengthEnabled == false &&
            mChannel1.mLengthEnable == true &&
            (mFrameSequencerStep & 1) == 1)
        {
            if (--mChannel1.mLengthTimer == 0 && mNR14.mTrigger == false)
                { mNR52.mChannel1On = false; }
        }

        if (mNR14.mTrigger == true)
            { TriggerChannel(AudioChannel::Channel1); }

        return true;
    }

    auto APU::WriteNR21 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore if `NR52.7` is clear.
        if (mNR52.mMasterOn == false)
            { return true; }

        // Write
        // - All bits writable.
        mNR21.mValue = pDataIn;

        // After Write
        // - Reload length timer with `(64 - (NR21.0-5))`
        // - If `NR21.0-5` is zero, then load with 64 instead.
        mChannel2.mLengthTimer = (mNR21.mLength == 0) ? 
            64 : (64 - mNR21.mLength);
        return true;
    }

    auto APU::WriteNR22 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore if `NR52.7` is clear.
        // - Writes while the channel is active can affect its current volume.
        if (mNR52.mMasterOn == false)
            { return true; }
        
        EnvelopeRegister newNR22 = { .mValue = pDataIn };
        if (mNR52.mChannel2On == true)
        {
            if (mNR22.mPace == 0 && mChannel2.mEnvelopeRunning == true)
                { mChannel2.mVolume = (mChannel2.mVolume + 1) & 0xF; }
            else if (mNR22.mIsSweepingUpwards == false)
                { mChannel2.mVolume = (mChannel2.mVolume + 2) & 0xF; }

            if (mNR22.mIsSweepingUpwards != newNR22.mIsSweepingUpwards)
                { mChannel2.mVolume = (16 - mChannel2.mVolume) & 0xF; }
        }

        // Write
        // - All bits writable.
        mNR22.mValue = pDataIn;

        // After Write
        // - If `NR22.3-7` are all clear, then disable the channel's DAC.
        // - If the channel's DAC is disabled, then disable the channel, too.
        mChannel2.mDAC.mEnable = ((mNR22.mValue & 0b11111000) != 0);
        if (mChannel2.mDAC.mEnable == false)
            { mNR52.mChannel2On = false; }

        return true;
    }

    auto APU::WriteNR23 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore if `NR52.7` is clear.
        if (mNR52.mMasterOn == false)
            { return true; }

        // Write
        // - All bits writable.
        mNR23 = pDataIn;
        return true;
    }

    auto APU::WriteNR24 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore if `NR52.7` is clear.
        // - Keep track of the previous state of the length timer's enable state.
        if (mNR52.mMasterOn == false)
            { return true; }
        bool wasLengthEnabled = mChannel2.mLengthEnable;

        // Write
        // - Bits 3 through 5 are unused; write `1`.
        // - Bits 0 through 3, 6 and 7 are writable.
        mNR24.mValue = 0b00111000 | (pDataIn & 0b11000111);

        // After Write
        // - Update the channel's length enable flag.
        // - If the length timer is enabled during the first half of the length
        //   period, then clock the length timer now.
        // - If clocking length here causes the length timer to reach zero, and
        //   the channel is not being triggered, then disable the channel.
        // - If `NR24.7` is now set, then trigger the channel.
        mChannel2.mLengthEnable = mNR24.mLengthEnable;
        if (wasLengthEnabled == false &&
            mChannel2.mLengthEnable == true &&
            (mFrameSequencerStep & 1) == 1)
        {
            if (--mChannel2.mLengthTimer == 0 && mNR24.mTrigger == false)
                { mNR52.mChannel2On = false; }
        }

        if (mNR24.mTrigger == true)
            { TriggerChannel(AudioChannel::Channel2); }

        return true;
    }

    auto APU::WriteNR30 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore if `NR52.7` is clear.
        if (mNR52.mMasterOn == false)
            { return true; }

        // Write
        // - Bits 0 through 6 are unused; write `1`.
        // - Bit 7 is writable.
        mNR30.mValue = 0b01111111 | (pDataIn & 0b10000000);

        // After Write
        // - Enable or disable the channel's DAC based on `NR30.7`.
        // - If the DAC is disabled, then disable the channel.
        mChannel3.mDAC.mEnable = mNR30.mDacEnable;
        if (mChannel3.mDAC.mEnable == false)
            { mNR52.mChannel3On = false; }

        return true;
    }

    auto APU::WriteNR31 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore if `NR52.7` is clear.
        if (mNR52.mMasterOn == true)
        {
            // Write
            // - All bits writable.
            // - Note that, for the wave channel, all 8 bits make up the
            //   initial length timer.
            mNR31.mValue = pDataIn;
        }

        // After Write
        // - Even if the write is ignored, still reload the channel's internal
        //   length timer with the write value `(256 - (pDataIn.0-7))`.
        // - If the write value is zero, then reload with 256.
        mChannel3.mLengthTimer = (pDataIn == 0) ? 256 : (256 - pDataIn);

        return true;
    }

    auto APU::WriteNR32 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore if `NR52.7` is clear.
        if (mNR52.mMasterOn == false)
            { return true; }

        // Write
        // - Bits 0 through 4 and 7 are unused; write `1`.
        // - Bits 5 and 6 are writable.
        mNR32.mValue = 0b10011111 | (pDataIn & 0b00110000);
        return true;
    }

    auto APU::WriteNR33 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore if `NR52.7` is clear.
        if (mNR52.mMasterOn == false)
            { return true; }

        // Write
        // - All bits writable.
        mNR33 = pDataIn;
        return true;
    }

    auto APU::WriteNR34 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore if `NR52.7` is clear.
        // - Keep track of the previous state of the length timer's enable state.
        if (mNR52.mMasterOn == false)
            { return true; }
        bool wasLengthEnabled = mChannel3.mLengthEnable;

        // Write
        // - Bits 3 through 5 are unused; write `1`.
        // - Bits 0 through 3, 6 and 7 are writable.
        mNR34.mValue = 0b00111000 | (pDataIn & 0b11000111);

        // After Write
        // - Update the channel's length enable flag.
        // - If the length timer is enabled during the first half of the length
        //   period, then clock the length timer now.
        // - If clocking length here causes the length timer to reach zero, and
        //   the channel is not being triggered, then disable the channel.
        // - If `NR34.7` is now set, then trigger the channel.
        mChannel3.mLengthEnable = mNR34.mLengthEnable;
        if (wasLengthEnabled == false &&
            mChannel3.mLengthEnable == true &&
            (mFrameSequencerStep & 1) == 1)
        {
            if (--mChannel3.mLengthTimer == 0 && mNR34.mTrigger == false)
                { mNR52.mChannel3On = false; }
        }

        if (mNR34.mTrigger == true)
            { TriggerChannel(AudioChannel::Channel3); }

        return true;
    }

    auto APU::WriteNR41 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore if `NR52.7` is clear.
        if (mNR52.mMasterOn == false)
            { return true; }

        // Write
        // - All bits writable.
        mNR41.mValue = pDataIn;

        // After Write
        // - Reload length timer with `(64 - (NR41.0-5))`
        // - If `NR41.0-5` is zero, then load with 64 instead.
        mChannel4.mLengthTimer = (mNR41.mLength == 0) ? 
            64 : (64 - mNR41.mLength);
        return true;
    }

    auto APU::WriteNR42 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore if `NR52.7` is clear.
        // - Writes while the channel is active can affect its current volume.
        if (mNR52.mMasterOn == false)
            { return true; }
        
        EnvelopeRegister newNR42 = { .mValue = pDataIn };
        if (mNR52.mChannel4On == true)
        {
            if (mNR42.mPace == 0 && mChannel4.mEnvelopeRunning == true)
                { mChannel4.mVolume = (mChannel4.mVolume + 1) & 0xF; }
            else if (mNR42.mIsSweepingUpwards == false)
                { mChannel4.mVolume = (mChannel4.mVolume + 2) & 0xF; }

            if (mNR42.mIsSweepingUpwards != newNR42.mIsSweepingUpwards)
                { mChannel4.mVolume = (16 - mChannel4.mVolume) & 0xF; }
        }

        // Write
        // - All bits writable.
        mNR42.mValue = pDataIn;

        // After Write
        // - If `NR42.3-7` are all clear, then disable the channel's DAC.
        // - If the channel's DAC is disabled, then disable the channel, too.
        mChannel4.mDAC.mEnable = ((mNR42.mValue & 0b11111000) != 0);
        if (mChannel4.mDAC.mEnable == false)
            { mNR52.mChannel4On = false; }

        return true;
    }

    auto APU::WriteNR43 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore if `NR52.7` is clear.
        if (mNR52.mMasterOn == false)
            { return true; }

        // Write
        // - All bits writable.
        mNR43.mValue = pDataIn;
        return true;
    }

    auto APU::WriteNR44 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore if `NR52.7` is clear.
        // - Keep track of the previous state of the length timer's enable state.
        if (mNR52.mMasterOn == false)
            { return true; }
        bool wasLengthEnabled = mChannel4.mLengthEnable;

        // Write
        // - Bits 3 through 5 are unused; write `1`.
        // - Bits 0 through 3, 6 and 7 are writable.
        mNR44.mValue = 0b00111000 | (pDataIn & 0b11000111);

        // After Write
        // - Update the channel's length enable flag.
        // - If the length timer is enabled during the first half of the length
        //   period, then clock the length timer now.
        // - If clocking length here causes the length timer to reach zero, and
        //   the channel is not being triggered, then disable the channel.
        // - If `NR44.7` is now set, then trigger the channel.
        mChannel4.mLengthEnable = mNR44.mLengthEnable;
        if (wasLengthEnabled == false &&
            mChannel4.mLengthEnable == true &&
            (mFrameSequencerStep & 1) == 1)
        {
            if (--mChannel4.mLengthTimer == 0 && mNR44.mTrigger == false)
                { mNR52.mChannel4On = false; }
        }

        if (mNR44.mTrigger == true)
            { TriggerChannel(AudioChannel::Channel4); }

        return true;
    }

    auto APU::WriteNR50 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore if `NR52.7` is clear.
        if (mNR52.mMasterOn == false)
            { return true; }

        // Write
        // - All bits writable.
        mNR50.mValue = pDataIn;
        return true;
    }

    auto APU::WriteNR51 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore if `NR52.7` is clear.
        if (mNR52.mMasterOn == false)
            { return true; }

        // Write
        // - All bits writable.
        mNR51.mValue = pDataIn;
        return true;
    }

    auto APU::WriteNR52 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Get the old enable state (`NR52.7`).
        bool wasMasterOn = mNR52.mMasterOn;

        // Write
        // - Bits 0 through 3 are read-only; retain their old values.
        // - Bits 4 through 6 are unused; write `1`.
        // - Bit 7 is writable.
        mNR52.mValue =
            (mNR52.mValue & 0b00001111) |
            0b01110000 |
            (pDataIn & 0b10000000);

        // After
        // - If `NR52.7` was clear, and is now set, then reset the APU's timing
        //   counters. Reset the frame sequencer's counter according to the
        //   APU-DIV bit in the timer's system counter.
        // - If `NR52.7` is now clear, then disable all channels and clear all
        //   port registers, except for this one. In DMG Mode, preserve the
        //   channels' internal length timers.
        if (wasMasterOn == false && mNR52.mMasterOn == true)
        {
            mCycles = 0;
            mFrameSequencerStep =
                (mSystem.GetTimer().CheckSystemCounterBit(12, true) == true) ?
                    7 : 0;
        }
        else if (wasMasterOn == true && mNR52.mMasterOn == false)
        {
            mNR52.mValue    &= ~(0b00001111);    
            mNR10.mValue    = 0x00;
            mNR11.mValue    = 0x00;
            mNR12.mValue    = 0x00;
            mNR13           = 0x00;
            mNR14.mValue    = 0x00;
            mNR21.mValue    = 0x00;
            mNR22.mValue    = 0x00;
            mNR23           = 0x00;
            mNR24.mValue    = 0x00;
            mNR30.mValue    = 0x00;
            mNR31.mValue    = 0x00;
            mNR32.mValue    = 0x00;
            mNR33           = 0x00;
            mNR34.mValue    = 0x00;
            mNR41.mValue    = 0x00;
            mNR42.mValue    = 0x00;
            mNR43.mValue    = 0x00;
            mNR44.mValue    = 0x00;
            mNR50.mValue    = 0x00;
            mNR51.mValue    = 0x00;

            std::uint16_t   ch1Length = mChannel1.mLengthTimer,
                            ch2Length = mChannel2.mLengthTimer,
                            ch3Length = mChannel3.mLengthTimer,
                            ch4Length = mChannel4.mLengthTimer;
            mChannel1 = {};
            mChannel2 = {};
            mChannel3 = {};
            mChannel4 = {};

            if (mSystem.IsCGB() == false)
            {
                mChannel1.mLengthTimer = ch1Length;
                mChannel2.mLengthTimer = ch2Length;
                mChannel3.mLengthTimer = ch3Length;
                mChannel4.mLengthTimer = ch4Length;
            }
        }

        return true;
    }
}

// Public Methods - Accessors **************************************************

namespace G10::GB
{
    auto APU::SetSampleRate (const std::uint32_t pSampleRate) -> void
    {
        mSampleRate = 
            stx::clamp(pSampleRate, kMinimumSampleRate, kMaximumSampleRate);
        CalculateCyclesPerSample();
    }
}

// Private Methods - Helpers ***************************************************

namespace G10::GB
{
    auto APU::CalculatePulseSweep () -> std::uint16_t
    {
        // Calculate the shifted sweep shadow, then the new period based on the
        // sweep direction.
        std::uint16_t 
            shifted = (mChannel1.mSweepShadow >> mNR10.mIndividualStep),
            newPeriod = (mNR10.mIsSweepingDownwards == true) ?
                (mChannel1.mSweepShadow - shifted) :
                (mChannel1.mSweepShadow + shifted);
        if (mNR10.mIsSweepingDownwards == true)
            { mChannel1.mSweepNegate = true; }

        // If the new period overflows (exceeds 2047), disable the channel.
        if (newPeriod > 0x7FF)
            { mNR52.mChannel1On = false; }

        return newPeriod;
    }

    auto APU::CalculateCyclesPerSample () -> std::uint64_t
    {
        mCyclesPerSample = 4194304 / mSampleRate;
        return mCyclesPerSample;
    }

    auto APU::DischargeCapacitor (DigitalAudioConverter& pDAC) -> void
    {
        pDAC.mCharge = std::fmaxf(0.0f, pDAC.mCharge - kCapacitorDischargeRate);
    }

    auto APU::ChargeCapacitor (DigitalAudioConverter& pDAC) -> void
    {
        pDAC.mCharge = std::fminf(1.0f, pDAC.mCharge + kCapacitorChargeRate);
    }

    auto APU::UpdateCapacitorCharge (DigitalAudioConverter& pDAC) -> void
    {
        if (pDAC.mEnable == true)   { ChargeCapacitor(pDAC); }
        else                        { DischargeCapacitor(pDAC); }
    }

    auto APU::GenerateSample () -> void
    {
        // If the APU is disabled, then discharge the channels' DACS and output
        // silence.
        if (mNR52.mMasterOn == false)
        {
            DischargeCapacitor(mChannel1.mDAC);
            DischargeCapacitor(mChannel2.mDAC);
            DischargeCapacitor(mChannel3.mDAC);
            DischargeCapacitor(mChannel4.mDAC);
            if (mSampleCallback != nullptr) 
                { mSampleCallback(mSystem, 0.0f, 0.0f); }
            if (mRawSampleCallback != nullptr)
                { mRawSampleCallback(mSystem, 0, 0); }
        
            return;
        }

        // Charge or discharge each channel's DAC, based on their respective
        // enable status.
        UpdateCapacitorCharge(mChannel1.mDAC);
        UpdateCapacitorCharge(mChannel2.mDAC);
        UpdateCapacitorCharge(mChannel3.mDAC);
        UpdateCapacitorCharge(mChannel4.mDAC);

        // Get the digital outputs of our channels' DACs. Apply a smooth-stepped
        // interpolation of the current charges in their capacitors to those
        // outputs.
        //
        // After that, translate these digital outputs into analog outputs.
        static constexpr auto kSmoothStep = [] (const float pIn) -> float
            { return (3.0f * pIn * pIn) - (2.0f * pIn * pIn * pIn); };
        float
            digitalCh1 = static_cast<float>(mChannel1.mDAC.mDigital) * kSmoothStep(mChannel1.mDAC.mCharge),
            digitalCh2 = static_cast<float>(mChannel2.mDAC.mDigital) * kSmoothStep(mChannel2.mDAC.mCharge),
            digitalCh3 = static_cast<float>(mChannel3.mDAC.mDigital) * kSmoothStep(mChannel3.mDAC.mCharge),
            digitalCh4 = static_cast<float>(mChannel4.mDAC.mDigital) * kSmoothStep(mChannel4.mDAC.mCharge),
            analogCh1  = -((digitalCh1 / 7.5f) - 1.0f),
            analogCh2  = -((digitalCh2 / 7.5f) - 1.0f),
            analogCh3  = -((digitalCh3 / 7.5f) - 1.0f),
            analogCh4  = -((digitalCh4 / 7.5f) - 1.0f);

        // Mix the channels appropriately, based on the current panning settings.
        float leftSample = 0.0f, rightSample = 0.0f;
        if (mNR52.mChannel1On == true)
        {
            if (mNR51.mChannel1Left == true) { leftSample += analogCh1; }
            if (mNR51.mChannel1Right == true) { rightSample += analogCh1; }
        }
        if (mNR52.mChannel2On == true)
        {
            if (mNR51.mChannel2Left == true) { leftSample += analogCh2; }
            if (mNR51.mChannel2Right == true) { rightSample += analogCh2; }
        }
        if (mNR52.mChannel3On == true)
        {
            if (mNR51.mChannel3Left == true) { leftSample += analogCh3; }
            if (mNR51.mChannel3Right == true) { rightSample += analogCh3; }
        }
        if (mNR52.mChannel4On == true)
        {
            if (mNR51.mChannel4Left == true) { leftSample += analogCh4; }
            if (mNR51.mChannel4Right == true) { rightSample += analogCh4; }
        }

        // Apply the master volume to these channels.
        float
            leftVolume = static_cast<float>(mNR50.mVolumeLeft + 1) / 8.0f,
            rightVolume = static_cast<float>(mNR50.mVolumeRight + 1) / 8.0f;
        leftSample *= leftVolume;
        rightSample *= rightVolume;

        // Apply the high pass filter.
        float highPassAlpha = (mSystem.IsCGB() == true) ?
            kHighPassAlphaCGB :
            kHighPassAlphaDMG;
        float 
            filteredLeft = leftSample - mHighPassLeft,
            filteredRight = rightSample - mHighPassRight;
        mHighPassLeft = leftSample - (filteredLeft * highPassAlpha);
        mHighPassRight = rightSample - (filteredRight * highPassAlpha);
        leftSample = filteredLeft;
        rightSample = filteredRight;

        // Derive the final sample by dividing by the channel count.
        leftSample /= 4.0f;
        rightSample /= 4.0f;

        // Send this sample to our callbacks.
        if (mSampleCallback != nullptr)
            { mSampleCallback(mSystem, leftSample, rightSample); }
        if (mRawSampleCallback != nullptr)
        {
            mRawSampleCallback(mSystem,
                static_cast<std::int16_t>(leftSample * 32767.0f),
                static_cast<std::int16_t>(rightSample * 32767.0f)
            );
        }
    }
}

// Private Methods - Channel Triggering & Initialization ***********************

namespace G10::GB
{
    auto APU::TriggerChannel (AudioChannel pChannel) -> void
    {
        switch (pChannel)
        {
            case AudioChannel::Channel1:
            {
                // If this is the first time the channel has been triggered
                // since the APU was last powered on, then suppress the first
                // wave sample.
                if (mChannel1.mTriggeredSincePowerOn == false)
                {
                    mChannel1.mFirstSampleSupressed = true;
                    mChannel1.mTriggeredSincePowerOn = true;
                }

                // If, at the time of trigger, the channel's length timer had
                // lapsed, then reload it with its maximum value (`64`).
                // - If the trigger happens during the first half of the channel's
                //   length period, and the length timer is currently enabled,
                //   then also clock the length timer here.
                if (mChannel1.mLengthTimer == 0)
                {
                    mChannel1.mLengthTimer = (
                        mNR14.mLengthEnable == true &&
                        (mFrameSequencerStep & 1) != 0
                    ) ? 63 : 64;
                }

                // Enable the channel if its DAC is enabled.
                // - If the DAC is disabled, then we're done here.
                if (mChannel1.mDAC.mEnable == true)
                    { mNR52.mChannel1On = true; }
                else { return; }

                // Reload the channel's volume and envelope timer.
                // - The timer is reloaded with the envelope's pace. If the pace
                //   is zero, then the timer is reloaded with `8`.
                // - If the next tick of the frame sequencer clocks this envelope,
                //   then add one more tick.
                mChannel1.mVolume = mNR12.mInitialVolume;
                mChannel1.mEnvelopeTimer = (mNR12.mPace == 0) ? 8 : mNR12.mPace;
                mChannel1.mEnvelopeRunning = true;
                if ((mFrameSequencerStep & 7) == 7)
                    { mChannel1.mEnvelopeTimer++; }

                // Initialize the period timer; preserve the lower two bits.
                std::uint16_t newPeriod = (
                    (static_cast<std::uint16_t>(mNR14.mPeriodHigh) << 8) | mNR13);
                std::uint16_t newPeriodTimer = 2048 - newPeriod;
                mChannel1.mPeriodTimer = (newPeriodTimer & ~0x3) |
                    (mChannel1.mPeriodTimer & 0x3);

                // Initialize the sweep unit.
                mChannel1.mSweepShadow = newPeriod;
                mChannel1.mSweepTimer = (mNR10.mPace == 0) ? 8 : mNR10.mPace;
                mChannel1.mSweepNegate = false;
                mChannel1.mSweepEnabled = (mNR10.mPace != 0 || mNR10.mIndividualStep != 0);
                if (mNR10.mIndividualStep != 0)
                    { CalculatePulseSweep(); }
            }   break;
            case AudioChannel::Channel2:
            {
                // Same thing as `AudioChannel::Channel1`, but without the
                // frequency sweep unit.
                if (mChannel2.mTriggeredSincePowerOn == false)
                {
                    mChannel2.mFirstSampleSupressed = true;
                    mChannel2.mTriggeredSincePowerOn = true;
                }

                if (mChannel2.mLengthTimer == 0)
                {
                    mChannel2.mLengthTimer = (
                        mNR24.mLengthEnable == true &&
                        (mFrameSequencerStep & 1) != 0
                    ) ? 63 : 64;
                }

                if (mChannel2.mDAC.mEnable == true)
                    { mNR52.mChannel2On = true; }
                else { return; }

                mChannel2.mVolume = mNR22.mInitialVolume;
                mChannel2.mEnvelopeTimer = (mNR22.mPace == 0) ? 8 : mNR22.mPace;
                mChannel2.mEnvelopeRunning = true;
                if ((mFrameSequencerStep & 7) == 7)
                    { mChannel2.mEnvelopeTimer++; }

                std::uint16_t newPeriodTimer = 2048 - (
                    (static_cast<std::uint16_t>(mNR24.mPeriodHigh) << 8) | mNR23);
                mChannel2.mPeriodTimer = (newPeriodTimer & ~0x3) |
                    (mChannel2.mPeriodTimer & 0x3);

            }   break;
            case AudioChannel::Channel3:
            {
                // Set up the length timer.
                if (mChannel3.mLengthTimer == 0)
                {
                    mChannel3.mLengthTimer = (
                        mNR34.mLengthEnable == true &&
                        (mFrameSequencerStep & 1) != 0
                    ) ? 255 : 256;
                }

                // Enable if DAC is enabled.
                if (mChannel3.mDAC.mEnable == true)
                    { mNR52.mChannel3On = true; }
                else { return; }

                // Reset position counter and reload period timer.
                // - The wave channel's period timer is reloaded with an
                //   additional 6-clock delay.
                std::uint16_t newPeriodTimer = 2048 - (
                    (static_cast<std::uint16_t>(mNR34.mPeriodHigh) << 8) | mNR33);
                mChannel3.mPeriodTimer = newPeriodTimer + 6;

                // Triggering this channel does not reload the last wave byte.
            }   break;
            case AudioChannel::Channel4:
            {
                // Set up the length timer.
                if (mChannel4.mLengthTimer == 0)
                {
                    mChannel4.mLengthTimer = (
                        mNR44.mLengthEnable == true &&
                        (mFrameSequencerStep & 1) != 0
                    ) ? 63 : 64;
                }

                // Enable if DAC is enabled.
                if (mChannel4.mDAC.mEnable == true)
                    { mNR52.mChannel4On = true; }
                else { return; }

                // Reload the channel's volume and envelope.
                mChannel4.mVolume = mNR42.mInitialVolume;
                mChannel4.mEnvelopeTimer = (mNR42.mPace == 0) ? 8 : mNR42.mPace;
                mChannel4.mEnvelopeRunning = true;
                if ((mFrameSequencerStep & 7) == 7)
                    { mChannel4.mEnvelopeTimer++; }

                // Initialize period timer and reset LFSR.
                std::uint16_t divisor = (mNR43.mClockDivider == 0) ? 8 :
                    mNR43.mClockDivider * 16;
                mChannel4.mPeriodTimer = (divisor << mNR43.mClockShift);
                mChannel4.mLFSR = 0;

            }   break;
        }
    }

    auto APU::InitializeChannel (AudioChannel pChannel) -> void
    {
        switch (pChannel)
        {
            case AudioChannel::Channel1:
            {
                mChannel1 = {};
                mChannel1.mDAC.mEnable      = ((mNR12.mValue & 0xF8) != 0);
                mChannel1.mLengthEnable     = (mNR14.mLengthEnable != 0);
                mChannel1.mLengthTimer      = 64 - (mNR11.mLength & 0x3F);
                mChannel1.mVolume           = mNR12.mInitialVolume;
            }   break;
            case AudioChannel::Channel2:
            {
                mChannel2 = {};
                mChannel2.mDAC.mEnable      = ((mNR22.mValue & 0xF8) != 0);
                mChannel2.mLengthEnable     = (mNR24.mLengthEnable != 0);
                mChannel2.mLengthTimer      = 64 - (mNR21.mLength & 0x3F);
                mChannel2.mVolume           = mNR22.mInitialVolume;
            }   break;
            case AudioChannel::Channel3:
            {
                mChannel3 = {};
                mChannel3.mDAC.mEnable      = (mNR30.mDacEnable != 0);
                mChannel3.mLengthEnable     = (mNR34.mLengthEnable != 0);
                mChannel3.mLengthTimer      = 256 - (mNR31.mValue & 0xFF);
            }   break;
            case AudioChannel::Channel4:
            {
                mChannel4 = {};
                mChannel4.mDAC.mEnable      = ((mNR42.mValue & 0xF8) != 0);
                mChannel4.mLengthEnable     = (mNR44.mLengthEnable != 0);
                mChannel4.mLengthTimer      = 64 - (mNR41.mLength & 0x3F);
                mChannel4.mVolume           = mNR42.mInitialVolume;
                mChannel4.mLFSR             = 0x7FFF;
            }   break;
        }
    }
}

// Private Methods - Period Dividers *******************************************

namespace G10::GB
{
    auto APU::ClockPulseChannels () -> void
    {
        auto clock = [this] (
            PulseChannel& pChannel, 
            LengthRegister& pNRX1, 
            std::uint8_t& pNRX3,
            ChannelControlRegister& pNRX4)
        {
            // Decrement period timer.
            // - If zero, then advance duty position and reload.
            if (pChannel.mPeriodTimer > 0) { pChannel.mPeriodTimer--; }
            if (pChannel.mPeriodTimer == 0)
            {
                pChannel.mPeriodTimer = 2048 - (
                    (static_cast<std::uint16_t>(pNRX4.mPeriodHigh) << 8) | 
                    pNRX3);
                pChannel.mDutyPosition = (pChannel.mDutyPosition + 1) & 7;
            }

            // Update digital output based on duty pattern and volume.
            // - Note: First sample after power-on trigger is suppressed in CGB
            //   Mode.
            if (pChannel.mFirstSampleSupressed == true)
            {
                pChannel.mDAC.mDigital = 0;
                pChannel.mFirstSampleSupressed = false;
            }
            else
            {
                std::uint8_t
                    duty = (pNRX1.mDutyCycle & 0x03),
                    pattern = kWaveDutyPatterns[duty],
                    bit = (pattern >> (7 - pChannel.mDutyPosition)) & 1;

                pChannel.mDAC.mDigital = bit ? pChannel.mVolume : 0;
            }
        };

        if (mNR52.mChannel1On == true && mChannel1.mDAC.mEnable == true)
            { clock(mChannel1, mNR11, mNR13, mNR14); }
        else
            { mChannel1.mDAC.mDigital = 0; }

        if (mNR52.mChannel2On == true && mChannel2.mDAC.mEnable == true)
            { clock(mChannel2, mNR21, mNR23, mNR24); }
        else
            { mChannel2.mDAC.mDigital = 0; }
    }

    auto APU::ClockWaveChannel () -> void
    {
        if (mNR52.mChannel3On == false || mChannel3.mDAC.mEnable == false)
        {
            mChannel3.mDAC.mDigital = 0;
            return;
        }

        // Decrement period timer.
        // - If zero, then advance position and reload.
        if (mChannel3.mPeriodTimer > 0) { mChannel3.mPeriodTimer--; }
        if (mChannel3.mPeriodTimer == 0)
        {
            mChannel3.mPeriodTimer = 2048 - (
                (static_cast<std::uint16_t>(mNR34.mPeriodHigh) << 8) | 
                mNR33);
            mChannel3.mWavePosition = (mChannel3.mWavePosition + 1) & 0x1F;
            mChannel3.mLastWaveByte = mWaveRAM[mChannel3.mWavePosition >> 1];
        }

        // Get the appropriate sample nibble.
        std::uint8_t sample = ((mChannel3.mWavePosition & 1) == 0) ?
            (mChannel3.mLastWaveByte >> 4) & 0x0F :
            mChannel3.mLastWaveByte & 0x0F;

        // Apply the channel's coarse volume.
        switch (static_cast<WaveOutputLevel>(mNR32.mOutputLevel))
        {
            case WaveOutputLevel::Mute:
                sample = 0;
                break;
            case WaveOutputLevel::Full:
                break;
            case WaveOutputLevel::Half:
                sample >>= 1;
                break;
            case WaveOutputLevel::Quarter:
                sample >>= 2;
                break;
        }

        // Update digital output based on the calculated sample.
        mChannel3.mDAC.mDigital = sample;
    }

    auto APU::ClockNoiseChannel () -> void
    {
        if (mNR52.mChannel4On == false || mChannel4.mDAC.mEnable == false)
        {
            mChannel4.mDAC.mDigital = 0;
            return;
        }
        
        // If the clock shift value is `>= 14`, then the LFSR does not clock,
        // but the channel still outputs noise according to the current LFSR
        // state.
        if (mNR43.mClockShift >= 14)
        {
            mChannel4.mDAC.mDigital = (mChannel4.mLFSR & 1) ? mChannel4.mVolume : 0;
            return;
        }

        // Decrement period timer.
        // - Once zero, clock LFSR and reload.
        if (mChannel4.mPeriodTimer > 0) { mChannel4.mPeriodTimer--; }
        if (mChannel4.mPeriodTimer == 0)
        {
            // Reload period timer.
            std::uint16_t divisor = (mNR43.mClockDivider == 0) ? 8 :
                (mNR43.mClockDivider * 16);
            mChannel4.mPeriodTimer = (divisor << mNR43.mClockShift);

            // Clock the LFSR.
            // - Calculate the XNOR of bits 0 and 1 - that is, take the XOR of
            //   these bits, then compliment the result.
            // - Shift the LFSR right by one bit.
            // - Set or clear bit 14 of the new LFSR according to the new bit.
            // - If `NR43.3` is set, then also set/clear bit 6.
            std::uint8_t
                bit0 = (mChannel4.mLFSR & 1),
                bit1 = (mChannel4.mLFSR & 2),
                newBit = !(bit0 ^ bit1);
            if (newBit != 0)
            {
                mChannel4.mLFSR |= 0x4000;
                if (mNR43.mIsShortWidth == true)
                    { mChannel4.mLFSR |= 0x0040; }
            }
            else
            {
                mChannel4.mLFSR &= ~0x4000;
                if (mNR43.mIsShortWidth == true)
                    { mChannel4.mLFSR &= ~0x0040; }
            }

            // Set output according to the new LFSR state.
            mChannel4.mDAC.mDigital = (mChannel4.mLFSR & 1) ? 0 : mChannel4.mVolume;
        }
    }
}

// Private Methods - Frame Sequencer *******************************************

namespace G10::GB
{
    auto APU::ClockFrameSequencer () -> void
    {
        if ((mFrameSequencerStep & 1) == 0)
            { ClockLengthTimers(); }
        if (mFrameSequencerStep == 2 || mFrameSequencerStep == 6)
            { ClockPulseSweepUnit(); }
        if (mFrameSequencerStep == 7)
            { ClockEnvelopeSweepUnits(); }

        mFrameSequencerStep = (mFrameSequencerStep + 1) & 7;
    }

    auto APU::ClockLengthTimers () -> void
    {
        auto clock = [this] (
            std::uint16_t& pLengthTimer,
            bool& pLengthEnable
        ) -> bool
        {
            if (pLengthEnable == true && pLengthTimer > 0)
            {
                if (--pLengthTimer == 0)
                    { return false; }
            }

            return true;
        };

        if (clock(mChannel1.mLengthTimer, mChannel1.mLengthEnable) == false)
            { mNR52.mChannel1On = false; }
        if (clock(mChannel2.mLengthTimer, mChannel2.mLengthEnable) == false)
            { mNR52.mChannel2On = false; }
        if (clock(mChannel3.mLengthTimer, mChannel3.mLengthEnable) == false)
            { mNR52.mChannel3On = false; }
        if (clock(mChannel4.mLengthTimer, mChannel4.mLengthEnable) == false)
            { mNR52.mChannel4On = false; }
    }

    auto APU::ClockPulseSweepUnit () -> void
    {
        if (mChannel1.mSweepTimer > 0) { mChannel1.mSweepTimer--; }
        if (mChannel1.mSweepTimer == 0)
        {
            mChannel1.mSweepTimer = (mNR10.mPace == 0) ? 8 : mNR10.mPace;
            if (mChannel1.mSweepEnabled && mNR10.mPace != 0)
            {
                auto newFreq = CalculatePulseSweep();
                if (newFreq <= 0x7FF && mNR10.mIndividualStep != 0)
                {
                    mChannel1.mSweepShadow = newFreq;
                    mNR13 = (newFreq & 0xFF);
                    mNR14.mPeriodHigh = (newFreq >> 8) & 0x07;
                    CalculatePulseSweep();
                }
            }
        }
    }

    auto APU::ClockEnvelopeSweepUnits () -> void
    {
        auto clockPulse = [this] (
            PulseChannel& pChannel,
            EnvelopeRegister& mNRX2
        )
        {
            if (pChannel.mEnvelopeRunning == true)
            {
                if (pChannel.mEnvelopeTimer > 0) { pChannel.mEnvelopeTimer--; }
                if (pChannel.mEnvelopeTimer == 0)
                {
                    pChannel.mEnvelopeTimer = (mNRX2.mPace == 0) ? 8 : mNRX2.mPace;
                    if (mNRX2.mPace != 0)
                    {
                        if (mNRX2.mIsSweepingUpwards == true)
                        {
                            if (pChannel.mVolume < 15)
                                { pChannel.mVolume++; }
                            else
                                { pChannel.mEnvelopeRunning = false; }
                        }
                        else
                        {
                            if (pChannel.mVolume > 0)
                                { pChannel.mVolume--; }
                            else
                                { pChannel.mEnvelopeRunning = false; }
                        }
                    }
                }
            }
        };

        clockPulse(mChannel1, mNR12);
        clockPulse(mChannel2, mNR22);
        if (mChannel4.mEnvelopeRunning == true)
        {
            if (mChannel4.mEnvelopeTimer > 0) { mChannel4.mEnvelopeTimer--; }
            if (mChannel4.mEnvelopeTimer == 0)
            {
                mChannel4.mEnvelopeTimer = (mNR42.mPace == 0) ? 8 : mNR42.mPace;
                if (mNR42.mPace != 0)
                {
                    if (mNR42.mIsSweepingUpwards == true)
                    {
                        if (mChannel4.mVolume < 15)
                            { mChannel4.mVolume++; }
                        else
                            { mChannel4.mEnvelopeRunning = false; }
                    }
                    else
                    {
                        if (mChannel4.mVolume > 0)
                            { mChannel4.mVolume--; }
                        else
                            { mChannel4.mEnvelopeRunning = false; }
                    }
                }
            }
        }
    }
}
