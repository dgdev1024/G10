/**
 * @file    G10.GB.PPU.cpp
 * @brief   Contains implementations for the G10.Boy's picture processing unit
 *          (PPU) component, and related definitions.
 */

// Includes ********************************************************************

#include <G10.GB.System.hpp>

// Public - Constructors & Destructor ******************************************

namespace G10::GB
{
    PPU::PPU (System& pSystem) :
        mSystem     { pSystem }
    {
    }
}

// Public Methods **************************************************************

namespace G10::GB
{
    auto PPU::Reset () -> void
    {
        // Initialize Memory Buffers
        std::ranges::fill(mVideoRAM0, 0x00);
        std::ranges::fill(mVideoRAM1, 0x00);
        std::ranges::fill(mOAM, 0x00);
        std::ranges::fill(mBgColorRAM, 0x00);
        std::ranges::fill(mObjColorRAM, 0x00);
        std::ranges::fill(mFrameBuffer, 0xFF);
        std::ranges::copy(kDefaultMonochromeColors, mMonochromeColors.begin());

        // Initialize Port Registers
        mDisplayControl.mValue = 0x91;
        mDisplayStatus.mValue = 0x85;
        mViewportY = mViewportX = 0x00;
        mWindowY = mWindowX = 0x00;
        mCurrentScanline = mScanlineCompare = 0x00;
        mBgMonochromePalette.mValue = 0xFC;
        mObjMonochromePalette0.mValue = 0xFF;
        mObjMonochromePalette1.mValue = 0xFF;
        if (mSystem.IsCGB() == true)
        {
            mOamDmaSourceAddress = 0x00000000;
            mVramDmaSourceAddress = 0xFFFFFFFF;
            mVramDmaDestinationAddress = 0xFFFFFFFF;
            mBgColorPaletteIndex.mValue = 0x00;
            mObjColorPaletteIndex.mValue = 0x00;
            mPPUAuxillarySettings.mValue = 0x04;
        }
        else
        {
            mOamDmaSourceAddress = 0x000000FF;
            mVramDmaSourceAddress = 0xFFFFFFFF;
            mVramDmaDestinationAddress = 0xFFFFFFFF;
            mBgColorPaletteIndex.mValue = 0x00;
            mObjColorPaletteIndex.mValue = 0x00;
            mPPUAuxillarySettings.mValue = 0x00;
        }

        // Initialize Internal State
        // - Dot Counters.
        mModeDots = mLineDots = mFrameDots = 0;

        // - Object Scan
        std::ranges::fill(mScanlineObjectIndices, 0x00);
        mScanlineObjectCount = 0;

        // - Pixel Transfer
        mPixelFetcher.Reset();
        mBgPixelFIFO.Clear();
        mObjPixelFIFO.Clear();

        // - Window Layer
        mWindowScanline = 0;
        mWindowVisible = mWindowTriggered = mWYConditionMet = false;
        
        // - Direct Memory Access
        mOamDmaActive = false;
        mOamDmaRestarting = false;
        mOamDmaSource = 0;
        mOamDmaBytesTransferred = 0;
        mOamDmaDelay = 0;
        mOamDmaDotCounter = 0;
        mVramDmaActive = false;
        mVramDmaSource = 0;
        mVramDmaDestination = 0;
        mVramDmaBlocksLeft = 0;

        // - Other Internal States
        mSpeedSwitchDM = DisplayMode::HorizontalBlank;
        mFrameJustFinished = false;
        mLcdJustEnabled = false;
        mFirstLineMode3 = false;
        mLyIncrementedEarly = false;
        mVramReadBlocked = false;   
    }

    auto PPU::Clock (const std::uint64_t& pCycle) -> bool
    {
        // Component Clock Rate: 
        // - 1 Dot per 2 CPU Cycles in Normal Speed
        // - 1 Dot per 4 CPU Cycles in High Speed
        bool isHighSpeed = mSystem.GetCPU().IsHighSpeed();
        if (isHighSpeed == true)
        // {
        //     if ((pCycle & 3) != 3)
        //         { return true; }
        // }
        // else
        {
            if ((pCycle & 1) != 1)
                { return true; }
        }

        // Tick the OAM DMA every M-cycle:
        // - 2 Dots in Normal Speed
        // - 1 Dot in High Speed
        mOamDmaDotCounter++;
        if (isHighSpeed == true || (mOamDmaDotCounter & 1) == 0)
            { mOamDmaDotCounter = 0; TickOamDMA(); }

        // If the LCD is off, then simulate one full frame of time spent.
        if (mDisplayControl.mLcdEnable == false)
        {
            if (++mFrameDots >= kDotsPerFrame)
            {
                mFrameDots = 0;
                mFrameJustFinished = true;
                if (mFrameCallback != nullptr)
                    { mFrameCallback(mSystem, mFrameBuffer, false); }
            }
            else { mFrameJustFinished = false; }

            return true;
        }

        // Increment our frame, mode and line dot counters, then tick the
        // appropriate display mode.
        mModeDots++;
        mLineDots++;
        mFrameDots++;
        switch (static_cast<DisplayMode>(mDisplayStatus.mDisplayMode))
        {
            case DisplayMode::HorizontalBlank:  TickHorizontalBlank(); break;
            case DisplayMode::VerticalBlank:    TickVerticalBlank(); break;
            case DisplayMode::ObjectScan:       TickObjectScan(); break;
            case DisplayMode::PixelTransfer:    TickPixelTransfer(); break;
        }

        return true;
    }
}

// Public Methods - Callbacks **************************************************

namespace G10::GB
{
    auto PPU::SetFrameCallback (const FrameCallback& pCallback) -> void
        { mFrameCallback = pCallback; }
    auto PPU::SetScanlineDelegate (const ScanlineDelegate& pDelegate) -> void
        { mScanlineDelegate = pDelegate; }
    auto PPU::SetCoincidenceDelegate (const CoincidenceDelegate& pDelegate) -> void
        { mCoincidenceDelegate = pDelegate; }
}

// Public Methods - Bus Access *************************************************

namespace G10::GB
{
    auto PPU::ReadVideoRAM (std::uint32_t pRelAddress, std::uint8_t& pDataOut) -> bool
    {
        bool restricted = 
            mSystem.mNoRestrict == false &&
            mDisplayControl.mLcdEnable == true && (
                mDisplayStatus.mDisplayMode == stx::under(DisplayMode::PixelTransfer) ||
                mVramReadBlocked == true
            );
        if (restricted == false)
            { pDataOut = mVideoRAM[pRelAddress & 0x1FFF]; }
        else
            { pDataOut = 0xFF; }

        return true;
    }

    auto PPU::ReadOAM (std::uint32_t pRelAddress, std::uint8_t& pDataOut) -> bool
    {
        bool restricted =
            mSystem.mNoRestrict == false &&
            mDisplayControl.mLcdEnable == true && (
                mDisplayStatus.mDisplayMode == stx::under(DisplayMode::PixelTransfer) ||
                mDisplayStatus.mDisplayMode == stx::under(DisplayMode::ObjectScan) ||
                mLyIncrementedEarly == true
            );
        if (restricted == false)
            { pDataOut = mOAM[pRelAddress % kOamSize]; }
        else
            { pDataOut = 0xFF; }

        return true;
    }

    auto PPU::ReadColorRAM (std::uint32_t pRelAddress, std::uint8_t& pDataOut) -> bool
    {
        // Direct color RAM reads are only available in CGB Mode.
        if (mSystem.IsCGB() == false)
        {
            pDataOut = 0xFF;
            return true;
        }

        // Direct color RAM reads are restricted during the `PixelTransfer`
        // mode when `LCDC.7` is set.
        bool restricted =
            mSystem.mNoRestrict == false &&
            mDisplayControl.mLcdEnable == true &&
            mDisplayStatus.mDisplayMode == stx::under(DisplayMode::PixelTransfer);
        if (restricted == true)
        {
            pDataOut = 0xFF;
            return true;
        }

        // Read from the appropriate color RAM.
        // - If `pRelAddress < 0x40`, then read from background color RAM.
        // - If `pRelAddress < 0x80`, then read from object color RAM.
        // - Otherwise, read open-bus.
        if (pRelAddress < kColorRamSize)
            { pDataOut = mBgColorRAM[pRelAddress]; }
        else if (pRelAddress < (kColorRamSize * 2))
            { pDataOut = mObjColorRAM[pRelAddress - kColorRamSize]; }
        else
            { pDataOut = 0xFF; }

        return true;
    }

