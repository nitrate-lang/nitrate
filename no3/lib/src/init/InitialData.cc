
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

#include <core/static/SPDX.hh>
#include <init/InitPackage.hh>
#include <nitrate-core/Assert.hh>
#include <optional>
#include <regex>
#include <sstream>
#include <string>

#include "core/package/Manifest.hh"

static const std::string_view DEFAULT_DOCKER_IGNORE = R"(.no3/
.git/
)";

static const std::string_view DEFAULT_GIT_IGNORE = R"(# Prerequisites
*.d

# Compiled Object files
*.slo
*.lo
*.o
*.obj

# Precompiled Headers
*.gch
*.pch

# Compiled Dynamic libraries
*.so
*.dylib
*.dll

# Fortran module files
*.mod
*.smod

# Compiled Static libraries
*.lai
*.la
*.a
*.lib

# Executables
*.exe
*.out
*.app

# Nitrate specific artifacts
.no3/

# Other
)";

static const std::string_view DEFAULT_CODE_OF_CONDUCT_MD = R"(# Contributor Covenant Code of Conduct

## Our Pledge

We as members, contributors, and leaders pledge to make participation in our
community a harassment-free experience for everyone, regardless of age, body
size, visible or invisible disability, ethnicity, sex characteristics, gender
identity and expression, level of experience, education, socio-economic status,
nationality, personal appearance, race, caste, color, religion, or sexual
identity and orientation.

We pledge to act and interact in ways that contribute to an open, welcoming,
diverse, inclusive, and healthy community.

## Our Standards

Examples of behavior that contributes to a positive environment for our
community include:

* Demonstrating empathy and kindness toward other people
* Being respectful of differing opinions, viewpoints, and experiences
* Giving and gracefully accepting constructive feedback
* Accepting responsibility and apologizing to those affected by our mistakes,
  and learning from the experience
* Focusing on what is best not just for us as individuals, but for the overall
  community

Examples of unacceptable behavior include:

* The use of sexualized language or imagery, and sexual attention or advances of
  any kind
* Trolling, insulting or derogatory comments, and personal or political attacks
* Public or private harassment
* Publishing others' private information, such as a physical or email address,
  without their explicit permission
* Other conduct which could reasonably be considered inappropriate in a
  professional setting

## Enforcement Responsibilities

Community leaders are responsible for clarifying and enforcing our standards of
acceptable behavior and will take appropriate and fair corrective action in
response to any behavior that they deem inappropriate, threatening, offensive,
or harmful.

Community leaders have the right and responsibility to remove, edit, or reject
comments, commits, code, wiki edits, issues, and other contributions that are
not aligned to this Code of Conduct, and will communicate reasons for moderation
decisions when appropriate.

## Scope

This Code of Conduct applies within all community spaces, and also applies when
an individual is officially representing the community in public spaces.
Examples of representing our community include using an official email address,
posting via an official social media account, or acting as an appointed
representative at an online or offline event.

## Enforcement

Instances of abusive, harassing, or otherwise unacceptable behavior may be
reported to the community leaders responsible for enforcement at
[INSERT CONTACT METHOD].
All complaints will be reviewed and investigated promptly and fairly.

All community leaders are obligated to respect the privacy and security of the
reporter of any incident.

## Enforcement Guidelines

Community leaders will follow these Community Impact Guidelines in determining
the consequences for any action they deem in violation of this Code of Conduct:

### 1. Correction

**Community Impact**: Use of inappropriate language or other behavior deemed
unprofessional or unwelcome in the community.

**Consequence**: A private, written warning from community leaders, providing
clarity around the nature of the violation and an explanation of why the
behavior was inappropriate. A public apology may be requested.

### 2. Warning

**Community Impact**: A violation through a single incident or series of
actions.

**Consequence**: A warning with consequences for continued behavior. No
interaction with the people involved, including unsolicited interaction with
those enforcing the Code of Conduct, for a specified period of time. This
includes avoiding interactions in community spaces as well as external channels
like social media. Violating these terms may lead to a temporary or permanent
ban.

### 3. Temporary Ban

**Community Impact**: A serious violation of community standards, including
sustained inappropriate behavior.

