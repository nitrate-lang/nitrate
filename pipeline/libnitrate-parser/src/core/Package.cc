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

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <nitrate-core/CatchAll.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-parser/Package.hh>
#include <sstream>

using namespace ncc::parse;

static bool IsNitratePackage(const std::filesystem::path &path) {
  using namespace ncc;

  const bool has_json_config = OMNI_CATCH(std::filesystem::exists(path / "no3.json")).value_or(false) &&
                               OMNI_CATCH(std::filesystem::is_regular_file(path / "no3.json")).value_or(false);

  const bool has_nit_config = OMNI_CATCH(std::filesystem::exists(path / "no3.nit")).value_or(false) &&
                              OMNI_CATCH(std::filesystem::is_regular_file(path / "no3.nit")).value_or(false);

  if (!has_json_config && !has_nit_config) {
    return false;
  }

  if (has_json_config) {
    Log << Trace << "Package: IsPackage: Found no3.json in: " << path;
  }

  if (has_nit_config) {
    Log << Trace << "Package: IsPackage: Found no3.nit in: " << path;
  }

  const bool has_src = OMNI_CATCH(std::filesystem::exists(path / "src")).value_or(false) &&
                       OMNI_CATCH(std::filesystem::is_directory(path / "src")).value_or(false);

  return has_src;
}

static std::string GetPackageName(const std::map<std::filesystem::path, bool> &cache,
                                  const std::filesystem::path &package_dir) {
  std::stringstream ss;
  for (const auto &maybe_subpkg : cache) {
    if (!maybe_subpkg.second) {
      continue;
    }

    if (package_dir != maybe_subpkg.first && package_dir.string().starts_with(maybe_subpkg.first.string())) {
      ss << maybe_subpkg.first.filename().string() << "::";
    }
  }

  ss << package_dir.filename().string();

  return ss.str();
}

static auto IsDirectory(const std::filesystem::path &path) -> bool {
  auto is_dir = OMNI_CATCH(std::filesystem::is_directory(path));
  if (!is_dir.has_value()) [[unlikely]] {
    ncc::Log << "Package: IsDirectory: Failed to check if path is directory: " << path;
    return false;
  }

  return is_dir.value();
}

class Package::PImpl {
public:
  ImportName m_pkg_name;
  LazyLoader m_loader;

  PImpl(ImportName name, LazyLoader loader) : m_pkg_name(std::move(name)), m_loader(std::move(loader)) {}
};

Package::Package(ImportName name, LazyLoader loader) : m_impl(std::make_shared<PImpl>(name, std::move(loader))) {}

Package::Package(const Package &other) = default;

Package &Package::operator=(const Package &other) {
  if (this != &other) [[likely]] {
    m_impl = other.m_impl;
  }
  return *this;
}

Package::Package(Package &&other) noexcept { m_impl = std::move(other.m_impl); }

Package &Package::operator=(Package &&other) noexcept {
  if (this != &other) [[likely]] {
    m_impl = std::move(other.m_impl);
  }
  return *this;
}

Package::~Package() = default;

bool Package::operator==(const Package &other) const {
  if (this->m_impl == nullptr && other.m_impl == nullptr) {
    return true;
  }

  if (this->m_impl == nullptr || other.m_impl == nullptr) {
    return false;
  }

  return m_impl->m_pkg_name == other.m_impl->m_pkg_name;
}

auto Package::PackageName() const -> ImportName { return m_impl ? m_impl->m_pkg_name : ImportName{""}; }

auto Package::Read() const -> const std::optional<PackageContents> & {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_loader.Get();
}