    auto PPU::WriteVideoRAM (std::uint32_t pRelAddress, std::uint8_t pDataIn) -> bool
    {
        bool restricted = 
            mSystem.mNoRestrict == false &&
            mDisplayControl.mLcdEnable == true && (
                mDisplayStatus.mDisplayMode == stx::under(DisplayMode::PixelTransfer) ||
                mVramReadBlocked == true
            );
        if (restricted == false)
        { 
            mVideoRAM[pRelAddress & 0x1FFF] = pDataIn; 
        }

        return true;
    }

    auto PPU::WriteOAM (std::uint32_t pRelAddress, std::uint8_t pDataIn) -> bool
    {
        bool restricted =
            mSystem.mNoRestrict == false &&
            mDisplayControl.mLcdEnable == true && (
                mDisplayStatus.mDisplayMode == stx::under(DisplayMode::PixelTransfer) ||
                mDisplayStatus.mDisplayMode == stx::under(DisplayMode::ObjectScan) ||
                mLyIncrementedEarly == true
            );
        if (restricted == false)
            { mOAM[pRelAddress % kOamSize] = pDataIn; }

        return true;
    }

    auto PPU::WriteColorRAM (std::uint32_t pRelAddress, std::uint8_t pDataIn) -> bool
    {
        // Direct color RAM writes are only available in CGB Mode.
        if (mSystem.IsCGB() == false)
            { return true; }

        // Direct color RAM writes are restricted during the `PixelTransfer`
        // mode when `LCDC.7` is set.
        bool restricted =
            mSystem.mNoRestrict == false &&
            mDisplayControl.mLcdEnable == true &&
            mDisplayStatus.mDisplayMode == stx::under(DisplayMode::PixelTransfer);
        if (restricted == true)
            { return true; }

        // Write to the appropriate color RAM.
        // - If `pRelAddress < 0x40`, then write to background color RAM.
        // - If `pRelAddress < 0x80`, then write to object color RAM.
        // - Otherwise, ignore the write.
        if (pRelAddress < kColorRamSize)
            { mBgColorRAM[pRelAddress] = pDataIn; }
        else if (pRelAddress < (kColorRamSize * 2))
            { mObjColorRAM[pRelAddress - kColorRamSize] = pDataIn; }

        return true;
    }
}

// Public Methods - Port Registers *********************************************

namespace G10::GB
{
    auto PPU::ReadLCDC (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        pDataOut = mDisplayControl.mValue;
        return true;
    }

    auto PPU::ReadSTAT (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - Bit 7 is unused; read `1`.
        // - Bits 0 through 6 are readable.
        pDataOut =
            0b10000000 |
            (mDisplayStatus.mValue & 0b01111111);
        return true;
    }

    auto PPU::ReadSCY (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        pDataOut = mViewportY;
        return true;
    }

    auto PPU::ReadSCX (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        pDataOut = mViewportX;
        return true;
    }

    auto PPU::ReadWY (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        pDataOut = mWindowY;
        return true;
    }

    auto PPU::ReadWX (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        pDataOut = mWindowX;
        return true;
    }

    auto PPU::ReadLY (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        pDataOut = mCurrentScanline;
        return true;
    }

    auto PPU::ReadLYC (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        pDataOut = mScanlineCompare;
        return true;
    }

    auto PPU::ReadBGP (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        pDataOut = mBgMonochromePalette.mValue;
        return true;
    }

    auto PPU::ReadOBP0 (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        pDataOut = mObjMonochromePalette0.mValue;
        return true;
    }

    auto PPU::ReadOBP1 (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        pDataOut = mObjMonochromePalette1.mValue;
        return true;
    }

    auto PPU::ReadBGPI (std::uint8_t& pDataOut) -> bool
    {
        // Before Read
        // - Open-bus in DMG Mode.
        if (mSystem.IsCGB() == false)
            { pDataOut = 0xFF; return true; }

        // Read
        // - Bit 6 is unused; read `1`.
        // - Bits 0 through 5 and 7 are readable.
        pDataOut =
            0b01000000 |
            (mBgColorPaletteIndex.mValue & 0b10111111);
        return true;
    }

    auto PPU::ReadBGPD (std::uint8_t& pDataOut) -> bool
    {
        // Before Read:
        // - Open-bus in DMG Mode.
        // - Open-bus during `PixelTransfer` mode if `LCDC.7` is set.
        if (
            mSystem.IsCGB() == false || (
                mSystem.mNoRestrict == false &&
                mDisplayControl.mLcdEnable == true && 
                mDisplayStatus.mDisplayMode == stx::under(DisplayMode::PixelTransfer)
            )
        ) { pDataOut = 0xFF; return true; }

        // Read
        // - All bits readable.
        // - `BGPD` reads a byte in BG CRAM at the index pointed to by
        //   `BGPI.0-5`.
        pDataOut = mBgColorRAM[mBgColorPaletteIndex.mIndex];
        return true;
    }

    auto PPU::ReadOBPI (std::uint8_t& pDataOut) -> bool
    {
        // Before Read:
        // - Open-bus in DMG Mode.
        if (mSystem.IsCGB() == false)
            { pDataOut = 0xFF; return true; }

        // Read
        // - Bit 6 is unused; read `1`.
        // - Bits 0 through 5 and 7 are readable.
        pDataOut =
            0b01000000 |
            (mObjColorPaletteIndex.mValue & 0b10111111);
        return true;
    }

    auto PPU::ReadOBPD (std::uint8_t& pDataOut) -> bool
    {
        // Before Read
        // - Open-bus in DMG Mode.
        // - Open-bus during `PixelTransfer` mode if `LCDC.7` is set.
        if (
            mSystem.IsCGB() == false || (
                mSystem.mNoRestrict == false &&
                mDisplayControl.mLcdEnable == true && 
                mDisplayStatus.mDisplayMode == stx::under(DisplayMode::PixelTransfer)
            )
        ) { pDataOut = 0xFF; return true; }

        // Read
        // - All bits readable.
        // - `OBPD` reads a byte in OBJ CRAM at the index pointed to by
        //   `OBPI.0-5`.
        pDataOut = mObjColorRAM[mObjColorPaletteIndex.mIndex];
        return true;
    }

    auto PPU::ReadPPUX (std::uint8_t& pDataOut) -> bool
    {
        // Before Read
        // - Open-bus in DMG Mode.
        if (mSystem.IsCGB() == false)
            { pDataOut = 0xFF; return true; }

        // Read
        // - `PPUX` is a custom port register exclusive to G10.Boy's PPU.
        // - Bits 3 through 7 are unused; read `1`.
        // - Bits 0 through 2 are readable.
        // - `PPUX.0` is the VRAM bank select bit, analogous to CGB `VBK`.
        // - `PPUX.1` is the object priority bit, analogous to CGB `OPRI`.
        // - `PPUX.2` is exclusive to G10.Boy's PPU. It controls whether or not
        //   the PPU should use color or monochrome palettes in CGB Mode.
        pDataOut =
            0b11111000 |
            (mPPUAuxillarySettings.mValue & 0b00000111);

        return true;
    }

    auto PPU::ReadODMAS (std::uint8_t& pDataOut) -> bool
    {
        // Read
        // - All bits readable.
        // - `ODMAS` is OAM DMA start register. Reading from it reads the number
        //   of bytes transferred in the current (or last) transfer process.
        pDataOut = mOamDmaBytesTransferred;
        return true;
    }

