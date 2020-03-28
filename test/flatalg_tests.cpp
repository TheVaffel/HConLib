#include "htest.hpp"

#define FLATALG_ALLOW_NO_AVX
#include <FlatAlg.hpp>

#include <sstream>

#include <random>

constexpr int m1 = 31;
constexpr int m2 = 340;
constexpr int m3 = 31;
constexpr int n = 100;

struct MState {
  falg::Matrix<m1, m2> *mat1;
  falg::Matrix<m1, m2> *mat2;
  falg::Matrix<m1, m2> *mat3;
};

struct M4State {
  falg::Matrix<m1, m2> *mat1;
  falg::Matrix<m1, m2> *mat2;
  falg::Matrix<m1, m2> *mat3;
  falg::Matrix<m1, m2> *mat4;
};

struct M4DState {
  falg::Matrix<m1, m2> *mat1;
  falg::Matrix<m2, m3> *mat2;
  falg::Matrix<m1, m3> *mat3;
  falg::Matrix<m1, m3> *mat4;
};

struct SState {
  falg::Matrix<m1, m2> *mat1;
  falg::flatalg_t *s0, *s1;
};

void dup_init(MState& mm) {
  mm.mat1 = new falg::Matrix<m1, m2>[n];
  mm.mat2 = new falg::Matrix<m1, m2>[n];
  mm.mat3 = new falg::Matrix<m1, m2>[n];
  for(int k = 0; k < n; k++) {
    for(int i = 0; i < m1; i++) {
      for(int j = 0; j < m2; j++) {
	mm.mat1[k](i, j) = falg::flatalg_t(rand() % 100);
	mm.mat2[k](i, j) = falg::flatalg_t(rand() % 100);
	mm.mat3[k](i, j) = mm.mat1[k](i, j);
      }
    }
  }
}

void m4_init(M4State& mm) {
  mm.mat1 = new falg::Matrix<m1, m2>[n];
  mm.mat2 = new falg::Matrix<m1, m2>[n];
  mm.mat3 = new falg::Matrix<m1, m2>[n];
  mm.mat4 = new falg::Matrix<m1, m2>[n];

  for(int k = 0; k < n; k++) {
    for(int i = 0; i < m1; i++) {
      for(int j = 0; j < m2; j++) {
	mm.mat1[k](i, j) = falg::flatalg_t(rand() % 100);
	mm.mat2[k](i, j) = falg::flatalg_t(rand() % 100);
      }
    }
  }
}

void m4_dottable_init(M4DState& mm) {
  mm.mat1 = new falg::Matrix<m1, m2>[n];
  mm.mat2 = new falg::Matrix<m2, m3>[n];
  mm.mat3 = new falg::Matrix<m1, m3>[n];
  mm.mat4 = new falg::Matrix<m1, m3>[n];

  for(int k = 0; k < n; k++) {
    for(int i = 0; i < m2; i++) {
      for(int j = 0; j < m1; j++) {
	mm.mat1[k](j, i) = falg::flatalg_t(rand() % 6);
      }
      for(int j = 0; j < m3; j++) {
	mm.mat2[k](i, j) = falg::flatalg_t(rand() % 6);
      }
    }
  }
}

void ss_init(SState& mm) {
  mm.mat1 = new falg::Matrix<m1, m2>[n];
  mm.s0 = new falg::flatalg_t[n];
  mm.s1 = new falg::flatalg_t[n];

  for(int k = 0; k < n; k++) {
    for(int i = 0; i < m1; i++) {
      for(int j = 0; j < m2; j++) {
	mm.mat1[k](i, j) = falg::flatalg_t(rand() % 100);
      }
    }
  }
}

void dup_destroy(MState& mm) {
  delete[] mm.mat1;
  delete[] mm.mat2;
  delete[] mm.mat3;
}

void m4_destroy(M4State& mm) {
  delete[] mm.mat1;
  delete[] mm.mat2;
  delete[] mm.mat3;
  delete[] mm.mat4;
}

void m4d_destroy(M4DState& mm) {
  delete[] mm.mat1;
  delete[] mm.mat2;
  delete[] mm.mat3;
  delete[] mm.mat4;
}

void ss_destroy(SState& ss) {
  delete[] ss.mat1;
  delete[] ss.s0;
  delete[] ss.s1;
}

void test_accum_add(MState& mm) {
  for(int i = 0; i < n; i++) {
    mm.mat1[i] += mm.mat2[i];
  }
}

void test_accum_sub(MState& mm) {
  for(int i = 0; i < n; i++) {
    mm.mat1[i] -= mm.mat2[i];
  }
}

void test_add(M4State& mm) {
  for(int i = 0; i < n; i++) {
    mm.mat3[i] = mm.mat1[i] + mm.mat2[i];
  }
}

