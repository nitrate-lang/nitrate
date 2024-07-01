////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
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

#define QUIXCC_INTERNAL

#include <core/Logger.h>
#include <core/sha160.h>
#include <lexer/Lex.h>
#include <preprocessor/Preprocessor.h>

#include <boost/filesystem.hpp>
#include <filesystem>
#include <quixcc/Quix.hpp>

#if defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h>
#else
#error "Unsupported platform"
#endif

using namespace libquixcc;
using namespace std;
using rstr = const string &;
using str = string;
using vec8 = vector<uint8_t>;

static str calculate_key(rstr data) {
  core::SHA160 sha;
  array<uint8_t, 20> hash;
  str key;

  sha.process(data);
  sha.finalize(hash);

  key.resize(40);
  for (size_t i = 0; i < 20; i++) {
    sprintf(&key[i * 2], "%02x", hash[i]);
  }

  return key;
}

str PrepEngine::build_macro_sourcecode(rstr parameter) {
  stringstream ss;

  /* Language version header */
  ss << "@use \"v" << job->version->first << "." << job->version->second
     << "\"\n\n";

  /* Include all defined statics */
  ss << "/* Begin Static Definitions */\n";
  for (const auto &[key, value] : *m_statics) {
    ss << "@define " << key << " = " << value << ";\n";
  }
  ss << "/* End Static Definitions */\n\n";

  /* Include all defined macros */
  ss << "/* Begin Header Inclusions */\n";
  for (const auto &header : m_already_included) {
    ss << "@import \"" << header << "\"\n";
  }
  ss << "/* End Header Inclusions */\n\n";

  /* Begin Macro Body */
  ss << "/* Begin Macro Body */\n";
  ss << "fn " << parameter << "\n";
  ss << "/* End Macro Body */\n\n";

  /* Begin Macro Entrypoint */
  ss << "/* Begin Macro Entrypoint */\n";
  ss << "pub \"C\" fn __quix_macro(argc: u32, argv: **i8): *i8 {\n";
  ss << "  import \"C\" {\n";
  ss << "    fn strdup(s: *i8): *i8;\n";
  ss << "  }\n\n";

  std::string placeholder_str_notimpl = "\"\\\"" + escape_string(escape_string(parameter)) + " not implemented\\\"\"";
  ss << "  let exp = " << placeholder_str_notimpl << ";\n";
  /// TODO: implement based on parameter return type / in parameters

  ss << "  ret strdup(exp);\n";
  ss << "}\n";

  std::cout << "\"" << ss.str() << "\"\n";

  return ss.str();
}

bool PrepEngine::acquire_shared_object(rstr metacode, vec8 &shared_object) {
  ssize_t shared_object_size = 0;
  size_t raw_output_size = 0;
  char *raw_output = nullptr;
  FILE *temp = nullptr;

  /*=================== CHECK CACHE FOR SHARED OBJECT ========================*/
  auto key = calculate_key(metacode);
  shared_object_size = quixcc_cache_has(key.c_str(), key.size());

  if (shared_object_size < 0) {
    /**
     * If the shared object is not in the cache, we need to compile the metacode
     * and write the shared object to the cache.
     */

    LOG(DEBUG) << "Cache miss on metacode: " << key << endl;

    /*=================== CREATE MEMORY BUFFER FOR SHARED OBJECT
     * ==============*/
    temp = open_memstream(&raw_output, &raw_output_size);
    if (!temp) {
      LOG(FATAL) << "Failed to open memory stream for metacode" << endl;
      return false;
    }

    try {
      /**
       * Build the compiler object and compile the metacode to a shared object.
       *
       * -fPIC is used to generate position-independent code, which is required
       * for shared objects.
       */

      auto builder = quixcc::CompilerBuilder()
                         .add_code(metacode.c_str(), metacode.size())
                         .set_flag("-fPIC")
                         .set_output(temp)
                         .set_flag("-c")
                         .target("");

      /* Add "our" include directories to the compiler */
      for (const auto &dir : m_include_dirs) {
        builder.add_include(dir);
      }

      if (!builder.verify()) {
        fclose(temp);
        free(raw_output);
        LOG(FAILED) << "Failed to verify compiler for metacode" << endl;
        return false;
      }

      auto compiler = builder.build();

      /* Get the object code and print any 2nd order compiler messages */
      if (!compiler.run().puts().ok()) {
        fclose(temp);
        free(raw_output);
        LOG(FAILED) << "Failed to build compiler for metacode" << endl;
        return false;
      }

      /* Close the memory stream and write the shared object to the cache */
      fclose(temp);

      shared_object.resize(raw_output_size);
      memcpy(shared_object.data(), raw_output, raw_output_size);
      free(raw_output);

      /* Write the shared object to the cache */
      if (!quixcc_cache_write(key.c_str(), key.size(), shared_object.data(),
                              shared_object.size())) {
        LOG(WARN) << "Failed to write shared object to cache" << endl;
      }

      return true;

    } catch (const exception &e) {
      fclose(temp);
      free(raw_output);
      LOG(FATAL) << "Failed to build compiler for metacode: " << e.what()
                 << endl;
      return false;
    }
  }

  LOG(DEBUG) << "Cache hit on metacode: " << key << endl;

  /*=================== READ SHARED OBJECT FROM CACHE ========================*/
  shared_object.resize(shared_object_size);
  if (!quixcc_cache_read(key.c_str(), key.size(), shared_object.data(),
                         shared_object_size)) {
    LOG(FATAL) << "Failed to read shared object from cache" << endl;
    return false;
  }

  return true;
}

