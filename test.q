/// TODO: Hook your bomb trigger to wire 13

// @import mcu::atmega328p;
// namemap mcu::atmega328p::io as io;

pub "C" volatile unsafe fn impure _start() {
  // @mcu::atmega328p::init_chip(stack: 0x200,
  //                             heap: 0,
  //                             f_cpu: 16_000_000,
  //                             serial: 9600);
  setup(); while => loop();   // Call user setup; Execute user loop
} impl [entrypoint, headless, noreturn];

fn impure noexcept setup() {
  io << "============================\n";
  io << "===    UNI-BOMBER 2.0    ===\n";
  io << "===  Microcontroller OS  ===\n";
  io << "============================\n";
  io << "===  Developed by:       ===\n";
  io << "===  @evilcorp69         ===\n";
  io << "============================\n";
  io << "===   16MHz ATmega328P   ===\n";
  io << "=== 2KB SRAM, 32KB Flash ===\n";
  io << "=== 1KB EEPROM, 23 GPIO  ===\n";
  io << "=== 6 ADC, 3 PWM, 3 UART ===\n";
  io << "=== 2 SPI, 1 I2C, 1 Wire ===\n";
  io << "=== 1 Analog Comp, 1 RTC ===\n";
  io << "=== 1 Watchdog, 1 Timer  ===\n";
  io << "=== 1 Brownout, 1 Reset  ===\n";
  io << "=== 1 Power Down, 1 Idle ===\n";
  io << "=== 1 Sleep, 1 Standby   ===\n";
  io << "=== 1 Extended Standby   ===\n";
  io << "============================\n";
  io << "=== Bombing in 3...2...1 ===\n";

  mcu::delay_ms(3000); // Delay 3 seconds
}

fn impure noexcept loop() => detonate_bomb();
fn impure noexcept detonate_bomb() => io.wire(13, io::HIGH);