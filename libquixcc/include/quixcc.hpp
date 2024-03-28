////////////////////////////////////////////////////////////////////////////////////
///                                                                              ///
///    ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///    ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///      ░▒▓█▓▒░                                                                 ///
///       ░▒▓██▓▒░                                                               ///
///                                                                              ///
///     * QUIX LANG COMPILER - The official compiler for the Quix language.      ///
///     * Copyright (c) 2024, Wesley C. Jones. All rights reserved.              ///
///     * License terms may be found in the LICENSE file.                        ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#ifndef __QUIXCC_HPP__
#define __QUIXCC_HPP__

#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <quixcc.h>

namespace quixcc
{

    /**
     * @brief Exception class for handling errors related to target triple.
     */
    class TargetTripleException : public std::runtime_error
    {
    public:
        TargetTripleException(const std::string &msg) : std::runtime_error(msg) {}
    };

    class TargetTriple
    {
        std::string m_triple;

    public:
        /**
         * @brief Default constructor.
         *
         * Initializes a TargetTriple object with default settings.
         *
         * @param triple The LLVM target triple.
         * @throw TargetTripleException if the target triple is invalid.
         * @note Will validate the target triple against the LLVM TargetRegistry.
         * @note An empty string is a special case and will use the Host Target Triple returned by `llvm::sys::getDefaultTargetTriple()`.
         */
        TargetTriple(const std::string &triple = "") { TargetTriple(triple.c_str()); }

        /**
         * @brief Default constructor.
         *
         * Initializes a TargetTriple object with default settings.
         *
         * @param triple The LLVM target triple.
         * @throw TargetTripleException if the target triple is invalid.
         * @note Will validate the target triple against the LLVM TargetRegistry.
         * @note An empty string is a special case and will use the Host Target Triple returned by `llvm::sys::getDefaultTargetTriple()`.
         */
        TargetTriple(const char *triple);

        /**
         * @brief Get the target triple.
         *
         * Retrieves the LLVM target triple.
         *
         * @return The LLVM target triple.
         */
        const std::string &triple() const { return m_triple; }
    };

    /**
     * @brief Compiler class for managing compilation jobs.
     *
     * This class provides functionality to manage compilation jobs using QUIX compiler.
     */
    class Compiler
    {
        std::vector<quixcc_job_t *> m_jobs;                                      ///< Vector to store compiler job instances.
        std::vector<std::pair<std::string, enum quixcc_msg_level_t>> m_messages; ///< Vector to store messages generated during compilation.
        std::vector<std::string> m_oscmds;                                       ///< Vector to store OS commands for post-compilation tasks.
        bool m_ok;                                                               ///< Flag indicating compilation status.

        Compiler(const Compiler &) = delete;
        Compiler(Compiler &&) = delete;
        Compiler &operator=(const Compiler &) = delete;

    public:
        /**
         * @brief Default constructor.
         *
         * Initializes a Compiler object.
         *
         * @param jobs Vector containing compilation job instances.
         */
        Compiler(std::vector<quixcc_job_t *> jobs, std::vector<std::string> oscmds);

        /**
         * @brief Destructor.
         *
         * Cleans up resources associated with the Compiler object.
         */
        ~Compiler();

        /**
         * @brief Execute the compilation job.
         *
         * Runs the compilation job with optional maximum thread count.
         *
         * @param max_threads Maximum number of threads to use for compilation (default is 4).
         * @return Reference to the Compiler object.
         */
        Compiler &run(size_t max_threads = 4);

        /**
         * @brief Get compilation messages.
         *
         * Retrieves messages generated during compilation.
         *
         * @return A constant reference to the vector of compilation messages.
         */
        const std::vector<std::pair<std::string, enum quixcc_msg_level_t>> &messages() const;

        /**
         * @brief Print generated messages to output streams.
         *
         * Prints messages generated after compilation to output streams.
         *
         * @param normal Output stream for normal messages (default is std::cout).
         * @param error Output stream for error messages (default is std::cerr).
         * @return Reference to the Compiler object.
         */
        Compiler &puts(std::ostream &normal = std::cout, std::ostream &error = std::cerr);

        /**
         * @brief Check if compilation was successful.
         *
         * Checks if the compilation job completed successfully.
         *
         * @return true if compilation was successful, false otherwise.
         */
        bool ok() const;

