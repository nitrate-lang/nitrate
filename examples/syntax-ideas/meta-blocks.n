@use "v1.0";

union no_default_construct<T: solid> {
  v: T = undef, 
  raw: [u8; sizeof<T>],

  pub fn new(v: T = undef) => this.v = v;
  pub fn get() ? => ret this.v;
  pub fn get_mut() !? => ret this.v;
  pub fn as_bytes() ? => ret this.raw;
  pub fn as_mut_bytes() !? => ret this.raw;

  meta {
    @inherit_semantics(T, overwrite: false);

    @assert(sizeof<T> > 0, 
        "T must have a size greater than 0");
    @assert(bit_sizeof<T> == 8 * sizeof<T>,
        "T must have a size that is a multiple of 8."
        "Bitfields are not supported.");
  }

  code_usage {
    let x = no_default_construct<u8>(v: 10);
    let y = x.as_bytes();
    assert(y[0] == x);
  };
};

union QResult<T> {
  val: byte_suffix<1, no_default_construct<T>, 0>,
  err: byte_suffic<1, %string, 1>,

  pub fn val ? {
    @({
      switch @build_type() {
        case "release": ret this.val.get().get();
        case "debug":
        case "test":
          @panic_if(!this,
              "QResult::val called on an empty QResult.");

          ret this.val.get().get();
      }
    });
  }

  pub fn err ? {
    ret this.err.get();
  }

  pub fn conv(bool) => ret this.val.suffix() == 0;

  pub fn operator! bool => ret this as bool == false;

  meta {
    @assert(sizeof<T> > 0,
        "T must have a size greater than 0.");
    @assert(bit_sizeof<T> == 8 * sizeof<T>,
        "T must have a size that is a multiple of 8."
        "Bitfields are not supported.");
    
    switch @build_type() {
        case "release":
            @inherit_semantics(T, overwrite: false);
            brk;
        case "debug":
        case "test":
            @inherit_and_wrap_semantics(T, overwrite: false, wrap_config: {
                push_preamble: fn () {
                    if (!this) {
                        @panic(@meta_chain() | "Attempted to access a QResult that is empty.");
                    }
                }
            });
            brk;
          
    }
  }
};

fn foo(): QResult<u8> {
  ret 10;
}
