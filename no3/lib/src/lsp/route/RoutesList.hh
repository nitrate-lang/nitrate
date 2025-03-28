#pragma once

#include <lsp/core/Server.hh>

namespace no3::lsp::srv {
  void DoInitialize(const RequestMessage&, ResponseMessage&);
  void DoInitialized(const NotificationMessage&);
  void DoShutdown(const RequestMessage&, ResponseMessage&);
  void DoExit(const NotificationMessage&);

  void SetTrace(const NotificationMessage&);

  void DoDidChange(const NotificationMessage&);
  void DoDidClose(const NotificationMessage&);
  void DoDidOpen(const NotificationMessage&);
  void DoDidSave(const NotificationMessage&);

  void DoFormatting(const RequestMessage&, ResponseMessage&);
}  // namespace no3::lsp::srv
