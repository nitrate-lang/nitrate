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
#include <ostream>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace no3::package {
  class Manifest final {
    Manifest() = default;

  public:
    enum class Category {
      StandardLibrary,
      Library,
      Application,
    };

    class SemanticVersion final {
    public:
      using Code = uint32_t;

      constexpr explicit SemanticVersion(Code major, Code minor, Code patch)
          : m_major(major), m_minor(minor), m_patch(patch) {}
      constexpr SemanticVersion() = default;

      [[nodiscard, gnu::pure]] auto operator<=>(const SemanticVersion& o) const = default;

      [[nodiscard, gnu::pure]] constexpr auto GetMajor() const -> Code { return m_major; }
      [[nodiscard, gnu::pure]] constexpr auto GetMinor() const -> Code { return m_minor; }
      [[nodiscard, gnu::pure]] constexpr auto GetPatch() const -> Code { return m_patch; }

      constexpr void SetMajor(Code major) { m_major = major; }
      constexpr void SetMinor(Code minor) { m_minor = minor; }
      constexpr void SetPatch(Code patch) { m_patch = patch; }

    private:
      Code m_major = 0;
      Code m_minor = 1;
      Code m_patch = 0;
    };

    class Contact final {
    public:
      enum class Role {
        Owner,
        Contributor,
        Maintainer,
        Support,
      };

      Contact(std::string name, std::string email, std::set<Role> roles,
              std::optional<std::string> phone = std::nullopt)
          : m_name(std::move(name)), m_email(std::move(email)), m_roles(std::move(roles)), m_phone(std::move(phone)) {}

      [[nodiscard, gnu::pure]] auto operator<=>(const Contact& o) const = default;

      [[nodiscard, gnu::pure]] auto GetName() const -> const std::string& { return m_name; }
      [[nodiscard, gnu::pure]] auto GetEmail() const -> const std::string& { return m_email; }
      [[nodiscard, gnu::pure]] auto GetRoles() const -> const std::set<Role>& { return m_roles; }
      [[nodiscard, gnu::pure]] auto GetPhone() const -> const std::optional<std::string>& { return m_phone; }
      [[nodiscard, gnu::pure]] auto ContainsPhone() const -> bool { return m_phone.has_value(); }

      void SetName(const std::string& name) { m_name = name; }
      void SetEmail(const std::string& email) { m_email = email; }

      void SetRoles(const std::set<Role>& roles) { m_roles = roles; }
      void ClearRoles() { m_roles.clear(); }
      void AddRole(Role role) { m_roles.insert(role); }
      void RemoveRole(Role role) { m_roles.erase(role); }

      void SetPhone(const std::optional<std::string>& phone) { m_phone = phone; }
      void ClearPhone() { m_phone.reset(); }

    private:
      std::string m_name;
      std::string m_email;
      std::set<Role> m_roles;
      std::optional<std::string> m_phone;
    };

    class Platforms final {
      std::vector<std::string> m_allow;
      std::vector<std::string> m_deny;

    public:
      Platforms() : m_allow({"*"}), m_deny({"*"}){};
      Platforms(std::vector<std::string> allow, std::vector<std::string> deny)
          : m_allow(std::move(allow)), m_deny(std::move(deny)) {}

      [[nodiscard]] auto operator<=>(const Platforms& o) const = default;

      [[nodiscard]] auto GetAllow() const -> const std::vector<std::string>& { return m_allow; }
      [[nodiscard]] auto GetDeny() const -> const std::vector<std::string>& { return m_deny; }

      void SetAllow(const std::vector<std::string>& allow) { m_allow = allow; }
      void ClearAllow() { m_allow.clear(); }
      void AddAllow(const std::string& allow) { m_allow.push_back(allow); }
      void RemoveAllow(const std::string& allow) {
        m_allow.erase(std::remove(m_allow.begin(), m_allow.end(), allow), m_allow.end());
      }

      void SetDeny(const std::vector<std::string>& deny) { m_deny = deny; }
      void ClearDeny() { m_deny.clear(); }
      void AddDeny(const std::string& deny) { m_deny.push_back(deny); }
      void RemoveDeny(const std::string& deny) {
        m_deny.erase(std::remove(m_deny.begin(), m_deny.end(), deny), m_deny.end());
      }
    };

    class Optimization final {
      static inline const std::string RAPID_KEY = std::string("rapid");
      static inline const std::string DEBUG_KEY = std::string("debug");
      static inline const std::string RELEASE_KEY = std::string("release");

      static bool IsRequiredProfile(const std::string& name) {
        return name == RAPID_KEY || name == DEBUG_KEY || name == RELEASE_KEY;
      }

    public:
      class Switch final {
      public:
        using Flag = std::string;
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

      class Requirements final {
        uint64_t m_min_cores;
        uint64_t m_min_memory;
        uint64_t m_min_storage;

      public:
        constexpr Requirements() : m_min_cores(0), m_min_memory(0), m_min_storage(0) {}
        constexpr explicit Requirements(uint64_t min_cores, uint64_t min_memory, uint64_t min_storage)
            : m_min_cores(min_cores), m_min_memory(min_memory), m_min_storage(min_storage) {}

        [[nodiscard]] auto operator<=>(const Requirements& o) const = default;

        [[nodiscard]] auto GetMinCores() const -> uint64_t { return m_min_cores; }
        [[nodiscard]] auto GetMinMemory() const -> uint64_t { return m_min_memory; }
        [[nodiscard]] auto GetMinStorage() const -> uint64_t { return m_min_storage; }

        [[nodiscard]] auto GetMinCores() -> uint64_t& { return m_min_cores; }
        [[nodiscard]] auto GetMinMemory() -> uint64_t& { return m_min_memory; }
        [[nodiscard]] auto GetMinStorage() -> uint64_t& { return m_min_storage; }

        void SetMinCores(uint64_t min_cores) { m_min_cores = min_cores; }
        void SetMinMemory(uint64_t min_memory) { m_min_memory = min_memory; }
        void SetMinStorage(uint64_t min_storage) { m_min_storage = min_storage; }
      };

      Optimization() {
        m_profiles[RAPID_KEY] = Switch();
        m_profiles[DEBUG_KEY] = Switch();
        m_profiles[RELEASE_KEY] = Switch();
      }

      explicit Optimization(Switch rapid, Switch debug, Switch release,
                            const std::unordered_map<std::string, Switch>& additional_profiles = {},
                            Requirements requirements = Requirements())
          : m_profiles(
                {{RAPID_KEY, std::move(rapid)}, {DEBUG_KEY, std::move(debug)}, {RELEASE_KEY, std::move(release)}}),
            m_requirements(requirements) {
        m_profiles.insert(additional_profiles.begin(), additional_profiles.end());
      }

      [[nodiscard]] auto operator<=>(const Optimization& o) const = default;

      [[nodiscard]] auto GetRapid() const -> const Switch& { return m_profiles.at(RAPID_KEY); }
      [[nodiscard]] auto GetDebug() const -> const Switch& { return m_profiles.at(DEBUG_KEY); }
      [[nodiscard]] auto GetRelease() const -> const Switch& { return m_profiles.at(RELEASE_KEY); }

      [[nodiscard]] auto GetRapid() -> Switch& { return m_profiles.at(RAPID_KEY); }
      [[nodiscard]] auto GetDebug() -> Switch& { return m_profiles.at(DEBUG_KEY); }
      [[nodiscard]] auto GetRelease() -> Switch& { return m_profiles.at(RELEASE_KEY); }

      [[nodiscard]] auto GetProfile(const std::string& name) const -> const Switch& { return m_profiles[name]; }
      [[nodiscard]] auto GetProfile(const std::string& name) -> Switch& { return m_profiles[name]; }

      [[nodiscard]] auto ContainsProfile(const std::string& name) const -> bool { return m_profiles.contains(name); }
      void SetProfile(const std::string& name, const Switch& profile) { m_profiles[name] = profile; }

      void ClearProfile(const std::string& name) {
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

      [[nodiscard]] auto GetRequirements() const -> const Requirements& { return m_requirements; }
      [[nodiscard]] auto GetRequirements() -> Requirements& { return m_requirements; }

      void SetRequirements(const Requirements& requirements) { m_requirements = requirements; }

    private:
      using Profiles = std::map<std::string, Switch>;

      mutable Profiles m_profiles;
      Requirements m_requirements;
    };

    class Dependency final {
      using UUID = std::string;

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

    Manifest(std::string name, Category category) : m_name(std::move(name)), m_category(category) {}
    Manifest(const Manifest&) = default;
    Manifest(Manifest&&) = default;
    Manifest& operator=(const Manifest&) = default;
    Manifest& operator=(Manifest&&) = default;

    [[nodiscard]] auto operator<=>(const Manifest& o) const = default;

    [[nodiscard]] auto GetName() const -> const std::string& { return m_name; }
    [[nodiscard]] auto GetDescription() const -> const std::string& { return m_description; }
    [[nodiscard]] auto GetLicense() const -> const std::string& { return m_license; }
    [[nodiscard]] auto GetCategory() const -> Category { return m_category; }
    [[nodiscard]] auto GetVersion() const -> const SemanticVersion& { return m_version; }
    [[nodiscard]] auto GetContacts() const -> const std::vector<Contact>& { return m_contacts; }
    [[nodiscard]] auto GetPlatforms() const -> const Platforms& { return m_platforms; }
    [[nodiscard]] auto GetOptimization() const -> const Optimization& { return m_optimization; }
    [[nodiscard]] auto GetDependencies() const -> const std::vector<Dependency>& { return m_dependencies; }

    [[nodiscard]] auto GetName() -> std::string& { return m_name; }
    [[nodiscard]] auto GetDescription() -> std::string& { return m_description; }
    [[nodiscard]] auto GetLicense() -> std::string& { return m_license; }
    [[nodiscard]] auto GetCategory() -> Category& { return m_category; }
    [[nodiscard]] auto GetVersion() -> SemanticVersion& { return m_version; }
    [[nodiscard]] auto GetContacts() -> std::vector<Contact>& { return m_contacts; }
    [[nodiscard]] auto GetPlatforms() -> Platforms& { return m_platforms; }
    [[nodiscard]] auto GetOptimization() -> Optimization& { return m_optimization; }
    [[nodiscard]] auto GetDependencies() -> std::vector<Dependency>& { return m_dependencies; }

    void SetName(std::string name) { m_name = std::move(name); }
    void SetDescription(std::string description) { m_description = std::move(description); }
    void SetLicense(std::string spdx_license) { m_license = std::move(spdx_license); }
    void SetCategory(Category category) { m_category = category; }
    void SetVersion(SemanticVersion version) { m_version = version; }
    void SetContacts(std::vector<Contact> contacts) { m_contacts = std::move(contacts); }
    void SetPlatforms(Platforms platforms) { m_platforms = std::move(platforms); }
    void SetOptimization(Optimization optimization) { m_optimization = std::move(optimization); }
    void SetDependencies(std::vector<Dependency> dependencies) { m_dependencies = std::move(dependencies); }

    void AddContact(const Contact& contact) { m_contacts.push_back(contact); }
    void ClearContacts() { m_contacts.clear(); }
    void RemoveContact(const Contact& contact) {
      m_contacts.erase(std::remove(m_contacts.begin(), m_contacts.end(), contact), m_contacts.end());
    }

    void AddPlatformAllow(const std::string& allow) { m_platforms.AddAllow(allow); }
    void RemovePlatformAllow(const std::string& allow) { m_platforms.RemoveAllow(allow); }
    void ClearPlatformAllow() { m_platforms.ClearAllow(); }
    void AddPlatformDeny(const std::string& deny) { m_platforms.AddDeny(deny); }
    void RemovePlatformDeny(const std::string& deny) { m_platforms.RemoveDeny(deny); }
    void ClearPlatformDeny() { m_platforms.ClearDeny(); }

    void AddOptimizationProfile(const std::string& name, const Optimization::Switch& profile) {
      m_optimization.SetProfile(name, profile);
    }
    void RemoveOptimizationProfile(const std::string& name) { m_optimization.ClearProfile(name); }
    void ClearOptimizationProfiles() { m_optimization.ClearAllProfiles(); }

    void AddDependency(const Dependency& dependency) { m_dependencies.push_back(dependency); }
    void ClearDependencies() { m_dependencies.clear(); }
    void RemoveDependency(const Dependency& dependency) {
      m_dependencies.erase(std::remove(m_dependencies.begin(), m_dependencies.end(), dependency), m_dependencies.end());
    }

    ///=============================================================================================
    ///==                               (DE)SERIALIZATION FUNCTIONS                               ==
    auto ToJson(std::ostream& os, bool& correct_schema, bool minify = false) const -> std::ostream&;
    static auto FromJson(std::istream& is) -> std::optional<Manifest>;
    static auto FromJson(std::string_view json) -> std::optional<Manifest>;

    ///=============================================================================================
    ///==                                  VALIDATION FUNCTIONS                                   ==
    [[nodiscard]] static auto IsValidLicense(std::string_view license) -> bool;
    [[nodiscard]] static auto IsValidName(std::string_view name) -> bool;
    [[nodiscard]] static auto GetNameRegex() -> std::string_view;

  private:
    std::string m_name;
    std::string m_description;
    std::string m_license = "LGPL-2.1";
    Category m_category = Category::Application;
    SemanticVersion m_version;
    std::vector<Contact> m_contacts;
    Platforms m_platforms;
    Optimization m_optimization;
    std::vector<Dependency> m_dependencies;
  };
}  // namespace no3::package
