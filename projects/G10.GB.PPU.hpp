/**
 * @file    G10.GB.PPU.hpp
 * @brief   Contains declarations for the G10.Boy's picture processing unit
 *          (PPU) component, and related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.GB.Common.hpp>

// Constants & Enumeartions ****************************************************

namespace G10::GB
{
    constexpr std::uint32_t
        kScreenWidth        = 160,
        kScreenHeight       = 144,
        kScreenPixelCount   = kScreenWidth * kScreenHeight;

    constexpr std::uint32_t
        kVramBankSize       = 0x2000,
        kOamSize            = 0xA0,
        kColorRamSize       = 0x40;

    constexpr std::uint8_t
        kMonochromeColorCount   = 4,
        kObjectCount            = 40,
        kObjectsPerScanline     = 10,
        kColorCount             = 32,
        kFIFOCapacity           = 16;

    constexpr std::uint32_t
        kVramTilesPerBank       = 384,
        kVramBytesPerTile       = 16,
        kVramTileDataSize       = kVramTilesPerBank * kVramBytesPerTile,
        kVramRenderBufferSize   = kVramTilesPerBank * 8 * 8,
        kOamRenderBufferSize    = kObjectCount * 8 * 8;

    constexpr std::uint32_t
        kDotsPerScanline        = 456,
        kScanlinesPerFrame      = 154,
        kDotsPerFrame           = kDotsPerScanline * kScanlinesPerFrame;

    constexpr const std::array<std::uint32_t, kMonochromeColorCount>
        kDefaultMonochromeColors = {
            0xFFFFFFFF,
            0xFFAAAAAA,
            0xFF555555,
            0xFF000000
        };

    enum class DisplayMode : std::uint8_t
    {
        HorizontalBlank     = 0b00,
        VerticalBlank       = 0b01,
        ObjectScan          = 0b10,
        PixelTransfer       = 0b11
    };

    enum class MonochromeColor : std::uint8_t
    {
        White               = 0b00,
        LightGray           = 0b01,
        DarkGray            = 0b10,
        Black               = 0b11
    };
}

// Types ***********************************************************************

namespace G10::GB
{
    class System;

    using FrameBuffer = std::array<std::uint32_t, kScreenPixelCount>;
    using FrameCallback = std::function<void (const System&, const FrameBuffer&, bool)>;
    using ScanlineDelegate = std::function<void (System&, FrameBuffer&, std::uint8_t)>;
    using CoincidenceDelegate = std::function<void (System&, FrameBuffer&, std::uint8_t)>;

    // using ScanlineView = std::span<std::uint32_t>;
    // using FrameCallback = std::function<void(const System&,
    //     const FrameBuffer& /* frameBuffer */, bool /* lcdEnable */)>;
    // using ScanlineDelegate = std::function<void(System&,
    //     ScanlineView& /* scanlineView */, std::uint8_t /* scanline */)>;
    // using CoincidenceDelegate = std::function<void(System&,
    //     ScanlineView& /* scanlineView */, std::uint8_t /* scanline */)>;
}

// Unions & Structures *********************************************************

namespace G10::GB
{
    union DisplayControlRegister final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t    mBgwEnable      : 1;
            std::uint8_t    mObjEnable      : 1;
            std::uint8_t    mObjSize        : 1;
            std::uint8_t    mBgTileMap      : 1;
            std::uint8_t    mBgwTileData    : 1;
            std::uint8_t    mWinEnable      : 1;
            std::uint8_t    mWinTileMap     : 1;
            std::uint8_t    mLcdEnable      : 1;
        };
    };

    union DisplayStatusRegister final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t    mDisplayMode    : 2;
            std::uint8_t    mCoincidence    : 1;
            std::uint8_t    mHBlankStat     : 1;
            std::uint8_t    mVBlankStat     : 1;
            std::uint8_t    mOamStat        : 1;
            std::uint8_t    mLycStat        : 1;
            std::uint8_t                    : 1;
        };
    };

    union MonochromePaletteRegister final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t    mColor0 : 2;
            std::uint8_t    mColor1 : 2;
            std::uint8_t    mColor2 : 2;
            std::uint8_t    mColor3 : 2;
        };
    };

    union ColorPaletteIndexRegister final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t    mIndex          : 6;
            std::uint8_t                    : 1;
            std::uint8_t    mAutoIncrement  : 1;
        };
    };

    union VramDmaControlRegister final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t    mLength : 7;
            std::uint8_t    mIsHDma : 1;
        };
    };

    union PPUAuxillarySettingsRegister final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t    mVramBank           : 1;
            std::uint8_t    mObjectSortPriority : 1;
            std::uint8_t    mUseColorPalette    : 1;
            std::uint8_t                        : 5;
        };
    };

    union TileAttributes final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t    mColorPalette       : 3;
            std::uint8_t    mVramBank           : 1;
            std::uint8_t    mMonochromePalette  : 1;
            std::uint8_t    mXFlip              : 1;
            std::uint8_t    mYFlip              : 1;
            std::uint8_t    mBgwPriority        : 1;
        };
    };

    struct Object final
    {
        std::uint8_t        mYPos;
        std::uint8_t        mXPos;
        std::uint8_t        mTileIndex;
        TileAttributes      mAttributes;
    };
}

