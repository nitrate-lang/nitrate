#include <chrono>
#include <fstream>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <nitrate-parser/Context.hh>
#include <sstream>

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

template <typename T>
struct Statistic {
  T m_total;
  T m_mean;
  T m_variance;
  T m_stddev;
};

template <typename T>
static auto CalculateStatistic(const std::vector<T> &data) -> Statistic<T> {
  T total = 0.0;
  for (const auto &value : data) {
    total += value;
  }
  T mean = total / data.size();

  T variance = 0.0;
  for (const auto &value : data) {
    variance += std::pow(value - mean, 2);
  }
  variance /= data.size();

  return {total, mean, variance, std::sqrt(variance)};
}

static size_t BenchEncode(const Expr *root) {
  std::stringstream ss;
  AstWriter writer(ss);
  root->Accept(writer);

  return ss.str().size();
}

static void DoBenchmark(const Expr *root) {
  constexpr size_t kNumIterations = 128;
  size_t encoded_size = 0;

  std::cout << "Starting benchmark..." << std::endl;
  std::cout << "  Rounds: " << kNumIterations << std::endl;

  std::vector<double> times;
  for (size_t i = 0; i < kNumIterations; i++) {
    std::cout << "Round " << i + 1 << " of " << kNumIterations << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    encoded_size = BenchEncode(root);
    auto end = std::chrono::high_resolution_clock::now();

    double nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    times.push_back(nanoseconds);
  }

  auto stats = CalculateStatistic(times);

  std::cout << "Benchmark results:" << std::endl;
  std::cout << "  Rounds: " << kNumIterations << std::endl;
  std::cout << "  Encoded size: " << encoded_size << " bytes" << std::endl;
  std::cout << "  Total time: " << stats.m_total << "ns" << std::endl;
  std::cout << "  Round time mean: " << stats.m_mean << "ns" << std::endl;
  std::cout << "  Round time variance: " << stats.m_variance << "ns" << std::endl;
  std::cout << "  Round time standard deviation: " << stats.m_stddev << "ns" << std::endl;
}

int main(int argc, char *argv[]) {
  std::vector<std::string> args(argv, argv + argc);

  if (args.size() < 2) {
    std::cerr << "Usage: " << args[0] << " <input-file>" << std::endl;
    return 1;
  }

  const auto input_file = args[1];
  std::ifstream input_stream(input_file);
  if (!input_stream.is_open()) {
    std::cerr << "Failed to open input file: " << input_file << std::endl;
    return 1;
  }

  auto environment = std::make_shared<Environment>();
  auto scanner = Tokenizer(input_stream, environment);
  auto parser = GeneralParser(scanner, {}, environment).Parse();
  if (!parser.Check()) {
    std::cerr << "Failed to parse input file: " << input_file << std::endl;
    return 1;
  }

  auto original_ast = parser.Get();

  DoBenchmark(original_ast.get());

  return 0;
}
