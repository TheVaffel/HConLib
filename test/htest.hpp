#include <iostream>
#include <chrono>

struct TestResult {
  std::string feedback;
  bool error = false;
};

template<typename state_t>
void run_test(const std::string& name,
	      void (*init)(state_t&),
	      void (*run)(state_t&),
	      TestResult (*evaluate)(state_t&),
	      void (*destroy)(state_t&)) {
  state_t st;
  init(st);

  std::chrono::steady_clock::time_point rstart = std::chrono::steady_clock::now();

  run(st);
  std::chrono::steady_clock::time_point  rend = std::chrono::steady_clock::now();

  std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(rend - rstart);

  TestResult res = evaluate(st);

  destroy(st);

  if(res.error) {
    std::cout << "[HTest] Encountered error when checking results of " << name << ": " << res.feedback << std::endl;
  } else {
    std::cout << "[HTest] Test " << name << " finished without errors in " << time_span.count() << " seconds" << std::endl;
  }
}
