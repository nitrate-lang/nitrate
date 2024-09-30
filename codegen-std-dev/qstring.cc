#include <cstddef>

extern "C" size_t strlen(const char *str);
extern "C" void *memcpy(void *dest, const void *src, size_t n);

template <typename T> class qslice final {
  T *m_ptr;
  size_t m_len;

public:
  qslice() : m_ptr(nullptr), m_len(0) {}
  qslice(T *ptr, size_t len) : m_ptr(ptr), m_len(len) {}

  T *data() { return m_ptr; }
  const T *data() const { return m_ptr; }
  size_t size() const { return m_len; }

  qslice slice(size_t start, size_t len) const { return qslice(m_ptr + start, len); }
};

class qstring final {
  char *m_str;
  size_t m_len;
  size_t m_cap;

public:
  qstring() : m_str(nullptr), m_len(0), m_cap(0) {}
  qstring(const char *str) : m_str(nullptr), m_len(0), m_cap(0) { assign(str); }
  qstring(const char *str, size_t len) : m_str(nullptr), m_len(0), m_cap(0) { assign(str, len); }
  qstring(const qslice<char> &slice) : m_str(nullptr), m_len(0), m_cap(0) { assign(slice.data(), slice.size()); }
  qstring(const qstring &other) : m_str(nullptr), m_len(0), m_cap(0) {
    assign(other.m_str, other.m_len);
  }
  qstring(qstring &&other) : m_str(other.m_str), m_len(other.m_len), m_cap(other.m_cap) {
    other.m_str = nullptr;
    other.m_len = 0;
    other.m_cap = 0;
  }
  ~qstring() { delete[] m_str; }

  qstring &operator=(const char *str) {
    assign(str);
    return *this;
  }
  qstring &operator=(const qstring &other) {
    assign(other.m_str, other.m_len);
    return *this;
  }
  qstring &operator=(qstring &&other) {
    delete[] m_str;
    m_str = other.m_str;
    m_len = other.m_len;
    m_cap = other.m_cap;
    other.m_str = nullptr;
    other.m_len = 0;
    other.m_cap = 0;
    return *this;
  }

  void assign(const char *str) { assign(str, strlen(str)); }

  void assign(const char *str, size_t len) {
    if (len > m_cap) {
      delete[] m_str;
      m_str = new char[len];
      m_cap = len;
    }

    memcpy(m_str, str, len);
    m_len = len;
  }

  const char *c_str() const { return m_str; }
  char *data() { return m_str; }
  size_t size() const { return m_len; }
  size_t capacity() const { return m_cap; }

  ///===--------------------------------------------------------------------===///

  qstring &concat(const qstring &other) {
    size_t new_len = m_len + other.m_len;
    if (new_len > m_cap) {
      char *new_str = new char[new_len];
      memcpy(new_str, m_str, m_len);
      memcpy(new_str + m_len, other.m_str, other.m_len);
      delete[] m_str;
      m_str = new_str;
      m_cap = new_len;
    } else {
      memcpy(m_str + m_len, other.m_str, other.m_len);
    }
    m_len = new_len;
    return *this;
  }

  qslice<char> slice(size_t start, size_t len) const {
    if (start >= m_len) {
      return qslice<char>();
    }

    if (start + len > m_len) {
      len = m_len - start;
    }

    return qslice<char>(m_str + start, len);
  }
};

int main() {
  qstring s("Hello, world!");

  s = s.concat(" This is a test.").slice(1, 20);
  return 0;
}