auto Package::CompileDirectory(std::filesystem::path folder_path) -> LazyLoader {
  LazyEval<std::optional<PackageContents>> load([path = std::move(folder_path)]() -> std::optional<PackageContents> {
    Log << Trace << "Package: Reading directory: " << path.string();

    PackageContents files;

    auto dir_it = OMNI_CATCH(std::filesystem::recursive_directory_iterator(path));
    if (!dir_it.has_value()) {
      Log << Error << "Package: Failed to create recursive directory iterator: " << path.string();
      return std::nullopt;
    }

    auto it = *dir_it;
    while (it != std::filesystem::recursive_directory_iterator()) {
      auto entry = *it;
      if (!OMNI_CATCH(++it, 0)) {
        Log << Debug << "Package: Failed to increment iterator: " << entry.path() << " (skipping)";
        continue;
      }

      if (OMNI_CATCH(std::filesystem::is_directory(entry.path())).value_or(false)) {
        Log << Trace << "Package: Skipping directory: " << entry.path();
        continue;
      }

      {  // Skip non source files
        auto ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext != ".nit") {
          Log << Trace << "Package: Skipping non-Nitrate file: " << entry.path();
          continue;
        }
      }

      auto loader = CompileFile(entry.path());
      const auto &maybe_content = loader.Get();
      if (!maybe_content.has_value()) [[unlikely]] {
        Log << Error << "Package: Failed to compile file: " << entry.path();
        return std::nullopt;
      }

      files.insert(maybe_content.value().begin(), maybe_content.value().end());
    }

    for (const auto &[path, content] : files) {
      Log << Debug << "Package: Found file (" << path;
    }
    Log << Trace << "Package: Found " << files.size() << " file(s) from directory: " << path.string();

    return files;
  });

  return load;
}

auto Package::CompileFile(std::filesystem::path file_path) -> LazyLoader {
  LazyEval<std::optional<PackageContents>> load([path = std::move(file_path)]() -> std::optional<PackageContents> {
    PackageContents files;

    auto load_file = LazyEval<std::optional<std::string>>([path]() -> std::optional<std::string> {
      Log << Trace << "Package: Reading...: " << path.string();

      std::ifstream f(path.string(), std::ios::in | std::ios::binary);
      if (!f.is_open()) {
        Log << Error << "Package: Failed to open file: " << path.string();
        return std::nullopt;
      }

      auto content = std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
      Log << Trace << "Package: Read " << content.size() << " byte(s) from file: " << path.string();

      return content;
    });

    files.emplace(path, load_file);

    return files;
  });

  return load;
}

NCC_EXPORT auto ncc::parse::FindPackages(const std::unordered_set<std::filesystem::path> &paths,
                                         const std::function<bool(const std::filesystem::path &)> &predicate)
    -> std::unordered_set<Package> {
  std::map<std::filesystem::path, bool> is_package_cache;
  std::unordered_set<Package> packages;

  for (auto path : paths) {
    if (!OMNI_CATCH(std::filesystem::exists(path)).value_or(false)) {
      ncc::Log << Debug << "Package: Path does not exist: " << path;
      continue;
    }

    auto path_abs = OMNI_CATCH(std::filesystem::absolute(path));
    if (!path_abs.has_value()) {
      ncc::Log << "Package: Failed to get absolute path: " << path;
      continue;
    }

    Log << Trace << "Package: Searching for packages in (absolute): " << path_abs.value();

    auto recursive_it = OMNI_CATCH(std::filesystem::recursive_directory_iterator(*path_abs));
    if (!recursive_it.has_value()) {
      ncc::Log << "Package: Failed to create recursive directory iterator: " << path_abs.value();
      continue;
    }

    auto it = *recursive_it;
    while (it != std::filesystem::recursive_directory_iterator()) {
      auto subpath = *it;
      if (!OMNI_CATCH(++it, 0)) {
        Log << Debug << "Package: Failed to increment iterator: " << subpath << " (skipping)";
        continue;
      }

      if (!IsDirectory(subpath.path())) {
        continue;
      }

      const bool is_pkg = IsNitratePackage(subpath);
      is_package_cache[subpath.path()] = is_pkg;
      if (!is_pkg) {
        continue;
      }

      if (predicate && !predicate(subpath)) {
        Log << Trace << "Package: Skipping directory due to predicate: " << subpath;
        continue;
      }

      auto pkg_name = GetPackageName(is_package_cache, subpath);
      auto loader = Package::CompileDirectory(subpath.path() / "src");

      Log << Debug << "Package: Found package: " << pkg_name;
      packages.emplace(pkg_name, std::move(loader));
    }
  }

  return packages;
}
