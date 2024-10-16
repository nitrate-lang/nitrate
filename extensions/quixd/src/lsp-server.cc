#include <server.hh>

bool start_language_server(std::iostream& channel, const Configuration& config) {
  (void)channel;
  (void)config;

  while (true) {
    std::string input;
    std::getline(channel, input);
    channel << "Received: " << input << "\n";
    channel << "Testing 123\n";
  }

  /**
   * - Implement robust logging
   * - Server JSON-RPC [Binding] for LSP protocol
   * - Get a mock client
   * - Implement the LSP protocol textDocument/didOpen, textDocument/didClose methods
   * - Implement the LSP protocol textDocument/didChange method
   * - Implement the LSP protocol textDocument/completion method
   * - Implement the LSP protocol textDocument/hover method
   * - Implement the LSP protocol textDocument/definition method
   * - Implement the LSP protocol textDocument/references method
   * - Implement the LSP protocol textDocument/documentSymbol method
   * - Implement the LSP protocol textDocument/documentHighlight method
   * - Implement the LSP protocol textDocument/documentLink method
   * - Implement the LSP protocol textDocument/documentColor method
   * - Implement the LSP protocol textDocument/colorPresentation method
   * - Implement the LSP protocol textDocument/formatting method
   * - Implement the LSP protocol textDocument/rangeFormatting method
   * - Implement the LSP protocol textDocument/onTypeFormatting method
   */

  /// TODO: Implement LSP
  return false;
}
