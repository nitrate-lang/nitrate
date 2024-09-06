//////////////////////////////////////////////////
// BEGIN: SPECIALIZATION INITIALIZATION         //
//////////////////////////////////////////////////
fn ___quixcc_ioctl_(cmd: string, ...): void;

___quixcc_ioctl_("parser_version", "1.0");
___quixcc_ioctl_("preprocessor_version", "1.0");
___quixcc_ioctl_("load_modules", "default");
___quixcc_ioctl_("set_stack_initilization", "true");
___quixcc_ioctl_("set_heap_default_clear", "true");
___quixcc_ioctl_("set_random_heap", "true");
//////////////////////////////////////////////////
// END:   SPECIALIZATION INITIALIZATION         //
//////////////////////////////////////////////////

//////////////////////////////////////////////////
// BEGIN: TARGET SPECIFIC SYMBOLS               //
//////////////////////////////////////////////////
subsystem cc: [intrin::Compiler] {
  type uptr_type = u64;
  type iptr_type = i64;
  type uword_type = u64;
  type iword_type  = i64;
  const target_triplet = "x86_64-unknown-linux-gnu";
  const local_triplet = "x86_64-unknown-linux-gnu";
  const start_time = 1725591217000;
  const cpu_features = ["sse3", "sse4.1", "sse4.2",
                        "avx", "avx2", "fma", "bmi",
                        "bmi2", "popcnt", "lzcnt",
                        "abm", "tbm", "adx", "f16c",
                        "fma4", "xop", "lwp", "fma3",
                        "sse4a", "ssse3", "sse2",
                        "sse", "mmx", "3dnow",
                        "3dnowext"];
};
//////////////////////////////////////////////////
// END:   TARGET SPECIFIC SYMBOLS               //
//////////////////////////////////////////////////

//////////////////////////////////////////////////
// BEGIN: DEFAULT IMPORTS                       //
//////////////////////////////////////////////////
// @import core::env;
// @import core::io;
// @import core::constant;
// @import core::type;
// @import core::trait;
// @import core::util;
//////////////////////////////////////////////////
// END:   DEFAULT IMPORTS                       //
//////////////////////////////////////////////////
