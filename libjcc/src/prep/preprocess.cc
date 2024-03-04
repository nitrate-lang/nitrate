#include <prep/preprocess.h>
#include <cstdio>
#include <cctype>
#include <stdexcept>
#include <cstring>
#include <iomanip>
#include <regex>
#include <cmath>
#include <error/message.h>
#include <macro.h>
#include <filesystem>
#include <prep/macros.h>

#define JLANG_HEADER_EXTENSION ".jh"
#define JLANG_STATICS_FILE "~statics.jh"

void libjcc::PrepEngine::setup()
{
    m_macro_parser.add_routine("define", libjcc::macro::ParseDefine);
}

void libjcc::PrepEngine::addpath(const std::string &path)
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

libjcc::PrepEngine::Entry libjcc::PrepEngine::build_statics_decl()
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
    if (!l.set_source(f, JLANG_STATICS_FILE))
        throw std::runtime_error("Failed to set source for statics declaration");
    return Entry(l, JLANG_STATICS_FILE, f, ptr);
}

bool libjcc::PrepEngine::set_source(FILE *src, const std::string &filename)
{
    StreamLexer l;
    if (!l.set_source(src, filename))
        return false;
    m_stack.push({l, filename, src});

    if (!m_statics.empty())
    {
        m_stack.push(build_statics_decl());
        m_include_files.push_back(JLANG_STATICS_FILE);
    }

    return true;
}

libjcc::Token libjcc::PrepEngine::next()
{
    Token tok = peek();
    m_tok = std::nullopt;
    return tok;
}

libjcc::Token libjcc::PrepEngine::peek()
{
    if (m_tok)
        return m_tok.value();

    m_tok = read_token();

    return m_tok.value();
}

bool libjcc::PrepEngine::get_static(const std::string &name, std::string &value) const
{
    auto it = m_statics.find(name);
    if (it == m_statics.end())
        return false;
    value = it->second;
    return true;
}

bool libjcc::PrepEngine::handle_import()
{
    // Get Identifier
    Token tok = m_stack.top().lexer.next();
    if (tok.type() != TokenType::Identifier)
    {
        PREPMSG(tok, Err::ERROR, "Expected identifier after import");
        return false;
    }

    std::string filename = std::get<std::string>(tok.val());
    filename = std::regex_replace(filename, std::regex("::"), "/") + JLANG_HEADER_EXTENSION;

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
        PREPMSG(tok, Err::ERROR, "Could not find file: \"%s.j\" in include directories [%s]", filename.c_str(), include_path.c_str());
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
            PREPMSG(tok, Err::ERROR, "Could not open file: \"%s.j\" in include directories [%s]", filepath.c_str(), include_path.c_str());
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

bool libjcc::PrepEngine::handle_macro(const libjcc::Token &tok)
{
    std::vector<libjcc::Token> expanded;
    if (!m_macro_parser.parse(tok, expanded))
    {
        PREPMSG(tok, Err::ERROR, "Failed to expand macro");
        return false;
    }

    for (auto &tok : expanded)
        m_buffer.push(tok);

    return true;
}

libjcc::Token libjcc::PrepEngine::read_token()
{
    libjcc::Token tok;

    while (true)
    {
        while (!m_buffer.empty())
        {
            tok = m_buffer.front();
            m_buffer.pop();
            return tok;
        }

        if (m_stack.empty())
            return Token(TokenType::Eof, "");

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
                return tok;

            fclose(m_stack.top().file);
            m_include_files.pop_back();
            if (m_stack.top().buffer)
                delete m_stack.top().buffer;
            m_stack.pop();
            continue;
        }

        break;
    }

    return tok;
}
