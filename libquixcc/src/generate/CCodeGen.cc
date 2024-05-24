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
///     * Copyright (C) 2024 Wesley C. Jones                                     ///
///                                                                              ///
///     The QUIX Compiler Suite is free software; you can redistribute it and/or ///
///     modify it under the terms of the GNU Lesser General Public               ///
///     License as published by the Free Software Foundation; either             ///
///     version 2.1 of the License, or (at your option) any later version.       ///
///                                                                              ///
///     The QUIX Compiler Suite is distributed in the hope that it will be       ///
///     useful, but WITHOUT ANY WARRANTY; without even the implied warranty of   ///
///     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        ///
///     Lesser General Public License for more details.                          ///
///                                                                              ///
///     You should have received a copy of the GNU Lesser General Public         ///
///     License along with the QUIX Compiler Suite; if not, see                  ///
///     <https://www.gnu.org/licenses/>.                                         ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#define QUIXCC_INTERNAL

#include <generate/CodeGen.h>
#include <mangle/Symbol.h>
#include <core/Logger.h>

#include <iomanip>
#include <openssl/evp.h>

using namespace libquixcc;
using namespace libquixcc::ir;
using namespace libquixcc::ir::delta;

static std::string escape_cstr(const std::string &str)
{
    std::string escaped;
    for (char c : str)
    {
        switch (c)
        {
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        case '\v':
            escaped += "\\v";
            break;
        case '\f':
            escaped += "\\f";
            break;
        case '\a':
            escaped += "\\a";
            break;
        case '\b':
            escaped += "\\b";
            break;
        case '\\':
            escaped += "\\\\";
            break;
        case '\"':
            escaped += "\\\"";
            break;
        default:
            escaped += c;
            break;
        }
    }
    return escaped;
}

std::string libquixcc::C11Codegen::ind()
{
    return std::string(m_state.indent * INDENT_SIZE, ' ');
}

std::string libquixcc::C11Codegen::gen(const ir::delta::I1 *n)
{
    return "qbool";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::I8 *n)
{
    return "qint8";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::I16 *n)
{
    return "qint16";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::I32 *n)
{
    return "qint32";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::I64 *n)
{
    return "qint64";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::I128 *n)
{
    return "qint128";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::U8 *n)
{
    return "quint8";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::U16 *n)
{
    return "quint16";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::U32 *n)
{
    return "quint32";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::U64 *n)
{
    return "quint64";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::U128 *n)
{
    return "quint128";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::F32 *n)
{
    return "qfloat32";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::F64 *n)
{
    return "qfloat64";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Void *n)
{
    return "qvoid";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Ptr *n)
{
    return gen(n->type) + " *";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::PacketDef *n)
{
    std::stringstream code;

    code << "struct quix_" << n->name << " {\n";

    m_state.indent++;

    for (size_t i = 0; i < n->fields.size(); i++)
        code << ind() << gen(n->fields[i].second) << " _" << std::to_string(i) << ";\n";

    m_state.indent--;

    code << "} __attribute__((packed));\n";

    m_state.structs.push_back(code.str());

    return "";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Packet *n)
{
    return "struct quix_" + n->def->name;
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Array *n)
{
    return gen(n->type) + "[" + std::to_string(n->size) + "]";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::FType *n)
{
    std::stringstream code;

    code << gen(n->ret) << " (*)(";
    for (size_t i = 0; i < n->params.size(); i++)
    {
        code << gen(n->params[i]);
        if (i < n->params.size() - 1)
            code << ", ";
        else if (n->variadic)
            code << ", ...";
    }

    code << ")";

    return code.str();
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Local *n)
{
    std::stringstream code;

    if (n->type->is<Array>())
    {
        code << gen(n->type->as<Array>()->type);
        code << " " << n->name << "[" << n->type->as<Array>()->size << "]";
    }
    else
    {
        code << gen(n->type);
        if (!n->type->is<Ptr>())
            code << " ";
        code << n->name;
    }

    if (n->value)
        code << " = " << gen(n->value);

    return code.str();
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Global *n)
{
    std::stringstream code;

    enum class Linkage
    {
        Private,
        Public,
        External,
    };

    Linkage linkage = Linkage::Private;

    if (n->value && n->value->is<Segment>())
    {
        auto seg = n->value->as<Segment>();

        if (n->_extern && (!seg->block))
            linkage = Linkage::External;
        else if (n->_extern && seg->block)
            linkage = Linkage::Public;
    }
    else
    {
        if (n->_extern && (!n->value))
            linkage = Linkage::External;
        else if (n->_extern && n->value)
            linkage = Linkage::Public;
    }

    switch (linkage)
    {
    case Linkage::Private:
        code << "static ";
        break;
    case Linkage::Public:
        code << "qexport ";
        break;
    case Linkage::External:
        code << "extern ";
        break;
    }

    // DeltaIR Stored functions as values called Segments
    if (n->value && n->value->is<Segment>())
    {
        auto old = m_state.segmentvar;
        m_state.segmentvar = n;
        code << gen(n->value);
        m_state.segmentvar = old;
    }
    else
    {
        code << gen(n->type);

        if (!n->type->is<Ptr>())
            code << " ";
        code << n->name;

        if (n->value)
            code << " = " << gen(n->value);
    }

    return code.str();
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Number *n)
{
    /// TODO: Implement Number 128 bit stuff

    return n->value;
}

