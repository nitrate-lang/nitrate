#pragma once

#include <quix/panic.hh>
#include <quix/stdint.hh>

namespace quix {
  class bit_view_byte final {
    unsigned char* m_addr;
    unsigned char m_off;
    bool m_v;

  public:
    constexpr bit_view_byte(bool v, unsigned char* addr, unsigned char off)
        : m_addr(addr), m_off(off), m_v(v) {}

    constexpr operator bool() const { return m_v; }

    constexpr inline bit_view_byte& operator=(bool v) {
      if (v) {
        *m_addr |= 1 << m_off;
      } else {
        *m_addr &= ~(1 << m_off);
      }

      return *this;
    }
  };

  class bit_view final {
    u128 m_bitaddr;
    u128 m_bitsize;

  public:
    bit_view(u128 bitaddr, u128 bitsize) : m_bitaddr(bitaddr), m_bitsize(bitsize) {}

    bit_view(unsigned char* byte_addr, unsigned char bit_offset, u128 bitsize) {
      m_bitaddr = static_cast<u128>(reinterpret_cast<u128>(byte_addr)) * 8 + bit_offset;
      m_bitsize = bitsize;
    }

    constexpr bit_view(const bit_view& bv) = default;

    template <typename T>
    static bit_view from(T& v, unsigned char bit_offset = 0, u128 bitsize = sizeof(T) * 8) {
      return bit_view(reinterpret_cast<unsigned char*>(&v), bit_offset, bitsize);
    }

    bit_view_byte operator[](u128 i) {
#ifndef NDEBUG
      if (i >= m_bitsize) {
        throw "bit_view: index out of bounds";
      }
#endif

      unsigned char* byte_addr = reinterpret_cast<unsigned char*>((m_bitaddr + i) / 8);
      unsigned char bit_off = (m_bitaddr + i) % 8;

      return bit_view_byte((*byte_addr >> bit_off) & 1, byte_addr, bit_off);
    }

    bool operator[](u128 i) const {
#ifndef NDEBUG
      if (i >= m_bitsize) {
        throw "bit_view: index out of bounds";
      }
#endif

      unsigned char* byte_addr = reinterpret_cast<unsigned char*>((m_bitaddr + i) / 8);
      unsigned char bit_off = (m_bitaddr + i) % 8;

      return (*byte_addr >> bit_off) & 1;
    }

    constexpr u128 size() const { return m_bitsize; }

    bit_view slice(u128 bit_begin, u128 bit_end) {
      return bit_view(m_bitaddr + bit_begin, bit_end - bit_begin);
    }

    bit_view& set(bool value) {
      for (u128 i = 0; i < m_bitsize; i++) {
        (*this)[i] = value;
      }

      return *this;
    }

    template <typename T>
    void operator=(const T& value) {
      const bit_view v_bv = bit_view::from(const_cast<T&>(value));

      for (u128 i = 0; i < v_bv.size() && i < m_bitsize; i++) {
        (*this)[i] = v_bv[i];
      }
    }
  };

  template <typename T, usize N>
  class view final {
    T* m_addr;
    usize m_size;

  public:
    view(T* addr, usize size) : m_addr(addr), m_size(size) {}

    view(const view& v) = default;

    T& operator[](usize i) {
#ifndef NDEBUG
      if (i >= m_size) {
        panic("view: index out of bounds");
      }
#endif

      return m_addr[i];
    }

    const T& operator[](usize i) const {
#ifndef NDEBUG
      if (i >= m_size) {
        panic("view: index out of bounds");
      }
#endif

      return m_addr[i];
    }

    usize size() const { return m_size; }

    view slice(usize begin, usize end) { return view(m_addr + begin, end - begin); }

    view& set(const T& value) {
      for (usize i = 0; i < m_size; i++) {
        m_addr[i] = value;
      }

      return *this;
    }

    void operator=(const view& v) {
      for (usize i = 0; i < v.size() && i < m_size; i++) {
        m_addr[i] = v[i];
      }
    }
  };
}  // namespace quix