// Classes *********************************************************************

namespace G10::GB
{
    class G10_API PPU final
    {   
        friend class System;

    public: // Constructors & Destructor ***************************************

        explicit PPU (System& pSystem);

    public: // Methods *********************************************************

        auto Reset () -> void;
        auto Clock (const std::uint64_t& pCycle) -> bool;

    public: // Methods - Callbacks *********************************************

        auto SetFrameCallback (const FrameCallback& pCallback) -> void;
        auto SetScanlineDelegate (const ScanlineDelegate& pDelegate) -> void;
        auto SetCoincidenceDelegate (const CoincidenceDelegate& pDelegate) -> void;

    public: // Methods - Bus Access ********************************************

        auto ReadVideoRAM (std::uint32_t pRelAddress, std::uint8_t& pDataOut) -> bool;
        auto ReadOAM (std::uint32_t pRelAddress, std::uint8_t& pDataOut) -> bool;
        auto ReadColorRAM (std::uint32_t pRelAddress, std::uint8_t& pDataOut) -> bool;

        auto WriteVideoRAM (std::uint32_t pRelAddress, std::uint8_t pDataIn) -> bool;
        auto WriteOAM (std::uint32_t pRelAddress, std::uint8_t pDataIn) -> bool;
        auto WriteColorRAM (std::uint32_t pRelAddress, std::uint8_t pDataIn) -> bool;

    public: // Methods - Port Registers ****************************************

        auto ReadLCDC (std::uint8_t& pDataOut) -> bool;
        auto ReadSTAT (std::uint8_t& pDataOut) -> bool;
        auto ReadSCY (std::uint8_t& pDataOut) -> bool;
        auto ReadSCX (std::uint8_t& pDataOut) -> bool;
        auto ReadWY (std::uint8_t& pDataOut) -> bool;
        auto ReadWX (std::uint8_t& pDataOut) -> bool;
        auto ReadLY (std::uint8_t& pDataOut) -> bool;
        auto ReadLYC (std::uint8_t& pDataOut) -> bool;
        auto ReadBGP (std::uint8_t& pDataOut) -> bool;
        auto ReadOBP0 (std::uint8_t& pDataOut) -> bool;
        auto ReadOBP1 (std::uint8_t& pDataOut) -> bool;
        auto ReadBGPI (std::uint8_t& pDataOut) -> bool;
        auto ReadBGPD (std::uint8_t& pDataOut) -> bool;
        auto ReadOBPI (std::uint8_t& pDataOut) -> bool;
        auto ReadOBPD (std::uint8_t& pDataOut) -> bool;
        auto ReadPPUX (std::uint8_t& pDataOut) -> bool;
        auto ReadODMAS (std::uint8_t& pDataOut) -> bool;
        auto ReadVDMA6 (std::uint8_t& pDataOut) -> bool;

