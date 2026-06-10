/**
 * @file    G10.ASM.Diagnostic.hpp
 * @brief   Contains declarations for the G10 Assembler's diagnostic class, and
 *          related definitions.
 */

#pragma once

// Includes ********************************************************************

#include <G10.ASM.Common.hpp>

// Constants & Enumerations ****************************************************

namespace G10::ASM
{
    enum class DiagnosticLevel
    {
        Verbose,
        Info,
        Warning,
        ExtraWarning,
        Error
    };
}

// Structures ******************************************************************

namespace G10::ASM
{
    struct DiagnosticReport final
    {
        DiagnosticLevel mLevel {};
        SourceLocation mLocation {};
        std::string mMessage {};
    };
}

// Types ***********************************************************************

namespace G10::ASM
{
    using DiagnosticCallback = std::function<void(const DiagnosticReport&)>;
}

// Classes *********************************************************************

namespace G10::ASM
{
    class G10_API Diagnostic final
    {
    public: // Methods - Reporting *********************************************

        template <typename... As>
        inline auto ReportVerbose (SourceLocation pLocation,
            std::string_view pFormat, As&&... pArgs) -> void
        {
            if (mVerboseEnabled == true)
            {
                Report(DiagnosticLevel::Verbose, pLocation, pFormat,
                    std::forward<As>(pArgs)...);
            }
        }

        template <typename... As>
        inline auto ReportVerbose (std::string_view pFormat, As&&... pArgs) -> void
        {
            if (mVerboseEnabled == true)
            {
                Report(DiagnosticLevel::Verbose, SourceLocation {}, pFormat,
                    std::forward<As>(pArgs)...);
            }
        }

        template <typename... As>
        inline auto ReportInfo (SourceLocation pLocation,
            std::string_view pFormat, As&&... pArgs) -> void
        {
            Report(DiagnosticLevel::Info, pLocation, pFormat,
                std::forward<As>(pArgs)...);
        }

        template <typename... As>
        inline auto ReportInfo (std::string_view pFormat, As&&... pArgs) -> void
        {
            Report(DiagnosticLevel::Info, SourceLocation {}, pFormat,
                std::forward<As>(pArgs)...);
        }

        template <typename... As>
        inline auto ReportWarning (SourceLocation pLocation,
            std::string_view pFormat, As&&... pArgs) -> void
        {
            if (mWarningsAreErrors == true)
            {
                Report(DiagnosticLevel::Error, pLocation, pFormat,
                    std::forward<As>(pArgs)...);
                mErrorCount++;
            }
            else
            {
                Report(DiagnosticLevel::Warning, pLocation, pFormat,
                    std::forward<As>(pArgs)...);
                mWarningCount++;
            }
        }

        template <typename... As>
        inline auto ReportWarning (std::string_view pFormat, As&&... pArgs) -> void
        {
            if (mWarningsAreErrors == true)
            {
                Report(DiagnosticLevel::Error, SourceLocation {}, pFormat,
                    std::forward<As>(pArgs)...);
                mErrorCount++;
            }
            else
            {
                Report(DiagnosticLevel::Warning, SourceLocation {}, pFormat,
                    std::forward<As>(pArgs)...);
                mWarningCount++;
            }
        }

        template <typename... As>
        inline auto ReportExtraWarning (SourceLocation pLocation,
            std::string_view pFormat, As&&... pArgs) -> void
        {
            if (mExtraWarningsEnabled == true)
            {
                if (mWarningsAreErrors == true)
                {
                    Report(DiagnosticLevel::Error, pLocation, pFormat,
                        std::forward<As>(pArgs)...);
                    mErrorCount++;
                    mWerrorCount++;
                }
                else
                {
                    Report(DiagnosticLevel::ExtraWarning, pLocation, pFormat,
                        std::forward<As>(pArgs)...);
                    mWarningCount++;
                }
            }
        }

        template <typename... As>
        inline auto ReportExtraWarning (std::string_view pFormat, As&&... pArgs) -> void
        {
            if (mExtraWarningsEnabled == true)
            {
                if (mWarningsAreErrors == true)
                {
                    Report(DiagnosticLevel::Error, SourceLocation {}, pFormat,
                        std::forward<As>(pArgs)...);
                    mErrorCount++;
                    mWerrorCount++;
                }
                else
                {
                    Report(DiagnosticLevel::ExtraWarning, SourceLocation {}, pFormat,
                        std::forward<As>(pArgs)...);
                    mWarningCount++;
                }

            }
        }

        template <typename... As>
        inline auto ReportError (SourceLocation pLocation,
            std::string_view pFormat, As&&... pArgs) -> void
        {
            Report(DiagnosticLevel::Error, pLocation, pFormat,
                std::forward<As>(pArgs)...);
            mErrorCount++;
        }

        template <typename... As>
        inline auto ReportError (std::string_view pFormat, As&&... pArgs) -> void
        {
            Report(DiagnosticLevel::Error, SourceLocation {}, pFormat,
                std::forward<As>(pArgs)...);
            mErrorCount++;
        }

    public: // Methods - Accessors *********************************************

        inline auto SetCallback (const DiagnosticCallback& pCallback) -> void
            { mCallback = pCallback; }
        inline auto GetReports () const -> const std::vector<DiagnosticReport>&
            { return mReports; }
        inline auto GetWarningCount () const -> std::size_t
            { return mWarningCount; }
        inline auto GetWerrorCount () const -> std::size_t
            { return mWerrorCount; }
        inline auto GetErrorCount () const -> std::size_t
            { return mErrorCount; }
        inline auto IsVerboseEnabled () const -> bool
            { return mVerboseEnabled; }
        inline auto SetVerboseEnabled (bool pEnabled) -> void
            { mVerboseEnabled = pEnabled; }
        inline auto IsExtraWarningsEnabled () const -> bool
            { return mExtraWarningsEnabled; }
        inline auto SetExtraWarningsEnabled (bool pEnabled) -> void
            { mExtraWarningsEnabled = pEnabled; }
        inline auto WarningsAreErrors () const -> bool
            { return mWarningsAreErrors; }
        inline auto SetWarningsAreErrors (bool pEnabled) -> void
            { mWarningsAreErrors = pEnabled; }

    private: // Methods ********************************************************

        template <typename... As>
        inline auto Report (DiagnosticLevel pLevel, SourceLocation pLocation,
            std::string_view pFormat, As&&... pArgs) -> void
        {
            const auto& report = mReports.emplace_back(
                DiagnosticReport {
                    .mLevel = pLevel,
                    .mLocation = pLocation,
                    .mMessage = std::vformat(
                        pFormat,
                        std::make_format_args(pArgs...)
                    )
                }
            );

            if (mCallback != nullptr)
                { mCallback(report); }
        }

    private: // Members ********************************************************

        DiagnosticCallback mCallback { nullptr };
        std::vector<DiagnosticReport> mReports {};
        std::size_t mWarningCount { 0 };
        std::size_t mErrorCount { 0 };
        std::size_t mWerrorCount { 0 };
        bool mVerboseEnabled { false };
        bool mExtraWarningsEnabled { false };
        bool mWarningsAreErrors { false };

    };
}
