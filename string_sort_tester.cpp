
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;
using ms = std::chrono::milliseconds;

struct CharComparator {
  std::uint64_t *counter;
  bool operator()(const std::string &a, const std::string &b) const {
    std::size_t i = 0;
    while (i < a.size() && i < b.size()) {
      ++*counter;
      if (a[i] < b[i])
        return true;
      if (a[i] > b[i])
        return false;
      ++i;
    }
    ++*counter;
    return a.size() < b.size();
  }
};

void str_merge_sort(std::vector<std::string> &, std::uint64_t &);
void str_quick_sort(std::vector<std::string> &, std::uint64_t &);
void msd_radix_sort(std::vector<std::string> &, std::uint64_t &);
void msd_radix_quick_sort(std::vector<std::string> &, std::uint64_t &);

static void merge_rec(std::vector<std::string> &a,
                      std::vector<std::string> &buf, int l, int r,
                      CharComparator cmp) {
  if (r - l <= 1)
    return;
  int m = (l + r) / 2;
  merge_rec(a, buf, l, m, cmp);
  merge_rec(a, buf, m, r, cmp);
  int i = l, j = m, k = l;
  while (i < m && j < r)
    buf[k++] = cmp(a[i], a[j]) ? a[i++] : a[j++];
  while (i < m)
    buf[k++] = a[i++];
  while (j < r)
    buf[k++] = a[j++];
  for (int t = l; t < r; ++t)
    a[t] = std::move(buf[t]);
}
void str_merge_sort(std::vector<std::string> &v, std::uint64_t &cmp) {
  CharComparator cc{&cmp};
  std::vector<std::string> buf(v.size());
  merge_rec(v, buf, 0, (int)v.size(), cc);
}

static void quick_rec(std::vector<std::string> &a, int l, int r, int d,
                      std::uint64_t &cmp) {
  if (r - l <= 1)
    return;
  auto ch = [](const std::string &s, int d) {
    return d < (int)s.size() ? s[d] : -1;
  };
  int lt = l, gt = r - 1, pivot = ch(a[l], d), i = l + 1;
  while (i <= gt) {
    int t = ch(a[i], d);
    ++cmp;
    if (t < pivot)
      std::swap(a[lt++], a[i++]);
    else {
      ++cmp;
      if (t > pivot)
        std::swap(a[i], a[gt--]);
      else
        ++i;
    }
  }
  quick_rec(a, l, lt, d, cmp);
  if (pivot >= 0)
    quick_rec(a, lt, gt + 1, d + 1, cmp);
  quick_rec(a, gt + 1, r, d, cmp);
}
void str_quick_sort(std::vector<std::string> &v, std::uint64_t &cmp) {
  quick_rec(v, 0, (int)v.size(), 0, cmp);
}

static void radix_rec(std::vector<std::string> &a, int l, int r, int d,
                      std::uint64_t &cmp) {
  if (r - l <= 1)
    return;
  const int R = 128;
  std::vector<int> cnt(R + 2);
  std::vector<std::string> buf(r - l);
  for (int i = l; i < r; ++i)
    ++cnt[(d < (int)a[i].size() ? a[i][d] : -1) + 2];
  for (int i = 1; i < R + 2; ++i)
    cnt[i] += cnt[i - 1];
  for (int i = l; i < r; ++i) {
    int c = (d < (int)a[i].size() ? a[i][d] : -1);
    buf[cnt[c + 1]++] = std::move(a[i]);
  }
  for (int i = 0; i < r - l; ++i)
    a[l + i] = std::move(buf[i]);
  int beg = l;
  for (int ch = 0; ch < R + 1; ++ch) {
    int end = l + cnt[ch + 1];
    if (end - beg > 1 && ch)
      radix_rec(a, beg, end, d + 1, cmp);
    beg = end;
  }
}
void msd_radix_sort(std::vector<std::string> &v, std::uint64_t &cmp) {
  radix_rec(v, 0, (int)v.size(), 0, cmp);
}

