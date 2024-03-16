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

#define QUIXCC_INTERNAL

#include <prep/Preprocesser.h>
#include <cstdio>
#include <cctype>
#include <stdexcept>
#include <cstring>
#include <iomanip>
#include <regex>
#include <cmath>
#include <error/Message.h>
#include <LibMacro.h>
#include <filesystem>
#include <prep/AllMacros.h>

#define QUIX_HEADER_EXTENSION ".qh"
#define QUIX_STATICS_FILE "~statics.qh"

void libquixcc::PrepEngine::setup()
{
    m_macro_parser.add_routine("define", libquixcc::macro::ParseDefine);
    m_macro_parser.add_routine("pragma", libquixcc::macro::ParsePragma);
    m_macro_parser.add_routine("print", libquixcc::macro::ParsePrint);
    m_macro_parser.add_routine("readstdin", libquixcc::macro::ParseReadStdin);
    m_macro_parser.add_routine("encoding", libquixcc::macro::ParseEncoding);
    m_macro_parser.add_routine("lang", libquixcc::macro::ParseLang);
    m_macro_parser.add_routine("author", libquixcc::macro::ParseAuthor);
    m_macro_parser.add_routine("license", libquixcc::macro::ParseLicense);
}

void libquixcc::PrepEngine::addpath(const std::string &path)
{
    m_include_dirs.insert(path);

    include_path = "";
    for (auto &dir : m_include_dirs)
    {
        include_path += std::filesystem::absolute(dir).string();

        if (dir != *m_include_dirs.rbegin())
            include_path += ":";
    }
}

libquixcc::PrepEngine::Entry libquixcc::PrepEngine::build_statics_decl()
{
    std::stringstream declcode;
    declcode << "/* BEGIN AUTOGENERATED STATIC DECLARATIONS */\n";
    for (auto &statics : m_statics)
    {
        message(*m_job, Err::DEBUG, "Adding static declaration: %s = %s", statics.first.c_str(), statics.second.c_str());
        declcode << "@define " << statics.first << " " << statics.second << "\n";
    }
    declcode << "/* END AUTOGENERATED STATIC DECLARATIONS */\n";

    auto ptr = new std::string(declcode.str());
    FILE *f = fmemopen((void *)ptr->c_str(), ptr->size(), "r");
    if (!f)
        throw std::runtime_error("Failed to create statics declaration file");
    StreamLexer l;
    if (!l.set_source(f, QUIX_STATICS_FILE))
        throw std::runtime_error("Failed to set source for statics declaration");
    return Entry(l, QUIX_STATICS_FILE, f, ptr);
}

void libquixcc::PrepEngine::push(libquixcc::Token tok)
{
    m_tok = tok;
}

bool libquixcc::PrepEngine::set_source(FILE *src, const std::string &filename)
{
    StreamLexer l;
    if (!l.set_source(src, filename))
        return false;
    m_stack.push({l, filename, src});

    if (!m_statics.empty())
    {
        m_stack.push(build_statics_decl());
        m_include_files.push_back(QUIX_STATICS_FILE);
    }

    return true;
}

libquixcc::Token libquixcc::PrepEngine::next()
{
    Token tok = peek();
    m_tok = std::nullopt;
    return tok;
}

libquixcc::Token libquixcc::PrepEngine::peek()
{
    if (m_tok)
        return m_tok.value();

    m_tok = read_token();

    return m_tok.value();
}

bool libquixcc::PrepEngine::get_static(const std::string &name, std::string &value) const
{
    auto it = m_statics.find(name);
    if (it == m_statics.end())
        return false;
    value = it->second;
    return true;
}

