@import std::io;

subsystem shell {

}

example_usage {
  fn main(): i32 {
    let cli = shell::create("dsfconsole", ansi: true, theme: {
      name: "quix:modern/blue",
    }) catch (err) {
      printn(f"Failed to create an interactive shell: {err}");
      ret 1;
    };

    cli.setup([
      {
        "name": "help",
        "description": "Show help",
        "handler": {
          "match": fn(args: [str]): bool {
            ret args == ["help"];
          },
          "run": fn(args: [str]): i32 {
            cli.print("Help is on the way!");
            ret 0;
          }
        }
      },
      {
        "name": "exit",
        "description": "Exit the shell",
        "handler": {
          "match": fn(args: [str]): bool {
            ret args == ["exit"];
          },
          "run": fn(args: [str]): i32 {
            cli.print("Goodbye!");
            cli.close();
            ret 0;
          }
        }
      }
    ]);
  }
}