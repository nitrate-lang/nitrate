////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <core/package/Manifest.hh>
#include <core/package/Package.hh>
#include <core/package/Resource.hh>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <string>

namespace no3::package {
  class PackageBuilder final {
    class PImpl;
    std::unique_ptr<PImpl> m_impl;

    auto AddResourceEx(const std::filesystem::path& path, std::span<const uint8_t> init) -> PackageBuilder&;

  public:
    PackageBuilder();
    PackageBuilder(const PackageBuilder&) = delete;
    PackageBuilder(PackageBuilder&&) = default;
    PackageBuilder& operator=(const PackageBuilder&) = delete;
    PackageBuilder& operator=(PackageBuilder&&) = default;
    ~PackageBuilder();

    void Reset();

    [[nodiscard]] auto Check() -> bool;
    [[nodiscard]] auto Build() -> std::optional<std::unique_ptr<Package>>;
    [[nodiscard]] auto BuildManifest() -> std::optional<std::unique_ptr<Manifest>>;

    using Category = Manifest::Category;
    using VerCode = Manifest::SemanticVersion::Code;

    auto SetName(std::string name) -> PackageBuilder&;
    auto SetCategory(Category category) -> PackageBuilder&;
    auto SetDescription(std::string description) -> PackageBuilder&;
    auto SetLicense(std::string spdx_license_id) -> PackageBuilder&;
    auto SetVersion(VerCode major, VerCode minor, VerCode patch) -> PackageBuilder&;

    auto AddContact(std::string name, std::string email, std::set<Manifest::Contact::Role> roles,
                    std::optional<std::string> phone = std::nullopt) -> PackageBuilder&;

    auto AddSupportedPlatform(std::string platform_regex) -> PackageBuilder&;
    auto AddUnsupportedPlatform(std::string platform_regex) -> PackageBuilder&;

    auto SetMinimumCPURequirement(uint32_t min_cores) -> PackageBuilder&;
    auto SetMinimumMemoryRequirement(uint64_t min_memory) -> PackageBuilder&;
    auto SetMinimumStorageRequirement(uint64_t min_storage) -> PackageBuilder&;

    /** @param path Path to the resource relative to the package root. */
    template <typename ResourceBuffer>
    auto AddResource(const std::filesystem::path& path,
                     const ResourceBuffer& init = ResourceBuffer()) -> PackageBuilder& {
      using ResourceBufferElement = std::remove_pointer_t<decltype(init.data())>;
      static_assert(std::is_trivially_copyable_v<ResourceBufferElement>, "Resource data must be trivially copyable");

      const auto* raw_bytes = reinterpret_cast<const uint8_t*>(init.data());
      const size_t bytes_count = init.size() * sizeof(ResourceBufferElement);
      const auto span = std::span<const uint8_t>(raw_bytes, bytes_count);

      return AddResourceEx(path, span);
    }
  };
}  // namespace no3::package
