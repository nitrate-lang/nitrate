#pragma once

[[noreturn]] void panic(const char* fmt, ...) {
  while (true) {
  }
}

#define panic_if(cond, fmt, ...) ((cond) && (panic(fmt, ##__VA_ARGS__), 0))