        auto WriteLCDC (std::uint8_t pDataIn) -> bool;
        auto WriteSTAT (std::uint8_t pDataIn) -> bool;
        auto WriteSCY (std::uint8_t pDataIn) -> bool;
        auto WriteSCX (std::uint8_t pDataIn) -> bool;
        auto WriteWY (std::uint8_t pDataIn) -> bool;
        auto WriteWX (std::uint8_t pDataIn) -> bool;
        auto WriteLYC (std::uint8_t pDataIn) -> bool;
        auto WriteBGP (std::uint8_t pDataIn) -> bool;
        auto WriteOBP0 (std::uint8_t pDataIn) -> bool;
        auto WriteOBP1 (std::uint8_t pDataIn) -> bool;
        auto WriteBGPI (std::uint8_t pDataIn) -> bool;
        auto WriteBGPD (std::uint8_t pDataIn) -> bool;
        auto WriteOBPI (std::uint8_t pDataIn) -> bool;
        auto WriteOBPD (std::uint8_t pDataIn) -> bool;
        auto WritePPUX (std::uint8_t pDataIn) -> bool;
        auto WriteODMAS () -> bool;
        auto WriteODMA1 (std::uint8_t pDataIn) -> bool;
        auto WriteODMA2 (std::uint8_t pDataIn) -> bool;
        auto WriteODMA3 (std::uint8_t pDataIn) -> bool;
        auto WriteVDMA0 (std::uint8_t pDataIn) -> bool;
        auto WriteVDMA1 (std::uint8_t pDataIn) -> bool;
        auto WriteVDMA2 (std::uint8_t pDataIn) -> bool;
        auto WriteVDMA3 (std::uint8_t pDataIn) -> bool;
        auto WriteVDMA4 (std::uint8_t pDataIn) -> bool;
        auto WriteVDMA5 (std::uint8_t pDataIn) -> bool;
        auto WriteVDMA6 (std::uint8_t pDataIn) -> bool;

    public: // Methods - Direct Memory Access **********************************

        auto IsOamDmaActive () const -> bool;
        auto IsVramDmaActive () const -> bool;

    public: // Methods - Rendering *********************************************

        auto RenderVideoRAM (bool pBank1) -> std::span<std::uint32_t>;
        auto RenderOAM () -> std::span<std::uint32_t>;

    public: // Methods - Accessors *********************************************

        inline auto GetVideoRAM (bool pBank1) -> std::span<std::uint8_t>
            { return (pBank1 == true) ? mVideoRAM1 : mVideoRAM0; }
        inline auto GetVideoRAM (bool pBank1) const -> std::span<const std::uint8_t>
            { return (pBank1 == true) ? mVideoRAM1 : mVideoRAM0; }
        inline auto GetOAM () -> std::span<std::uint8_t>
            { return mOAM; }
        inline auto GetOAM () const -> std::span<const std::uint8_t>
            { return mOAM; }
        inline auto GetColorRAM (bool pGetObjectCRAM) -> std::span<std::uint8_t>
            { return (pGetObjectCRAM == true) ? mObjColorRAM : mBgColorRAM; }
        inline auto GetColorRAM (bool pGetObjectCRAM) const -> std::span<const std::uint8_t>
            { return (pGetObjectCRAM == true) ? mObjColorRAM : mBgColorRAM; }
        inline auto GetFrameBuffer () -> FrameBuffer&
            { return mFrameBuffer; }
        inline auto GetFrameBuffer () const -> const FrameBuffer&
            { return mFrameBuffer; }

        inline auto AcknowledgeFrame () -> bool
        {
            if (mFrameJustFinished == true)
            {
                mFrameJustFinished = false;
                return true;
            }
            
            return false;
        }

    private: // Methods - Helpers **********************************************

        auto CheckForWindow () -> bool;
        auto CheckForStatInterrupt () -> bool;
        auto CheckForCoincidence () -> bool;
        auto IncrementScanline () -> void;
        auto UpdateStatLine (bool pJustEnteredVBlank) -> void;
        auto IsUsingColorPalette () -> bool;
        auto GetPixelColor (std::uint8_t pColorIndex, std::uint8_t pPaletteIndex, 
            bool pIsObject) -> std::uint32_t;
        auto GetObject (std::uint8_t pOamIndex) -> Object&;

    private: // Methods - Internal Buffer Access *******************************

        auto ReadVideoRAMInternal (std::uint8_t pBank, std::uint32_t pIndex) -> std::uint8_t;
        auto ReadOAMInternal (std::uint32_t pIndex) -> std::uint8_t;
        auto ReadColorRAMInternal (std::uint32_t pIndex, bool pIsBg) -> std::uint8_t;
        auto WriteVideoRAMInternal (std::uint8_t pBank, std::uint32_t pIndex, std::uint8_t pData) -> void;
        auto WriteOAMInternal (std::uint32_t pIndex, std::uint8_t pData) -> void;
        auto WriteColorRAMInternal (std::uint32_t pIndex, bool pIsBg, std::uint8_t pData) -> void;

