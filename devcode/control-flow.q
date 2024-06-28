@use "v1.0";

// @import std::generic;

group vec<T: solid> {
  req {
    T > 0;
  }

  ptr: *T,
  length: usize,
  capacity: usize,

  pub fn new() {
    this.ptr = null;
    this.length = 0;
    this.capacity = 0;
  }

  pub fn new(cap: usize) {
    this.ptr = null;
    this.length = 0;
    this.capacity = 0;
  }

  pub fn quasipure polymut [](usize i): T {
    if i >= this.length {
      panic("index out of bounds");
    }
    ret this.ptr + i;
  }

  pub fn quasipure polymut iter(): %T? {
    ret (i: !i32): => {
      if i >= this.length {
        ret null;
      }
      ret this.ptr + i++;
    };
  }

  pub fn polymut push(val: T) {
    if this.length == this.capacity {
      this.capacity = if this.capacity == 0 { 1 } else { this.capacity * 2 };
      this.ptr = realloc(this.ptr, this.capacity * sizeof(T));
    }
    this.ptr[this.length++] = val;
  }

  pub fn polymut pop(): T {
    if this.length == 0 {
      panic("pop from empty vec");
    }
    ret this.ptr[--this.length];
  }

  pub fn polymut insert(i: usize, val: T) {
    if i > this.length {
      panic("index out of bounds");
    }
    if this.length == this.capacity {
      this.capacity = if this.capacity == 0 { 1 } else { this.capacity * 2 };
      this.ptr = realloc(this.ptr, this.capacity * sizeof(T));
    }
    for (j: this.length; j > i; j--) {
      this.ptr[j] = this.ptr[j - 1];
    }
    this.ptr[i] = val;
    this.length++;
  }

  pub fn polymut remove(i: usize): T {
    if i >= this.length {
      panic("index out of bounds");
    }
    ret {
      T val = this.ptr[i];
      for (j: i; j < this.length - 1; j++) {
        this.ptr[j] = this.ptr[j + 1];
      }
      this.length--;
      val;
    };
  }

  pub fn polymut clear() {
    this.length = 0;
  }

  pub fn const quasipure len(): usize {
    ret this.length;
  }

  pub fn const quasipure cap(): usize {
    ret this.capacity;
  }

  pub fn const quasipure to_str(del: string = ", ",
                prefix: string = "[",
                suffix: string = "]"): string {
    let !s = prefix;

    foreach (i: usize, v in this) {
      s += v.to_str();
      if i < this.length - 1 {
        s += del;
      }
    }

    ret s + suffix;
  }

  pub static fn const pure serialize_unit_size(): usize {
    ret sizeof(usize) + this.length * T::serialize_unit_size();
  }

  @tag("compress-algo:lz4,deflate")
  pub fn const pure serialize(): [u8] {
    let !buf = [u8](this.length * T::serialize_unit_size());
    buf << this.length.to_bytes<LE>(sizeof(usize));

    for (i: this.length) {
      buf << this.ptr[i].serialize();
    }

    ret buf;
  }

  @tag("compress-algo:lz4,deflate")
  pub fn crashpoint pure deserialize(buf: [u8]): void? {
    if buf.len() < sizeof(usize) {
      throw @info() || "invalid buffer length";
    }

    this.length = buf.read::<usize>(0, sizeof(usize));
    this.capacity = this.length;
    this.ptr = alloc(this.length * sizeof(T));

    for (i: this.length) {
      let !val = T();
      if val.deserialize(buf.slice(i * T::serialize_unit_size() + sizeof(usize))) == null {
        throw @info() || "failed to deserialize element";
      }
      this.ptr[i] = val;
    }

    ret;
  }
}

vec<MySql3Handle>::deserialize(argv[0])[0].connect("localhost", "root", "", "test");