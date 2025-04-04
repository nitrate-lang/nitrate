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

#include <algorithm>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace no3::package::manifest {
  using String = std::string;

  enum class Category {
    StandardLibrary,
    Library,
    Application,
  };

  class SemanticVersion final {
    using Code = uint32_t;

    Code m_major = 0;
    Code m_minor = 1;
    Code m_patch = 0;

  public:
    constexpr explicit SemanticVersion(Code major, Code minor, Code patch)
        : m_major(major), m_minor(minor), m_patch(patch) {}
    constexpr SemanticVersion() = default;

    [[nodiscard]] auto operator<=>(const SemanticVersion& o) const = default;

    [[nodiscard]] constexpr auto GetMajor() const -> Code { return m_major; }
    [[nodiscard]] constexpr auto GetMinor() const -> Code { return m_minor; }
    [[nodiscard]] constexpr auto GetPatch() const -> Code { return m_patch; }

    constexpr void SetMajor(Code major) { m_major = major; }
    constexpr void SetMinor(Code minor) { m_minor = minor; }
    constexpr void SetPatch(Code patch) { m_patch = patch; }
  };

  class Contact final {
  public:
    enum class Role {
      Owner,
      Contributor,
      Maintainer,
      Support,
    };

    Contact(String name, String email, std::vector<Role> roles, std::optional<String> phone = std::nullopt)
        : m_name(std::move(name)), m_email(std::move(email)), m_roles(std::move(roles)), m_phone(std::move(phone)) {}

    [[nodiscard]] auto operator<=>(const Contact& o) const = default;

    [[nodiscard]] auto GetName() const -> const String& { return m_name; }
    [[nodiscard]] auto GetEmail() const -> const String& { return m_email; }
    [[nodiscard]] auto GetRoles() const -> const std::vector<Role>& { return m_roles; }
    [[nodiscard]] auto GetPhone() const -> const std::optional<String>& { return m_phone; }
    [[nodiscard]] auto ContainsPhone() const -> bool { return m_phone.has_value(); }

    void SetName(const String& name) { m_name = name; }
    void SetEmail(const String& email) { m_email = email; }

    void SetRoles(const std::vector<Role>& roles) { m_roles = roles; }
    void ClearRoles() { m_roles.clear(); }

    void SetPhone(const std::optional<String>& phone) { m_phone = phone; }
    void ClearPhone() { m_phone.reset(); }

  private:
    String m_name;
    String m_email;
    std::vector<Role> m_roles;
    std::optional<String> m_phone;
  };

  class Platforms final {
    std::vector<String> m_allow;
    std::vector<String> m_deny;

  public:
    Platforms() : m_allow({"*"}), m_deny({"*"}){};
    Platforms(std::vector<String> allow, std::vector<String> deny)
        : m_allow(std::move(allow)), m_deny(std::move(deny)) {}

    [[nodiscard]] auto operator<=>(const Platforms& o) const = default;

    [[nodiscard]] auto GetAllow() const -> const std::vector<String>& { return m_allow; }
    [[nodiscard]] auto GetDeny() const -> const std::vector<String>& { return m_deny; }

    void SetAllow(const std::vector<String>& allow) { m_allow = allow; }
    void ClearAllow() { m_allow.clear(); }
    void AddAllow(const String& allow) { m_allow.push_back(allow); }
    void RemoveAllow(const String& allow) {
      m_allow.erase(std::remove(m_allow.begin(), m_allow.end(), allow), m_allow.end());
    }

    void SetDeny(const std::vector<String>& deny) { m_deny = deny; }
    void ClearDeny() { m_deny.clear(); }
    void AddDeny(const String& deny) { m_deny.push_back(deny); }
    void RemoveDeny(const String& deny) { m_deny.erase(std::remove(m_deny.begin(), m_deny.end(), deny), m_deny.end()); }
  };

  class Optimization final {
    static inline const String RAPID_KEY = String("rapid");
    static inline const String DEBUG_KEY = String("debug");
    static inline const String RELEASE_KEY = String("release");

    static bool IsRequiredProfile(const String& name) {
      return name == RAPID_KEY || name == DEBUG_KEY || name == RELEASE_KEY;
    }

  public:
    class Switch final {
    public:
      using Flag = String;
      using Flags = std::set<Flag>;

      Switch() = default;
      explicit Switch(Flags alpha, Flags beta, Flags gamma, Flags llvm, Flags lto, Flags runtime)
          : m_alpha(std::move(alpha)),
            m_beta(std::move(beta)),
            m_gamma(std::move(gamma)),
            m_llvm(std::move(llvm)),
            m_lto(std::move(lto)),
            m_runtime(std::move(runtime)) {}

      [[nodiscard]] auto operator<=>(const Switch& o) const = default;

      [[nodiscard]] auto GetAlpha() const -> const Flags& { return m_alpha; }
      [[nodiscard]] auto GetBeta() const -> const Flags& { return m_beta; }
      [[nodiscard]] auto GetGamma() const -> const Flags& { return m_gamma; }
      [[nodiscard]] auto GetLLVM() const -> const Flags& { return m_llvm; }
      [[nodiscard]] auto GetLTO() const -> const Flags& { return m_lto; }
      [[nodiscard]] auto GetRuntime() const -> const Flags& { return m_runtime; }

      [[nodiscard]] auto GetAlpha() -> Flags& { return m_alpha; }
      [[nodiscard]] auto GetBeta() -> Flags& { return m_beta; }
      [[nodiscard]] auto GetGamma() -> Flags& { return m_gamma; }
      [[nodiscard]] auto GetLLVM() -> Flags& { return m_llvm; }
      [[nodiscard]] auto GetLTO() -> Flags& { return m_lto; }
      [[nodiscard]] auto GetRuntime() -> Flags& { return m_runtime; }

      void SetAlpha(const Flags& alpha) { m_alpha = alpha; }
      void SetBeta(const Flags& beta) { m_beta = beta; }
      void SetGamma(const Flags& gamma) { m_gamma = gamma; }
      void SetLLVM(const Flags& llvm) { m_llvm = llvm; }
      void SetLTO(const Flags& lto) { m_lto = lto; }
      void SetRuntime(const Flags& runtime) { m_runtime = runtime; }

      void ClearAlpha() { m_alpha.clear(); }
      void ClearBeta() { m_beta.clear(); }
      void ClearGamma() { m_gamma.clear(); }
      void ClearLLVM() { m_llvm.clear(); }
      void ClearLTO() { m_lto.clear(); }
      void ClearRuntime() { m_runtime.clear(); }

      void SetAlphaFlag(const Flag& flag) { m_alpha.insert(flag); }
      void SetBetaFlag(const Flag& flag) { m_beta.insert(flag); }
      void SetGammaFlag(const Flag& flag) { m_gamma.insert(flag); }
      void SetLLVMFlag(const Flag& flag) { m_llvm.insert(flag); }
      void SetLTOFlag(const Flag& flag) { m_lto.insert(flag); }
      void SetRuntimeFlag(const Flag& flag) { m_runtime.insert(flag); }

      void ClearAlphaFlag(const Flag& flag) { m_alpha.erase(flag); }
      void ClearBetaFlag(const Flag& flag) { m_beta.erase(flag); }
      void ClearGammaFlag(const Flag& flag) { m_gamma.erase(flag); }
      void ClearLLVMFlag(const Flag& flag) { m_llvm.erase(flag); }
      void ClearLTOFlag(const Flag& flag) { m_lto.erase(flag); }
      void ClearRuntimeFlag(const Flag& flag) { m_runtime.erase(flag); }

      [[nodiscard]] auto ContainsAlphaFlag(const Flag& flag) const -> bool { return m_alpha.contains(flag); }
      [[nodiscard]] auto ContainsBetaFlag(const Flag& flag) const -> bool { return m_beta.contains(flag); }
      [[nodiscard]] auto ContainsGammaFlag(const Flag& flag) const -> bool { return m_gamma.contains(flag); }
      [[nodiscard]] auto ContainsLLVMFlag(const Flag& flag) const -> bool { return m_llvm.contains(flag); }
      [[nodiscard]] auto ContainsLTOFlag(const Flag& flag) const -> bool { return m_lto.contains(flag); }
      [[nodiscard]] auto ContainsRuntimeFlag(const Flag& flag) const -> bool { return m_runtime.contains(flag); }

    private:
      Flags m_alpha;
      Flags m_beta;
      Flags m_gamma;
      Flags m_llvm;
      Flags m_lto;
      Flags m_runtime;
    };

    Optimization() {
      m_profiles[RAPID_KEY] = Switch();
      m_profiles[DEBUG_KEY] = Switch();
      m_profiles[RELEASE_KEY] = Switch();
    }

    explicit Optimization(Switch rapid, Switch debug, Switch release,
                          const std::unordered_map<String, Switch>& additional_profiles = {})
        : m_profiles(
              {{RAPID_KEY, std::move(rapid)}, {DEBUG_KEY, std::move(debug)}, {RELEASE_KEY, std::move(release)}}) {
      m_profiles.insert(additional_profiles.begin(), additional_profiles.end());
    }

    [[nodiscard]] auto operator<=>(const Optimization& o) const = default;

    [[nodiscard]] auto GetRapid() const -> const Switch& { return m_profiles.at(RAPID_KEY); }
    [[nodiscard]] auto GetDebug() const -> const Switch& { return m_profiles.at(DEBUG_KEY); }
    [[nodiscard]] auto GetRelease() const -> const Switch& { return m_profiles.at(RELEASE_KEY); }

    [[nodiscard]] auto GetRapid() -> Switch& { return m_profiles.at(RAPID_KEY); }
    [[nodiscard]] auto GetDebug() -> Switch& { return m_profiles.at(DEBUG_KEY); }
    [[nodiscard]] auto GetRelease() -> Switch& { return m_profiles.at(RELEASE_KEY); }

    [[nodiscard]] auto GetProfile(const String& name) const -> const Switch& { return m_profiles[name]; }
    [[nodiscard]] auto GetProfile(const String& name) -> Switch& { return m_profiles[name]; }

    [[nodiscard]] auto ContainsProfile(const String& name) const -> bool { return m_profiles.contains(name); }
    void SetProfile(const String& name, const Switch& profile) { m_profiles[name] = profile; }

    void ClearProfile(const String& name) {
      if (!IsRequiredProfile(name)) {
        m_profiles.erase(name);
      }
    }

    void ClearAllProfiles() {
      m_profiles.clear();

      m_profiles[RAPID_KEY] = Switch();
      m_profiles[DEBUG_KEY] = Switch();
      m_profiles[RELEASE_KEY] = Switch();
    }

  private:
    using Profiles = std::map<String, Switch>;

    mutable Profiles m_profiles;
  };

  class Dependency final {
    using UUID = String;

    UUID m_uuid;
    SemanticVersion m_version;

  public:
    Dependency(UUID uuid, SemanticVersion version) : m_uuid(std::move(uuid)), m_version(version) {}

    [[nodiscard]] auto operator<=>(const Dependency& o) const = default;

    [[nodiscard]] auto GetUUID() const -> const UUID& { return m_uuid; }
    [[nodiscard]] auto GetVersion() const -> const SemanticVersion& { return m_version; }

    void SetUUID(const UUID& uuid) { m_uuid = uuid; }
    void SetVersion(const SemanticVersion& version) { m_version = version; }
  };

  class Manifest final {
    String m_name;
    String m_description;
    String m_spdx_license = "LGPL-2.1";
    Category m_category;
    SemanticVersion m_version;
    std::vector<Contact> m_contacts;
    Platforms m_platforms;
    Optimization m_optimization;
    std::vector<Dependency> m_dependencies;

  public:
    Manifest(String name, Category category) : m_name(std::move(name)), m_category(category) {}

    [[nodiscard]] auto operator<=>(const Manifest& o) const = default;

    [[nodiscard]] auto GetName() const -> const String& { return m_name; }
    [[nodiscard]] auto GetDescription() const -> const String& { return m_description; }
    [[nodiscard]] auto GetSPDXLicense() const -> const String& { return m_spdx_license; }
    [[nodiscard]] auto GetCategory() const -> Category { return m_category; }
    [[nodiscard]] auto GetVersion() const -> const SemanticVersion& { return m_version; }
    [[nodiscard]] auto GetContacts() const -> const std::vector<Contact>& { return m_contacts; }
    [[nodiscard]] auto GetPlatforms() const -> const Platforms& { return m_platforms; }
    [[nodiscard]] auto GetOptimization() const -> const Optimization& { return m_optimization; }
    [[nodiscard]] auto GetDependencies() const -> const std::vector<Dependency>& { return m_dependencies; }

    [[nodiscard]] auto GetName() -> String& { return m_name; }
    [[nodiscard]] auto GetDescription() -> String& { return m_description; }
    [[nodiscard]] auto GetSPDXLicense() -> String& { return m_spdx_license; }
    [[nodiscard]] auto GetCategory() -> Category& { return m_category; }
    [[nodiscard]] auto GetVersion() -> SemanticVersion& { return m_version; }
    [[nodiscard]] auto GetContacts() -> std::vector<Contact>& { return m_contacts; }
    [[nodiscard]] auto GetPlatforms() -> Platforms& { return m_platforms; }
    [[nodiscard]] auto GetOptimization() -> Optimization& { return m_optimization; }
    [[nodiscard]] auto GetDependencies() -> std::vector<Dependency>& { return m_dependencies; }

    void SetName(String name) { m_name = std::move(name); }
    void SetDescription(String description) { m_description = std::move(description); }
    void SetSPDXLicense(String spdx_license) { m_spdx_license = std::move(spdx_license); }
    void SetCategory(Category category) { m_category = category; }
    void SetVersion(SemanticVersion version) { m_version = version; }
    void SetContacts(std::vector<Contact> contacts) { m_contacts = std::move(contacts); }
    void SetPlatforms(Platforms platforms) { m_platforms = std::move(platforms); }
    void SetOptimization(Optimization optimization) { m_optimization = std::move(optimization); }
    void SetDependencies(std::vector<Dependency> dependencies) { m_dependencies = std::move(dependencies); }

    void AddContact(const Contact& contact) { m_contacts.push_back(contact); }
    void RemoveContact(const Contact& contact) {
      m_contacts.erase(std::remove(m_contacts.begin(), m_contacts.end(), contact), m_contacts.end());
    }
    void ClearContacts() { m_contacts.clear(); }

    void AddPlatformAllow(const String& allow) { m_platforms.AddAllow(allow); }
    void RemovePlatformAllow(const String& allow) { m_platforms.RemoveAllow(allow); }
    void ClearPlatformAllow() { m_platforms.ClearAllow(); }
    void AddPlatformDeny(const String& deny) { m_platforms.AddDeny(deny); }
    void RemovePlatformDeny(const String& deny) { m_platforms.RemoveDeny(deny); }
    void ClearPlatformDeny() { m_platforms.ClearDeny(); }

    void AddOptimizationProfile(const String& name, const Optimization::Switch& profile) {
      m_optimization.SetProfile(name, profile);
    }
    void RemoveOptimizationProfile(const String& name) { m_optimization.ClearProfile(name); }
    void ClearOptimizationProfiles() { m_optimization.ClearAllProfiles(); }

    void AddDependency(const Dependency& dependency) { m_dependencies.push_back(dependency); }
    void RemoveDependency(const Dependency& dependency) {
      m_dependencies.erase(std::remove(m_dependencies.begin(), m_dependencies.end(), dependency), m_dependencies.end());
    }
    void ClearDependencies() { m_dependencies.clear(); }
  };
}  // namespace no3::package::manifest