const int TH = 74;
static void radix_quick_rec(std::vector<std::string> &a, int l, int r, int d,
                            std::uint64_t &cmp) {
  if (r - l <= 1)
    return;
  if (r - l < TH) {
    quick_rec(a, l, r, d, cmp);
    return;
  }
  const int R = 128;
  std::vector<int> cnt(R + 2);
  std::vector<std::string> buf(r - l);
  for (int i = l; i < r; ++i)
    ++cnt[(d < (int)a[i].size() ? a[i][d] : -1) + 2];
  for (int i = 1; i < R + 2; ++i)
    cnt[i] += cnt[i - 1];
  for (int i = l; i < r; ++i) {
    int c = (d < (int)a[i].size() ? a[i][d] : -1);
    buf[cnt[c + 1]++] = std::move(a[i]);
  }
  for (int i = 0; i < r - l; ++i)
    a[l + i] = std::move(buf[i]);
  int beg = l;
  for (int ch = 0; ch < R + 1; ++ch) {
    int end = l + cnt[ch + 1];
    if (end - beg > 1 && ch)
      radix_quick_rec(a, beg, end, d + 1, cmp);
    beg = end;
  }
}
void msd_radix_quick_sort(std::vector<std::string> &v, std::uint64_t &cmp) {
  radix_quick_rec(v, 0, (int)v.size(), 0, cmp);
}
class StringSortTester {
public:
  using SortFunc =
      std::function<void(std::vector<std::string> &, std::uint64_t &)>;
  void add_algorithm(const std::string &n, const SortFunc &f) {
    algs.push_back(std::make_pair(n, f));
  }

  void run_for_file(const std::string &file, const std::string &prefix,
                    std::size_t max_n = 3000, std::size_t step = 100) {
    std::vector<std::string> all;
    if (!read_file(file, all) || all.size() < max_n) {
      std::cerr << "Skip: " << file << '\n';
      return;
    }

    std::ofstream csvT(prefix + "_time_sorts.csv");
    std::ofstream csvC(prefix + "_compares_sorts.csv");
    write_header(csvT);
    write_header(csvC);

    for (std::size_t n = step; n <= max_n; n += step) {
      std::vector<std::string> base(all.begin(), all.begin() + n);
      csvT << n;
      csvC << n;

      for (std::size_t idx = 0; idx < algs.size(); ++idx) {
        const SortFunc &sort = algs[idx].second;
        std::vector<std::string> v = base;
        double tms;
        std::uint64_t cmp = timed(v, sort, tms);
        csvT << ',' << (std::uint64_t)tms;
        csvC << ',' << cmp;
      }
      csvT << '\n';
      csvC << '\n';
      std::cout << prefix << " n=" << n << " done\n";
    }
  }

  void run_all(const std::vector<std::pair<std::string, std::string>> &files) {
    for (std::size_t idx = 0; idx < files.size(); ++idx)
      run_for_file(files[idx].first, files[idx].second);
  }

private:
  std::vector<std::pair<std::string, SortFunc>> algs;

  static bool read_file(const std::string &path, std::vector<std::string> &v) {
    std::ifstream in(path);
    if (!in)
      return false;
    v.clear();
    std::string line;
    while (std::getline(in, line))
      v.emplace_back(std::move(line));
    return true;
  }

  static void write_header(std::ofstream &os) {
    os << "array_len,str_quick,str_merge,msd_radix,msd_radix_quick,merge_"
          "default,quick_default\n";
  }

  static std::uint64_t timed(std::vector<std::string> &v, const SortFunc &f,
                             double &ms_out) {
    std::uint64_t cmp = 0;
    auto t0 = Clock::now();
    f(v, cmp);
    auto t1 = Clock::now();
    ms_out = std::chrono::duration_cast<ms>(t1 - t0).count();
    return cmp;
  }
};

int main() {
  std::ios::sync_with_stdio(false);

  StringSortTester tst;
  tst.add_algorithm("str_quick", str_quick_sort);
  tst.add_algorithm("str_merge", str_merge_sort);
  tst.add_algorithm("msd_radix", msd_radix_sort);
  tst.add_algorithm("msd_radix_quick", msd_radix_quick_sort);

  tst.add_algorithm("merge_default",
                    [](std::vector<std::string> &v, std::uint64_t &c) {
                      CharComparator cmp{&c};
                      std::stable_sort(v.begin(), v.end(), cmp);
                    });
  tst.add_algorithm("quick_default",
                    [](std::vector<std::string> &v, std::uint64_t &c) {
                      CharComparator cmp{&c};
                      std::sort(v.begin(), v.end(), cmp);
                    });

  const std::vector<std::pair<std::string, std::string>> files = {
      {"random_strings.txt", "random"},
      {"reverse_sorted_strings.txt", "reversed"},
      {"almost_sorted_strings.txt", "almost_sorted"}};

  tst.run_all(files);
  std::cout << "Benchmark finished.\n";
  return 0;
}
