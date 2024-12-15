#include <nitrate/code.hh>

int main() {
  const std::string source = R"(
    pub fn main(): i32 {
      @(n.debug("This is a debug message!"))
      @(n.info("Hello from the preprocessor!"))
      @(n.warn("This is a warning!"))
      @(n.error("This is an error!"))
      
      ret @(return n.starttime());
    }
  )";

  std::string tokens_json;
  if (auto future = nitrate::pipeline(source, tokens_json, {"seq"}).get()) {
    std::cout << tokens_json;

    return 0;
  } else {
    std::cerr << "Compilation failed" << std::endl;

    return 1;
  }
}