void test_sub(M4State& mm) {
  for(int i = 0; i < n; i++) {
    mm.mat3[i] = mm.mat1[i] - mm.mat2[i];
  }
}

void test_mul(M4DState& mm) {
  for(int i = 0; i < n; i++) {
    mm.mat3[i] = mm.mat1[i] * mm.mat2[i];
  }
}

void test_sqnorm(SState& ss) {
  for(int i = 0; i < n; i++) {
    ss.s0[i] = ss.mat1[i].sqNorm();
  }
}

template<int n, int m>
TestResult checkEqual(int nn,
		      const falg::Matrix<n, m>* mm1,
		      const falg::Matrix<n, m>* mm2) {
  
  TestResult res;

  for(int k = 0; k < nn; k++) {
    for(int i = 0; i < n * m; i++) {
      if(mm1[k][i] != mm2[k][i]) {
	res.error = true;

	std::ostringstream oss;
	oss << "Mismatch at element " << k << ", " << i;
	res.feedback = oss.str();
	break;
      }
    }
    if(res.error) {
      break;
    }
  }

  return res;
}

TestResult checkEqual(int n,
		      const falg::flatalg_t* s0,
		      const falg::flatalg_t* s1) {
  TestResult res;
  
  for(int i = 0; i < n; i++) {
    if(std::abs(s0[i]/s1[i] - 1.0) > 1e-5) {
      res.error = true;
      std::ostringstream oss;
      oss << "Mismatch at element " << i << ", expected " << s1[i] << ", got " << s0[i];
      res.feedback = oss.str();
      break;
    }
  }
  return res;
}

TestResult evaluate_accum_add(MState& mm) {
  for(int k = 0; k < n; k++) {
    for(int i = 0; i < m1 * m2; i++) {
      mm.mat3[k][i] += mm.mat2[k][i];
    }
  }
  return checkEqual(n, mm.mat1, mm.mat3);
}

TestResult evaluate_accum_sub(MState& mm) {
  for(int k = 0; k < n; k++) {
    for(int i = 0; i < m1 * m2; i++) {
      mm.mat3[k][i] -= mm.mat2[k][i];
    }
  }
  return checkEqual(n, mm.mat1, mm.mat3);
}

TestResult evaluate_add(M4State& mm) {
  for(int k = 0; k < n; k++) {
    for(int i = 0; i < m1 * m2; i++) {
      mm.mat4[k][i] = mm.mat1[k][i] + mm.mat2[k][i];
    }
  }

  return checkEqual(n, mm.mat3, mm.mat4);
}

TestResult evaluate_sub(M4State& mm) {
  for(int k = 0; k < n; k++) {
    for(int i = 0; i < m1 * m2; i++) {
      mm.mat4[k][i] = mm.mat1[k][i] - mm.mat2[k][i];
    }
  }

  return checkEqual(n, mm.mat3, mm.mat4);
}

TestResult evaluate_dot(M4DState& mm) {
  for(int l = 0; l < n; l++) {
    for(int i = 0; i < m1 ;i++) {
      for(int j = 0; j < m3; j++) {
	mm.mat4[l](i, j) = 0;
	for(int k = 0; k < m2; k++) {
	  mm.mat4[l](i, j) += mm.mat1[l](i, k) * mm.mat2[l](k, j);
	}
      }
    }
  }

  /* std::cout << "Finished muls, m1 = " << mm.mat1[0].str() << ", m2 = " << mm.mat2[0].str() <<
     ", m3 = " << mm.mat3[0].str() << ", m4 = " << mm.mat4[0].str() << std::endl; */

  return checkEqual(n, mm.mat3, mm.mat4);
}

TestResult evaluate_sqnorm(SState& ss) {
  for(int i = 0; i < n; i++) {
    ss.s1[i] = 0;
    for(int j = 0; j < m1 * m2; j++) {
      ss.s1[i] += ss.mat1[i][j] * ss.mat1[i][j];
    }
  }

  // std::cout << "First mat was " << ss.mat1[0].str() << std::endl;
  return checkEqual(n, ss.s0, ss.s1);
}

void run_flatalg_tests() {
  std::cout << "-----------------" << std::endl;
  run_test<MState>("accumulated addition", dup_init, test_accum_add, evaluate_accum_add, dup_destroy);
  run_test<MState>("accumulated subtraction", dup_init, test_accum_sub, evaluate_accum_sub, dup_destroy);
  run_test<M4State>("addition", m4_init, test_add, evaluate_add, m4_destroy);
  run_test<M4State>("subtraction", m4_init, test_sub, evaluate_sub, m4_destroy);
  run_test<SState>("square norm", ss_init, test_sqnorm, evaluate_sqnorm, ss_destroy);
  run_test<M4DState>("matrix multiplication", m4_dottable_init, test_mul, evaluate_dot, m4d_destroy);
  std::cout << "-----------------" << std::endl;
}