**Consequence**: A temporary ban from any sort of interaction or public
communication with the community for a specified period of time. No public or
private interaction with the people involved, including unsolicited interaction
with those enforcing the Code of Conduct, is allowed during this period.
Violating these terms may lead to a permanent ban.

### 4. Permanent Ban

**Community Impact**: Demonstrating a pattern of violation of community
standards, including sustained inappropriate behavior, harassment of an
individual, or aggression toward or disparagement of classes of individuals.

**Consequence**: A permanent ban from any sort of public interaction within the
community.

## Attribution

This Code of Conduct is adapted from the [Contributor Covenant][homepage],
version 2.1, available at
[https://www.contributor-covenant.org/version/2/1/code_of_conduct.html][v2.1].

Community Impact Guidelines were inspired by
[Mozilla's code of conduct enforcement ladder][Mozilla CoC].

For answers to common questions about this code of conduct, see the FAQ at
[https://www.contributor-covenant.org/faq][FAQ]. Translations are available at
[https://www.contributor-covenant.org/translations][translations].

[homepage]: https://www.contributor-covenant.org
[v2.1]: https://www.contributor-covenant.org/version/2/1/code_of_conduct.html
[Mozilla CoC]: https://github.com/mozilla/diversity
[FAQ]: https://www.contributor-covenant.org/faq
[translations]: https://www.contributor-covenant.org/translations
)";

static const std::string_view DEFAULT_GIT_KEEP;

static const std::string_view DEFAULT_LIB_N = R"(@use "v1.0";

import std::io;

scope example_lib {
  pub fn foo(): i32 {
    print("Hello, world!");
    ret 20;
  }

  pub fn pure bar(x: i32, y: str): i32 {
    print("x: ", x, ", y: ", y);
    ret x + y.len();
  }
}
)";

static const std::string_view DEFAULT_MAIN_N = R"(@use "v1.0";

import std.io;
import std.time;

pub fn main(args: [str]): i32 {
  let day = std::time::now().day_of_week();
  print(f"Welcome, it is a beautiful {day}!");

  if "--help" in args || "-h" in args {
    print("Usage: main [options]");
    print("Options:");
    print("  --help: Display this help message.");
    print("  --version: Display the version of the program.");
    ret 0;
  }

  if "--version" in args || "-v" in args {
    print("main v1.0.0");
    ret 0;
  }

  ret 0;
}
)";

auto no3::package::GenerateGitKeep() -> std::string { return std::string(DEFAULT_GIT_KEEP); }
auto no3::package::GenerateGitIgnore() -> std::string { return std::string(DEFAULT_GIT_IGNORE); }
auto no3::package::GenerateDockerIgnore() -> std::string { return std::string(DEFAULT_DOCKER_IGNORE); }
auto no3::package::GenerateDefaultLibrarySource() -> std::string { return std::string(DEFAULT_LIB_N); }
auto no3::package::GenerateDefaultMainSource() -> std::string { return std::string(DEFAULT_MAIN_N); }
auto no3::package::GenerateCodeOfConduct() -> std::string { return std::string(DEFAULT_CODE_OF_CONDUCT_MD); }

static auto GetGithubUsername(const std::string& name) -> std::optional<std::string> {
  if (name.starts_with("@gh-")) {
    return name.substr(4, name.find('/') - 4);
  }

  return std::nullopt;
}

static auto BeutifyName(std::string name) -> std::string {
  std::replace(name.begin(), name.end(), '-', ' ');

  // capitalize each word
  for (size_t i = 0; i < name.size(); i++) {
    if (i == 0 || name[i - 1] == ' ') {
      name[i] = std::toupper(name[i]);
    }
  }

  return name;
}

static auto GetPackageName(const std::string& name) -> std::string { return name.substr(name.find('/') + 1); }

auto no3::package::GenerateSecurityPolicy(const std::string& package_name) -> std::string {
  // Note this security policy contains a bug bounty clause.

  const auto github_username = GetGithubUsername(package_name);
  const auto name = GetPackageName(package_name);
  const auto nice_name = BeutifyName(name);

  std::string content;

  content +=
      R"(# Reporting Security Issues

The ("{{project_name_nice}}") project team and community take security bugs in
the ("{{project_name_nice}}") project seriously.
We appreciate your efforts to disclose your findings responsibly and will make
every effort to acknowledge your contributions. Pursuant thereto, and contingent
on the notability of the issue and the availability of monetary resources, we
may offer a reward for the responsible disclosure of security vulnerabilities.

)";

  if (github_username.has_value()) {
    content += R"(Please use the GitHub Security Advisory
["Report a Vulnerability"](https://github.com/{{gh_username}}/{{project_name}}/security/advisories/new)
tab to report a security issue.

)";

    content = std::regex_replace(content, std::regex(R"(\{\{gh_username\}\})"), github_username.value());
  }

  content +=

      R"(The ("{{project_name_nice}}") project team will send a response indicating the next steps in handling
your report. After the initial reply to your report, the security team will keep
you informed of the progress toward a fix and full announcement and may ask for
additional information or guidance.

Report security bugs in third-party modules to the person or team maintaining the module.

Thank you for keeping the ("{{project_name_nice}}") project and its community safe.

---
*This security policy is auto-generated for the ("{{project_name_nice}}") project.*
)";

  content = std::regex_replace(content, std::regex(R"(\{\{project_name_nice\}\})"), nice_name);
  content = std::regex_replace(content, std::regex(R"(\{\{project_name\}\})"), name);

  return content;
}

