/**
 * @file    G10.CPU.Core.hpp
 * @brief   Contains declarations for the G10 CPU Core class, and related
 *          definitions.
 */

// Includes ********************************************************************

#include <G10.CPU.IBus.hpp>

// Structures & Unions *********************************************************

namespace G10::CPU
{
    struct Instruction final
    {
        std::uint8_t        mOpcode { 0 };
        std::uint8_t        mParamX { 0 };
        std::uint8_t        mParamY { 0 };

    public: // Constructors & Destructor ***************************************

        inline Instruction () = default;
        inline explicit Instruction (std::uint16_t pRaw) :
            mOpcode     { static_cast<std::uint8_t>((pRaw >> 8) & 0xFF) },
            mParamX     { static_cast<std::uint8_t>((pRaw >> 4) & 0x0F) },
            mParamY     { static_cast<std::uint8_t>((pRaw >> 0) & 0x0F) }
        {}

    public: // Methods *********************************************************

        inline auto CombinedParam () const -> std::uint8_t
        {
            return ((mParamX & 0xF) << 4) | (mParamY & 0xF);
        }

    public: // Operators *******************************************************

        inline auto operator= (std::uint16_t pRaw) -> Instruction&
        {
            mOpcode = static_cast<std::uint8_t>((pRaw >> 8) & 0xFF);
            mParamX = static_cast<std::uint8_t>((pRaw >> 4) & 0x0F);
            mParamY = static_cast<std::uint8_t>((pRaw >> 0) & 0x0F);
            return *this;   
        }

        inline operator std::uint16_t () const
        {
            return
                static_cast<std::uint16_t>(mOpcode) << 8 |
                static_cast<std::uint16_t>(mParamX & 0xF) << 4 |
                static_cast<std::uint16_t>(mParamY & 0xF);            
        }

    };

    union FlagsRegister final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t                : 3;
            std::uint8_t    mOverflow   : 1;
            std::uint8_t    mCarry      : 1;
            std::uint8_t    mHalfCarry  : 1;
            std::uint8_t    mSubtract   : 1;
            std::uint8_t    mZero       : 1;
        };
    };

    union SpeedRegister final
    {
        std::uint8_t        mValue;
        struct
        {
            std::uint8_t    mArmed      : 1;
            std::uint8_t    mActive     : 1;
            std::uint8_t                : 5;
            std::uint8_t    mHigh       : 1;
        };
    };
}

// Types ***********************************************************************

namespace G10::CPU
{
    class Core;

    // Hot-path callbacks: use raw function pointers for performance
    using InstructionDecodeDelegate = std::function<bool (Core&, Instruction&)>;
    using InstructionExecuteCallback = std::function<void (const Core&, const Instruction&)>;
    using ThrottleCallback = std::function<void (const Core&, const double&, const std::uint64_t&)>;
    
    // Non-critical callbacks: keep as std::function for flexibility
    using InterruptServiceCallback = std::function<void(const Core&, std::uint8_t)>;
    using StoppedDelegate = std::function<void(Core&)>;
}

// Classes *********************************************************************

namespace G10::CPU
{
    class G10_API Core final
    {
    private: // Friends ********************************************************

        friend class Executive;

    public: // Methods *********************************************************

        explicit Core (IBus& pBus);
        auto Reset () -> void;
        auto Wake () -> void;

        inline auto IsStopped () const -> bool
            { return mStopFlag == true || mDoubleFaultFlag == true; }
        inline auto IsSwitchingSpeed () const -> bool
            { return mSpeedRegister.mActive == true; }
        inline auto IsHighSpeed () const -> bool
            { return mSpeedRegister.mHigh == true; }
        inline auto GetTicks () const -> const std::uint64_t&
            { return mTicksConsumed; }
        inline auto GetRegister (std::uint8_t pIndex) const -> std::uint32_t
            { return mRegisters[pIndex & 0xF]; }
        inline auto GetProgramCounter () const -> std::uint32_t
            { return mProgramCounter; }
        inline auto GetStackPointer () const -> std::uint32_t
            { return mStackPointer; }
        inline auto GetFlagsRegister () const -> std::uint8_t
            { return mFlagsRegister.mValue; }
        inline auto SetProgramCounter (std::uint32_t pValue) -> void
            { mProgramCounter = pValue; }

        inline auto SetClockSpeed (const double& pHertz) -> void
        {
            mClockSpeed = 1.0 / ((pHertz > 0.0) ? pHertz : kDefaultClockSpeed);
            mHighClockSpeed = mClockSpeed / 2.0;
        }

    public: // Methods - Callbacks *********************************************

        auto SetInstructionDecodeDelegate (const InstructionDecodeDelegate& delegate) -> void;
        auto SetInstructionExecuteCallback (const InstructionExecuteCallback& callback) -> void;
        auto SetInterruptServiceCallback (const InterruptServiceCallback& callback) -> void;
        auto SetStoppedDelegate (const StoppedDelegate& delegate) -> void;
        auto SetThrottleCallback (const ThrottleCallback& callback) -> void;

    public: // Methods - Timing Management *************************************

        auto AddTickCycles (std::uint64_t pCycles) -> bool;
        auto AddMachineCycles (std::uint64_t pCycles) -> bool;
        auto FlushTickCycles () -> bool;

    public: // Methods - Interrupts and Exceptions *****************************