bool PrepEngine::write_shared_object_to_temp_file(
    const vec8 &shared_object,
    unique_ptr<str, function<void(str *)>> &tempfile) {
  /*=================== WRITE SHARED OBJECT TO TEMP FILE =====================*/
  auto tmpname =
      filesystem::absolute(boost::filesystem::unique_path().native()).string() +
      ".o";

  ofstream temp_file(tmpname, ios::binary);
  if (!temp_file.is_open()) {
    LOG(FATAL) << "Failed to open temp file for shared object" << endl;
    return false;
  }

  temp_file.write(reinterpret_cast<const char *>(shared_object.data()),
                  shared_object.size());
  temp_file.close();

  if (system(("gcc -shared -o " + tmpname + ".so " + tmpname).c_str()) != 0) {
    LOG(FATAL) << "Failed to compile shared object file during macro function "
                  "synthesis.\n";
    return false;
  }

  std::filesystem::remove(tmpname);

  tmpname += ".so";

  /*==================== CREATE A MANAGED DELETER FOR THE TEMP FILE =========*/
  tempfile = unique_ptr<str, void (*)(str *)>(new str(tmpname), [](str *s) {
    if (s) {
      filesystem::remove(*s);
      delete s;
    }
  });

  return true;
}

bool PrepEngine::load_shared_object(
    rstr filename, unique_ptr<void, function<void(void *)>> &managed_handle) {
  void *handle_raw = nullptr;

  /*==================== LOAD SHARED OBJECT INTO MEMORY =====================*/

  dlerror();  // Clear any existing errors

  handle_raw = dlopen(filename.c_str(), RTLD_LAZY);
  if (!handle_raw) {
    LOG(FATAL) << "Preprocessor failed to load shared object file during macro "
                  "function synthesis.\n";
    LOG(FATAL) << "Dlerror: " << dlerror() << endl;
    return false;
  }

  managed_handle.reset(handle_raw);

  return true;
}

bool PrepEngine::load_function_from_shared_object(
    rstr function_name, unique_ptr<void, function<void(void *)>> &handle) {
  void *fn = nullptr;
  quixcc_macro_fn_t fn_ptr = nullptr;

  /*==================== FETCH FUNCTION POINTER FROM SHARED OBJECT ===========*/

  LOG(DEBUG) << "Fetching internal symbol (Fn): __quix_macro" << endl;
  fn = dlsym(handle.get(), "__quix_macro");
  if (!fn) {
    LOG(FATAL) << "Preprocessor failed to get function pointer from shared "
                  "lib during macro "
                  "function synthesis\n";
    LOG(FATAL) << "Dlerror: " << dlerror() << endl;
    return false;
  }

  /*==================== INSERT FUNCTION POINTER INTO CTX ====================*/
  fn_ptr = reinterpret_cast<quixcc_macro_fn_t>(fn);
  job->m_dlhandles.insert(move(handle));
  job->m_macros[function_name] = fn_ptr;

  return true;
}

bool PrepEngine::ParseFn(const Token &tok, rstr directive, rstr parameter) {
  /// TODO: Implement the Fn directive

  (void)tok;
  (void)directive;

  str metacode;
  vec8 shared_object;
  unique_ptr<str, function<void(str *)>> temp_file_access(nullptr, [](str *s) {
    if (s) {
      filesystem::remove(*s);
      delete s;
    }
  });
  unique_ptr<void, function<void(void *)>> managed_handle(nullptr,
                                                          [this](void *h) {
                                                            if (h) {
                                                              dlclose(h);
                                                            }
                                                          });

  /*========================= SYNTHESIZE MACRO CODE= ========================*/
  metacode = build_macro_sourcecode(parameter);

  /*=================== COMPILE METACODE TO SHARED OBJECT ===================*/
  if (!acquire_shared_object(metacode, shared_object)) {
    LOG(FAILED) << "Failed to compile macro function" << tok << endl;
    return false;
  }

  /*================== WRITE SHARED OBJECT TO TEMP FILE =====================*/
  if (!write_shared_object_to_temp_file(shared_object, temp_file_access)) {
    return false;
  }

  /*==================== LOAD SHARED OBJECT INTO MEMORY =====================*/
  if (!load_shared_object(*temp_file_access, managed_handle)) {
    return false;
  }

  /*================== FETCH FUNCTION POINTER FROM SHARED OBJECT ============*/
  str name = parameter.substr(parameter.find_first_not_of(" \t\n"));
  name = name.substr(0, name.find_first_of("("));

  if (!load_function_from_shared_object(name, managed_handle)) {
    return false;
  }

  return true;
}