static auto URLEncode(std::string_view text) -> std::string {
  std::stringstream ss;

  for (const auto& c : text) {
    if ((std::isalnum(c) != 0) || c == '-' || c == '_' || c == '.' || c == '~') {
      ss << c;
    } else {
      ss << '%' << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    }
  }

  return ss.str();
}

static auto ShieldsIOEscapeContent(std::string text) -> std::string {
  text = std::regex_replace(text, std::regex("-"), "--");
  return URLEncode(text);
}

auto no3::package::GenerateReadme(const InitOptions& options) -> std::string {
  const auto gh_username = GetGithubUsername(options.m_package_name);
  const auto name = GetPackageName(options.m_package_name);
  const auto nice_name = BeutifyName(name);
  const auto shields_io_license = ShieldsIOEscapeContent(options.m_package_license);
  const auto spdx_license = options.m_package_license;
  const auto project_description = options.m_package_description;
  const auto* project_category = [&]() {
    switch (options.m_package_category) {
      case Manifest::Category::Library:
        return "library";
      case Manifest::Category::StandardLibrary:
        return "stdlib";
      case Manifest::Category::Executable:
        return "exe";
    }
  }();

  std::string content;

  content += R"(# {{project_name_nice}}

![](https://img.shields.io/badge/license-{{project_escaped_spdx_license}}-b3e32d.svg)
![](https://img.shields.io/badge/package_kind-{{project_category}}-cyan.svg)
![](https://img.shields.io/badge/cmake_integration-true-purple.svg)

## Overview

{{project_description}}

## Table of Contents

- [{{project_name_nice}}](#{{project_name}})
  - [Overview](#overview)
  - [Table of Contents](#table-of-contents)
  - [Installation](#installation)
  - [Features](#features)
  - [Technology](#technology)
  - [Contributing](#contributing)
  - [License](#license)

## Installation

)";

  if (gh_username.has_value()) {
    switch (options.m_package_category) {
      case Manifest::Category::Library:
      case Manifest::Category::StandardLibrary: {
        content += R"(```bash
# Change the working directory to your package
cd <your_project>

# Install this package as a dependency
nitrate install https://github.com/{{gh_username}}/{{project_name}}
```)";
        break;
      }

      case Manifest::Category::Executable: {
        content += R"(```bash
nitrate install https://github.com/{{gh_username}}/{{project_name}}
```)";
      }
    }

    content = std::regex_replace(content, std::regex(R"(\{\{gh_username\}\})"), gh_username.value());
  } else {
    if (options.m_package_category == Manifest::Category::StandardLibrary) {
      content += R"(This package should be installed by default with the Nitrate toolchain.)";
    } else {
      content += R"(TODO: Write instructions on how to install this package.)";
    }
  }

  content += R"(

## Features

| Feature Name | Feature Description                  |
| ------------ | ------------------------------------ |
| Feature A    | Providing better handling of issue A |
| Feature B    | Providing better handling of issue B |
| Feature C    | Providing better handling of issue C |

## Technology

| Tech Name        | URL                  |
| ---------------- | -------------------- |
| Nitrate Language | https://nitrate.dev/ |

## Contributing

Contributions are welcome! Please submit a pull request or open an issue if you have suggestions.

## License

This project is licensed under the **{{project_spdx_license}}** license. See the [LICENSE](LICENSE) file for more information.
)";

  content = std::regex_replace(content, std::regex(R"(\{\{project_name\}\})"), name);
  content = std::regex_replace(content, std::regex(R"(\{\{project_name_nice\}\})"), nice_name);
  content = std::regex_replace(content, std::regex(R"(\{\{project_escaped_spdx_license\}\})"), shields_io_license);
  content = std::regex_replace(content, std::regex(R"(\{\{project_spdx_license\}\})"), spdx_license);
  content = std::regex_replace(content, std::regex(R"(\{\{project_description\}\})"), project_description);
  content = std::regex_replace(content, std::regex(R"(\{\{project_category\}\})"), project_category);

  return content;
}

auto no3::package::GenerateContributingPolicy(const InitOptions& options) -> std::string {
  const auto nice_name = BeutifyName(GetPackageName(options.m_package_name));

  std::string content;

  content += R"(# Contributing to the ("{{project_name_nice}}") Project

**LEGAL NOTICE**

1. Regarding Your contributions and the legality thereof, all intellectual property
   delivered to the ("Maintainers") of this ("{{project_name_nice}}") project is
   required to be usable by the ("Maintainers") for any purpose reasonably
   foreseeable and/or expected by a software project maintainer.

2. To decline compliance with clause 1, conspicuously state these declinations at
least once per submission that does not comply with clause 1.

In summary, this means granting the project maintainers an eternal, worldwide, nonexclusive,
revocable license to use Your content to interact with You and the project's community.
The actual ownership of Your submissions is not affected by this clause.
)";

  content = std::regex_replace(content, std::regex(R"(\{\{project_name_nice\}\})"), nice_name);

  return content;
}

auto no3::package::GenerateCMakeListsTxt(const std::string& package_name) -> std::string {
  const auto project_name = GetPackageName(package_name);

  std::string content;

  content += R"(cmake_minimum_required(VERSION 3.15)
set(THIS_PROJECT_NAME "nitrate-{{project_name}}")
project(${THIS_PROJECT_NAME})

################################################################################
# USER CONFIGURATION SECTION
################################################################################
set(PACKAGE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
find_program(NITRATE_TOOL_EXE "nitrate" REQUIRED)

################################################################################
# SCRIPT INVARIANT ASSERTIONS
################################################################################
message(STATUS "Found compiler at ${NITRATE_TOOL_EXE}")
if(NOT BUILD_MODE)
  set(BUILD_MODE "--rapid")
elseif(NOT BUILD_MODE STREQUAL "--rapid" AND NOT BUILD_MODE STREQUAL "--debug" AND NOT BUILD_MODE STREQUAL "--release")
  message(FATAL_ERROR "Invalid build mode: ${BUILD_MODE}. Valid options are '--rapid', '--debug', '--release'")
else()
  message(STATUS "Building ${THIS_PROJECT_NAME} in ${BUILD_MODE} mode")
endif()

################################################################################
# INVOKE THE NITRATE BUILD TOOL
################################################################################
add_custom_target(
  ${THIS_PROJECT_NAME} # Set the target name
  ALL # Run this target everytime; Caching is the nitrate build tools job.
  COMMAND ${NITRATE_TOOL_EXE} build ${BUILD_MODE} ${PACKAGE_DIRECTORY} # Invoke the build tool
)
)";

  content = std::regex_replace(content, std::regex(R"(\{\{project_name\}\})"), project_name);

  return content;
}

auto no3::package::GenerateLicense(const std::string& spdx_license) -> std::string {
  return constants::GetSPDXLicenseText(spdx_license).value_or("");
}
