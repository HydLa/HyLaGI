#pragma once
#include <chrono>
#include <string>

namespace hydla {
namespace timer {

class Timer {
public:
  Timer();
  ~Timer();
  /**
   * 計測開始時刻を現在時刻に設定する
   */
  void restart();

  /**
   * 呼ばれた時点での経過時刻を得る
   */
  std::chrono::nanoseconds get_time() const;

  /**
   * 計測した時間を文字列で返す
   */
  std::string get_time_string() const;

  /**
   * 経過時刻を対応する単位で返す
   */
  unsigned long int get_elapsed_h() const;
  unsigned long int get_elapsed_m() const;
  unsigned long int get_elapsed_s() const;
  unsigned long int get_elapsed_ms() const;
  unsigned long int get_elapsed_us() const;
  unsigned long int get_elapsed_ns() const;

private:
  /**
   * 測定開始時の値
   */
  std::chrono::time_point<std::chrono::steady_clock> start_point_;
};
} //  namespace timer
} //  namespace hydla
