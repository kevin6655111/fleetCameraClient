#include <chrono>

class Timer {
  public:
    Timer() : last_time(std::chrono::steady_clock::now()) {}

    bool has_elapsed(int milliseconds) {
      auto now = std::chrono::steady_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count();

      if (duration >= milliseconds) {
        last_time = now;
        return true;
      }

      return false;
    }

    void reset() {
      last_time = std::chrono::steady_clock::now();
    }

    std::chrono::time_point<std::chrono::steady_clock> get_last_time() const {
      return last_time;
    }

  private:
    std::chrono::time_point<std::chrono::steady_clock> last_time;
};