        /**
         * @brief Finish chaining and return the Compiler Object by value
         *
         * @return Compiler
         */
        Compiler done() const;
    };

    /**
     * @brief Builder class for constructing Compiler objects with specific configurations.
     *
     * This class provides a convenient interface to construct Compiler objects with various settings
     * such as input files, compiler flags, output file, target platform, and verbosity level.
     */
    class CompilerBuilder
    {
        std::vector<std::pair<FILE *, std::string>> m_files; ///< Vector to store input files.
        std::vector<std::string> m_flags;                    ///< Vector to store compiler flags.
        std::vector<std::string> m_oscmds;                   ///< Vector to store OS commands for post-compilation tasks.
        FILE *m_input;                                       ///< Input stream for providing source code directly.
        FILE *m_output;                                      ///< Output stream for capturing compiled output.
        TargetTriple m_target;                               ///< Target platform for compilation.
        bool m_verbose;                                      ///< Flag indicating verbosity level.
        bool m_debug;                                        ///< Flag indicating debug mode.
        bool m_disregard;                                    ///< Flag indicating whether to disregard output from compilation.

    public:
        /**
         * @brief Default constructor.
         *
         * Initializes a CompilerBuilder object with default settings.
         */
        CompilerBuilder();

        /**
         * @brief Destructor.
         *
         * Cleans up resources associated with the CompilerBuilder object.
         */
        ~CompilerBuilder();

        /**
         * @brief Set input file(s) for compilation.
         *
         * Adds one or more input files for compilation.
         *
         * @param file Path to an input file.
         * @return Reference to the CompilerBuilder object.
         */
        CompilerBuilder &in(const std::string &file);

        /**
         * @brief Set input file(s) for compilation.
         *
         * Adds multiple input files for compilation.
         *
         * @param files Vector containing paths to input files.
         * @return Reference to the CompilerBuilder object.
         */
        CompilerBuilder &in(const std::vector<std::string> &files);

        /**
         * @brief Provide source code directly for compilation.
         *
         * Sets the source code directly for compilation instead of using input files.
         *
         * @param code The source code to be compiled.
         * @return Reference to the CompilerBuilder object.
         */
        CompilerBuilder &src(const char *code, size_t size);

        /**
         * @brief Set input stream for providing source code directly.
         *
         * Sets the input stream for providing source code directly for compilation.
         *
         * @param input Input string stream for source code.
         * @return Reference to the CompilerBuilder object.
         */
        CompilerBuilder &src(FILE *input);

        /**
         * @brief Set output file for compiled output.
         *
         * Sets the output file where the compiled output will be written.
         *
         * @param file Path to the output file.
         * @return Reference to the CompilerBuilder object.
         */
        CompilerBuilder &out(const std::string &file);

        /**
         * @brief Set output stream for compiled output.
         *
         * Sets the output stream where the compiled output will be captured.
         *
         * @param output Output string stream for compiled output.
         * @return Reference to the CompilerBuilder object.
         */
        CompilerBuilder &out(FILE *output);

        /**
         * @brief Add compiler flag.
         *
         * Adds a compiler flag to the list of compiler options.
         *
         * @param flag The compiler flag to add.
         * @return Reference to the CompilerBuilder object.
         */
        CompilerBuilder &opt(const std::string &flag);

        /**
         * @brief Add multiple compiler flags.
         *
         * Adds multiple compiler flags to the list of compiler options.
         *
         * @param flags Vector containing compiler flags.
         * @return Reference to the CompilerBuilder object.
         */
        CompilerBuilder &opt(const std::vector<std::string> &flags);

        /**
         * @brief Add OS command for post-compilation tasks.
         *
         * Adds an OS command to the list of post-compilation tasks.
         *
         * @param cmd The OS command to add.
         * @return Reference to the CompilerBuilder object.
         * @note Commands will be invoked in the order they are added.
         * @note The args will be transformed as follows:
         *    - std::string pre = "";
         *    - foreach (auto &arg : args) { pre += "export " + arg + ";" }
         *    - cmd = pre + cmd;
         *    - system(cmd.c_str()) == 0 // success
         */
        CompilerBuilder &post(const std::string &cmd, const std::vector<std::string> &args = {});

        /**
         * @brief Disregard output from compilation.
         *
         * Sets the flag to disregard output from compilation.
         *
         * @param disregard Flag indicating whether to disregard output from compilation (default is true).
         * @return Reference to the CompilerBuilder object.
         */
        CompilerBuilder &disregard(bool disregard = true);