        auto RequestInterrupt (std::uint8_t pLine) -> bool;
        auto RaiseException (Exception pException) -> bool;
        auto RaiseException (std::uint8_t pException) -> bool;

    public: // Methods - General-Purpose Registers *****************************

        auto ReadRegisterDW (std::uint8_t pIndex, std::uint32_t& pValueOut) const -> bool;
        auto ReadRegisterW  (std::uint8_t pIndex, std::uint16_t& pValueOut) const -> bool;
        auto ReadRegisterHB (std::uint8_t pIndex, std::uint8_t&  pValueOut) const -> bool;
        auto ReadRegisterLB (std::uint8_t pIndex, std::uint8_t&  pValueOut) const -> bool;

        auto WriteRegisterDW (std::uint8_t pIndex, std::uint32_t pValue) -> bool;
        auto WriteRegisterW  (std::uint8_t pIndex, std::uint16_t pValue) -> bool;
        auto WriteRegisterHB (std::uint8_t pIndex, std::uint8_t  pValue) -> bool;
        auto WriteRegisterLB (std::uint8_t pIndex, std::uint8_t  pValue) -> bool;

    public: // Methods - Special Purpose Registers *****************************

        auto ReadProgramCounter (std::uint32_t& pValueOut) const -> bool;
        auto ReadStackPointer (std::uint32_t& pValueOut) const -> bool;
        auto ReadFlagsRegister (std::uint8_t& pValueOut) const -> bool;
        auto ReadFlag (Flag pFlag, bool& pValueOut) const -> bool;

        auto WriteProgramCounter (std::uint32_t pValue) -> bool;
        auto WriteStackPointer (std::uint32_t pValue) -> bool;
        auto WriteFlagsRegister (std::uint8_t pValue) -> bool;
        auto WriteFlag (Flag pFlag, bool pValue) -> bool;

    public: // Methods - Hardware Registers ************************************

        auto ReadHardwareRegister (HardwareRegister pRegister, std::uint8_t& pValueOut) -> bool;
        auto WriteHardwareRegister (HardwareRegister pRegister, std::uint8_t pValue) -> bool;

    public: // Methods - Bus Access ********************************************

        auto ReadMemoryDW (std::uint32_t pAddress, std::uint32_t& pValueOut, bool pBigEndian = false) -> bool;
        auto ReadMemoryW  (std::uint32_t pAddress, std::uint16_t& pValueOut, bool pBigEndian = false) -> bool;
        auto ReadMemoryB  (std::uint32_t pAddress, std::uint8_t&  pValueOut) -> bool;

        auto WriteMemoryDW (std::uint32_t pAddress, std::uint32_t pValue, bool pBigEndian = false) -> bool;
        auto WriteMemoryW  (std::uint32_t pAddress, std::uint16_t pValue, bool pBigEndian = false) -> bool;
        auto WriteMemoryB  (std::uint32_t pAddress, std::uint8_t  pValue) -> bool;

        auto FetchMemoryDW (std::uint32_t& pValueOut, bool pBigEndian = false) -> bool;
        auto FetchMemoryW  (std::uint16_t& pValueOut, bool pBigEndian = false) -> bool;
        auto FetchMemoryB  (std::uint8_t&  pValueOut) -> bool;

        auto PopStackDW (std::uint32_t& pValueOut, bool pBigEndian = false) -> bool;
        auto PushStackDW (std::uint32_t& pValue, bool pBigEndian = false) -> bool;

    private: // Methods - Bus Access Timing ************************************

        auto ReadFromBus (std::uint32_t pAddress, std::uint8_t& pValueOut) -> bool;
        auto WriteToBus (std::uint32_t pAddress, std::uint8_t pValue) -> bool;

    private: // Members ********************************************************

        IBus& mBus;

        double mClockSpeed { kDefaultClockSpeed };
        double mHighClockSpeed { kDefaultHighClockSpeed };

        InstructionDecodeDelegate mInstructionDecodeDelegate { nullptr };
        InstructionExecuteCallback mInstructionExecuteCallback { nullptr }; 
        InterruptServiceCallback mInterruptServiceCallback { nullptr };
        StoppedDelegate mStoppedDelegate { nullptr };
        ThrottleCallback mThrottleCallback { nullptr };

        std::uint64_t mTicksPending { 0 };
        std::uint64_t mTicksConsumed { 0 };

        std::array<std::uint32_t, kRegisterCount> mRegisters;
        std::uint32_t mProgramCounter { kInitialProgramCounter };
        std::uint32_t mStackPointer { kInitialStackPointer };
        FlagsRegister mFlagsRegister { .mValue = 0 };

        std::uint32_t mInterruptEnableRegister { 1 };
        std::uint32_t mInterruptRequestRegister { 0 };
        SpeedRegister mSpeedRegister { .mValue = 0 };
        
        Instruction mInstructionRegister { 0 };
        std::uint32_t mInstructionPointer { 0 };
        std::uint32_t mImmediateData { 0 };
        std::uint32_t mImmediatePointer { 0 };

        Exception mException { Exception::None };
        Exception mExceptionPending { Exception::None };

        bool mInterruptMasterEnable { false };
        bool mInterruptMasterPending { false };
        bool mHaltFlag { false };
        bool mStopFlag { false };
        bool mJustStopped { false };
        bool mDoubleFaultFlag { false };

    };
}