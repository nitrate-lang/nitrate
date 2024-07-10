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

#include <core/SHA160.h>
#include <quixcc/IR/Q/Function.h>
#include <quixcc/IR/Q/QIR.h>
#include <quixcc/IR/Q/Variable.h>
#include <quixcc/core/Logger.h>
#include <quixcc/lexer/Lex.h>
#include <quixcc/parsetree/Parser.h>
#include <quixcc/preprocessor/Preprocessor.h>

#include <boost/filesystem.hpp>
#include <filesystem>
#include <fstream>
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

static quixcc_cache_key_t calculate_key(rstr data) {
  core::SHA160 sha;
  quixcc_cache_key_t key;

  sha.process(data);
  sha.finalize(key.key);

  return key;
}

std::optional<std::string> PrepEngine::generate_adapter_entrypoint(
    PrepEngine::rstr metastatic) {
  stringstream ss;

  /* Begin Macro Entrypoint */
  ss << "/* Begin Macro Entrypoint */\n";
  ss << "pub \"C\" fn __quix_macro(argc: u32, argv: *string): string {\n";
  ss << "  import \"C\" {\n";
  ss << "    fn strdup(s: string): string;\n";
  ss << "  }\n\n";

  std::unique_ptr<ir::q::QModule> qmodule;
  ir::q::Segment *metafn = nullptr;
  std::string mname;

  { /* Parse and generate Quix-IR for the metastatic definition */
    std::shared_ptr<BlockNode> block;
    auto lexer = clone();
    lexer->set_source(metastatic, "metastatic");
    if (!parse(*job, lexer.get(), block, false, true) || !block) {
      LOG(ERROR) << "Failed to parse metastatic" << endl;
      return std::nullopt;
    }

    if (block->m_stmts.size() != 1) {
      LOG(ERROR) << "Metastatic must be a single function definition" << endl;
      return std::nullopt;
    }

    qmodule = std::make_unique<ir::q::QModule>("metatstatic");
    if (!qmodule->from_ptree(job, block)) {
      LOG(ERROR) << "Failed to convert metastatic to QIR" << endl;
      return std::nullopt;
    }

    if (qmodule->root()->children.size() != 1) {
      LOG(ERROR) << "Metastatic must be a single function definition" << endl;
      return std::nullopt;
    }

    if (!qmodule->root()->children.at(0)->is<ir::q::Global>()) {
      LOG(ERROR) << "Metastatic must be a function definition" << endl;
      return std::nullopt;
    }

    if (!qmodule->root()
             ->children.at(0)
             ->as<ir::q::Global>()
             ->value->is<ir::q::Segment>()) {
      LOG(ERROR) << "Metastatic must be a function definition" << endl;
      return std::nullopt;
    }

    /* Save the function name and the function definition */
    auto glob = qmodule->root()->children.at(0)->as<ir::q::Global>();
    metafn = glob->value->as<ir::q::Segment>();
    mname = glob->name;
  }

  /* Code to validate the number of arguments */
  size_t argc = metafn->params.size();
  ss << "  if argc != " << argc
     << " {\n"
        "    ret strdup(\"__scanner_halt__\");\n"
        "  }\n\n";

  /* Generate code to convert strings to the expected internal types */
  std::string arguments;
  size_t i = 0;

  for (auto it = metafn->params.begin(); it != metafn->params.end();
       it++, i++) {
    auto pparam = *it;
    auto p = pparam.second;

    switch ((ir::q::QType)p->ntype) {
      using namespace ir::q;

      case QType::I1:  /* 0-1 */
      case QType::U8:  /* 0-255 */
      case QType::U16: /* 0-65535 */
      case QType::I8:  /* -128-127 */
      case QType::I16: /* -32768-32767 */
      case QType::I32: /* -2147483648-2147483647 */
        arguments += "atoi(argv[" + to_string(i) + "])";
        break;
      case QType::U32: /* 0-4294967295 */
      case QType::I64: /* -9223372036854775808-9223372036854775807 */
        arguments += "atol(argv[" + to_string(i) + "])";
        break;
      case QType::U64: /* 0-18446744073709551615 */
        arguments += "atoll(argv[" + to_string(i) + "])";
        break;
      case QType::F32: /* ??? */
      case QType::F64: /* ??? */
        arguments += "atof(argv[" + to_string(i) + "])";
        break;
      case QType::I128:
      case QType::U128:
        LOG(ERROR) << "128-bit integers are not yet supported in macro "
                      "functions"
                   << endl;
        return std::nullopt;
      case QType::IntrinsicType: {
        switch (p->as<IntrinsicType>()->name) {
          case QIntrinsicType::String:
            arguments += "argv[" + to_string(i) + "]";
            break;
          default:
            LOG(ERROR) << "Unsupported intrinsic type in macro function"
                       << endl;
            return std::nullopt;
        }
        break;
      }

      default:
        LOG(ERROR) << "Unsupported parameter type in macro function" << endl;
        return std::nullopt;
    }

    if (i < argc - 1) {
      arguments += ", ";
    }
  }

  /* Call the internal function; Use `to_str()` builtin to convert to string */
  ss << "  ret strdup(" + mname + "(" + arguments + ").to_str());\n";
  ss << "}\n";

  ss << "/* End Macro Entrypoint */\n";

  return ss.str();
};

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

  /// TODO: change 'pub' here to 'namestyle' keyword (invent it)

  /* Begin Macro Body */
  ss << "/* Begin Macro Body */\n";
  ss << "pub \"C\" fn " << parameter << "\n";
  ss << "/* End Macro Body */\n\n";

  /* Begin Macro Entrypoint */
  auto ret = generate_adapter_entrypoint("pub \"C\" fn " + parameter + "\n");
  if (!ret.has_value()) {
    LOG(ERROR) << "Failed to generate adapter entrypoint" << endl;
    return "";
  }

  ss << ret.value();

  // std::cout << "\"" << ss.str() << "\"\n";

  return ss.str();
}