    auto PPU::ReadVDMA6 (std::uint8_t& pDataOut) -> bool
    {
        // Before Read
        // - Open-bus in DMG Mode.
        if (mSystem.IsCGB() == false)
            { pDataOut = 0xFF; return true; }

        // Read
        // - All bits readable.
        // - `VDMA6` is the VRAM DMA control register, analogous to CGB `HDMA5`.
        pDataOut = mVramDmaControl.mValue;
        return true;
    }

    auto PPU::WriteLCDC (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Capture old `LCDC.7` (LCD enable).
        bool oldLcdEnable = mDisplayControl.mLcdEnable;

        // Write
        // - All bits writable.
        mDisplayControl.mValue = pDataIn;

        // After Write
        // - Check for rising/falling edge in `LCDC.7`.
        // - If falling edge outside of `VerticalBlank` mode, retain the old bit.
        if (oldLcdEnable == true && mDisplayControl.mLcdEnable == false)
        {
            // Falling edge detected. Make sure we are in `VerticalBlank`.
            if (mDisplayStatus.mDisplayMode != stx::under(DisplayMode::VerticalBlank))
                { mDisplayControl.mLcdEnable = true; }
            else
            {
                // LCD disabled. Clear stat line, reset display mode and clear
                // framebuffer.
                mDisplayStatus.mDisplayMode = stx::under(DisplayMode::HorizontalBlank);
                mStatInterruptLine = false;
                std::ranges::fill(mFrameBuffer, 0xFF);
            }
        }
        else if (oldLcdEnable == false && mDisplayControl.mLcdEnable == true)
        {
            // Rising edge detected.
            // - Reset scanline and check for coincidence.
            bool oldCoincidence = mDisplayStatus.mCoincidence;
            mCurrentScanline = 0;
            mDisplayStatus.mCoincidence = (mCurrentScanline == mScanlineCompare);
            if (mDisplayStatus.mCoincidence == true && oldCoincidence == false)
                { UpdateStatLine(false); }

            // - Reset dot counters.
            mFrameDots = mLineDots = mModeDots = 0;

            // - Reset window state.
            mWindowScanline = 0;
            mWindowTriggered = false;
            mWYConditionMet = false;

            // The first scanline after `LCDC.7` enable starts in `HorizontalBlank`
            // mode instead of the normal `ObjectScan` mode, then proceeds into
            // `PixelTransfer`, skipping the `ObjectScan` mode altogether.
            //
            // `STAT.0-1` read `0` during this period.
            mDisplayStatus.mDisplayMode = stx::under(DisplayMode::HorizontalBlank);
            mLcdJustEnabled = true;
            mFirstLineMode3 = false;
            mLyIncrementedEarly = false;
            mVramReadBlocked = false;
            mStatInterruptLine = false;
            ClearScanlineObjects();
        }

        return true;
    }

    auto PPU::WriteSTAT (std::uint8_t pDataIn) -> bool
    {
        // Write
        // - Bits 0 through 2 are read-only; retain their original values.
        // - Bit 7 is unused; write `1`.
        // - Bits 3 through 6 are writable.
        mDisplayStatus.mValue =
            (mDisplayStatus.mValue & 0b00000111) |
            0b10000000 |
            (pDataIn & 0b01111000);

        // After Write:
        // - Update the stat interrupt line.
        UpdateStatLine(false);

        return true;
    }

    auto PPU::WriteSCY (std::uint8_t pDataIn) -> bool
    {
        // Write
        // - All bits writable.
        mViewportY = pDataIn;
        return true;
    }

    auto PPU::WriteSCX (std::uint8_t pDataIn) -> bool
    {
        // Write
        // - All bits writable.
        mViewportX = pDataIn;
        return true;
    }

    auto PPU::WriteWY (std::uint8_t pDataIn) -> bool
    {
        // Write
        // - All bits writable.
        mWindowY = pDataIn;
        return true;
    }

    auto PPU::WriteWX (std::uint8_t pDataIn) -> bool
    {
        // Write
        // - All bits writable.
        mWindowX = pDataIn;
        return true;
    }

    auto PPU::WriteLYC (std::uint8_t pDataIn) -> bool
    {
        // Write
        // - All bits writable.
        mScanlineCompare = pDataIn;

        // After Write
        // - If `LCDC.7` is set, then check for line coincidence.
        if (mDisplayControl.mLcdEnable == true)
        {
            bool oldCoincidence = mDisplayStatus.mCoincidence;
            mDisplayStatus.mCoincidence = (mCurrentScanline == mScanlineCompare);
            if (mDisplayStatus.mCoincidence == true && oldCoincidence == false)
                { UpdateStatLine(false); }
        }

        return true;
    }

    auto PPU::WriteBGP (std::uint8_t pDataIn) -> bool
    {
        // Write
        // - All bits writable.
        mBgMonochromePalette.mValue = pDataIn;
        return true;
    }

    auto PPU::WriteOBP0 (std::uint8_t pDataIn) -> bool
    {
        // Write
        // - All bits writable.
        mObjMonochromePalette0.mValue = pDataIn;
        return true;
    }

    auto PPU::WriteOBP1 (std::uint8_t pDataIn) -> bool
    {
        // Write
        // - All bits writable.
        mObjMonochromePalette1.mValue = pDataIn;
        return true;
    }

    auto PPU::WriteBGPI (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore in DMG Mode.
        if (mSystem.IsCGB() == false)
            { return true; }

        // Write
        // - Bit 6 is unused; write `1`.
        // - Bits 0 through 5 and 7 are writable.
        mBgColorPaletteIndex.mValue =
            0b01000000 |
            (pDataIn & 0b10111111);

        return true;
    }

    auto PPU::WriteBGPD (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore in DMG Mode.
        // - Ignore during `PixelTransfer` mode if `LCDC.7` is set.
        if (mSystem.IsCGB() == false)
            { return true; }
        bool restricted =
            mSystem.mNoRestrict == false &&
            mDisplayControl.mLcdEnable == true &&
            mDisplayStatus.mDisplayMode == stx::under(DisplayMode::PixelTransfer);

        // Write
        // - If not restricted, all bits writable.
        // - Writing to `BGPD` writes to a byte in the background color RAM,
        //   pointed to by `BGPI.0-5`.
        if (restricted == false)
            { mBgColorRAM[mBgColorPaletteIndex.mIndex] = pDataIn; }

        // After Write
        // - Even if restricted, if `BGPI.7` is set, then increment the index
        //   in `BGPI.0-5`.
        if (mBgColorPaletteIndex.mAutoIncrement == true)
            { mBgColorPaletteIndex.mIndex++; }

        return true;
    }

    auto PPU::WriteOBPI (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore in DMG Mode.
        if (mSystem.IsCGB() == false)
            { return true; }

        // Write
        // - Bit 6 is unused; write `1`.
        // - Bits 0 through 5 and 7 are writable.
        mObjColorPaletteIndex.mValue =
            0b01000000 |
            (pDataIn & 0b10111111);

        return true;
    }

    auto PPU::WriteOBPD (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore in DMG Mode.
        // - Ignore during `PixelTransfer` mode if `LCDC.7` is set.
        if (mSystem.IsCGB() == false)
            { return true; }
        bool restricted =
            mSystem.mNoRestrict == false &&
            mDisplayControl.mLcdEnable == true &&
            mDisplayStatus.mDisplayMode == stx::under(DisplayMode::PixelTransfer);

        // Write
        // - If not restricted, all bits writable.
        // - Writing to `OBPD` writes to a byte in the object color RAM,
        //   pointed to by `OBPI.0-5`.
        if (restricted == false)
            { mObjColorRAM[mObjColorPaletteIndex.mIndex] = pDataIn; }

        // After Write
        // - Even if restricted, if `OBPI.7` is set, then increment the index
        //   in `OBPI.0-5`.
        if (mObjColorPaletteIndex.mAutoIncrement == true)
            { mObjColorPaletteIndex.mIndex++; }

        return true;
    }

