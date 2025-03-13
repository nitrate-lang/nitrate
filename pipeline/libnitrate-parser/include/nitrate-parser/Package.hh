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

#ifndef __NITRATE_AST_PACKAGE_H__
#define __NITRATE_AST_PACKAGE_H__

#include <filesystem>
#include <functional>
#include <memory>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/String.hh>
#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace ncc::parse {
  class NCC_EXPORT ImportName final {
    mutable std::optional<std::string> m_name;
    mutable std::optional<std::vector<std::string_view>> m_chain;

    static auto Validate(const std::string &name) -> bool;

  public:
    ImportName() = default;
    ImportName(std::string name);

    auto operator==(const ImportName &other) const -> bool = default;
    auto operator<=>(const ImportName &other) const = default;
    auto operator*() const -> const std::string & { return GetName(); }
    auto operator->() const -> const std::string * { return &GetName(); }
    auto operator!() const -> bool { return !IsValid(); }

    [[nodiscard]] auto GetName() const -> const std::string & { return m_name.value(); }
    [[nodiscard]] auto IsValid() const -> bool { return m_name.has_value(); }
    [[nodiscard]] auto GetChain() const -> const std::vector<std::string_view> &;
  };

  auto operator<<(std::ostream &os, const ImportName &name) -> std::ostream &;

  class NCC_EXPORT Package final {
    class PImpl;
    mutable std::shared_ptr<PImpl> m_impl;

    template <typename T>
    class LazyEval final {
      std::function<T()> m_eval;
      mutable std::optional<T> m_value;

    public:
      LazyEval(std::function<T()> eval) : m_eval(std::move(eval)) {}

      auto Get() const -> const T & {
        if (!m_value.has_value()) {
          m_value = m_eval();
        }

        return m_value.value();
      }
    };

  public:
    struct PackageContents : public std::unordered_map<std::filesystem::path, LazyEval<std::optional<std::string>>> {
      PackageContents() = default;
      PackageContents(const PackageContents &other) = delete;
      auto operator=(const PackageContents &other) -> PackageContents & = delete;
      PackageContents(PackageContents &&other) noexcept = default;
      auto operator=(PackageContents &&other) noexcept -> PackageContents & = default;
    };
    using LazyLoader = LazyEval<std::optional<PackageContents>>;

    Package(ImportName name, LazyLoader loader);
    Package(const Package &other);
    auto operator=(const Package &other) -> Package &;
    Package(Package &&other) noexcept;
    auto operator=(Package &&other) noexcept -> Package &;
    ~Package();

    auto operator==(const Package &other) const -> bool;

    [[nodiscard]] auto PackageName() const -> ImportName;
    [[nodiscard]] auto Read() const -> const std::optional<PackageContents> &;

    static auto CompileDirectory(std::filesystem::path folder_path) -> LazyLoader;
    static auto CompileFile(std::filesystem::path file_path) -> LazyLoader;
  };

  auto FindPackages(const std::unordered_set<std::filesystem::path> &paths,
                    const std::function<bool(const std::filesystem::path &)> &predicate = nullptr)
      -> std::unordered_set<Package>;
}  // namespace ncc::parse

namespace std {
  template <>
  struct hash<ncc::parse::Package> {
    auto operator()(const ncc::parse::Package &pkg) const -> size_t {
      return std::hash<std::string>{}(pkg.PackageName().GetName());
    }
  };
}  // namespace std

#endif
