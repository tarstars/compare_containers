#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

struct Config {
  std::vector<std::size_t> sizes;
  std::size_t trials = 5;
  std::uint64_t seed = 1337;
  std::string output_csv = "results/benchmark.csv";
  std::string output_log = "results/run.log";
};

void PrintUsage(const char *argv0) {
  std::cout
      << "Usage: " << argv0 << " [options]\n\n"
      << "Options:\n"
      << "  --sizes N1,N2,...   Sizes to benchmark (default: 1000,3000,10000,30000,100000,300000)\n"
      << "  --trials N          Trials per size (default: 5)\n"
      << "  --seed N            RNG seed (default: 1337)\n"
      << "  --output PATH       CSV output path (default: results/benchmark.csv)\n"
      << "  --log PATH          Log output path (default: results/run.log)\n"
      << "  --help              Show this help\n";
}

std::vector<std::size_t> ParseSizes(const std::string &arg) {
  std::vector<std::size_t> sizes;
  std::stringstream ss(arg);
  std::string item;
  while (std::getline(ss, item, ',')) {
    if (!item.empty()) {
      sizes.push_back(static_cast<std::size_t>(std::stoull(item)));
    }
  }
  return sizes;
}

Config ParseArgs(int argc, char **argv) {
  Config cfg;
  cfg.sizes = {1000, 3000, 10000, 30000, 100000, 300000};

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--help") {
      PrintUsage(argv[0]);
      std::exit(0);
    } else if (arg == "--sizes" && i + 1 < argc) {
      cfg.sizes = ParseSizes(argv[++i]);
    } else if (arg == "--trials" && i + 1 < argc) {
      cfg.trials = static_cast<std::size_t>(std::stoull(argv[++i]));
    } else if (arg == "--seed" && i + 1 < argc) {
      cfg.seed = static_cast<std::uint64_t>(std::stoull(argv[++i]));
    } else if (arg == "--output" && i + 1 < argc) {
      cfg.output_csv = argv[++i];
    } else if (arg == "--log" && i + 1 < argc) {
      cfg.output_log = argv[++i];
    } else {
      std::cerr << "Unknown or incomplete option: " << arg << "\n";
      PrintUsage(argv[0]);
      std::exit(1);
    }
  }
  return cfg;
}

std::vector<int> MakeKeys(std::size_t n, std::uint64_t seed) {
  std::vector<int> keys(n);
  std::iota(keys.begin(), keys.end(), 0);
  std::mt19937_64 rng(seed);
  std::shuffle(keys.begin(), keys.end(), rng);
  return keys;
}

std::vector<double> SortTimes(std::vector<double> times) {
  std::sort(times.begin(), times.end());
  return times;
}

template <typename Fn>
double MedianTime(std::size_t trials, Fn &&runner) {
  std::vector<double> times;
  times.reserve(trials);
  for (std::size_t t = 0; t < trials; ++t) {
    times.push_back(runner());
  }
  auto sorted = SortTimes(times);
  std::size_t mid = sorted.size() / 2;
  if (sorted.size() % 2 == 0) {
    return (sorted[mid - 1] + sorted[mid]) / 2.0;
  }
  return sorted[mid];
}

volatile std::uint64_t g_sink = 0;

double BenchmarkMap(const std::vector<int> &keys,
                    const std::vector<int> &queries,
                    std::size_t trials) {
  auto runner = [&]() {
    auto start = std::chrono::steady_clock::now();
    std::map<int, int> m;
    for (int k : keys) {
      m[k] = k;
    }
    for (int q : queries) {
      auto it = m.find(q);
      if (it != m.end()) {
        g_sink += static_cast<std::uint64_t>(it->second);
      }
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    return elapsed.count();
  };
  return MedianTime(trials, runner);
}

double BenchmarkUnorderedMap(const std::vector<int> &keys,
                             const std::vector<int> &queries,
                             std::size_t trials) {
  auto runner = [&]() {
    auto start = std::chrono::steady_clock::now();
    std::unordered_map<int, int> m;
    m.reserve(keys.size() * 2);
    m.max_load_factor(0.7f);
    for (int k : keys) {
      m[k] = k;
    }
    for (int q : queries) {
      auto it = m.find(q);
      if (it != m.end()) {
        g_sink += static_cast<std::uint64_t>(it->second);
      }
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    return elapsed.count();
  };
  return MedianTime(trials, runner);
}

}  // namespace

int main(int argc, char **argv) {
  Config cfg = ParseArgs(argc, argv);

  std::ofstream csv(cfg.output_csv);
  if (!csv) {
    std::cerr << "Failed to open CSV output: " << cfg.output_csv << "\n";
    return 1;
  }

  std::ofstream log(cfg.output_log);
  if (!log) {
    std::cerr << "Failed to open log output: " << cfg.output_log << "\n";
    return 1;
  }

  log << "Benchmark config\n";
  log << "sizes=";
  for (std::size_t i = 0; i < cfg.sizes.size(); ++i) {
    if (i > 0) {
      log << ",";
    }
    log << cfg.sizes[i];
  }
  log << "\ntrials=" << cfg.trials << "\nseed=" << cfg.seed << "\n\n";

  csv << "size,map_ms,unordered_map_ms\n";
  csv << std::fixed << std::setprecision(3);

  for (std::size_t size : cfg.sizes) {
    auto keys = MakeKeys(size, cfg.seed + size);
    auto queries = keys;
    std::mt19937_64 rng(cfg.seed + size * 101);
    std::shuffle(queries.begin(), queries.end(), rng);

    double map_ms = BenchmarkMap(keys, queries, cfg.trials);
    double umap_ms = BenchmarkUnorderedMap(keys, queries, cfg.trials);

    csv << size << "," << map_ms << "," << umap_ms << "\n";
    log << "size=" << size << " map_ms=" << map_ms << " unordered_map_ms=" << umap_ms
        << "\n";

    std::cout << "Size " << size << ": map=" << map_ms << " ms, unordered_map="
              << umap_ms << " ms\n";
  }

  return 0;
}