std::string libquixcc::C11Codegen::gen(const ir::delta::String *n)
{
    return "\"" + escape_cstr(n->value) + "\"";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::List *n)
{
    std::stringstream code;

    code << "{";

    for (size_t i = 0; i < n->values.size(); i++)
    {
        code << gen(n->values[i]);
        if (i < n->values.size() - 1)
            code << ", ";
    }

    code << "}";

    return code.str();
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Ident *n)
{
    return n->name;
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Assign *n)
{
    return gen(n->var) + " = " + gen(n->value);
}

std::string libquixcc::C11Codegen::gen(const ir::delta::AddressOf *n)
{
    return "&" + gen(n->lhs);
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Deref *n)
{
    return "*" + gen(n->lhs);
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Member *n)
{
    std::stringstream code;

    code << gen(n->lhs);

    if (n->lhs->infer()->is_ptr())
        code << "->";
    else
        code << ".";

    code << "_" << n->field;

    return code.str();
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Index *n)
{
    return gen(n->expr) + "[" + gen(n->index) + "]";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::SCast *n)
{
    return "(" + gen(n->type) + ")" + gen(n->value);
}

std::string libquixcc::C11Codegen::gen(const ir::delta::UCast *n)
{
    return "(" + gen(n->type) + ")" + gen(n->value);
}

std::string libquixcc::C11Codegen::gen(const ir::delta::PtrICast *n)
{
    return "(quintptr)" + gen(n->value);
}

std::string libquixcc::C11Codegen::gen(const ir::delta::IPtrCast *n)
{
    return "(" + gen(n->type) + ")" + "(quintptr)" + gen(n->value);
}