    auto PPU::WritePPUX (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignored in DMG Mode.
        if (mSystem.IsCGB() == false)
            { return true; }

        // Write
        // - Bits 3 through 7 are unused; write `1`.
        // - Bits 0 through 2 are writable.
        mPPUAuxillarySettings.mValue =
            0b11111000 |
            (pDataIn & 0b00000111);

        // After Write
        // - Set active VRAM bank based on `PPUX.0`.
        mVideoRAM = (mPPUAuxillarySettings.mVramBank == 1) ?
            mVideoRAM1 : mVideoRAM0;

        return true;
    }

    auto PPU::WriteODMAS () -> bool
    {
        // Write
        // - Any write to this register initiates an OAM DMA transfer process.
        mOamDmaRestarting = (mOamDmaActive == true && mOamDmaDelay == 0);
        mOamDmaActive = true;
        mOamDmaSource = (mOamDmaSourceAddress & 0xFFFFFF00);
        mOamDmaBytesTransferred = 0;
        mOamDmaDelay = 2;               // Startup delay of 2 PPU dots.
        return true;
    }

    auto PPU::WriteODMA1 (std::uint8_t pDataIn) -> bool
    {
        // Write
        // - All bits writable.
        // - `ODMA1` is the second least significant byte of the 32-bit OAM DMA 
        //   source address.
        mOamDmaSourceAddress =
            (mOamDmaSourceAddress & 0xFFFF00FF) | 
            (static_cast<std::uint32_t>(pDataIn) << 8);
        return true;
    }

    auto PPU::WriteODMA2 (std::uint8_t pDataIn) -> bool
    {
        // Write
        // - All bits writable.
        // - `ODMA2` is the second most significant byte of the 32-bit OAM DMA source
        //   address.
        mOamDmaSourceAddress = 
            (mOamDmaSourceAddress & 0xFF00FFFF) | 
            (static_cast<std::uint32_t>(pDataIn) << 16);
        return true;
    }

    auto PPU::WriteODMA3 (std::uint8_t pDataIn) -> bool
    {
        // Write
        // - All bits writable.
        // - `ODMA3` is the most significant byte of the 32-bit OAM DMA source
        //   address.
        mOamDmaSourceAddress = 
            (mOamDmaSourceAddress & 0x00FFFFFF) | 
            (static_cast<std::uint32_t>(pDataIn) << 24);
        return true;
    }

    auto PPU::WriteVDMA0 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore in DMG Mode.
        if (mSystem.IsCGB() == false)
            { return true; }

        // Write
        // - All bits writable.
        // - `VDMA0` is the least significant byte of the 32-bit VRAM DMA source
        //   address.
        mVramDmaSourceAddress = 
            (mVramDmaSourceAddress & 0xFFFFFF00) | 
            static_cast<std::uint32_t>(pDataIn);
        return true;
    }

    auto PPU::WriteVDMA1 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore in DMG Mode.
        if (mSystem.IsCGB() == false)
            { return true; }

        // Write
        // - All bits writable.
        // - `VDMA1` is the second least significant byte of the 32-bit VRAM DMA 
        //   source address.
        mVramDmaSourceAddress =
            (mVramDmaSourceAddress & 0xFFFF00FF) | 
            (static_cast<std::uint32_t>(pDataIn) << 8);
        return true;
    }

    auto PPU::WriteVDMA2 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore in DMG Mode.
        if (mSystem.IsCGB() == false)
            { return true; }

        // Write
        // - All bits writable.
        // - `VDMA2` is the second most significant byte of the 32-bit VRAM DMA 
        //   source address.
        mVramDmaSourceAddress = 
            (mVramDmaSourceAddress & 0xFF00FFFF) | 
            (static_cast<std::uint32_t>(pDataIn) << 16);
        return true;
    }

    auto PPU::WriteVDMA3 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore in DMG Mode.
        if (mSystem.IsCGB() == false)
            { return true; }

        // Write
        // - All bits writable.
        // - `VDMA3` is the most significant byte of the 32-bit VRAM DMA source
        //   address.
        mVramDmaSourceAddress = 
            (mVramDmaSourceAddress & 0x00FFFFFF) | 
            (static_cast<std::uint32_t>(pDataIn) << 24);
        return true;
    }

    auto PPU::WriteVDMA4 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore in DMG Mode.
        if (mSystem.IsCGB() == false)
            { return true; }

        // Write
        // - All bits writable.
        // - `VDMA4` is the least significant byte of the 32-bit VRAM DMA
        //   destination address.
        mVramDmaDestinationAddress =
            (mVramDmaDestinationAddress & 0xFFFFFF00) |
            static_cast<std::uint32_t>(pDataIn);
        return true;
    }

    auto PPU::WriteVDMA5 (std::uint8_t pDataIn) -> bool
    {
        // Before Write
        // - Ignore in DMG Mode.
        if (mSystem.IsCGB() == false)
            { return true; }

        // Write
        // - All bits writable.
        // - `VDMA5` is the second least significant byte of the 32-bit VRAM DMA
        //   destination address.
        mVramDmaDestinationAddress =
            (mVramDmaDestinationAddress & 0xFFFF00FF) |
            (static_cast<std::uint32_t>(pDataIn) << 8);
        return true;
    }

    auto PPU::WriteVDMA6 (std::uint8_t pDataIn) -> bool    
    {
        // Before Write
        // - Ignore in DMG Mode.
        if (mSystem.IsCGB() == false)
            { return true; }

        // Write
        // - All bits writable.
        // - `VDMA6` is the control register for VRAM DMA transfers.
        mVramDmaControl.mValue = pDataIn;

        // After Write
        // - `VDMA6.7` was cleared during an ongoing HDMA transfer, then cancel
        //   that transfer.
        bool cancelled = false;
        if (mVramDmaActive == true && mVramDmaControl.mIsHDma == false)
        {
            mVramDmaActive = false;
            mVramDmaControl.mLength = 
                static_cast<std::uint8_t>(mVramDmaBlocksLeft - 1);
            cancelled = true;
        }

        // Otherwise:
        // - If `VDMA6.7` is set, then initate a "Horizontal Blank VRAM DMA" 
        //   transfer (HDMA). This transfer is carried out in blocks, one per
        //   `HorizontalBlank` period.
        // - If `VDMA6.7` is clear, then initiate a `General VRAM DMA" transfer
        //   (GDMA). This transfer is carried out all at once, in a blocking
        //   manner.
        if (cancelled == false)
        {
            mVramDmaActive = true;
            mVramDmaSource = (mVramDmaSourceAddress & 0xFFFFFFF0);
            mVramDmaDestination = 0x80008000 | (mVramDmaDestinationAddress & 0x1FF0);
            mVramDmaBlocksLeft = mVramDmaControl.mLength + 1;

            if (mVramDmaControl.mIsHDma == false)
            {
                while (mVramDmaActive == true)
                    { TickVramDMA(); }
            }
        }

        return true;
    }
}

// Public Methods - Direct Memory Access ***************************************

namespace G10::GB
{
    auto PPU::IsOamDmaActive () const -> bool
    {
        return mOamDmaActive == true && (
            mOamDmaDelay == 0 ||
            mOamDmaRestarting == true
        );
    }

    auto PPU::IsVramDmaActive () const -> bool
    {
        return mVramDmaActive == true && mVramDmaBlocksLeft > 0;
    }
}

// Private Methods - Helpers ***************************************************

namespace G10::GB
{
    auto PPU::CheckForWindow () -> bool
    {
        if (mWindowVisible == false)
            { return false; }

        auto windowX = static_cast<std::int16_t>(mWindowX) - 7;
        if (windowX < 0) { windowX = 0; }

        return (
            mPixelFetcher.mPixelsTransferred >= 
                static_cast<std::uint8_t>(windowX)
        );
    }