    private: // Methods - Object Pixel Fetching ********************************

        auto CheckForObjectFetch () -> bool;
        auto PrefetchObjectTileAddress () -> void;
        auto FetchObjectTileData (bool pIsHighByte) -> void;
        auto PushObjectPixels () -> void;
        auto TickObjectPixelFetcher () -> void;

    private: // Methods - Pixel Tranfer ****************************************

        auto PreparePixelFetcher () -> void;
        auto FetchTileIndex () -> void;
        auto FetchTileData (bool pIsHighByte) -> void;
        auto PushPixels () -> void;
        auto TickPixelFetcher () -> void;
        auto TransferPixel () -> void;

    private: // Methods - Object Scan ******************************************

        auto ScanObject (std::uint8_t pOamIndex) -> void;
        auto SortScanlineObjects () -> void;
        auto ClearScanlineObjects () -> void;

    private: // Methods - Direct Memory Access *********************************

        auto TickOamDMA () -> void;
        auto TickVramDMA () -> void;

    private: // Methods - Display Mode Timing & Management *********************

        auto EnterDisplayMode (DisplayMode pMode) -> void;
        auto TickHorizontalBlank () -> void;
        auto TickVerticalBlank () -> void;
        auto TickObjectScan () -> void;
        auto TickPixelTransfer () -> void;

    private: // Constants & Enumerations ***************************************

        enum class FetchMode
        {
            Index,
            DataLow,
            DataHigh,
            Idle,
            Push
        };

    private: // Structures *****************************************************

        struct Pixel final
        {
            std::uint8_t    mColorIndex { 0 };
            std::uint8_t    mPaletteIndex { 0 };
            std::uint8_t    mObjectPriority { 0 };
            bool            mBgwPriority { false };
        };

        struct PixelFIFO final
        {
            std::array<Pixel, kFIFOCapacity>
                            mPixels;
            std::uint8_t    mSize { 0 };
            std::uint8_t    mHead { 0 };
            std::uint8_t    mTail { 0 };

        public: // Methods *****************************************************

            inline auto Push (const Pixel& pPixel) -> bool
            {
                if (mSize >= kFIFOCapacity)
                    { return false; }

                mPixels[mTail] = pPixel;
                mTail = (mTail + 1) % kFIFOCapacity;
                mSize++;

                return true;
            }

            inline auto Pop (Pixel* pOptPixelOut) -> bool
            {
                if (mSize == 0)
                    { return false; }

                if (pOptPixelOut != nullptr)
                    { *pOptPixelOut = mPixels[mHead]; }
                mHead = (mHead + 1) % kFIFOCapacity;
                mSize--;

                return true;
            }

            inline auto Clear () -> void
                { mSize = mHead = mTail = 0; }

        };

        struct PixelFetcher final
        {
            FetchMode               mMode;
            std::uint16_t           mTileMapBase;
            std::uint16_t           mTileMapAddress;
            bool                    mIsInWindow;
            std::uint8_t            mBgwIndex;
            TileAttributes          mBgwAttributes;
            std::uint16_t           mBgwDataBase;
            std::uint16_t           mBgwDataAddress;
            std::uint8_t            mBgwDataLow;
            std::uint8_t            mBgwDataHigh;
            bool                    mIsFetchingObject;
            std::uint16_t           mObjFetchMask;
            std::uint8_t            mObjOamIndex;
            std::uint8_t            mObjPriority;
            std::int8_t             mObjFetchStep;
            std::uint8_t            mObjPenaltyDots;
            std::uint16_t           mObjDataAddress;
            std::uint8_t            mObjDataLow;
            std::uint8_t            mObjDataHigh;
            std::uint8_t            mFetchX;
            std::uint8_t            mPixelsTransferred;
            std::uint8_t            mPixelsToDiscard;
            std::uint8_t            mStateDots;
            bool                    mInitialFetch;  // First tile fetch is discarded

        public: // Methods *****************************************************

