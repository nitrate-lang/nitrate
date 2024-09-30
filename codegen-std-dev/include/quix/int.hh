#pragma once

#include <quix/bit_view.hh>
#include <quix/stdint.hh>

namespace quix {
  template <typename T>
  class qint;

  typedef qint<bool> qbool;
  typedef qint<i8> qint8;
  typedef qint<i16> qint16;
  typedef qint<i32> qint32;
  typedef qint<i64> qint64;
  typedef qint<i128> qint128;
  typedef qint<u8> quint8;
  typedef qint<u16> quint16;
  typedef qint<u32> quint32;
  typedef qint<u64> quint64;

  typedef qint<f16> qf16;
  typedef qint<f32> qf32;
  typedef qint<f64> qf64;
  typedef qint<f128> qf128;

  template <typename T>
  class qint final {
    T m_v;

  public:
    qint(T v = 0) : m_v(v) {}

    T get() const { return m_v; }

    qint& operator++() { return ++m_v, *this; }
    qint& operator--() { return --m_v, *this; }
    qint operator++(int) { return m_v++; }
    qint operator--(int) { return m_v--; }
    qint operator+() const { return +m_v; }
    qint operator-() const { return -m_v; }
    qint operator~() const { return ~m_v; }
    qint operator!() const { return !m_v; }

#define OP(op)                        \
  template <typename U>               \
  auto operator op(qint<U> o) const { \
    auto x = m_v op o.get();          \
    return qint<decltype(x)>(x);      \
  }
    OP(+);
    OP(-);
    OP(*);
    OP(/);
    OP(%);
    OP(&);
    OP(|);
    OP(^);
    OP(<<);
    OP(>>);
    OP(&&);
    OP(||);
    OP(==);
    OP(!=);
    OP(<);
    OP(>);
    OP(<=);
    OP(>=);
    OP(&=);
    OP(|=);
    OP(^=);
    OP(<<=);
    OP(>>=);
    OP(+=);
    OP(-=);
    OP(*=);
    OP(/=);
    OP(%=);
#undef OP

    bit_view as_bits() { return bit_view::from(m_v); }
    const bit_view as_bits() const { return bit_view::from(m_v); }

    view<u8, sizeof(T)> as_bytes() {
      return view<u8, sizeof(T)>(reinterpret_cast<u8*>(&m_v), sizeof(T));
    }
    const view<u8, sizeof(T)> as_bytes() const {
      return view<u8, sizeof(T)>(reinterpret_cast<u8*>(&m_v), sizeof(T));
    }
  };

}  // namespace quix
