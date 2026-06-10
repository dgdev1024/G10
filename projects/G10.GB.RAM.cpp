/**
 * @file    G10.GB.RAM.cpp
 * @brief   Contains implementations for the G10.Boy's general-purpose RAM
 *          component, and related definitions.
 */

// Includes ********************************************************************

#include <G10.GB.System.hpp>

// Public - Constructors & Destructor ******************************************

namespace G10::GB
{
    RAM::RAM (System& pSystem) :
        mSystem     { pSystem }
    {
    }
}

// Public - Methods ************************************************************

namespace G10::GB
{
    auto RAM::Reset () -> void
    {
        std::fill(mWorkRAM.begin(), mWorkRAM.end(), 0x00);
        std::fill(mExternalRAM.begin(), mExternalRAM.end(), 0x00);
        std::fill(mQuickRAM.begin(), mQuickRAM.end(), 0x00);
    }

    auto RAM::ResizeWorkRAM (std::uint32_t pSize) -> bool
    {
        if (pSize < kMinWramSize || pSize > kMaxWramSize)
            { return false; }

        mWorkRAM.resize(pSize);
        return true;
    }

    auto RAM::ResizeExternalRAM (std::uint32_t pSize) -> bool
    {
        if (pSize == 0)
        {
            mExternalRAM.clear();
            return true;
        }
        else if (pSize < kMinSramSize || pSize > kMaxSramSize)
            { return false; }

        mExternalRAM.resize(pSize);
        return true;
    }

    auto RAM::LoadExternalRAM (const fs::path& pPath) -> stx::expect_good
    {
        auto resolved = fs::absolute(pPath).lexically_normal();
        if (fs::exists(resolved) == false)
        {
            return stx::fmt_unexpected(
                "External RAM file '{}' not found.",
                resolved.string());
        }
        else if (fs::is_regular_file(resolved) == false)
        {
            return stx::fmt_unexpected(
                "Path '{}' does not resolve to a regular file.",
                resolved.string());
        }

        auto fileSize = fs::file_size(resolved);
        if (fileSize != mExternalRAM.size())
        {
            return stx::fmt_unexpected(
                "File size mismatch for external RAM file '{}'. "
                "Expected {}, got {}.",
                resolved.string(),
                mExternalRAM.size(),
                fileSize);
        }

        std::fstream file { resolved, std::ios::in | std::ios::binary };
        if (file.is_open() == false)
        {
            return stx::fmt_unexpected(
                "Could not open external RAM file '{}' for reading.",
                resolved.string());
        }

        file.read(reinterpret_cast<char*>(mExternalRAM.data()), mExternalRAM.size());
        file.close();

        return {};
    }

    auto RAM::SaveExternalRAM (const fs::path& pPath) -> stx::expect_good
    {
        auto resolved = fs::absolute(pPath).lexically_normal();
        std::fstream file { resolved, std::ios::out | std::ios::binary };
        if (file.is_open() == false)
        {
            return stx::fmt_unexpected(
                "Could not open external RAM file '{}' for writing.",
                resolved.string());
        }

        file.write(reinterpret_cast<const char*>(mExternalRAM.data()), mExternalRAM.size());
        file.close();

        return {};
    }
}

// Public Methods - Bus Access *************************************************

namespace G10::GB
{
    auto RAM::ReadWorkRAM (std::uint32_t pRelAddress, std::uint8_t& pDataOut) -> bool
    {
        if (pRelAddress >= mWorkRAM.size())
            { return false; }

        pDataOut = mWorkRAM[pRelAddress];
        return true;
    }

    auto RAM::ReadExternalRAM (std::uint32_t pRelAddress, std::uint8_t& pDataOut) -> bool
    {
        if (pRelAddress >= mExternalRAM.size())
            { return false; }

        pDataOut = mExternalRAM[pRelAddress];
        return true;
    }

    auto RAM::ReadQuickRAM (std::uint32_t pRelAddress, std::uint8_t& pDataOut) -> bool
    {
        if (pRelAddress >= mQuickRAM.size())
            { return false; }

        pDataOut = mQuickRAM[pRelAddress];
        return true;
    }

    auto RAM::WriteWorkRAM (std::uint32_t pRelAddress, std::uint8_t pDataIn) -> bool
    {
        if (pRelAddress >= mWorkRAM.size())
            { return false; }

        mWorkRAM[pRelAddress] = pDataIn;
        return true;
    }

    auto RAM::WriteExternalRAM (std::uint32_t pRelAddress, std::uint8_t pDataIn) -> bool
    {
        if (pRelAddress >= mExternalRAM.size())
            { return false; }

        mExternalRAM[pRelAddress] = pDataIn;
        return true;
    }

    auto RAM::WriteQuickRAM (std::uint32_t pRelAddress, std::uint8_t pDataIn) -> bool
    {
        if (pRelAddress >= mQuickRAM.size())
            { return false; }

        mQuickRAM[pRelAddress] = pDataIn;
        return true;
    }
}