static std::string escape_into_unqiue_ident(const std::string &str)
{
    const char VALID_CHARS[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
    std::array<uint8_t, 32> hash;
    unsigned int len = 0;

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit(ctx, EVP_sha256());
    EVP_DigestUpdate(ctx, str.c_str(), str.size());
    EVP_DigestFinal(ctx, (uint8_t *)hash.data(), &len);
    EVP_MD_CTX_free(ctx);

    std::stringstream ss;
    for (size_t i = 0; i < 12; i++)
        ss << VALID_CHARS[hash[i] % (sizeof(VALID_CHARS) - 1)];

    return "_" + ss.str();
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Bitcast *n)
{
    std::stringstream fn;

    fn << "static inline " << gen(n->type) << " ";

    std::string name = "qbc" + escape_into_unqiue_ident(gen(n->value->infer()) + gen(n->type));

    fn << name << "(" << gen(n->value->infer()) << " x) {\n";
    fn << ind() << "union { " << gen(n->value->infer()) << " x; " << gen(n->type) << " y; } u;\n";
    fn << ind() << "u.x = x;\n";
    fn << ind() << "return u.y;\n";
    fn << "}\n";

    m_state.autogen.insert(fn.str());

    return name + "(" + gen(n->value) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::IfElse *n)
{
    std::stringstream code;

    code << "if (" << gen(n->cond) << ") ";
    code << gen(n->then);

    if (n->els)
        code << "else " << gen(n->els);

    return code.str();
}

std::string libquixcc::C11Codegen::gen(const ir::delta::While *n)
{
    std::stringstream code;

    code << "while (" << gen(n->cond) << ") ";
    code << gen(n->body);

    return code.str();
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Jmp *n)
{
    return "goto " + n->target;
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Label *n)
{
    std::stringstream code;

    code << n->name << ":";

    code << ind() << gen(n->code);

    return code.str();
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Ret *n)
{
    if (n->value)
        return "return " + gen(n->value);
    else
        return "return";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Call *n)
{
    std::stringstream code;

    code << n->callee << "(";
    for (size_t i = 0; i < n->args.size(); i++)
    {
        code << gen(n->args[i]);
        if (i < n->args.size() - 1)
            code << ", ";
    }

    code << ")";

    return code.str();
}

std::string libquixcc::C11Codegen::gen(const ir::delta::PtrCall *n)
{
    std::stringstream code;

    code << "(" << gen(n->callee) << ")(";
    for (size_t i = 0; i < n->args.size(); i++)
    {
        code << gen(n->args[i]);
        if (i < n->args.size() - 1)
            code << ", ";
    }

    code << ")";

    return code.str();
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Halt *n)
{
    /// TODO: Implement Halt
    throw std::runtime_error("C11Codegen: Halt not implemented");
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Block *n)
{
    std::stringstream code;

    code << "{\n";

    m_state.indent++;

    for (auto &node : n->stmts)
        code << ind() << gen(node) << ";\n";

    m_state.indent--;

    code << ind() << "}";

    return code.str();
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Segment *n)
{
    std::stringstream code;

    if (!m_state.segmentvar)
        throw std::runtime_error("C11Codegen: Segment outside of global scope");

    code << gen(m_state.segmentvar->type->as<FType>()->ret) << " ";
    code << m_state.segmentvar->name << "(";

    for (size_t i = 0; i < n->params.size(); i++)
    {
        code << gen(n->params[i].second);

        if (n->params[i].second->is<Ptr>())
            code << n->params[i].first;
        else
            code << " " << n->params[i].first;

        if (i < n->params.size() - 1)
            code << ", ";
        else if (n->variadic)
            code << ", ...";
    }

    code << ")";

    if (n->block)
        code << " " << gen(n->block);

    return code.str();
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Add *n)
{
    return "(" + gen(n->lhs) + " + " + gen(n->rhs) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Sub *n)
{
    return "(" + gen(n->lhs) + " - " + gen(n->rhs) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Mul *n)
{
    return "(" + gen(n->lhs) + " * " + gen(n->rhs) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Div *n)
{
    return "(" + gen(n->lhs) + " / " + gen(n->rhs) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Mod *n)
{
    return "(" + gen(n->lhs) + " % " + gen(n->rhs) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::BitAnd *n)
{
    return "(" + gen(n->lhs) + " & " + gen(n->rhs) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::BitOr *n)
{
    return "(" + gen(n->lhs) + " | " + gen(n->rhs) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::BitXor *n)
{
    return "(" + gen(n->lhs) + " ^ " + gen(n->rhs) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::BitNot *n)
{
    return "(~" + gen(n->operand) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Shl *n)
{
    return "(" + gen(n->lhs) + " << " + gen(n->rhs) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Shr *n)
{
    return "(" + gen(n->lhs) + " >> " + gen(n->rhs) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Rotl *n)
{
    return "QROTL_EXPR(" + gen(n->lhs) + ", " + gen(n->rhs) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Rotr *n)
{
    return "QROTR_EXPR(" + gen(n->lhs) + ", " + gen(n->rhs) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Eq *n)
{
    return "(" + gen(n->lhs) + " == " + gen(n->rhs) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Ne *n)
{
    return "(" + gen(n->lhs) + " != " + gen(n->rhs) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Lt *n)
{
    return "(" + gen(n->lhs) + " < " + gen(n->rhs) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Gt *n)
{
    return "(" + gen(n->lhs) + " > " + gen(n->rhs) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Le *n)
{
    return "(" + gen(n->lhs) + " <= " + gen(n->rhs) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Ge *n)
{
    return "(" + gen(n->lhs) + " >= " + gen(n->rhs) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::And *n)
{
    return "(" + gen(n->lhs) + " && " + gen(n->rhs) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Or *n)
{
    return "(" + gen(n->lhs) + " || " + gen(n->rhs) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Not *n)
{
    return "(!" + gen(n->operand) + ")";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::Xor *n)
{
    return "((!" + gen(n->lhs) + " && " + gen(n->rhs) + ") || (" + gen(n->lhs) + " && !" + gen(n->rhs) + "))";
}

std::string libquixcc::C11Codegen::gen(const ir::delta::RootNode *n)
{
    std::stringstream code;

    /* Comment headers */
    code << "/////////////////////////////////////////////////////////\n";
    code << "///                                                   ///\n";
    code << "///    ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒   ///\n";
    code << "///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒   ///\n";
    code << "///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒   ///\n";
    code << "///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░   ///\n";
    code << "///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒   ///\n";
    code << "///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒   ///\n";
    code << "///    ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒   ///\n";
    code << "///      ░▒▓█▓▒░                                      ///\n";
    code << "///       ░▒▓██▓▒░                                    ///\n";
    code << "///                                                   ///\n";
    code << "///   * This C (11) code was generated by the QUIXCC  ///\n";
    code << "///     Language Compiler Suite.                      ///\n";
    code << "///                                                   ///\n";
    code << "///   Transcompiled source code may be encumbered by  ///\n";
    code << "///   original source code licensing.                 ///\n";
    code << "///                                                   ///\n";
    code << "/////////////////////////////////////////////////////////\n\n";

    /* Include standard integer header */
    code << "#include <stdint.h>\n";
    code << "#include <stdbool.h>\n\n";

    code << "#ifndef __QUIX_CODEGEN__\n";
    code << "#define __QUIX_CODEGEN__\n\n";

    /* Define our language primitive types */
    code << "/* QUIX Primitive Type Definitions */\n";
    code << "typedef int8_t qint8;\n";
    code << "typedef int16_t qint16;\n";
    code << "typedef int32_t qint32;\n";
    code << "typedef int64_t qint64;\n";
    code << "typedef bool qbool;\n";
    code << "typedef uint8_t quint8;\n";
    code << "typedef uint16_t quint16;\n";
    code << "typedef uint32_t quint32;\n";
    code << "typedef uint64_t quint64;\n";
    code << "typedef float qfloat32;\n";
    code << "typedef double qfloat64;\n";
    code << "typedef void qvoid;\n\n";
    code << "#ifndef __SIZEOF_INT128__\n";
    code << "#error \"QUIXCC requires support for __int128 to support qint128 and quint128 types.\"\n";
    code << "#endif\n\n";
    code << "typedef __int128 qint128;\n";
    code << "typedef unsigned __int128 quint128;\n\n";

    /* Architecture-specific definitions */
    code << "/* Architecture-specific definitions */\n";
    code << "#if UINTPTR_MAX == 0xFF\n";
    code << "#define QPTRSIZE 8\n";
    code << "#elif UINTPTR_MAX == 0xFFFF\n";
    code << "#define QPTRSIZE 16\n";
    code << "#elif UINTPTR_MAX == 0xFFFFFFFF\n";
    code << "#define QPTRSIZE 32\n";
    code << "#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFFu\n";
    code << "#define QPTRSIZE 64\n";
    code << "#else\n";
    code << "#error \"Target architecture not supported by QUIXCC \" \\\n";
    code << "       \"because pointer size is unknown.\"\n";
    code << "#endif\n\n";

    code << "#if QPTRSIZE == 8\n";
    code << "typedef quint8 quintptr;\n";
    code << "typedef qint8 qintptr;\n";
    code << "#elif QPTRSIZE == 16\n";
    code << "typedef quint16 quintptr;\n";
    code << "typedef qint16 qintptr;\n";
    code << "#elif QPTRSIZE == 32\n";
    code << "typedef quint32 quintptr;\n";
    code << "typedef qint32 qintptr;\n";
    code << "#elif QPTRSIZE == 64\n";
    code << "typedef quint64 quintptr;\n";
    code << "typedef qint64 qintptr;\n";
    code << "#endif\n\n";

    /* Macros */
    code << "/* Macro definitions */\n";
    code << "#ifndef qexport\n";
    code << "#if defined(__clang__)\n";
    code << "#define qexport __attribute__((visibility(\"default\")))\n";
    code << "#elif defined(__GNUC__)\n";
    code << "#define qexport __attribute__((visibility(\"default\")))\n";
    code << "#else\n";
    code << "#error \"Unsupported compiler; cannot define qexport.\"\n";
    code << "#endif\n";
    code << "#endif\n\n";
    code << "#ifndef QROTL_EXPR\n";
    code << "#define QROTL_EXPR(x, n) ((x << n) | (x >> (8 * sizeof(x) - n)))\n";
    code << "#endif\n";
    code << "#ifndef QROTR_EXPR\n";
    code << "#define QROTR_EXPR(x, n) ((x >> n) | (x << (8 * sizeof(x) - n)))\n";
    code << "#endif\n\n";

    code << "#endif /* __QUIX_CODEGEN__ */\n\n";

    std::stringstream cu;

    for (size_t i = 0; i < n->children.size(); i++)
    {
        auto tmp = gen(n->children[i]);
        if (tmp.empty())
            continue;

        cu << tmp;
        if (i < n->children.size() - 1)
            cu << ";\n\n";
        else
            cu << ";\n";
    }

    for (auto &node : m_state.structs)
        code << node << "\n";

    for (auto &node : m_state.autogen)
        code << node << "\n";

    code << cu.str();

    return code.str();
}

#define match(type)    \
    if (n->is<type>()) \
    return gen(n->as<type>())

std::string libquixcc::C11Codegen::gen(const libquixcc::ir::delta::Value *n)
{
    match(I1);
    match(I8);
    match(I16);
    match(I32);
    match(I64);
    match(I128);
    match(U8);
    match(U16);
    match(U32);
    match(U64);
    match(U128);
    match(F32);
    match(F64);
    match(Void);
    match(Ptr);
    match(PacketDef);
    match(Packet);
    match(Array);
    match(FType);

    match(Local);
    match(Global);
    match(Number);
    match(String);
    match(List);
    match(Ident);
    match(Assign);
    match(AddressOf);
    match(Deref);
    match(Member);
    match(Index);
    match(SCast);
    match(UCast);
    match(PtrICast);
    match(IPtrCast);
    match(Bitcast);
    match(IfElse);
    match(While);
    match(Jmp);
    match(Label);
    match(Ret);
    match(Call);
    match(PtrCall);
    match(Halt);
    match(Block);
    match(Segment);
    match(Add);
    match(Sub);
    match(Mul);
    match(Div);
    match(Mod);
    match(BitAnd);
    match(BitOr);
    match(BitXor);
    match(BitNot);
    match(Shl);
    match(Shr);
    match(Rotl);
    match(Rotr);
    match(Eq);
    match(Ne);
    match(Lt);
    match(Gt);
    match(Le);
    match(Ge);
    match(And);
    match(Or);
    match(Not);
    match(Xor);
    match(RootNode);

    throw std::runtime_error("Codegen failed: codegen not implemented for value: " + std::to_string(n->ntype));
}

bool libquixcc::C11Codegen::codegen(const std::unique_ptr<libquixcc::ir::delta::IRDelta> &ir, std::ostream &os)
{
    C11Codegen codegen;

    os << codegen.gen(ir->root());
    return true; /* Errors -> exceptions */
}
