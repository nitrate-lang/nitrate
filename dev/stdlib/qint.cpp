typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;
typedef unsigned long long size_t;

#define qassert(x) ((void)0)

#include <array>
#include <bit>

class bit_slice {
  uint8_t* m_ptr;
  size_t m_num_bits;
  uint8_t m_bit_offset;

public:
  bit_slice() : m_ptr(nullptr), m_num_bits(0), m_bit_offset(0) {}
  bit_slice(uint8_t* ptr, uint8_t bit_offset, size_t num_bits)
      : m_ptr(ptr), m_num_bits(num_bits), m_bit_offset(bit_offset) {
    qassert(m_bit_offset < 8);
  }

  bool operator[](size_t i) {
    qassert(i < m_num_bits);
    return (m_ptr[i / 8] >> ((i + m_bit_offset) % 8)) & 1;
  }

  void set(size_t i, bool v) {
    qassert(i < m_num_bits);
    if (v) {
      m_ptr[i / 8] |= 1 << ((i + m_bit_offset) % 8);
    } else {
      m_ptr[i / 8] &= ~(1 << ((i + m_bit_offset) % 8));
    }
  }
};

template <typename T>
class qint_base {
  T m_v;

public:
  qint_base(T v) : m_v(v) {}
  qint_base(long long hi, long long lo) : m_v(((T)hi << 64) | lo) {}

  explicit operator T() const { return m_v; }

  template <typename U>
  operator U() {
    return (U)m_v;
  }

  T operator()() const { return m_v; }
  T& operator()() { return m_v; }

  //////////////////////////////////////////////////////////////////////

  std::array<qint_base<uint8_t>, sizeof(T)>& as_bytes() {
    return *std::bit_cast<std::array<qint_base<uint8_t>, sizeof(T)>*>(&m_v);
  }

  bit_slice as_bits() { return bit_slice(reinterpret_cast<uint8_t*>(&m_v), 0, sizeof(T) * 8); }
};

#define IMPL_OP(O)                                                    \
  template <typename L, typename R>                                   \
  auto operator O(const qint_base<L>& lhs, const qint_base<R>& rhs) { \
    return qint_base(lhs() O rhs());                                  \
  }
IMPL_OP(+);
IMPL_OP(-);
IMPL_OP(*);
IMPL_OP(/);
IMPL_OP(%);
IMPL_OP(&);
IMPL_OP(|);
IMPL_OP(^);
IMPL_OP(<<);
IMPL_OP(>>);
IMPL_OP(&&);
IMPL_OP(||);
IMPL_OP(==);
IMPL_OP(!=);
IMPL_OP(<);
IMPL_OP(>);
IMPL_OP(<=);
IMPL_OP(>=);
#undef IMPL_OP

#define IMPL_OP(O)                                               \
  template <typename L, typename R>                              \
  auto& operator O(qint_base<L>& lhs, const qint_base<R>& rhs) { \
    return (lhs() O rhs()), lhs;                                 \
  }
IMPL_OP(+=);
IMPL_OP(-=);
IMPL_OP(*=);
IMPL_OP(/=);
IMPL_OP(%=);
IMPL_OP(&=);
IMPL_OP(|=);
IMPL_OP(^=);
IMPL_OP(<<=);
IMPL_OP(>>=);
#undef IMPL_OP

typedef qint_base<bool> qbool_t;
typedef qint_base<int8_t> qint8_t;
typedef qint_base<uint8_t> quint8_t;
typedef qint_base<int16_t> qint16_t;
typedef qint_base<uint16_t> quint16_t;
typedef qint_base<int32_t> qint32_t;
typedef qint_base<uint32_t> quint32_t;
typedef qint_base<int64_t> qint64_t;
typedef qint_base<uint64_t> quint64_t;
typedef qint_base<__int128> qint128_t;
typedef qint_base<unsigned __int128> quint128_t;

int main() {
  quint32_t p = 10;

  p.as_bits()[40];
}
