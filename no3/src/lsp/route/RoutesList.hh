#pragma once

#include <lsp/core/server.hh>

namespace no3::lsp::srv {
  void DoInitialize(const RequestMessage&, ResponseMessage&);
  void DoInitialized(const NotificationMessage&);
  void DoShutdown(const RequestMessage&, ResponseMessage&);
  void DoExit(const NotificationMessage&);

  void DoDidChange(const NotificationMessage&);
  void DoDidClose(const NotificationMessage&);
  void DoDidOpen(const NotificationMessage&);
  void DoDidSave(const NotificationMessage&);
  void DoDocumentColor(const RequestMessage&, ResponseMessage&);
  void DoFormatting(const RequestMessage&, ResponseMessage&);
}  // namespace no3::lsp::srv
