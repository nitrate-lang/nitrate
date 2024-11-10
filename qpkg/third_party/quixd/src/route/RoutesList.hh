#pragma once

#include <core/server.hh>

void do_initialize(const lsp::RequestMessage&, lsp::ResponseMessage&);
void do_initialized(const lsp::NotificationMessage&);
void do_shutdown(const lsp::RequestMessage&, lsp::ResponseMessage&);
void do_exit(const lsp::NotificationMessage&);

void do_completion(const lsp::RequestMessage&, lsp::ResponseMessage&);
void do_declaration(const lsp::RequestMessage&, lsp::ResponseMessage&);
void do_definition(const lsp::RequestMessage&, lsp::ResponseMessage&);
void do_didChange(const lsp::NotificationMessage&);
void do_didClose(const lsp::NotificationMessage&);
void do_didOpen(const lsp::NotificationMessage&);
void do_didSave(const lsp::NotificationMessage&);
void do_documentColor(const lsp::RequestMessage&, lsp::ResponseMessage&);
void do_formatting(const lsp::RequestMessage&, lsp::ResponseMessage&);
