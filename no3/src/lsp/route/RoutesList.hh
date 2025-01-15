#pragma once

#include <lsp/core/server.hh>

void DoInitialize(const lsp::RequestMessage&, lsp::ResponseMessage&);
void DoInitialized(const lsp::NotificationMessage&);
void DoShutdown(const lsp::RequestMessage&, lsp::ResponseMessage&);
void DoExit(const lsp::NotificationMessage&);

void DoCompletion(const lsp::RequestMessage&, lsp::ResponseMessage&);
void DoDeclaration(const lsp::RequestMessage&, lsp::ResponseMessage&);
void DoDefinition(const lsp::RequestMessage&, lsp::ResponseMessage&);
void DoDidChange(const lsp::NotificationMessage&);
void DoDidClose(const lsp::NotificationMessage&);
void DoDidOpen(const lsp::NotificationMessage&);
void DoDidSave(const lsp::NotificationMessage&);
void DoDocumentColor(const lsp::RequestMessage&, lsp::ResponseMessage&);
void DoFormatting(const lsp::RequestMessage&, lsp::ResponseMessage&);