    auto PPU::CheckForStatInterrupt () -> bool
    {
        return (
            (mDisplayStatus.mHBlankStat == true &&
                mDisplayStatus.mDisplayMode == stx::under(DisplayMode::HorizontalBlank)) ||
            (mDisplayStatus.mVBlankStat == true &&
                mDisplayStatus.mDisplayMode == stx::under(DisplayMode::VerticalBlank)) ||
            (mDisplayStatus.mOamStat == true &&
                mDisplayStatus.mDisplayMode == stx::under(DisplayMode::ObjectScan)) ||
            (mDisplayStatus.mLycStat == true &&
                mDisplayStatus.mCoincidence == true)
        );
    }

    auto PPU::CheckForCoincidence () -> bool
    {
        // A line coincidence condition occurs when the current scanline and
        // scanline compare registers match (`LY == LYC`).
        mDisplayStatus.mCoincidence = (mCurrentScanline == mScanlineCompare);
        if (mDisplayStatus.mCoincidence == true)
        {
            // If one occurs, update the stat line and call the coincidence
            // delegate, if one is registered.
            UpdateStatLine(false);
            if (mCoincidenceDelegate != nullptr)
            {
                mCoincidenceDelegate(mSystem, mFrameBuffer, mScanlineCompare);
            }
        }

        return mDisplayStatus.mCoincidence;
    }

    auto PPU::IncrementScanline () -> void
    {
        // 1. If a part of the window layer was rendered on this scanline, then
        //    also increment the window scanline counter.
        if (mWindowTriggered == true)
        {
            mWindowTriggered = false;
            mWindowScanline++;
        }

        // 2. Increment the current scanline counter. If it overflows (`>= 154`),
        //    then reset it and the window scanline counter to zero.
        if (++mCurrentScanline >= kScanlinesPerFrame)
        {
            mCurrentScanline = 0;
            mWindowScanline = 0;
        }

        // 3. Clear the line coincidence flag. The actual coincidence check is
        //    performed during the mode transition process.
        mDisplayStatus.mCoincidence = false;
        UpdateStatLine(false);
    }

    auto PPU::UpdateStatLine (bool pJustEnteredVBlank) -> void
    {
        // Update the line.
        bool newStatLine = CheckForStatInterrupt();

        // In DMG Mode, if `STAT.5` is set, then the `DisplayStatus` interrupt
        // normally requested upon entering the `ObjectScan` mode can also
        // be requested upon entering a `VerticalBlank` period.
        if (
            mSystem.IsCGB() == false &&
            mDisplayStatus.mOamStat == true &&
            mDisplayStatus.mDisplayMode == stx::under(DisplayMode::VerticalBlank) &&
            pJustEnteredVBlank == true
        ) { newStatLine = true; }

        // If updating the stat line results in a rising edge in that line,
        // then request a `DisplayStatus` interrupt of the CPU.
        if (mStatInterruptLine == false && newStatLine == true)
        {
            mSystem.GetCPU().RequestInterrupt(stx::under(Interrupt::DisplayStatus));
        }

        // Record the new stat line.
        mStatInterruptLine = newStatLine;
    }

    auto PPU::IsUsingColorPalette () -> bool
    {
        // Determine the appropriate palette to get our color from.
        //
        // - If in CGB Mode and `PPUX.2` is set, then derive from the color
        //   palette memory.
        // - If in DMG Mode or `PPUX.2` is clear, then derive from the 
        //   appropriate monochrome palette.
        return
            mSystem.IsCGB() == true &&
            mPPUAuxillarySettings.mUseColorPalette == true;
    }

    auto PPU::GetPixelColor (std::uint8_t pColorIndex, std::uint8_t pPaletteIndex, 
        bool pIsObject) -> std::uint32_t
    {
        // 1. Which palette are we using?
        bool useColorPalette = IsUsingColorPalette();

        // 2. Derive the color based on the palette selection.
        if (useColorPalette == true)
        {
            // 3. Determine the index in Color RAM of the color bytes to fetch.
            //    Then fetch those bytes.
            std::uint8_t
                index = (((pPaletteIndex & 0b111) * 4) + (pColorIndex & 0b11)) * 2,
                colorLow = ReadColorRAMInternal(index, (pIsObject == false)),
                colorHigh = ReadColorRAMInternal(index + 1, (pIsObject == false));

            // 4. Extract the 5-bit red, green and blue components.
            //    Convert these into 8-bit components.
            std::uint8_t
                red5        =   (colorLow & 0b11111),
                green5      =   ((colorLow & 0b11100000) >> 5) | 
                                ((colorHigh & 0b00000011) << 3),
                blue5       =   ((colorHigh & 0b01111100) >> 2),
                red8        =   (red5 << 3) | (red5 >> 2),
                green8      =   (green5 << 3) | (green5 >> 2),
                blue8       =   (blue5 << 3) | (blue5 >> 2);

            // Return the color in 32-bit ARGB8888 format.
            return  0xFF000000 |
                    (red8 << 16) |
                    (green8 << 8) |
                    (blue8);
        }
        else
        {
            // Determine the monochrome palette register to use.
            auto& palette = mBgMonochromePalette;
            if (pIsObject == true)
            {
                palette = (pPaletteIndex == 0) ? 
                    mObjMonochromePalette0 : mObjMonochromePalette1;
            }

            switch (pColorIndex & 0b11)
            {
                case 0: return mMonochromeColors[palette.mColor0];
                case 1: return mMonochromeColors[palette.mColor1];
                case 2: return mMonochromeColors[palette.mColor2];
                case 3: return mMonochromeColors[palette.mColor3];
                default: return 0;
            }
        }
    }

    auto PPU::GetObject (std::uint8_t pOamIndex) -> Object&
    {
        return *reinterpret_cast<Object*>(
            mOAM.data() + ((pOamIndex % kObjectCount) * 4));
    }
}

// Private Methods - Internal Buffer Access ************************************

namespace G10::GB
{
    auto PPU::ReadVideoRAMInternal (std::uint8_t pBank, std::uint32_t pIndex) -> std::uint8_t
    {
        if (mSystem.IsCGB() == false)
            { pBank = 0; }

        return ((pBank & 1) == 0) ? 
            mVideoRAM0[pIndex & 0x1FFF] : 
            mVideoRAM1[pIndex & 0x1FFF];
    }

    auto PPU::ReadOAMInternal (std::uint32_t pIndex) -> std::uint8_t
    {
        return mOAM[pIndex % kOamSize];
    }

    auto PPU::ReadColorRAMInternal (std::uint32_t pIndex, bool pIsBg) -> std::uint8_t
    {
        return (pIsBg == true) ?
            mBgColorRAM[pIndex % kColorRamSize] :
            mObjColorRAM[pIndex % kColorRamSize];
    }

    auto PPU::WriteVideoRAMInternal (std::uint8_t pBank, std::uint32_t pIndex, std::uint8_t pData) -> void
    {
        if (mSystem.IsCGB() == false)
            { pBank = 0; }

        if ((pBank & 1) == 0)
            { mVideoRAM0[pIndex & 0x1FFF] = pData; }
        else
            { mVideoRAM1[pIndex & 0x1FFF] = pData; }
    }

    auto PPU::WriteOAMInternal (std::uint32_t pIndex, std::uint8_t pData) -> void
    {
        mOAM[pIndex % kOamSize] = pData;
    }

    auto PPU::WriteColorRAMInternal (std::uint32_t pIndex, bool pIsBg, std::uint8_t pData) -> void
    {
        if (mSystem.IsCGB() == false)
            { pIsBg = true; }

        if (pIsBg == true)
            { mBgColorRAM[pIndex % kColorRamSize] = pData; }
        else
            { mObjColorRAM[pIndex % kColorRamSize] = pData; }
    }
}

// Private Methods - Object Pixel Fetching *************************************

