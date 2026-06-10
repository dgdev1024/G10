/**
 * @file    G10.ASM.Preprocessor.hpp
 * @brief   Contains declarations for the G10 Assembler's preprocessor
 *          class, and related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.ASM.TokenCursor.hpp>
#include <G10.ASM.Diagnostic.hpp>
#include <G10.ASM.Preprocessor.Value.hpp>

// Types ***********************************************************************

namespace G10::ASM
{
    using PreprocessorMacroArgument = std::variant<
        std::monostate,
        PreprocessorValue,
        TokenSlice
    >;
}

// Constants & Enumerations ****************************************************

namespace G10::ASM
{
    enum class PreprocessStatus
    {
        OK,
        Continue,
        Break,
        Return,
        Error
    };
}

// Structures ******************************************************************

namespace G10::ASM
{
    struct PreprocessorSymbol final
    {
        PreprocessorValue   mValue {};
        bool                mIsConstant { false };
        SourceLocation      mLocation {};
    };

    struct PreprocessorMacro final
    {
        std::string                 mName {};
        std::vector<std::string>    mNamedParameters {};
        std::vector<Token>          mBody {};
        PreprocessorInteger         mInvocationCount { 0 };
        SourceLocation              mLocation {};
    };

    struct PreprocessorMacroCall final
    {
        stx::optional_ref<PreprocessorMacro>    mMacro {};
        std::vector<PreprocessorMacroArgument>  mAllArguments {};
        std::deque<PreprocessorMacroArgument>   mArguments {};
        SourceLocation                          mLocation {};
    };

    struct PreprocessorConditional final
    {
        TokenSlice          mExprSlice {};
        TokenSlice          mBodySlice {};
    };
}

// Classes *********************************************************************

namespace G10::ASM
{
    class G10_API Preprocessor final
    {
    public: // Constructors & Destructor ***************************************

        explicit Preprocessor (Diagnostic& pDiag);

    public: // Methods - Input & Output ****************************************

        auto PreprocessInput (TokenSlice pSlice) -> bool;
        auto GetOutput () const -> const std::string&;

    public: // Methods - Options ***********************************************

        auto SetInterpolationDepthLimit (std::size_t pInterpolationDepthLimit) -> void;
        auto SetLoopDepthLimit (std::size_t pLoopDepthLimit) -> void;
        auto SetIncludeDepthLimit (std::size_t pIncludeDepthLimit) -> void;
        auto SetRecursionDepthLimit (std::size_t pRecursionDepthLimit) -> void;
        auto SetIncludeDirectories (const std::vector<std::string>& pIncludeDirs) -> void;

    private: // Methods - Preprocess *******************************************

        auto Preprocess (TokenSlice pSlice) -> PreprocessStatus;

    private: // Methods - Helpers **********************************************

        auto AcknowledgeStatus () -> PreprocessStatus;
        auto ResetStatus () -> void;

    private: // Methods - Dispatch *********************************************

        auto DispatchKeyword (TokenCursor& pCursor, const Token& pToken) -> bool;
        auto DispatchDirective (TokenCursor& pCursor, const Keyword& pKeyword, const SourceLocation& pLocation) -> bool;
        auto DispatchIdentifier (TokenCursor& pCursor, const Token& pToken) -> bool;
        auto DispatchParameter (TokenCursor& pCursor, const Token& pToken) -> bool;
        auto DispatchPassthrough (TokenCursor& pCursor, const Token& pToken) -> bool;

    private: // Methods - Dispatch - Print & Debug *****************************

        auto DispatchPrint (TokenCursor& pCursor, const SourceLocation& pLocation, bool pNewline) -> bool;
        auto DispatchInfo (TokenCursor& pCursor, const SourceLocation& pLocation) -> bool;
        auto DispatchWarning (TokenCursor& pCursor, const SourceLocation& pLocation) -> bool;
        auto DispatchError (TokenCursor& pCursor, const SourceLocation& pLocation) -> bool;
        auto DispatchAssert (TokenCursor& pCursor, const SourceLocation& pLocation) -> bool;

    private: // Methods - Dispatch - Symbols ***********************************

        auto DispatchLet (TokenCursor& pCursor, const SourceLocation& pLocation) -> bool;
        auto DispatchConst (TokenCursor& pCursor, const SourceLocation& pLocation) -> bool;

    private: // Methods - Dispatch - Conditionals ******************************
    
        auto DispatchIf (TokenCursor& pCursor, const SourceLocation& pLocation) -> bool;

    private: // Methods - Dispatch - Loops *************************************

        auto DispatchRepeat (TokenCursor& pCursor, const SourceLocation& pLocation) -> bool;
        auto DispatchWhile (TokenCursor& pCursor, const SourceLocation& pLocation) -> bool;
        auto DispatchFor (TokenCursor& pCursor, const SourceLocation& pLocation) -> bool;
        auto DispatchContinue (TokenCursor& pCursor, const SourceLocation& pLocation) -> bool;
        auto DispatchBreak (TokenCursor& pCursor, const SourceLocation& pLocation) -> bool;

    private: // Methods - Dispatch - Macros ************************************

        auto DispatchMacro (TokenCursor& pCursor, const SourceLocation& pLocation) -> bool;
        auto DispatchMacroCall (TokenCursor& pCursor, const SourceLocation& pLocation,
            PreprocessorMacro& pMacro) -> bool;
        auto DispatchShift (TokenCursor& pCursor, const SourceLocation& pLocation) -> bool;
        auto DispatchReturn (TokenCursor& pCursor, const SourceLocation& pLocation) -> bool;

    private: // Methods - Dispatch - Include ***********************************

        auto DispatchInclude (TokenCursor& pCursor, const SourceLocation& pLocation) -> bool;
        auto DispatchOnce (TokenCursor& pCursor, const SourceLocation& pLocation) -> bool;

    private: // Methods - Expressions - Helpers ********************************

        auto CollectAndEvaluate (TokenCursor& pCursor) -> PreprocessorValue;
    
    private: // Methods - Expressions - Applicators ****************************

        auto ApplyCompound (PreprocessorValue& pLeft, 
            const PreprocessorValue& pRight, const Token& pOperator) -> PreprocessorValue&;
        auto ApplyBinary (const PreprocessorValue& pLeft, 
            const PreprocessorValue& pRight, const Token& pOperator) -> PreprocessorValue;
        auto ApplyUnary (const PreprocessorValue& pRight, 
            const Token& pOperator) -> PreprocessorValue;

    private: // Methods - Expressions - Arithmetic *****************************

        auto EvaluateExpression (TokenCursor& pCursor) -> PreprocessorValue;
        auto EvaluateLogicalOr (TokenCursor& pCursor) -> PreprocessorValue;
        auto EvaluateLogicalAnd (TokenCursor& pCursor) -> PreprocessorValue;
        auto EvaluateBitwiseOr (TokenCursor& pCursor) -> PreprocessorValue;
        auto EvaluateBitwiseXor (TokenCursor& pCursor) -> PreprocessorValue;
        auto EvaluateBitwiseAnd (TokenCursor& pCursor) -> PreprocessorValue;
        auto EvaluateEquality (TokenCursor& pCursor) -> PreprocessorValue;
        auto EvaluateComparison (TokenCursor& pCursor) -> PreprocessorValue;
        auto EvaluateShift (TokenCursor& pCursor) -> PreprocessorValue;
        auto EvaluateAdditive (TokenCursor& pCursor) -> PreprocessorValue;
        auto EvaluateMultiplicative (TokenCursor& pCursor) -> PreprocessorValue;
        auto EvaluateExponent (TokenCursor& pCursor) -> PreprocessorValue;
        auto EvaluateUnary (TokenCursor& pCursor) -> PreprocessorValue;
        auto EvaluatePrimary (TokenCursor& pCursor) -> PreprocessorValue;

    private: // Methods - Expressions - Symbols ********************************

        auto EvaluateSymbol (const std::string& pName) -> PreprocessorValue;
        auto EvaluateBuiltinSymbol (const SourceLocation& pLocation,
            const PreprocessorBuiltinSymbol& pSymbol) -> PreprocessorValue;

    private: // Methods - Expressions - Functions ******************************

        auto EvaluateBuiltinFunction (TokenCursor& pCursor,
            const PreprocessorFunction& pFunction) -> PreprocessorValue;

    private: // Methods - Expressions - Interpolation **************************

        auto InterpolateString (const SourceLocation& pLocation,
            const std::string& pText, std::size_t pDepth = 0)
                -> std::optional<std::string>;
        auto InterpolateIdentifier (const SourceLocation& pLocation,
            const std::string& pText, std::size_t pDepth = 0)
                -> std::optional<std::string>;

    private: // Methods - Text Output Emission *********************************

        auto EmitNewline () -> void;
        auto EmitText (std::string_view pText) -> void;
        auto EmitToken (const Token& pToken) -> void;
        auto EmitValue (const PreprocessorValue& pValue) -> void;

    private: // Members ********************************************************

        Diagnostic& mDiag;

        std::size_t mLimitInterpolationDepth    { kDefaultInterpolationDepth };
        std::size_t mLimitLoopDepth             { kDefaultLoopDepth };
        std::size_t mLimitIncludeDepth          { kDefaultIncludeDepth };
        std::size_t mLimitRecursionDepth        { kDefaultRecursionDepth };

        std::size_t         mInterpolationDepth         { 0 };
        std::size_t         mLoopDepth                  { 0 };
        std::size_t         mIncludeDepth               { 0 };
        std::size_t         mRecursionDepth             { 0 };
        std::int64_t        mUniqueCounter              { 1 };
        PreprocessStatus    mPendingStatus              { PreprocessStatus::OK };
        bool                mPassthroughExpr            { false };

        std::vector<fs::path> mIncludeDirs {};
        std::unordered_map<std::string, PreprocessorSymbol> mSymbols {};
        std::unordered_map<std::string, PreprocessorMacro> mMacros {};
        std::unordered_set<std::string> mOnceFiles {};
        std::vector<PreprocessorMacroCall> mMacroCallStack {};

        std::string mOutput { "" };

    };
}
