pub "C" {
  fn LogA(args: [string]) {
    help << "This is a debug message";
    info << "This is an info message";
    warn << "This is a warning message";
    fail << "This is an error message";
    kill << "This is a fatal message";
  }

  // fn LogB(args: [string]) {
  //   >>> "This is a debug message"   <<<;
  //   >>> "This is an info message"   <<<;
  //   >>> "This is a warning message" <<<;
  //   >>> "This is an error message"  <<<;
  //   >>> "This is a fatal message"   <<<;
  // }

  fn LogC(args: [string]) {
    log.debug("This is a debug message");
    log.info("This is an info message");
    log.warn("This is a warning message");
    log.error("This is an error message");
    log.fatal("This is a fatal message");
  }

  fn LogD(args: [string]) {
    "This is a debug message".psh!<debug>();
    "This is an info message".psh!<info>();
    "This is a warning message".psh!<warn>();
    "This is an error message".psh!<error>();
    "This is a fatal message".psh!<fatal>();
  }

  fn LogD(args: [string]) {
    "This is a debug message".psh_debug();
    "This is an info message".psh_info();
    "This is a warning message".psh_warn();
    "This is an error message".psh_error();
    "This is a fatal message".psh_fatal();
  }
}

pub generic <T: [a: i32, b: i32, c: i32] = [0,0,0]>
group ExprTemplate {
  const alpha = T.a + T.b + T.c;

  pub fn ExprTemplate(x: i32) {
    let y = x + alpha;
    log.debug("The value of y is: " + y);
  }
}

pub generic <T: solidtype, M: std::allocator_t = std::alloc<T>>
group std::btree {
  //
} with [typeid_as('e3f850c4-5885-4b46-8083-3d75219f3070',
                  std::btree<T, M>::to_port_brepr()),
                  auto];

fn main() {
  @std::init_functor(fn () {
    std::rand::stochastic(false);
  });

  let bt = std::btree<uuid>();

}

std::impl::init_functor_ptr__ = fn () {};

std::impl::init_functor_ptr__ = _QAnon_44fd78dd94e85ad6