            inline auto Reset () -> void
            {
                std::memset(this, 0, sizeof(*this));
                mObjPriority = 0xFF;
            }

        };

    private: // Members ********************************************************

        System& mSystem;

        // Callbacks
        FrameCallback       mFrameCallback;
        ScanlineDelegate    mScanlineDelegate;
        CoincidenceDelegate mCoincidenceDelegate;

        // Memory Buffers
        FrameBuffer mFrameBuffer;
        std::array<std::uint8_t, kVramBankSize> mVideoRAM0;
        std::array<std::uint8_t, kVramBankSize> mVideoRAM1;
        std::array<std::uint8_t, kOamSize> mOAM;
        std::array<std::uint8_t, kColorRamSize> mBgColorRAM;
        std::array<std::uint8_t, kColorRamSize> mObjColorRAM;
        std::array<std::uint32_t, kMonochromeColorCount> mMonochromeColors;
        std::span<std::uint8_t> mVideoRAM { mVideoRAM0 };

        // Port Registers - PPU
        DisplayControlRegister          mDisplayControl;
        DisplayStatusRegister           mDisplayStatus;
        std::uint8_t                    mViewportY { 0 };
        std::uint8_t                    mViewportX { 0 };
        std::uint8_t                    mWindowY { 0 };
        std::uint8_t                    mWindowX { 0 };
        std::uint8_t                    mCurrentScanline { 0 };
        std::uint8_t                    mScanlineCompare { 0 };
        MonochromePaletteRegister       mBgMonochromePalette;
        MonochromePaletteRegister       mObjMonochromePalette0;
        MonochromePaletteRegister       mObjMonochromePalette1;
        ColorPaletteIndexRegister       mBgColorPaletteIndex;
        ColorPaletteIndexRegister       mObjColorPaletteIndex;
        PPUAuxillarySettingsRegister    mPPUAuxillarySettings;

        // Port Registers - Direct Memory Access
        std::uint32_t                   mOamDmaSourceAddress { 0 };
        std::uint32_t                   mVramDmaSourceAddress { 0 };
        std::uint32_t                   mVramDmaDestinationAddress { 0 };
        VramDmaControlRegister          mVramDmaControl;

        // Internal State - Dot Counters
        std::uint32_t   mModeDots { 0 };
        std::uint32_t   mLineDots { 0 };
        std::uint32_t   mFrameDots { 0 };

        // Internal State - Object Scan
        std::array<std::uint8_t, kObjectsPerScanline> mScanlineObjectIndices;
        std::uint8_t mScanlineObjectCount;

        // Internal State - Pixel Transfer
        PixelFetcher mPixelFetcher;
        PixelFIFO mBgPixelFIFO;
        PixelFIFO mObjPixelFIFO;

        // Internal State - Window Layer
        bool            mWYConditionMet { false };
        bool            mWindowTriggered { false };
        bool            mWindowVisible { false };
        std::uint8_t    mWindowScanline { 0 };

        // Internal State - Direct Memory Access
        bool            mOamDmaActive { false };
        bool            mOamDmaRestarting { false };
        std::uint32_t   mOamDmaSource { 0 };
        std::uint8_t    mOamDmaBytesTransferred { 0 };
        std::uint8_t    mOamDmaDelay { 0 };
        std::uint8_t    mOamDmaDotCounter { 0 };
        bool            mVramDmaActive { false };
        std::uint32_t   mVramDmaSource { 0 };
        std::uint32_t   mVramDmaDestination { 0 };
        std::uint16_t   mVramDmaBlocksLeft { 0 };

        // Internal State - `LCD_STAT` Interrupt Line
        bool            mStatInterruptLine { false };

        // Internal State - Other
        bool            mFrameJustFinished { false };
        bool            mLcdJustEnabled { false };
        bool            mFirstLineMode3 { false };
        bool            mLyIncrementedEarly { false };
        bool            mVramReadBlocked { false };
        DisplayMode     mSpeedSwitchDM { DisplayMode::HorizontalBlank };

        // Internal State - VRAM/OAM Renders
        std::array<std::uint32_t, kVramRenderBufferSize> mRenderedVRAM0;
        std::array<std::uint32_t, kVramRenderBufferSize> mRenderedVRAM1;
        std::array<std::uint32_t, kOamRenderBufferSize> mRenderedOAM;

    };
}
