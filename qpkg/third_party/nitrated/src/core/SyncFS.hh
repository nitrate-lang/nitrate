#pragma once

#include <atomic>
#include <memory>
#include <optional>
#include <string>

class SyncFS {
  struct Impl;
  std::unique_ptr<Impl> m_impl;
  std::string m_current;
  std::mutex m_mutex;
  std::atomic<bool> m_opened;

  /**
   * @brief Get the current file compressed size in bytes.
   * @note This function is thread-safe.
   * @note This is how much space the file takes in memory.
   */
  std::optional<size_t> compressed_size();

  SyncFS();
  ~SyncFS();

  SyncFS(const SyncFS&) = delete;
  SyncFS& operator=(const SyncFS&) = delete;
  SyncFS(SyncFS&&) = delete;

public:
  enum class OpenCode {
    OK,
    NOT_FOUND,
    ALREADY_OPEN,
    OPEN_FAILED,
  };

  enum class CloseCode {
    OK,
    NOT_OPEN,
    CLOSE_FAILED,
  };

  enum class ReplaceCode {
    OK,
    NOT_OPEN,
    REPLACE_FAILED,
  };

  static SyncFS& the();

  /**
   * @brief Set the current file to act on.
   * @note This is a thread-local operation.
   */
  void select_uri(std::string_view uri);

  ///===========================================================================
  /// BEGIN: Data synchronization functions
  ///===========================================================================

  /**
   * @brief Open the current file.
   * @note This function is thread-safe.
   */
  OpenCode open(std::string_view mime_type);

  /**
   * @brief Close the current file.
   * @note This function is thread-safe.
   */
  CloseCode close();

  /**
   * @brief Replace a portion of the current file with new text.
   * @note This function is thread-safe.
   */
  ReplaceCode replace(size_t offset, size_t length, std::string_view text);

  ///===========================================================================
  /// END: Data synchronization functions
  ///===========================================================================

  ///===========================================================================
  /// BEGIN: Data access functions
  ///===========================================================================

  /**
   * @brief Get the current file mime type.
   * @note This function is thread-safe.
   */
  std::optional<const char*> mime_type();

  /**
   * @brief Get the current file size in bytes.
   * @note This function is thread-safe.
   */
  std::optional<size_t> size();

  /**
   * @brief Get the current file content.
   * @param[out] content The content of the current file.
   * @return True if the file was read successfully, false otherwise.
   * @note This function is thread-safe.
   */
  bool read_current(std::string& content);

  using Digest = std::array<uint8_t, 20>;

  std::optional<Digest> thumbprint();

  void wait_for_open();

  // /**
  //  * @brief Convert a row-column pair to an absolute offset.
  //  * @param row The row number (0-based).
  //  * @param col The column number (0-based).
  //  * @return The absolute offset in bytes.
  //  * @note The row and column arguments are clamped to the file boundaries. If you read the
  //  5000th
  //  * column when the file has only 3 columns, the function will return the offset in the 3rd
  //  column.
  //  * If you return the 5000th row when the file has only 3 rows, the function will return the
  //  offset
  //  * in the 3rd row.
  //  * @note This function is thread-safe.
  //  */
  // std::optional<size_t> rc2abs(size_t row, size_t col) const;

  ///===========================================================================
  /// END: Data access functions
  ///===========================================================================
};
