subsystem A {
  enum Color {
    Red,
    Green,
    Blue
  }

  subsystem B {
    enum Color {
      Red,
      Green,
      Blue
    }

    subsystem C {
      enum Color {
        Red,
        Green,
        Blue
      }

      fn test() {
        let x = B::Color::Red; // Error
        let y = A::Color; // OK
      }
    }
  }
}