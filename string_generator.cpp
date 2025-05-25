#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

class StringGenerator {
public:
  StringGenerator(int minLen = 10, int maxLen = 200, double swapRatio = 0.01)
      : minLen_(minLen), maxLen_(maxLen), swapRatio_(swapRatio),
        rng_(std::chrono::steady_clock::now().time_since_epoch().count()),
        lenDist_(minLen_, maxLen_),
        charDist_(0, static_cast<int>(alphabet_.size() - 1)) {}

  std::string makeRandomString() {
    int len = lenDist_(rng_);
    std::string s;
    s.reserve(len);
    for (int i = 0; i < len; ++i)
      s.push_back(alphabet_[charDist_(rng_)]);
    return s;
  }

  std::vector<std::string> makeRandomArray(std::size_t n) {
    std::vector<std::string> v;
    v.reserve(n);
    for (std::size_t i = 0; i < n; ++i)
      v.push_back(makeRandomString());
    return v;
  }

  std::vector<std::string> makeReverseSortedArray(std::size_t n) {
    auto v = makeRandomArray(n);
    std::sort(v.begin(), v.end(), std::greater<>());
    return v;
  }

  std::vector<std::string> makeAlmostSortedArray(std::size_t n) {
    auto v = makeRandomArray(n);
    std::sort(v.begin(), v.end());
    std::size_t swaps = static_cast<std::size_t>(n * swapRatio_);
    std::uniform_int_distribution<std::size_t> idxDist(0, n - 1);
    for (std::size_t i = 0; i < swaps; ++i) {
      std::size_t a = idxDist(rng_), b = idxDist(rng_);
      std::swap(v[a], v[b]);
    }
    return v;
  }

  static void saveTxt(const std::vector<std::string> &v,
                      const std::string &filename) {
    std::ofstream out(filename);
    if (!out)
      throw std::runtime_error("Cannot open file " + filename);
    for (const auto &s : v)
      out << s << '\n';
  }

private:
  int minLen_, maxLen_;
  double swapRatio_;
  std::mt19937 rng_;
  std::uniform_int_distribution<int> lenDist_;
  std::uniform_int_distribution<int> charDist_;

  static const std::string alphabet_;
};

const std::string StringGenerator::alphabet_ = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                               "abcdefghijklmnopqrstuvwxyz"
                                               "0123456789"
                                               "!@#%:;^&*()-.";

int main() {
  constexpr std::size_t MAX_SIZE = 3000;
  try {
    StringGenerator gen;
    auto randomArr = gen.makeRandomArray(MAX_SIZE);
    auto reverseSortedArr = gen.makeReverseSortedArray(MAX_SIZE);
    auto almostSortedArr = gen.makeAlmostSortedArray(MAX_SIZE);

    StringGenerator::saveTxt(randomArr, "random_strings.txt");
    StringGenerator::saveTxt(reverseSortedArr, "reverse_sorted_strings.txt");
    StringGenerator::saveTxt(almostSortedArr, "almost_sorted_strings.txt");

    std::cout << "TXT files have been created.\n";
  } catch (const std::exception &ex) {
    std::cerr << "Error: " << ex.what() << '\n';
    return 1;
  }
  return 0;
}
