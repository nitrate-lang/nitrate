////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///           ░▒▓██████▓▒░░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░            ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░                  ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓███████▓▒░░▒▓███████▓▒░░▒▓█▓▒▒▓███▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///           ░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░            ///
///             ░▒▓█▓▒░                                                      ///
///              ░▒▓██▓▒░                                                    ///
///                                                                          ///
///   * QUIX PACKAGE MANAGER - The official tool for the Quix language.      ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <atomic>
#include <build/Engine.hh>
#include <cassert>
#include <conf/Validate.hh>
#include <fstream>
#include <iostream>
#include <mutex>
#include <quixcc/Quix.hpp>
#include <set>
#include <thread>

#ifdef __linux__
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#endif

using namespace qpkg::core;
using namespace qpkg::build;

qpkg::build::Engine::Engine(const std::string &package_src, const std::string &output, int jobs,
                            std::unique_ptr<cache::ICache> cache,
                            qpkg::build::BuildSecurityConfig security,
                            qpkg::build::Optimization optimization, bool debug, bool verbose) {
  m_package_src = package_src;
  m_output = output;
  m_cache = std::move(cache);
  m_security = security;
  m_optimization = optimization;
  m_debug = debug;
  m_verbose = verbose;

  m_jobs = jobs;

  if (m_jobs == 0) {
#ifdef __linux__
    m_jobs = sysconf(_SC_NPROCESSORS_ONLN);

    if (m_jobs < 1)
      m_jobs = 1;
#else
    m_jobs = 1;
#endif
  }

  if (!quixcc_lib_init()) {
    LOG(core::ERROR) << "Failed to initialize QuixCC" << std::endl;
    exit(1);
  }
}

std::optional<qpkg::conf::Config>
qpkg::build::Engine::load_config(const std::filesystem::path &base) {
  if (std::filesystem::exists(base / "qpkg.yaml"))
    return conf::YamlConfigParser().parsef(base / "qpkg.yaml");
  else {
    LOG(core::ERROR) << "No configuration file found in package source directory" << std::endl;
    return std::nullopt;
  }
}

std::vector<std::string> qpkg::build::Engine::get_source_files(const std::filesystem::path &base) {
  std::vector<std::string> source_code_files;

  for (const auto &path : m_config["sources"].as<std::vector<std::string>>()) {
    std::filesystem::path abs(base / path);

    if (std::filesystem::is_directory(abs)) {
      for (const auto &entry : std::filesystem::recursive_directory_iterator(abs))
        if (entry.path().string().ends_with(".q") && entry.is_regular_file())
          source_code_files.push_back(std::filesystem::canonical(entry.path()).string());
    } else if (abs.string().ends_with(".q") && std::filesystem::is_regular_file(abs)) {
      source_code_files.push_back(std::filesystem::canonical(abs).string());
    }
  }

  return source_code_files;
}

static qpkg::cache::CacheKey compute_cachekey(const std::filesystem::path &file) {
  std::ifstream in(file, std::ios::binary);
  if (!in)
    throw std::runtime_error("Failed to open file");

  uint8_t buffer[4096];
  qpkg::cache::CacheHasher hasher;

  while (in) {
    in.read(reinterpret_cast<char *>(buffer), sizeof(buffer));
    hasher.update(buffer, in.gcount());
  }

  hasher.update(reinterpret_cast<const uint8_t *>(file.string().c_str()), file.string().size());

  return hasher.finalize();
}

bool qpkg::build::Engine::build_source_file(const std::filesystem::path &base,
                                            const std::filesystem::path &build_dir,
                                            const std::filesystem::path &file) const {
  cache::CacheKey key;
  std::string outfile = build_dir.string() + "/" + file.filename().string() + ".o";

  if (m_cache) {
    key = compute_cachekey(file);

    m_cache->acquire_lock();

    if (m_cache->contains(key)) {
      LOG(core::DEBUG) << "Using cached object file for " << file << std::endl;

      auto cache_file = m_cache->loadf(key);
      std::filesystem::copy_file(cache_file, outfile,
                                 std::filesystem::copy_options::overwrite_existing);

      m_cache->release_lock();
      return true;
    }

    m_cache->release_lock();
  }

  LOG(core::DEBUG) << "Compiling source file " << file << std::endl;

  quixcc::CompilerBuilder builder;
  builder.set_output(outfile);
  builder.set_debug(m_debug);
  builder.set_verbosity(m_verbose ? quixcc::Verbosity::VERY_VERBOSE : quixcc::Verbosity::NORMAL);
  switch (m_optimization) {
  case Optimization::OPTIMIZE_DEFAULT:
    builder.set_optimization(quixcc::OptimizationLevel::SPEED_2);
    break;
  case Optimization::OPTIMIZE_SPEED:
    builder.set_optimization(quixcc::OptimizationLevel::SPEED_4);
    break;
  case Optimization::OPTIMIZE_SIZE:
    builder.set_optimization(quixcc::OptimizationLevel::SIZE);
    break;
  }

  for (const auto &header : m_config["headers"].as<std::vector<std::string>>())
    builder.add_include((base / header).string());

  builder.add_include(base.string());
  builder.add_source(file.string());
  builder.opt("-c");
  for (const auto &flag : m_config["cflags"].as<std::vector<std::string>>())
    builder.opt(flag);

  if (m_build_type == BuildType::SHAREDLIB)
    builder.opt("-fPIC");
  else if (m_build_type == BuildType::STATICLIB)
    builder.opt("-fPIC");

  builder.target(m_config["triple"].as<std::string>());
  builder.cpu(m_config["cpu"].as<std::string>());

  if (!builder.build().run(1).puts().ok())
    return false;

  if (m_cache) {
    m_cache->acquire_lock();
    m_cache->storef(key, outfile);
    m_cache->release_lock();
  }

  return true;
}