namespace G10::GB
{
    auto PPU::CheckForObjectFetch () -> bool
    {
        if (mPixelFetcher.mIsFetchingObject == true ||
            (mSystem.IsCGB() == false && mDisplayControl.mObjEnable == false))
            { return false; }

        std::int16_t bestPriority = -1;
        std::uint8_t bestOamIndex = 0;

        for (std::uint8_t i = 0; i < mScanlineObjectCount; ++i)
        {
            std::uint8_t oamIndex = mScanlineObjectIndices[i];
            if ((mPixelFetcher.mObjFetchMask & (1 << i)) != 0)
                { continue; }

            const Object& obj = GetObject(oamIndex);
            std::int16_t    
                screenX = (static_cast<std::int16_t>(obj.mXPos) - 8),
                triggerX = (screenX < 0) ? 0 : screenX;

            if (
                mPixelFetcher.mPixelsTransferred >= 
                    static_cast<std::uint8_t>(triggerX) &&
                (bestPriority < 0 || i < static_cast<std::uint8_t>(bestPriority))
            )
            {
                bestPriority = i;
                bestOamIndex = oamIndex;
            }
        }

        if (bestPriority < 0)
            { return false; }

        const Object& bestObj = GetObject(bestOamIndex);
        std::uint8_t objX = bestObj.mXPos;
        std::uint8_t scxMod = (mViewportX & 7);
        std::uint8_t alignment = (objX + scxMod) & 7;
        std::uint8_t penalty = 5 - ((alignment < 5) ? alignment : 5);

        mPixelFetcher.mIsFetchingObject = true;
        mPixelFetcher.mObjFetchStep = -static_cast<std::int8_t>(penalty);
        mPixelFetcher.mObjOamIndex = bestOamIndex;
        mPixelFetcher.mObjPriority = static_cast<std::uint8_t>(bestPriority);
        mPixelFetcher.mObjFetchMask |= (1 << bestPriority);

        return true;
    }

    auto PPU::PrefetchObjectTileAddress () -> void
    {
        const Object& obj = GetObject(mPixelFetcher.mObjOamIndex);
        std::uint8_t objHeight = (mDisplayControl.mObjSize == 0) ? 8 : 16;
        std::int16_t screenY = (static_cast<std::int16_t>(obj.mYPos) - 16);
        std::uint8_t tileRow = 
            static_cast<std::uint8_t>(mCurrentScanline - 
                static_cast<std::uint8_t>(screenY));

        if (obj.mAttributes.mYFlip == true)
            { tileRow = (objHeight - 1) - tileRow; }

        std::uint8_t tileIndex = obj.mTileIndex;
        if (objHeight == 16)
        {
            if (tileRow < 8)
                { tileIndex &= 0xFE; }
            else
                { tileIndex |= 0x01; tileRow -= 8; }
        }

        mPixelFetcher.mObjDataAddress = 
            (static_cast<std::uint16_t>(tileIndex) * 16) +
                (static_cast<std::uint16_t>(tileRow) * 2);
    }

    auto PPU::FetchObjectTileData (bool pIsHighByte) -> void
    {
        const Object& obj = GetObject(mPixelFetcher.mObjOamIndex);
        std::uint8_t bank = (mSystem.IsCGB() == true) ?
            obj.mAttributes.mVramBank : 0;
        std::uint16_t address = mPixelFetcher.mObjDataAddress + 
            ((pIsHighByte == true) ? 1 : 0);
        std::uint8_t value = ReadVideoRAMInternal(bank, address);

        if (pIsHighByte == true)
            { mPixelFetcher.mObjDataHigh = value; }
        else
            { mPixelFetcher.mObjDataLow = value; }
    }

    auto PPU::PushObjectPixels () -> void
    {
        const Object& obj = GetObject(mPixelFetcher.mObjOamIndex);
        std::uint8_t objPriority = mPixelFetcher.mObjPriority;

        static constexpr Pixel transparentPixel = {
            .mColorIndex = 0,
            .mPaletteIndex = 0,
            .mObjectPriority = 0xFF,
            .mBgwPriority = true
        };

        while (mObjPixelFIFO.mSize < 8)
            { mObjPixelFIFO.Push(transparentPixel); }

        std::int16_t screenX = (static_cast<std::int16_t>(obj.mXPos) - 8);
        std::uint8_t startPixel = (screenX < 0) ? 
            static_cast<std::uint8_t>(-screenX) : 0;

        for (std::uint8_t i = startPixel; i < 8; ++i)
        {
            std::int16_t pixelX = screenX + i;
            if (pixelX >= static_cast<std::int16_t>(kScreenWidth))
                { break; }
            else if (
                pixelX < 
                static_cast<std::int16_t>(mPixelFetcher.mPixelsTransferred)
            )
                { continue; }

            std::size_t fifoPos = 
                static_cast<std::size_t>(pixelX - 
                    mPixelFetcher.mPixelsTransferred);
            if (fifoPos >= 8)
                { continue; }

            std::uint8_t bit = (obj.mAttributes.mXFlip == true) ? i : (7 - i);
            std::uint8_t lowBit = (mPixelFetcher.mObjDataLow >> bit) & 1;
            std::uint8_t highBit = (mPixelFetcher.mObjDataHigh >> bit) & 1;
            std::uint8_t colorIndex = ((highBit << 1) | lowBit);
            if (colorIndex == 0)
                { continue; }

            std::uint8_t paletteIndex = 
                (IsUsingColorPalette() == true) ?
                    obj.mAttributes.mColorPalette : 
                    obj.mAttributes.mMonochromePalette;

            std::uint8_t fifoIndex = static_cast<std::uint8_t>(
                (mObjPixelFIFO.mHead + fifoPos) % kFIFOCapacity);
            Pixel& pixel = mObjPixelFIFO.mPixels[fifoIndex];
            if (pixel.mColorIndex == 0 || objPriority < pixel.mObjectPriority)
            {
                pixel.mColorIndex = colorIndex;
                pixel.mPaletteIndex = paletteIndex;
                pixel.mObjectPriority = objPriority;
                pixel.mBgwPriority = obj.mAttributes.mBgwPriority;
            }
        }
    }

    auto PPU::TickObjectPixelFetcher () -> void
    {
        if (mPixelFetcher.mObjFetchStep < 0)
        {
            TickPixelFetcher();
            mPixelFetcher.mObjFetchStep++;
            return;
        }

        mPixelFetcher.mObjFetchStep++;
        switch (mPixelFetcher.mObjFetchStep)
        {
            case 1:
            case 2:
                TickPixelFetcher();
                break;
            case 3:
                PrefetchObjectTileAddress();
                FetchObjectTileData(false);
                break;
            case 4:
                FetchObjectTileData(true);
                break;
            case 5:
                PushObjectPixels();
                break;
            case 6:
                mPixelFetcher.mIsFetchingObject = false;
                break;
            default:
                break;
        }
    }
}

// Private Methods - Pixel Tranfer *********************************************

namespace G10::GB
{
    auto PPU::PreparePixelFetcher () -> void
    {
        mBgPixelFIFO.Clear();
        mObjPixelFIFO.Clear();

        mPixelFetcher.mMode = FetchMode::Index;
        mPixelFetcher.mStateDots = 0;
        mPixelFetcher.mFetchX = 0;
        mPixelFetcher.mPixelsTransferred = 0;
        mPixelFetcher.mPixelsToDiscard = (mViewportX % 8);
        mPixelFetcher.mIsInWindow = false;
        mPixelFetcher.mInitialFetch = true;

        mPixelFetcher.mBgwIndex = 0;
        mPixelFetcher.mBgwAttributes.mValue = 0;
        mPixelFetcher.mBgwDataLow = 0;
        mPixelFetcher.mBgwDataHigh = 0;

        mPixelFetcher.mIsFetchingObject = false;
        mPixelFetcher.mObjFetchMask = 0;
        mPixelFetcher.mObjFetchStep = 0;
        mPixelFetcher.mObjOamIndex = 0;
        mPixelFetcher.mObjPriority = 0;
        mPixelFetcher.mObjDataAddress = 0;
        mPixelFetcher.mObjDataLow = 0;
        mPixelFetcher.mObjDataHigh = 0;
    }

