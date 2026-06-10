/**
 * @file    G10.ASM.Syntax.hpp
 * @brief   Contains declarations for the G10 Assembler's syntax node
 *          structures, and related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.ASM.Token.hpp>

// Structures ******************************************************************

namespace G10::ASM
{
    struct G10_API SyntaxNode
    {
        SourceLocation mLocation {};

    public:
        virtual auto Stringify (std::size_t pIndent = 0) const -> std::string = 0;

    };

    // Expression Nodes ********************************************************

    struct G10_API ExpressionNode : SyntaxNode {};

    struct G10_API IntegerExpressionNode final : ExpressionNode
    {
        std::uint32_t mValue {};

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;

    };

    struct G10_API StringExpressionNode final : ExpressionNode
    {
        std::string mValue {};

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;

    };

    using StringAndRegister = std::pair<std::string, CPU::Register>;

    struct G10_API RegisterExpressionNode final : ExpressionNode
    {
        CPU::Register mRegister {};
        std::string mName {};

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;

    };

    struct G10_API ConditionExpressionNode final : ExpressionNode
    {
        CPU::Condition mCondition {};
        std::string mName {};

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;
    };

    struct G10_API LabelExpressionNode final : ExpressionNode
    {
        std::string mSymbol {};

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;

    };

    struct G10_API SectionNameExpressionNode final : ExpressionNode
    {
        SectionName mSectionName {};
        std::string mName {};

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;

    };

    struct G10_API BinaryExpressionNode final : ExpressionNode
    {
        std::shared_ptr<ExpressionNode> mLeft {};
        std::shared_ptr<ExpressionNode> mRight {};
        bool mIsSubtraction { false };

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;

    };

    struct G10_API PointerExpressionNode final : ExpressionNode
    {
        std::shared_ptr<ExpressionNode> mExpression {};

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;

    };

    // Operand Nodes ***********************************************************

    struct G10_API OperandNode : SyntaxNode {};

    // Directive Nodes *********************************************************

    struct G10_API DirectiveNode : SyntaxNode {};

    struct G10_API ByteDirectiveNode final : DirectiveNode
    {
        stx::sptr_vector<ExpressionNode> mOperands {};

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;

    };

    struct G10_API WordDirectiveNode final : DirectiveNode
    {
        stx::sptr_vector<ExpressionNode> mOperands {};

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;

    };

    struct G10_API DoubleWordDirectiveNode final : DirectiveNode
    {
        stx::sptr_vector<ExpressionNode> mOperands {};

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;

    };

    struct G10_API StringDirectiveNode final : DirectiveNode
    {
        stx::sptr_vector<ExpressionNode> mOperands {};

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;

    };

    struct G10_API SpaceDirectiveNode final : DirectiveNode
    {
        stx::sptr_vector<ExpressionNode> mOperands {};

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;

    };

    struct G10_API IncbinDirectiveNode final : DirectiveNode
    {
        std::shared_ptr<StringExpressionNode> mFilename {};
        std::shared_ptr<IntegerExpressionNode> mOffset {};
        std::shared_ptr<IntegerExpressionNode> mSize {};

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;
        
    };

    struct G10_API ExportDirectiveNode final : DirectiveNode
    {
        stx::sptr_vector<LabelExpressionNode> mLabels {};

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;

    };

    struct G10_API ImportDirectiveNode final : DirectiveNode
    {
        stx::sptr_vector<LabelExpressionNode> mLabels {};

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;

    };

    struct G10_API SectionDirectiveNode final : DirectiveNode
    {
        std::shared_ptr<StringExpressionNode> mDisplayName {};
        std::shared_ptr<SectionNameExpressionNode> mSectionName {};
        std::shared_ptr<ExpressionNode> mFixedAddress {};

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;

    };

    struct G10_API OrgDirectiveNode final : DirectiveNode
    {
        std::shared_ptr<IntegerExpressionNode> mAddress;

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;

    };

    struct G10_API AlignDirectiveNode final : DirectiveNode
    {
        std::shared_ptr<IntegerExpressionNode> mBoundary;

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;
        
    };

    // Statement Nodes *********************************************************

    struct G10_API StatementNode : SyntaxNode {};

    struct G10_API LabelStatementNode final : StatementNode
    {
        LabelExpressionNode mLabelExpr {};

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;

    };

    struct G10_API InstructionStatementNode final : StatementNode
    {
        CPU::InstructionType mType {};
        stx::sptr_vector<ExpressionNode> mOperands {};
        std::string mName {};

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;

    };

    // Module Node *************************************************************
    
    struct G10_API SyntaxModule final : SyntaxNode
    {
        stx::sptr_vector<SyntaxNode> mNodes {};

    public:
        auto Stringify (std::size_t pIndent = 0) const -> std::string override;

    };
}