bool qpkg::build::Engine::link_objects(const std::vector<std::filesystem::path> &objects) const {
  std::string cmd = "qld";

  for (const auto &flag : m_config["lflags"].as<std::vector<std::string>>())
    cmd += " " + flag;

  cmd += " -o '" + m_output + "'";

  switch (m_build_type) {
  case BuildType::SHAREDLIB:
    cmd += " -sharedlib";
    break;
  case BuildType::STATICLIB:
    cmd += " -staticlib";
    break;
  case BuildType::EXECUTABLE:
    break;
  }

  for (const auto &file : objects)
    cmd += " '" + file.string() + "'";

  if (m_debug)
    cmd += " -g";

  if (m_verbose)
    cmd += " -vv";

  LOG(core::DEBUG) << "Invoking linker: " << cmd << std::endl;

  if (system(cmd.c_str()) != 0)
    return false;

  return true;
}

bool qpkg::build::Engine::run_threads(const std::filesystem::path &base,
                                      const std::vector<std::string> &source_files,
                                      const std::filesystem::path &build_dir,
                                      qpkg::core::Process &log) const {
  size_t i, tcount;
  std::vector<std::thread> threads;
  std::atomic<bool> tainted = false;

  assert(source_files.size() > 0);

  tcount = std::min<size_t>(m_jobs, source_files.size());

  float w = 1.0f / source_files.size();

  for (i = 0; i < tcount; i++) {
    threads.push_back(
        std::thread([this, &base, &log, w, &build_dir, &source_files, i, tcount, &tainted]() {
          for (size_t j = i; j < source_files.size(); j += tcount) {
            log.good("Building QUIX object " + source_files[j]);

            if (!build_source_file(base, build_dir, source_files[j])) {
              log.error("Failed to build QUIX object " + source_files[j]);
              tainted = true;
              return;
            }

            log += w;
          }
        }));
  }

  for (auto &thread : threads)
    thread.join();

  return !tainted;
}

int qpkg_main(std::vector<std::string> args);

bool qpkg::build::Engine::build_package(const std::filesystem::path &base,
                                        const std::vector<std::string> &source_files,
                                        const std::filesystem::path &build_dir) {
  for (const auto &pkg : m_config["packages"].as<std::vector<std::string>>()) {
    std::filesystem::path pkg_path = std::filesystem::absolute(base / pkg);

    std::vector<std::string> args = {"qpkg", "build", pkg_path.string()};
    if (m_verbose)
      args.push_back("-v");
    if (!m_cache)
      args.push_back("-N");

    if (qpkg_main(args) != 0) {
      return false;
    }
  }

  auto log = Process::create("Build");

  log.info("Building package " + m_package_name);

  if (!source_files.empty()) {
    if (!run_threads(base, source_files, build_dir, log)) {
      LOG(core::ERROR) << "Failed to build package " << m_package_name << std::endl;
      return false;
    }

    std::vector<std::filesystem::path> objects;
    for (const auto &file : source_files)
      objects.push_back(build_dir / (std::filesystem::path(file).filename().string() + ".o"));

    if (m_config["nolink"].as<bool>() == false) {
      switch (m_build_type) {
      case BuildType::SHAREDLIB:
        log.bold("Linking QUIX shared library lib" + m_package_name + ".so");
        break;
      case BuildType::STATICLIB:
        log.bold("Linking QUIX static library lib" + m_package_name + ".a");
        break;
      case BuildType::EXECUTABLE:
        log.bold("Linking QUIX executable " + m_package_name);
        break;
      }

      if (!link_objects(objects)) {
        LOG(core::ERROR) << "Failed to link object files" << std::endl;
        return false;
      }

      log.info("Built target " + m_package_name, 1);
    } else {
      LOG(core::DEBUG) << "Skipping linking" << std::endl;

      log.info("Skipped linking");
    }

    return true;
  } else if (m_config["packages"].as<std::vector<std::string>>().empty()) {
    LOG(core::WARN) << "No source files found. Nothing to build" << std::endl;
    return true;
  }

  return true;
}

bool qpkg::build::Engine::run() {
  std::filesystem::path base(m_package_src);

  auto _config = load_config(base);

  if (!_config || !conf::ValidateConfig(*_config, base)) {
    LOG(core::ERROR) << "Failed to validate configuration" << std::endl;
    return false;
  }

  m_config = *_config;
  conf::PopulateConfig(m_config);

  m_package_name = m_config["name"].as<std::string>();

  std::filesystem::path build_dir = base / ".qpkg" / "build";

  if (!std::filesystem::exists(build_dir))
    std::filesystem::create_directories(build_dir);

  if (m_config["target"].as<std::string>() == "sharedlib")
    m_build_type = BuildType::SHAREDLIB;
  else if (m_config["target"].as<std::string>() == "staticlib")
    m_build_type = BuildType::STATICLIB;
  else if (m_config["target"].as<std::string>() == "executable")
    m_build_type = BuildType::EXECUTABLE;
  else {
    LOG(core::ERROR) << "Invalid target type" << std::endl;
    return false;
  }

  if (m_output.empty()) {
    switch (m_build_type) {
    case BuildType::SHAREDLIB:
      m_output = base / ("lib" + m_config["name"].as<std::string>() + ".so");
      std::filesystem::remove(m_output);
      break;
    case BuildType::STATICLIB:
      m_output = base / ("lib" + m_config["name"].as<std::string>() + ".a");
      std::filesystem::remove(m_output);
      break;
    case BuildType::EXECUTABLE:
      m_output = base / m_config["name"].as<std::string>();
      std::filesystem::remove(m_output);
      break;
    }
  }

  return build_package(base, get_source_files(base), build_dir);
}