bool libquixcc::PrepEngine::handle_import()
{
    // Get Identifier
    Token tok = m_stack.top().lexer.next();
    if (tok.type() != TokenType::Identifier)
    {
        PREPMSG(tok, Err::ERROR, "Expected identifier after import");
        return false;
    }

    std::string filename = std::get<std::string>(tok.val());
    filename = std::regex_replace(filename, std::regex("::"), "/") + QUIX_HEADER_EXTENSION;

    PREPMSG(tok, Err::DEBUG, "Requested import of file: %s", filename.c_str());

    tok = m_stack.top().lexer.next();
    if (tok.type() != TokenType::Punctor || std::get<Punctor>(tok.val()) != Punctor::Semicolon)
    {
        PREPMSG(tok, Err::ERROR, "Expected semicolon after import");
        return false;
    }

    PREPMSG(tok, Err::DEBUG, "Searching for file: %s in include directories [%s]", filename.c_str(), include_path.c_str());
    std::string filepath;
    for (auto &dir : m_include_dirs)
    {
        std::string path = dir + "/" + filename;
        if (!std::filesystem::exists(path))
            continue;

        filepath = path;
        break;
    }

    if (filepath.empty())
    {
        PREPMSG(tok, Err::ERROR, "Could not find file: \"%s\" in include directories [%s]", filename.c_str(), include_path.c_str());
        return false;
    }

    filepath = std::filesystem::absolute(filepath).string();

    // circular include detection
    if (std::find(m_include_files.begin(), m_include_files.end(), filepath) != m_include_files.end())
    {
        std::string msg;
        for (auto &file : m_include_files)
            msg += "  -> " + file + "\n";
        msg += "  -> " + filepath + "\n";
        PREPMSG(tok, Err::ERROR, "Circular include detected: \n%s", msg.c_str());
        PREPMSG(tok, Err::ERROR, "Refusing to enter infinite loop. Try to split your dependencies into smaller files.");
        return false;
    }

    // don't double include in the same file
    if (!m_stack.top().already_included.contains(filepath))
    {
        FILE *f;

        if (!(f = fopen(filepath.c_str(), "r")))
        {
            PREPMSG(tok, Err::ERROR, "Could not open file: \"%s.q\" in include directories [%s]", filepath.c_str(), include_path.c_str());
            return false;
        }

        m_stack.top().already_included.insert(filepath);
        m_include_files.push_back(filepath);
        m_stack.push({StreamLexer(), filepath, f});
        m_stack.top().lexer.set_source(f, filepath);
        return true;
    }
    else
    {
        PREPMSG(tok, Err::ERROR, feedback[PREP_DUPLICATE_IMPORT], m_stack.top().path.c_str(), filepath.c_str());
        return false;
    }

    return true;
}

#include <iostream>

bool libquixcc::PrepEngine::handle_macro(const libquixcc::Token &tok)
{
    std::vector<libquixcc::Token> expanded;
    if (!m_macro_parser.parse(tok, expanded))
    {
        PREPMSG(tok, Err::ERROR, "Failed to expand macro");
        return false;
    }

    for (auto &tok : expanded)
    {
        if (tok.type() == TokenType::MacroSingleLine || tok.type() == TokenType::MacroBlock)
        {
            if (!handle_macro(tok))
                return false;
        }
        else
        {
            m_buffer.push(tok);
        }
    }

    return true;
}

libquixcc::Token libquixcc::PrepEngine::read_token()
{
    libquixcc::Token tok;

    while (true)
    {
        while (!m_buffer.empty())
        {
            tok = m_buffer.front();
            m_buffer.pop();
            goto end;
        }

        if (m_stack.empty())
        {
            tok = Token(TokenType::Eof, "");
            goto end;
        }

        tok = m_stack.top().lexer.next();

        if (tok.type() == TokenType::Keyword && std::get<Keyword>(tok.val()) == Keyword::Import)
        {
            if (!handle_import())
                PREPMSG(tok, Err::ERROR, "Failed to handle import");

            continue;
        }
        else if (tok.type() == TokenType::MacroSingleLine || tok.type() == TokenType::MacroBlock)
        {
            if (!handle_macro(tok))
                PREPMSG(tok, Err::ERROR, "Failed to process macro");

            continue;
        }
        else if (tok.type() == TokenType::Eof)
        {
            if (m_stack.size() == 1)
                goto end;

            fclose(m_stack.top().file);
            m_include_files.pop_back();
            if (m_stack.top().buffer)
                delete m_stack.top().buffer;
            m_stack.pop();
            continue;
        }

        break;
    }

end:
    // std::cout << "tok: " << tok.serialize() << std::endl;
    return tok;
}
