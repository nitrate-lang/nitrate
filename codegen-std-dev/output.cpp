typedef unsigned long long u64;

template <typename T>
class qint_base;

typedef qint_base<unsigned __int128> quint128_t;
typedef qint_base<__int128> qint128_t;

template <typename T>
class qint_base {
  T m_v;

public:
  qint_base(T v) : m_v(v) {}
  qint_base(quint128_t v) : m_v(v()) {}
  qint_base(u64 hi, u64 lo) : m_v(((T)hi << 64) | lo) {}
  qint_base(u64 v) : m_v(v) {}

  explicit operator T() const { return m_v; }

  qint_base& operator=(const quint128_t& rhs) { return (m_v = rhs()), *this; }

  T operator()() const { return m_v; }
  T& operator()() { return m_v; }

  qint_base operator+(const qint_base& rhs) { return m_v + rhs.m_v; }
  qint_base operator-(const qint_base& rhs) { return m_v - rhs.m_v; }
  qint_base operator*(const qint_base& rhs) { return m_v * rhs.m_v; }
  qint_base operator/(const qint_base& rhs) { return m_v / rhs.m_v; }
  qint_base operator%(const qint_base& rhs) { return m_v % rhs.m_v; }
  qint_base operator&(const qint_base& rhs) { return m_v & rhs.m_v; }
  qint_base operator|(const qint_base& rhs) { return m_v | rhs.m_v; }
  qint_base operator^(const qint_base& rhs) { return m_v ^ rhs.m_v; }
  qint_base operator<<(const qint_base& rhs) { return m_v << rhs.m_v; }
  qint_base operator>>(const qint_base& rhs) { return m_v >> rhs.m_v; }

  bool operator&&(const qint_base& rhs) { return m_v && rhs.m_v; }
  bool operator||(const qint_base& rhs) { return m_v || rhs.m_v; }
  bool operator==(const qint_base& rhs) { return m_v == rhs.m_v; }
  bool operator!=(const qint_base& rhs) { return m_v != rhs.m_v; }
  bool operator<(const qint_base& rhs) { return m_v < rhs.m_v; }
  bool operator>(const qint_base& rhs) { return m_v > rhs.m_v; }
  bool operator<=(const qint_base& rhs) { return m_v <= rhs.m_v; }
  bool operator>=(const qint_base& rhs) { return m_v >= rhs.m_v; }

  qint_base& operator+=(const qint_base& rhs) { return (m_v += rhs.m_v), *this; }
  qint_base& operator-=(const qint_base& rhs) { return (m_v -= rhs.m_v), *this; }
  qint_base& operator*=(const qint_base& rhs) { return (m_v *= rhs.m_v), *this; }
  qint_base& operator/=(const qint_base& rhs) { return (m_v /= rhs.m_v), *this; }
  qint_base& operator%=(const qint_base& rhs) { return (m_v %= rhs.m_v), *this; }
  qint_base& operator&=(const qint_base& rhs) { return (m_v &= rhs.m_v), *this; }
  qint_base& operator|=(const qint_base& rhs) { return (m_v |= rhs.m_v), *this; }
  qint_base& operator^=(const qint_base& rhs) { return (m_v ^= rhs.m_v), *this; }
  qint_base& operator<<=(const qint_base& rhs) { return (m_v <<= rhs.m_v), *this; }
  qint_base& operator>>=(const qint_base& rhs) { return (m_v >>= rhs.m_v), *this; }
};
