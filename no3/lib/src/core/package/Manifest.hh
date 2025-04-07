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
      Executable,
    };

    class Version final {
    public:
      using Code = uint32_t;

      constexpr Version(Code major, Code minor, Code patch) : m_major(major), m_minor(minor), m_patch(patch) {}
      constexpr Version() = default;

      [[nodiscard, gnu::pure]] auto operator<=>(const Version& o) const = default;

      [[nodiscard, gnu::pure]] constexpr auto GetMajor() const -> Code { return m_major; }
      [[nodiscard, gnu::pure]] constexpr auto GetMinor() const -> Code { return m_minor; }
      [[nodiscard, gnu::pure]] constexpr auto GetPatch() const -> Code { return m_patch; }

      constexpr auto SetMajor(Code major) -> Version& {
        m_major = major;
        return *this;
      }
      constexpr auto SetMinor(Code minor) -> Version& {
        m_minor = minor;
        return *this;
      }
      constexpr auto SetPatch(Code patch) -> Version& {
        m_patch = patch;
        return *this;
      }

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

      auto SetName(const std::string& name) -> Contact& {
        m_name = name;
        return *this;
      }
      auto SetEmail(const std::string& email) -> Contact& {
        m_email = email;
        return *this;
      }

      auto SetRoles(const std::set<Role>& roles) -> Contact& {
        m_roles = roles;
        return *this;
      }
      auto ClearRoles() -> Contact& {
        m_roles.clear();
        return *this;
      }
      auto AddRole(Role role) -> Contact& {
        m_roles.insert(role);
        return *this;
      }
      auto RemoveRole(Role role) -> Contact& {
        m_roles.erase(role);
        return *this;
      }

      auto SetPhone(const std::optional<std::string>& phone) -> Contact& {
        m_phone = phone;
        return *this;
      }
      auto ClearPhone() -> Contact& {
        m_phone.reset();
        return *this;
      }

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

      auto SetAllow(const std::vector<std::string>& allow) -> Platforms& {
        m_allow = allow;
        return *this;
      }
      auto ClearAllow() -> Platforms& {
        m_allow.clear();
        return *this;
      }
      auto AddAllow(const std::string& allow) -> Platforms& {
        m_allow.push_back(allow);
        return *this;
      }
      auto RemoveAllow(const std::string& allow) -> Platforms& {
        m_allow.erase(std::remove(m_allow.begin(), m_allow.end(), allow), m_allow.end());
        return *this;
      }

      auto SetDeny(const std::vector<std::string>& deny) -> Platforms& {
        m_deny = deny;
        return *this;
      }
      auto ClearDeny() -> Platforms& {
        m_deny.clear();
        return *this;
      }
      auto AddDeny(const std::string& deny) -> Platforms& {
        m_deny.push_back(deny);
        return *this;
      }
      auto RemoveDeny(const std::string& deny) -> Platforms& {
        m_deny.erase(std::remove(m_deny.begin(), m_deny.end(), deny), m_deny.end());
        return *this;
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

        auto SetAlpha(const Flags& alpha) -> Switch& {
          m_alpha = alpha;
          return *this;
        }
        auto SetBeta(const Flags& beta) -> Switch& {
          m_beta = beta;
          return *this;
        }
        auto SetGamma(const Flags& gamma) -> Switch& {
          m_gamma = gamma;
          return *this;
        }
        auto SetLLVM(const Flags& llvm) -> Switch& {
          m_llvm = llvm;
          return *this;
        }
        auto SetLTO(const Flags& lto) -> Switch& {
          m_lto = lto;
          return *this;
        }
        auto SetRuntime(const Flags& runtime) -> Switch& {
          m_runtime = runtime;
          return *this;
        }

        auto ClearAlpha() -> Switch& {
          m_alpha.clear();
          return *this;
        }
        auto ClearBeta() -> Switch& {
          m_beta.clear();
          return *this;
        }
        auto ClearGamma() -> Switch& {
          m_gamma.clear();
          return *this;
        }
        auto ClearLLVM() -> Switch& {
          m_llvm.clear();
          return *this;
        }
        auto ClearLTO() -> Switch& {
          m_lto.clear();
          return *this;
        }
        auto ClearRuntime() -> Switch& {
          m_runtime.clear();
          return *this;
        }

        auto SetAlphaFlag(const Flag& flag) -> Switch& {
          m_alpha.insert(flag);
          return *this;
        }
        auto SetBetaFlag(const Flag& flag) -> Switch& {
          m_beta.insert(flag);
          return *this;
        }
        auto SetGammaFlag(const Flag& flag) -> Switch& {
          m_gamma.insert(flag);
          return *this;
        }
        auto SetLLVMFlag(const Flag& flag) -> Switch& {
          m_llvm.insert(flag);
          return *this;
        }
        auto SetLTOFlag(const Flag& flag) -> Switch& {
          m_lto.insert(flag);
          return *this;
        }
        auto SetRuntimeFlag(const Flag& flag) -> Switch& {
          m_runtime.insert(flag);
          return *this;
        }

        auto ClearAlphaFlag(const Flag& flag) -> Switch& {
          m_alpha.erase(flag);
          return *this;
        }
        auto ClearBetaFlag(const Flag& flag) -> Switch& {
          m_beta.erase(flag);
          return *this;
        }
        auto ClearGammaFlag(const Flag& flag) -> Switch& {
          m_gamma.erase(flag);
          return *this;
        }
        auto ClearLLVMFlag(const Flag& flag) -> Switch& {
          m_llvm.erase(flag);
          return *this;
        }
        auto ClearLTOFlag(const Flag& flag) -> Switch& {
          m_lto.erase(flag);
          return *this;
        }
        auto ClearRuntimeFlag(const Flag& flag) -> Switch& {
          m_runtime.erase(flag);
          return *this;
        }

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
        constexpr Requirements() : m_min_cores(1), m_min_memory(2 * 1024 * 1024), m_min_storage(0) {}
        constexpr explicit Requirements(uint64_t min_cores, uint64_t min_memory, uint64_t min_storage)
            : m_min_cores(min_cores), m_min_memory(min_memory), m_min_storage(min_storage) {}

        [[nodiscard]] auto operator<=>(const Requirements& o) const = default;

        [[nodiscard]] auto GetMinCores() const -> uint64_t { return m_min_cores; }
        [[nodiscard]] auto GetMinMemory() const -> uint64_t { return m_min_memory; }
        [[nodiscard]] auto GetMinStorage() const -> uint64_t { return m_min_storage; }

        [[nodiscard]] auto GetMinCores() -> uint64_t& { return m_min_cores; }
        [[nodiscard]] auto GetMinMemory() -> uint64_t& { return m_min_memory; }
        [[nodiscard]] auto GetMinStorage() -> uint64_t& { return m_min_storage; }

        auto SetMinCores(uint64_t min_cores) -> Requirements& {
          m_min_cores = min_cores;
          return *this;
        }
        auto SetMinMemory(uint64_t min_memory) -> Requirements& {
          m_min_memory = min_memory;
          return *this;
        }
        auto SetMinStorage(uint64_t min_storage) -> Requirements& {
          m_min_storage = min_storage;
          return *this;
        }
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
      auto SetProfile(const std::string& name, const Switch& profile) -> Optimization& {
        m_profiles[name] = profile;
        return *this;
      }

      auto ClearProfile(const std::string& name) -> Optimization& {
        if (!IsRequiredProfile(name)) {
          m_profiles.erase(name);
        }
        return *this;
      }

      auto ClearAllProfiles() -> Optimization& {
        m_profiles.clear();

        m_profiles[RAPID_KEY] = Switch();
        m_profiles[DEBUG_KEY] = Switch();
        m_profiles[RELEASE_KEY] = Switch();
        return *this;
      }

      [[nodiscard]] auto GetRequirements() const -> const Requirements& { return m_requirements; }
      [[nodiscard]] auto GetRequirements() -> Requirements& { return m_requirements; }

      auto SetRequirements(const Requirements& requirements) -> Optimization& {
        m_requirements = requirements;
        return *this;
      }

    private:
      using Profiles = std::map<std::string, Switch>;

      mutable Profiles m_profiles;
      Requirements m_requirements;
    };

    class Dependency final {
      using UUID = std::string;

      UUID m_uuid;
      Version m_version;

    public:
      Dependency(UUID uuid, Version version) : m_uuid(std::move(uuid)), m_version(version) {}

      [[nodiscard]] auto operator<=>(const Dependency& o) const = default;

      [[nodiscard]] auto GetUUID() const -> const UUID& { return m_uuid; }
      [[nodiscard]] auto GetVersion() const -> const Version& { return m_version; }

      auto SetUUID(const UUID& uuid) -> Dependency& {
        m_uuid = uuid;
        return *this;
      }
      auto SetVersion(const Version& version) -> Dependency& {
        m_version = version;
        return *this;
      }
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
    [[nodiscard]] auto GetVersion() const -> const Version& { return m_version; }
    [[nodiscard]] auto GetContacts() const -> const std::vector<Contact>& { return m_contacts; }
    [[nodiscard]] auto GetPlatforms() const -> const Platforms& { return m_platforms; }
    [[nodiscard]] auto GetOptimization() const -> const Optimization& { return m_optimization; }
    [[nodiscard]] auto GetDependencies() const -> const std::vector<Dependency>& { return m_dependencies; }

    [[nodiscard]] auto GetName() -> std::string& { return m_name; }
    [[nodiscard]] auto GetDescription() -> std::string& { return m_description; }
    [[nodiscard]] auto GetLicense() -> std::string& { return m_license; }
    [[nodiscard]] auto GetCategory() -> Category& { return m_category; }
    [[nodiscard]] auto GetVersion() -> Version& { return m_version; }
    [[nodiscard]] auto GetContacts() -> std::vector<Contact>& { return m_contacts; }
    [[nodiscard]] auto GetPlatforms() -> Platforms& { return m_platforms; }
    [[nodiscard]] auto GetOptimization() -> Optimization& { return m_optimization; }
    [[nodiscard]] auto GetDependencies() -> std::vector<Dependency>& { return m_dependencies; }

    auto SetName(std::string name) -> Manifest& {
      m_name = std::move(name);
      return *this;
    }
    auto SetDescription(std::string description) -> Manifest& {
      m_description = std::move(description);
      return *this;
    }
    auto SetLicense(std::string spdx_license) -> Manifest& {
      m_license = std::move(spdx_license);
      return *this;
    }
    auto SetCategory(Category category) -> Manifest& {
      m_category = category;
      return *this;
    }
    auto SetVersion(Version version) -> Manifest& {
      m_version = version;
      return *this;
    }
    auto SetContacts(std::vector<Contact> contacts) -> Manifest& {
      m_contacts = std::move(contacts);
      return *this;
    }
    auto SetPlatforms(Platforms platforms) -> Manifest& {
      m_platforms = std::move(platforms);
      return *this;
    }
    auto SetOptimization(Optimization optimization) -> Manifest& {
      m_optimization = std::move(optimization);
      return *this;
    }
    auto SetDependencies(std::vector<Dependency> dependencies) -> Manifest& {
      m_dependencies = std::move(dependencies);
      return *this;
    }

    auto AddContact(const Contact& contact) -> Manifest& {
      m_contacts.push_back(contact);
      return *this;
    }
    auto ClearContacts() -> Manifest& {
      m_contacts.clear();
      return *this;
    }
    auto RemoveContact(const Contact& contact) -> Manifest& {
      m_contacts.erase(std::remove(m_contacts.begin(), m_contacts.end(), contact), m_contacts.end());
      return *this;
    }

    auto AddPlatformAllow(const std::string& allow) -> Manifest& {
      m_platforms.AddAllow(allow);
      return *this;
    }
    auto RemovePlatformAllow(const std::string& allow) -> Manifest& {
      m_platforms.RemoveAllow(allow);
      return *this;
    }
    auto ClearPlatformAllow() -> Manifest& {
      m_platforms.ClearAllow();
      return *this;
    }
    auto AddPlatformDeny(const std::string& deny) -> Manifest& {
      m_platforms.AddDeny(deny);
      return *this;
    }
    auto RemovePlatformDeny(const std::string& deny) -> Manifest& {
      m_platforms.RemoveDeny(deny);
      return *this;
    }
    auto ClearPlatformDeny() -> Manifest& {
      m_platforms.ClearDeny();
      return *this;
    }

    auto AddOptimizationProfile(const std::string& name, const Optimization::Switch& profile) -> Manifest& {
      m_optimization.SetProfile(name, profile);
      return *this;
    }
    auto RemoveOptimizationProfile(const std::string& name) -> Manifest& {
      m_optimization.ClearProfile(name);
      return *this;
    }
    auto ClearOptimizationProfiles() -> Manifest& {
      m_optimization.ClearAllProfiles();
      return *this;
    }

    auto AddDependency(const Dependency& dependency) -> Manifest& {
      m_dependencies.push_back(dependency);
      return *this;
    }
    auto ClearDependencies() -> Manifest& {
      m_dependencies.clear();
      return *this;
    }
    auto RemoveDependency(const Dependency& dependency) -> Manifest& {
      m_dependencies.erase(std::remove(m_dependencies.begin(), m_dependencies.end(), dependency), m_dependencies.end());
      return *this;
    }

    ///=============================================================================================
    ///==                               (DE)SERIALIZATION FUNCTIONS                               ==
    auto ToJson(std::ostream& os, bool& correct_schema, bool minify = false) const -> std::ostream&;
    auto ToJson(bool& correct_schema, bool minify = false) const -> std::string;
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
    Category m_category = Category::Executable;
    Version m_version;
    std::vector<Contact> m_contacts;
    Platforms m_platforms;
    Optimization m_optimization;
    std::vector<Dependency> m_dependencies;
  };
}  // namespace no3::package