bool PrepEngine::acquire_shared_object(rstr metacode, vec8 &output) {
  char *rawbuf = nullptr;
  FILE *temp = nullptr;

  ///=========================================================================
  /// BEGIN: CHECK CACHE FOR SHARED OBJECT
  ///=========================================================================

  auto key = calculate_key(metacode);
  ssize_t objsize = quixcc_cache_has(&key);

  if (objsize > 0) {
    LOG(DEBUG) << "Cache hit on metacode" << endl;

    /*========== READ SHARED OBJECT FROM CACHE ============*/
    output.resize(objsize);
    if (!quixcc_cache_read(&key, output.data(), objsize)) {
      LOG(FATAL) << "Failed to read shared object from cache" << endl;
      return false;
    }

    /**
     * Validation is not our responsibility.
     * The cache provider is assumed to be reliable.
     */

    return true;
  }

  ///=========================================================================
  /// END: CHECK CACHE FOR SHARED OBJECT
  ///=========================================================================

  try {
    {
      ///=========================================================================
      /// BEGIN: COMPILE METACODE TO OBJECT
      ///=========================================================================

      LOG(DEBUG) << "Cache miss on metacode" << endl;

      size_t rawsz = 0;
      if ((temp = open_memstream(&rawbuf, &rawsz)) == nullptr) {
        LOG(FATAL) << "Failed to open memory stream for metacode" << endl;
        return false;
      }

      auto builder =
          ::quixcc::CompilerBuilder()
              .add_code(metacode.c_str(), metacode.size()) /* Add code */
              .set_flag("-fPIC") /* Generate position-independent code */
              .set_output(temp)  /* Output to memory stream */
              .set_flag("-c")    /* Compile only */
              .target("");       /* Default target */

      /* Carry over include directories */
      for (const auto &dir : m_include_dirs) {
        builder.add_include(dir);
      }

      /* Verify the compiler instance */
      if (!builder.verify()) {
        fclose(temp);
        free(rawbuf);
        LOG(FAILED) << "Failed to verify compiler for metacode" << endl;
        return false;
      }

      /* Build and run the compiler */
      if (!builder.build().run().puts().ok()) {
        fclose(temp);
        free(rawbuf);
        LOG(FAILED) << "Failed to build compiler for metacode" << endl;
        return false;
      }

      /* Implicitly Flush and close the memory stream */
      fclose(temp);

      /* Copy into STL vector */
      output.resize(rawsz);
      memcpy(output.data(), rawbuf, rawsz);
      free(rawbuf);

      ///=========================================================================
      /// END: COMPILE METACODE TO OBJECT
      ///=========================================================================
    }

    ///=========================================================================
    /// BEGIN: COMPILE OBJECT TO SHARED OBJECT
    ///=========================================================================

    {
      using namespace filesystem;
      namespace bfs = boost::filesystem;

      auto tmpname = absolute(bfs::unique_path().native()).string() + ".o";

      {  /// Write object to temp file
        ofstream temp_file(tmpname, ios::binary);
        if (!temp_file.is_open()) {
          LOG(FATAL) << "Failed to open temp file for shared object" << endl;
          return false;
        }

        temp_file.write((const char *)output.data(), output.size());
      }  /// End Write object to temp file

      {  /// INVOKE OS COMMAND TO COMPILE OBJECT TO SHARED OBJECT
        const std::string gcc_cmd =
            "qld -sharedlib -o " + tmpname + ".so " + tmpname;
        LOG(DEBUG) << "Compiling shared object: " << gcc_cmd << endl;

        if (system(gcc_cmd.c_str()) != 0) {
          std::filesystem::remove(tmpname);
          LOG(FATAL)
              << "Failed to compile shared object file during macro function "
                 "synthesis"
              << endl;
          return false;
        }

        std::filesystem::remove(tmpname);  // Remove the input object file
        tmpname += ".so";  // Update the filename to the output file
      }  /// End INVOKE OS COMMAND TO COMPILE OBJECT TO SHARED OBJECT

      {  /// READ OUTPUT FILE INTO MEMORY
        ifstream shared_object_file(tmpname, ios::binary);
        if (!shared_object_file.is_open()) {
          LOG(FATAL) << "Failed to open shared object file for reading" << endl;
          return false;
        }

        shared_object_file.seekg(0, ios::end);
        objsize = shared_object_file.tellg();
        shared_object_file.seekg(0, ios::beg);

        output.resize(objsize);
        if (!shared_object_file.read((char *)output.data(), objsize)) {
          LOG(FATAL) << "Failed to read shared object file" << endl;
          return false;
        }
      }  /// End READ OUTPUT FILE INTO MEMORY
    }

    /* Write shared object to cache */
    if (!quixcc_cache_write(&key, output.data(), output.size())) {
      LOG(WARN) << "libquixcc: Failed to write shared object to cache "
                   "provider. (no cache provider?)"
                << endl;
    }

    return true;
  } catch (const exception &e) {
    fclose(temp);
    free(rawbuf);
    LOG(FATAL) << "Failed to build compiler for metacode: " << e.what() << endl;
    return false;
  } catch (...) {
    fclose(temp);
    free(rawbuf);
    LOG(FATAL) << "Failed to build compiler for metacode: unknown error"
               << endl;
    return false;
  }

  ///=========================================================================
  /// END: COMPILE OBJECT TO SHARED OBJECT
  ///=========================================================================
}

