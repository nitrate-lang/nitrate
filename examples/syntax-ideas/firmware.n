@use "v1.0";

// @import chip<atmega328p>;
// @import board<uno>;
// @import core;

@(define STACK_SIZE = 512);

fn print_cb_impl(data: [u8]) {
  board::serial::write(data);             ~> "Write data to the serial port";
}

fn pure halt_cb_impl(): null {
  /* The Nitrate Specification dictates that the halt callback MUST:
   *   1. Reset the entire system such that all state is lost,
   *   2. OR literally **halt** the system.
   *
   * In normal cases, the halt callback would exit the process.
   * However, in the case of embedded systems, this doesn't make sense.
   * And so therefore, we must reset the MCU.
   **/

  board::reset();
}

fn panic_cb_impl(msg: [u8]) {
  printn(f"Panic: {msg}");
  halt();
}

pub "C" fn _start() {
  // @undefn("core::_start()");                ~> "Remove the default _start() function";

  core::mem::heap::disable();               ~> "Disable the heap";
  core::mem::stack::set_size(STACK_SIZE);   ~> "Set the stack size";
  core::bare_init(halt_cb: halt_cb_impl,    ~> "Initialize bare-metal core";
                   panic_cb: panic_cb_impl,
                   write_cb: print_cb_impl);

  setup();
  while true => loop();
}

fn setup() {
  /* In general, the setup function should:
   * 1. Initialize the serial port (or print won't work.)
   * 2. Initialize your peripherals.
   **/
}

fn loop() {
  /* In general, the loop function should:
   * 1. Read from sensors
   * 2. Process data
   * 3. Write to peripherals
   **/
}