        /**
         * @brief Set target triple for compilation.
         *
         * Sets the target triple for compilation.
         *
         * @param target The LLVM target triple.
         * @return Reference to the CompilerBuilder object.
         */
        CompilerBuilder &target(TargetTriple target);

        /**
         * @brief Set verbosity level.
         *
         * Sets the verbosity level for compilation messages.
         *
         * @param verbose Flag indicating verbosity level (default is true).
         * @return Reference to the CompilerBuilder object.
         */
        CompilerBuilder &verbose(bool verbose = true);

        /**
         * @brief Set debug mode.
         *
         * Sets the debug mode for compilation.
         *
         * @param debug Flag indicating debug mode (default is true).
         * @return Reference to the CompilerBuilder object.
         */
        CompilerBuilder &debug(bool debug = true);

        /**
         * @brief Verify the configuration.
         *
         * Verifies the configuration settings before building the Compiler object.
         *
         * @return true if the configuration is valid, false otherwise.
         */
        bool verify();

        /**
         * @brief Build the Compiler object.
         *
         * Constructs a Compiler object with the configured settings.
         *
         * @return The constructed Compiler object.
         */
        Compiler build();
    };

    /**
     * @brief Factory class for creating instances of CompilerBuilder with predefined configurations.
     *
     * This class provides static methods to create CompilerBuilder instances with specific configurations
     * tailored for various compilation tasks.
     */
    class CompilerBuilderFactory
    {
    public:
        /**
         * @brief Create a CompilerBuilder instance for linting.
         * @return CompilerBuilder configured for linting.
         */
        static CompilerBuilder createLinter();

        /**
         * @brief Create a CompilerBuilder instance for generating intermediate representation (IR).
         * @return CompilerBuilder configured for generating IR.
         */
        static CompilerBuilder createIR();

        /**
         * @brief Create a CompilerBuilder instance for generating IR bitcode.
         * @return CompilerBuilder configured for generating IR bitcode.
         */
        static CompilerBuilder createIRBitcode();

        /**
         * @brief Create a CompilerBuilder instance for generating assembly code.
         * @return CompilerBuilder configured for generating assembly code.
         */
        static CompilerBuilder createAssembly();

        /**
         * @brief Create a CompilerBuilder instance for generating object files.
         * @return CompilerBuilder configured for generating object files.
         */
        static CompilerBuilder createObject();

        /**
         * @brief Create a CompilerBuilder instance for generating executable files.
         * @return CompilerBuilder configured for generating executable files.
         */
        static CompilerBuilder createExecutable();

        /**
         * @brief Create a CompilerBuilder instance for generating UPX-compressed executable files.
         * @return CompilerBuilder configured for generating UPX-compressed executable files.
         */
        static CompilerBuilder createUpxCompressedExecutable();

        /**
         * @brief Create a CompilerBuilder instance for generating shellcode.
         * @return CompilerBuilder configured for generating shellcode.
         */
        static CompilerBuilder createShellcode();

        /**
         * @brief Create a CompilerBuilder instance for generating shared libraries.
         * @return CompilerBuilder configured for generating shared libraries.
         */
        static CompilerBuilder createSharedLibrary();

        /**
         * @brief Create a CompilerBuilder instance for generating static libraries.
         * @return CompilerBuilder configured for generating static libraries.
         */
        static CompilerBuilder createStaticLibrary();

        /**
         * @brief Create a CompilerBuilder instance for generating documentation.
         * @return CompilerBuilder configured for generating documentation.
         */
        static CompilerBuilder createDocumentation();
    };

    /**
     * @brief Class for compiling code as shellcode and executing it within the current process.
     *
     * This class provides a static method to compile code as shellcode and execute it.
     */
    class Execute
    {
    public:
        /**
         * @brief Compile code as shellcode and execute within the current process.
         *
         * @param code The code to compile and execute.
         * @param messages Output stream for compilation messages (default is std::cout).
         * @return true if the code was executed successfully, false otherwise.
         *
         * @warning Although failed jobs will return false, the actual code may segfault if it is not correct.
         */
        static bool exec(const std::string &code, std::ostream &messages = std::cout);
    };
}

#endif // __QUIXCC_HPP__