    auto PPU::FetchTileIndex () -> void
    {
        if (mWindowVisible == false)
            { mPixelFetcher.mIsInWindow = false; }

        mPixelFetcher.mTileMapBase =
            (mPixelFetcher.mIsInWindow == true) ?
                ((mDisplayControl.mWinTileMap == 0) ? 0x1800 : 0x1C00) :
                ((mDisplayControl.mBgTileMap == 0) ? 0x1800 : 0x1C00);

        std::uint8_t tilemapX = 0, tilemapY = 0;
        if (mPixelFetcher.mIsInWindow == true)
        {
            tilemapX = (mPixelFetcher.mFetchX & 0x1F);
            tilemapY = ((mWindowScanline / 8) & 0x1F);
        }
        else
        {
            tilemapX = ((mPixelFetcher.mFetchX + (mViewportX / 8)) & 0x1F);
            tilemapY = (((mCurrentScanline + mViewportY) / 8) & 0x1F);
        }

        mPixelFetcher.mTileMapAddress =
            mPixelFetcher.mTileMapBase + (tilemapY * 32) + tilemapX;

        mPixelFetcher.mBgwIndex = 
            ReadVideoRAMInternal(0, mPixelFetcher.mTileMapAddress);
        if (mSystem.IsCGB() == true)
        {
            mPixelFetcher.mBgwAttributes.mValue =
                ReadVideoRAMInternal(1, mPixelFetcher.mTileMapAddress);
        }
        else
        {
            mPixelFetcher.mBgwAttributes.mValue = 0;
        }

        if (mDisplayControl.mBgwTileData == 0)
        {
            mPixelFetcher.mBgwDataBase = 0x1000;
            mPixelFetcher.mBgwDataAddress = mPixelFetcher.mBgwDataBase +
                (static_cast<std::int8_t>(mPixelFetcher.mBgwIndex) * 16);
        }
        else
        {
            mPixelFetcher.mBgwDataBase = 0x0000;
            mPixelFetcher.mBgwDataAddress = 
                (static_cast<std::uint16_t>(mPixelFetcher.mBgwIndex) * 16);
        }

        std::uint8_t tileRow = (mPixelFetcher.mIsInWindow == true) ?
            (mWindowScanline % 8) :
            ((mCurrentScanline + mViewportY) % 8);
        if (mSystem.IsCGB() == true && 
            mPixelFetcher.mBgwAttributes.mYFlip == true)
            { tileRow = (7 - tileRow); }

        mPixelFetcher.mBgwDataAddress += (tileRow * 2);
        mPixelFetcher.mMode = FetchMode::DataLow;
        mPixelFetcher.mStateDots = 0;
    }

    auto PPU::FetchTileData (bool pIsHighByte) -> void
    {
        std::uint8_t bank =
            (mSystem.IsCGB() == true) ? 
                mPixelFetcher.mBgwAttributes.mVramBank : 0;
        std::uint16_t address = mPixelFetcher.mBgwDataAddress + 
            (pIsHighByte ? 1 : 0);
        std::uint8_t value = ReadVideoRAMInternal(bank, address);

        if (pIsHighByte == false)
        {
            mPixelFetcher.mBgwDataLow = value;
            mPixelFetcher.mMode = FetchMode::DataHigh;
        }
        else if (mPixelFetcher.mInitialFetch == true)
        {
            mPixelFetcher.mBgwDataHigh = value;
            mPixelFetcher.mInitialFetch = false;
            mPixelFetcher.mMode = FetchMode::Index;
        }
        else
        {
            mPixelFetcher.mBgwDataHigh = value;
            mPixelFetcher.mMode = FetchMode::Push;
        }

        mPixelFetcher.mStateDots = 0;
    }

    auto PPU::PushPixels () -> void
    {
        if (mBgPixelFIFO.mSize > 0)
            { return; }

        for (std::uint8_t i = 0; i < 8; ++i)
        {
            std::uint8_t bit =
                (mSystem.IsCGB() == true && 
                    mPixelFetcher.mBgwAttributes.mXFlip == true) ?
                        i : (7 - i);

            std::uint8_t lowBit = (mPixelFetcher.mBgwDataLow >> bit) & 1;
            std::uint8_t highBit = (mPixelFetcher.mBgwDataHigh >> bit) & 1;
            std::uint8_t colorIndex = ((highBit << 1) | lowBit);

            if (mSystem.IsCGB() == false && mDisplayControl.mBgwEnable == false)
                { colorIndex = 0; }

            Pixel pixel {
                .mColorIndex = colorIndex,
                .mPaletteIndex = mPixelFetcher.mBgwAttributes.mColorPalette,
                .mObjectPriority = 0xFF,
                .mBgwPriority = mPixelFetcher.mBgwAttributes.mBgwPriority
            };

            mBgPixelFIFO.Push(pixel);
        }

        mPixelFetcher.mFetchX = ((mPixelFetcher.mFetchX + 1) & 0x1F);
        mPixelFetcher.mMode = FetchMode::Index;
        mPixelFetcher.mStateDots = 0;
    }

    auto PPU::TickPixelFetcher () -> void
    {
        if (mPixelFetcher.mMode == FetchMode::Push || 
            mPixelFetcher.mStateDots >= 1)
        {
            switch (mPixelFetcher.mMode)
            {
                case FetchMode::Index:
                    FetchTileIndex();
                    break;
                case FetchMode::DataLow:
                    FetchTileData(false);
                    break;
                case FetchMode::DataHigh:
                    FetchTileData(true);
                    break;
                case FetchMode::Push:
                    PushPixels();
                    break;
                default:
                    break;
            }
        }
        else
        {
            mPixelFetcher.mStateDots++;
        }
    }

    auto PPU::TransferPixel () -> void    
    {
        if (mPixelFetcher.mPixelsTransferred >= kScreenWidth)
            { return; }

        if (mPixelFetcher.mIsInWindow == false && CheckForWindow() == true)
        {
            mPixelFetcher.mMode = FetchMode::Index;
            mPixelFetcher.mStateDots = 0;
            mPixelFetcher.mFetchX = 0;
            mPixelFetcher.mPixelsToDiscard = (mWindowX < 7) ? (7 - mWindowX) : 0;
            mBgPixelFIFO.Clear();
            mPixelFetcher.mIsInWindow = true;
            mWindowTriggered = true;
        }

        if (mBgPixelFIFO.mSize == 0)
            { return; }

        if (mPixelFetcher.mPixelsToDiscard > 0)
        {
            mBgPixelFIFO.Pop(nullptr);
            mPixelFetcher.mPixelsToDiscard--;
            return;
        }

        Pixel finalPixel {}, objPixel {};
        mBgPixelFIFO.Pop(&finalPixel);
        bool 
            hasObjectPixel = mObjPixelFIFO.Pop(&objPixel),
            useObjectPixel = false;

        if (mDisplayControl.mObjEnable == true && 
            hasObjectPixel == true && 
            objPixel.mColorIndex != 0)
        {
            if (mSystem.IsCGB() == true)
            {
                if (finalPixel.mColorIndex == 0 || 
                    mDisplayControl.mBgwEnable == false)
                    { useObjectPixel = true; }
                else if (finalPixel.mBgwPriority == true || 
                         objPixel.mBgwPriority == true)
                    { useObjectPixel = false; }
                else
                    { useObjectPixel = true; }
            }
            else
            {
                if (mDisplayControl.mObjEnable == false)
                    { useObjectPixel = false; }
                else if (
                    mDisplayControl.mBgwEnable == false ||
                    finalPixel.mColorIndex == 0 ||
                    objPixel.mBgwPriority == false
                )
                    { useObjectPixel = true; }
                else
                    { useObjectPixel = false; }
            }
        }

        if (useObjectPixel == true)
        {
            finalPixel.mBgwPriority = objPixel.mBgwPriority;
            finalPixel.mPaletteIndex = objPixel.mPaletteIndex;
            finalPixel.mColorIndex = objPixel.mColorIndex;
            finalPixel.mObjectPriority = objPixel.mObjectPriority;
        }

        std::size_t fbIndex = (mCurrentScanline * kScreenWidth) + 
            mPixelFetcher.mPixelsTransferred;
        mFrameBuffer[fbIndex] = GetPixelColor(
            finalPixel.mColorIndex,
            finalPixel.mPaletteIndex,
            useObjectPixel
        );
        mPixelFetcher.mPixelsTransferred++;
    }
}

