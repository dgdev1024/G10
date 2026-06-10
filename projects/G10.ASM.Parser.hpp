/**
 * @file    G10.ASM.Parser.hpp
 * @brief   Contains declarations for the G10 Assembler's parser component,
 *          and related definitions.
 */

// Includes ********************************************************************

#include <G10.ASM.Diagnostic.hpp>
#include <G10.ASM.TokenCursor.hpp>
#include <G10.ASM.Syntax.hpp>

// Classes *********************************************************************

namespace G10::ASM
{
    class G10_API Parser final
    {
    public: // Constructors & Destructor ***************************************

        explicit Parser (Diagnostic& pDiag);

    public: // Methods - Input & Output ****************************************

        auto ParseInput (TokenSlice pSlice) -> bool;
        auto GetOutput () const -> const SyntaxModule&;

    private: // Methods - Dispatch *********************************************
    
        auto DispatchKeyword (const Keyword& pKeyword,
            const SourceLocation& pLocation, TokenCursor& pCursor) -> bool;
        auto DispatchDirective (const Keyword& pKeyword,
            const SourceLocation& pLocation, TokenCursor& pCursor) -> bool;

    private: // Methods - Dispatch - Directives ********************************

        auto DispatchByteDirective (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> bool;
        auto DispatchWordDirective (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> bool;
        auto DispatchDoubleWordDirective (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> bool;
        auto DispatchStringDirective (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> bool;
        auto DispatchSpaceDirective (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> bool;
        auto DispatchIncbinDirective (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> bool;
        auto DispatchExportDirective (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> bool;
        auto DispatchImportDirective (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> bool;
        auto DispatchSectionDirective (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> bool;
        auto DispatchOrgDirective (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> bool;
        auto DispatchAlignDirective (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> bool;

    private: // Methods - Dispatch - Statements ********************************

        auto DispatchLabelStatement (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> bool;
        auto DispatchInstructionStatement (const Keyword& pKeyword,
            const SourceLocation& pLocation, TokenCursor& pCursor) -> bool;

    private: // Methods - Evaluate *********************************************

        auto EvaluateIntegerExpression (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> std::shared_ptr<IntegerExpressionNode>;
        auto EvaluateStringExpression (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> std::shared_ptr<StringExpressionNode>;
        auto EvaluateRegisterExpression (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> std::shared_ptr<RegisterExpressionNode>;
        auto EvaluateConditionExpression (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> std::shared_ptr<ConditionExpressionNode>;
        auto EvaluateLabelExpression (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> std::shared_ptr<LabelExpressionNode>;
        auto EvaluateSectionNameExpression (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> std::shared_ptr<SectionNameExpressionNode>;
        auto EvaluateBinaryExpression (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> std::shared_ptr<ExpressionNode>;
        auto EvaluatePointerExpression (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> std::shared_ptr<PointerExpressionNode>;
        auto EvaluateOperandExpression (const SourceLocation& pLocation, 
            TokenCursor& pCursor) -> std::shared_ptr<ExpressionNode>;

    private: // Members ********************************************************

        Diagnostic& mDiag;
        SyntaxModule mOutput {};

    };
}