
BOOST_SYMBOL_EXPORT auto Lexer::peek_token(uint8_t k) - > std::optional<Token> {
  assert(k > 0 && "k must be greater than 0 to peek at tokens.");

  while (m_token_queue.size() < k) {
    auto token = parse_next_token();
    if (!token.has_value()) [[unlikely]] {
      return std::nullopt;  // No more tokens available
    }
    m_token_queue.push_back(std::move(token.value()));
  }

  const size_t index = k - 1;  // k is 1-based, so we access k-1
  assert(index < m_token_queue.size() && "Not enough tokens in the queue to peek at k-th token.");

  return m_token_queue[index];
}