bool PrepEngine::write_shared_object_to_temp_file(
    rstr metacode, unique_ptr<str, function<void(str *)>> &tempfile) {
  /*=================== COMPILE METACODE TO SHARED OBJECT ===================*/
  vec8 shared_object;
  if (!acquire_shared_object(metacode, shared_object)) {
    return false;
  }

  /*=================== WRITE SHARED OBJECT TO TEMP FILE =====================*/
  auto tmpname =
      filesystem::absolute(boost::filesystem::unique_path().native()).string() +
      ".o";

  {
    ofstream temp_file(tmpname, ios::binary);
    if (!temp_file.is_open()) {
      LOG(FATAL) << "Failed to open temp file for shared object" << endl;
      return false;
    }

    temp_file.write(reinterpret_cast<const char *>(shared_object.data()),
                    shared_object.size());
    temp_file.close();
  }

  std::string gcc_cmd = "gcc -shared -o " + tmpname + ".so " + tmpname;
  LOG(DEBUG) << "Compiling shared object: " << gcc_cmd << endl;

  if (system(gcc_cmd.c_str()) != 0) {
    std::filesystem::remove(tmpname);
    LOG(FATAL) << "Failed to compile shared object file during macro function "
                  "synthesis.\n"
               << endl;
    return false;
  }

  std::filesystem::remove(tmpname);

  tmpname += ".so";

  /*==================== CREATE A MANAGED DELETER FOR THE TEMP FILE =========*/
  tempfile =
      unique_ptr<str, std::function<void(str *)>>(new str(tmpname), [](str *s) {
        if (s) {
          std::filesystem::remove(*s);
          delete s;
        }
      });

  return true;
}

bool PrepEngine::load_shared_object(
    std::unique_ptr<std::string, std::function<void(std::string *)>> &filename,
    unique_ptr<void, function<void(void *)>> &managed_handle) {
  void *handle_raw = nullptr;

  /*==================== LOAD SHARED OBJECT INTO MEMORY =====================*/

  dlerror();  // Clear any existing errors

  handle_raw = dlopen(filename->c_str(), RTLD_LAZY);
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
  quixcc::quixcc_macro_fn_t fn_ptr = nullptr;

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
  fn_ptr = reinterpret_cast<quixcc::quixcc_macro_fn_t>(fn);
  job->m_dlhandles.insert(move(handle));
  job->m_macros[function_name] = fn_ptr;

  return true;
}

bool PrepEngine::ParseFn(const Token &tok, rstr directive, rstr parameter) {
  (void)tok;
  (void)directive;

  str metacode;
  unique_ptr<str, function<void(str *)>> temp_file_access(nullptr, [](str *s) {
    if (s) {
      filesystem::remove(*s);
      delete s;
    }
  });
  unique_ptr<void, function<void(void *)>> managed_handle(nullptr, [](void *h) {
    if (h) {
      dlclose(h);
    }
  });

  /*========================= SYNTHESIZE MACRO CODE= ========================*/
  metacode = build_macro_sourcecode(parameter);

  /*================== WRITE SHARED OBJECT TO TEMP FILE =====================*/
  if (!write_shared_object_to_temp_file(metacode, temp_file_access)) {
    LOG(FAILED) << "Failed to compile macro function" << tok << endl;
    return false;
  }

  /*==================== LOAD SHARED OBJECT INTO MEMORY =====================*/
  if (!load_shared_object(temp_file_access, managed_handle)) {
    LOG(FAILED) << "Failed to compile macro function" << tok << endl;
    return false;
  }

  /*================== FETCH FUNCTION POINTER FROM SHARED OBJECT ============*/
  str name = parameter.substr(parameter.find_first_not_of(" \t\n"));
  name = name.substr(0, name.find_first_of("("));

  if (!load_function_from_shared_object(name, managed_handle)) {
    LOG(FAILED) << "Failed to compile macro function" << tok << endl;
    return false;
  }

  return true;
}