// Private Methods - Object Scan ***********************************************

namespace G10::GB
{
    auto PPU::ScanObject (std::uint8_t pOamIndex) -> void
    {
        if (mScanlineObjectCount >= kObjectsPerScanline)
            { return; }

        const Object& obj = GetObject(pOamIndex);
        std::uint8_t objHeight = (mDisplayControl.mObjSize == 1) ? 16 : 8;

        std::int16_t objTop = (static_cast<std::int16_t>(obj.mYPos) - 16);
        if (mCurrentScanline >= objTop && 
            mCurrentScanline < objTop + objHeight)
        {
            mScanlineObjectIndices[mScanlineObjectCount++] = pOamIndex;
        }
    }

    auto PPU::SortScanlineObjects () -> void
    {
        bool sortByX = (mSystem.IsCGB() == false ||
            mPPUAuxillarySettings.mObjectSortPriority == 1);

        for (std::uint8_t i = 0; i < mScanlineObjectCount; ++i)
        {
            for (std::uint8_t j = 0; j < mScanlineObjectCount - 1 - i; ++j)
            {
                std::uint8_t oamA = mScanlineObjectIndices[j];
                std::uint8_t oamB = mScanlineObjectIndices[j + 1];
                const Object& objA = GetObject(oamA);
                const Object& objB = GetObject(oamB);
                bool shouldSwap = false;

                if (sortByX == true)
                {
                    if (objA.mXPos > objB.mXPos)
                        { shouldSwap = true; }
                    else if (objA.mXPos == objB.mXPos && oamA > oamB)
                        { shouldSwap = true; }
                }
                else
                {
                    if (oamA > oamB)
                        { shouldSwap = true; }
                }

                if (shouldSwap == true)
                {
                    mScanlineObjectIndices[j + 1] = oamA;
                    mScanlineObjectIndices[j] = oamB;
                }
            }
        }
    }

    auto PPU::ClearScanlineObjects () -> void
    {
        mScanlineObjectCount = 0;
    }
}

// Private Methods - Direct Memory Access **************************************

namespace G10::GB
{
    auto PPU::TickOamDMA () -> void
    {
        if (mOamDmaActive == false)
            { return; }

        if (mOamDmaDelay > 0)
        {
            if (--mOamDmaDelay == 0)
                { mOamDmaRestarting = false; }
            return;
        }

        std::uint8_t oamByte = 0xFF;
        std::uint32_t sourceAddress = mOamDmaSource + mOamDmaBytesTransferred;
        mSystem.Read(sourceAddress, oamByte);
        WriteOAMInternal(mOamDmaBytesTransferred, oamByte);
        mOamDmaBytesTransferred++;

        if (mOamDmaBytesTransferred >= kOamSize)
            { mOamDmaActive = false; }
    }

    auto PPU::TickVramDMA () -> void
    {
        if (mSystem.IsCGB() == false)
            { return; }

        if (mVramDmaActive == false)
            { return; }

        std::uint8_t value = 0x00;

        mSystem.mNoRestrict = true;
        for (std::uint8_t i = 0; i < 16; ++i)
        {
            mSystem.Read(mVramDmaSource++, value);
            mSystem.Write(mVramDmaDestination++, value);
        }
        mSystem.mNoRestrict = false;

        if (mVramDmaBlocksLeft > 0)
            { mVramDmaBlocksLeft--; }
        mVramDmaActive = (mVramDmaBlocksLeft > 0);
    }
}

// Private Methods - Display Mode Timing & Management **************************

namespace G10::GB
{
    auto PPU::EnterDisplayMode (DisplayMode pMode) -> void
    {
        DisplayMode oldMode = static_cast<DisplayMode>(mDisplayStatus.mDisplayMode);
        mModeDots = 0;
        mDisplayStatus.mDisplayMode = stx::under(pMode);

        switch (pMode)
        {
            case DisplayMode::HorizontalBlank:
            {
                mVramReadBlocked = false;
                UpdateStatLine(false);
                TickVramDMA();

                if (mScanlineDelegate != nullptr)
                {
                    mScanlineDelegate(mSystem, mFrameBuffer, mCurrentScanline);
                }
            } break;
            case DisplayMode::VerticalBlank:
            {
                mVramReadBlocked = false;
                mSystem.GetCPU().RequestInterrupt(
                    stx::under(Interrupt::VerticalBlank));
                CheckForCoincidence();
                UpdateStatLine(true);
                mFrameJustFinished = true;
                if (mFrameCallback != nullptr)
                {
                    mFrameCallback(mSystem, mFrameBuffer, true);
                }
            } break;
            case DisplayMode::ObjectScan:
            {
                CheckForCoincidence();
                UpdateStatLine(false);
                ClearScanlineObjects();
            } break;
            case DisplayMode::PixelTransfer:
            {
                SortScanlineObjects();
                PreparePixelFetcher();

                if (mWYConditionMet == false)
                    { mWYConditionMet = (mCurrentScanline == mWindowY); }

                mWindowVisible =
                    (mDisplayControl.mWinEnable == true) &&
                    mWYConditionMet == true &&
                    (mSystem.IsCGB() == true || mDisplayControl.mBgwEnable == true);
            } break;
        }
    }

    auto PPU::TickHorizontalBlank () -> void
    {
        if (mLcdJustEnabled == true && mCurrentScanline == 0)
        {
            if (mModeDots >= 80)
            {
                mLcdJustEnabled = false;
                mFirstLineMode3 = true;
                EnterDisplayMode(DisplayMode::PixelTransfer);
            }
            return;
        }

        if (mLyIncrementedEarly == false &&
            mLineDots >= (kDotsPerScanline - 4))
        {
            mLyIncrementedEarly = true;
            IncrementScanline();
        }

        if (mLineDots >= kDotsPerScanline)
        {
            mLyIncrementedEarly = false;
            mLineDots = 0;

            if (mCurrentScanline >= kScreenHeight)
                { EnterDisplayMode(DisplayMode::VerticalBlank); }
            else
                { EnterDisplayMode(DisplayMode::ObjectScan); }
        }
    }

    auto PPU::TickVerticalBlank () -> void
    {
        mFrameJustFinished = false;
        if (mLineDots >= kDotsPerScanline)
        {
            IncrementScanline();
            mLineDots = 0;

            if (mFrameDots >= kDotsPerFrame || mCurrentScanline == 0)
            {
                mFrameDots = 0;
                mWindowScanline = 0;
                mWindowTriggered = false;
                mWYConditionMet = false;
                EnterDisplayMode(DisplayMode::ObjectScan);
            }
            else
            {
                CheckForCoincidence();
            }
        }
    }

    auto PPU::TickObjectScan () -> void
    {
        if ((mModeDots & 1) == 0)
        {
            std::uint8_t oamIndex = 
                static_cast<std::uint8_t>((mModeDots / 2) - 1);
            ScanObject(oamIndex);

            // Note: There is no OAM bug in the G10.Boy in DMG mode.
        }

        if (mModeDots >= 80)
        {
            mVramReadBlocked = true;
            EnterDisplayMode(DisplayMode::PixelTransfer);
        }
        else if (mSystem.IsCGB() == false && mModeDots >= 76)
            { mVramReadBlocked = true; }
    }

    auto PPU::TickPixelTransfer () -> void
    {
        CheckForObjectFetch();
        if (mPixelFetcher.mIsFetchingObject == true)
        {
            TickObjectPixelFetcher();
        }
        else
        {
            TickPixelFetcher();
            TransferPixel();
        }

        if (
            mPixelFetcher.mPixelsTransferred >= kScreenWidth ||
            mModeDots >= 289
        )
        {
            EnterDisplayMode(DisplayMode::HorizontalBlank);
        }
    }
}
