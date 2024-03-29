#include <cpp11.hpp>
// #include <Rinternals.h>
// #include <stdbool.h>
// #include <math.h>

#define R_NO_REMAP

// Author: Nick Christofides
// Date: 20-Feb-2024

// Below are a complete set of C++ functions for comparing doubles
// mimicking the ==, <=, <, > and >= operators with a tolerance

// Relative differences are used except when either x or y is very close to zero
// in which case absolute differences are used

bool cpp_double_is_na(double x){
  return !(x == x);
}
bool cpp_double_both_same_inf(double x, double y){
  return (x == R_PosInf && y == R_PosInf) || (x == R_NegInf && y == R_NegInf);
}
bool cpp_double_any_inf(double x, double y){
  return x == R_PosInf || y == R_PosInf || x == R_NegInf || y == R_NegInf;
}
double cpp_double_abs_diff(double x, double y){
  return std::fabs(x - y);
}
double cpp_double_rel_diff(double x, double y){
  return cpp_double_abs_diff(x, y) / std::fmax(std::fabs(x), std::fabs(y));
}
bool cpp_double_any_zero(double x, double y, double tolerance){
  return ( std::fabs(x) < tolerance ) || ( std::fabs(y) < tolerance );
}

// Testing equality

bool cpp_double_equal(double x, double y, double tolerance){
  double ax = std::fabs(x);
  double ay = std::fabs(y);
  bool any_zero = ( ax < tolerance ) || ( ay < tolerance );
  double adiff = std::fabs(x - y);
  double rdiff = adiff / std::fmax(ax, ay);
  double out;
  if (any_zero){
    out = (adiff < tolerance);
  } else {
    out = cpp_double_both_same_inf(x, y) || (rdiff < tolerance);
  }
  return out;
}
bool cpp_double_equal_abs(double x, double y, double tolerance){
  double adiff = std::fabs(x - y);
  return cpp_double_both_same_inf(x, y) || (adiff < tolerance);
}
bool cpp_double_equal_strict(double x, double y, double tolerance){
  return cpp_double_equal_abs(x, y, tolerance) &&
    cpp_double_equal(x, y, tolerance);
}

// Testing >, >=, < and <=
bool cpp_double_gt(double x, double y, double tolerance){
  double out;
  double diff = (x - y);
  bool any_zero = cpp_double_any_zero(x, y, tolerance);
  if (any_zero || cpp_double_any_inf(x, y)){
    out = diff > tolerance;
  } else {
    out = (diff / std::fmax(std::fabs(x), std::fabs(y))) > tolerance;
  }
  return out;
}
bool cpp_double_lt(double x, double y, double tolerance){
  double out;
  double diff = (x - y);
  bool any_zero = cpp_double_any_zero(x, y, tolerance);
  if (any_zero || cpp_double_any_inf(x, y)){
    out = diff < -tolerance;
  } else {
    out = (diff / std::fmax(std::fabs(x), std::fabs(y))) < -tolerance;
  }
  return out;
}
bool cpp_double_gte(double x, double y, double tolerance){
  return cpp_double_gt(x, y, tolerance) || cpp_double_equal(x, y, tolerance);
}
bool cpp_double_lte(double x, double y, double tolerance){
  return cpp_double_lt(x, y, tolerance) || cpp_double_equal(x, y, tolerance);
}


[[cpp11::register]]
SEXP cpp_double_equal_vectorised(SEXP x, SEXP y, SEXP tolerance) {
  // double tolerance = std::sqrt(std::numeric_limits<double>::epsilon());
  R_xlen_t x_len = Rf_xlength(x);
  R_xlen_t y_len = Rf_xlength(y);
  R_xlen_t tol_len = Rf_xlength(tolerance);
  R_xlen_t n = std::max(std::max(x_len, y_len), tol_len);
  if (x_len <= 0 || y_len <= 0 || tol_len <= 0){
    // Avoid loop if any are length zero vectors
    n = 0;
  }
  double *p_x = REAL(x);
  double *p_y = REAL(y);
  double *p_t = REAL(tolerance);
  SEXP out = Rf_protect(Rf_allocVector(LGLSXP, n));
  int *p_out = LOGICAL(out);
  R_xlen_t xi, yi, ti;
  for (R_xlen_t i = 0; i < n; ++i) {
    xi = i % x_len;
    yi = i % y_len;
    ti = i % tol_len;
    p_out[i] = cpp_double_equal(p_x[xi], p_y[yi], p_t[ti]);
    // If either is NA, out is NA
    if (cpp_double_is_na(p_x[xi]) ||
        cpp_double_is_na(p_y[yi]) ||
        cpp_double_is_na(p_t[ti])){
      p_out[i] = NA_LOGICAL;
    }
  }
  Rf_unprotect(1);
  return out;
}

[[cpp11::register]]
SEXP cpp_double_gt_vectorised(SEXP x, SEXP y, SEXP tolerance) {
  // double tolerance = std::sqrt(std::numeric_limits<double>::epsilon());
  R_xlen_t x_len = Rf_xlength(x);
  R_xlen_t y_len = Rf_xlength(y);
  R_xlen_t tol_len = Rf_xlength(tolerance);
  R_xlen_t n = std::max(std::max(x_len, y_len), tol_len);
  if (x_len <= 0 || y_len <= 0 || tol_len <= 0){
    // Avoid loop if any are length zero vectors
    n = 0;
  }
  double *p_x = REAL(x);
  double *p_y = REAL(y);
  double *p_t = REAL(tolerance);
  SEXP out = Rf_protect(Rf_allocVector(LGLSXP, n));
  int *p_out = LOGICAL(out);
  R_xlen_t xi, yi, ti;
  for (R_xlen_t i = 0; i < n; ++i) {
    xi = i % x_len;
    yi = i % y_len;
    ti = i % tol_len;
    p_out[i] = cpp_double_gt(p_x[xi], p_y[yi], p_t[ti]);
    // If either is NA, out is NA
    if (cpp_double_is_na(p_x[xi]) ||
        cpp_double_is_na(p_y[yi]) ||
        cpp_double_is_na(p_t[ti])){
      p_out[i] = NA_LOGICAL;
    }
  }
  Rf_unprotect(1);
  return out;
}

[[cpp11::register]]
SEXP cpp_double_gte_vectorised(SEXP x, SEXP y, SEXP tolerance) {
  // double tolerance = std::sqrt(std::numeric_limits<double>::epsilon());
  R_xlen_t x_len = Rf_xlength(x);
  R_xlen_t y_len = Rf_xlength(y);
  R_xlen_t tol_len = Rf_xlength(tolerance);
  R_xlen_t n = std::max(std::max(x_len, y_len), tol_len);
  if (x_len <= 0 || y_len <= 0 || tol_len <= 0){
    // Avoid loop if any are length zero vectors
    n = 0;
  }
  double *p_x = REAL(x);
  double *p_y = REAL(y);
  double *p_t = REAL(tolerance);
  SEXP out = Rf_protect(Rf_allocVector(LGLSXP, n));
  int *p_out = LOGICAL(out);
  R_xlen_t xi, yi, ti;
  for (R_xlen_t i = 0; i < n; ++i) {
    xi = i % x_len;
    yi = i % y_len;
    ti = i % tol_len;
    p_out[i] = cpp_double_gte(p_x[xi], p_y[yi], p_t[ti]);
    // If either is NA, out is NA
    if (cpp_double_is_na(p_x[xi]) ||
        cpp_double_is_na(p_y[yi]) ||
        cpp_double_is_na(p_t[ti])){
      p_out[i] = NA_LOGICAL;
    }
  }
  Rf_unprotect(1);
  return out;
}

[[cpp11::register]]
SEXP cpp_double_lt_vectorised(SEXP x, SEXP y, SEXP tolerance) {
  // double tolerance = std::sqrt(std::numeric_limits<double>::epsilon());
  R_xlen_t x_len = Rf_xlength(x);
  R_xlen_t y_len = Rf_xlength(y);
  R_xlen_t tol_len = Rf_xlength(tolerance);
  R_xlen_t n = std::max(std::max(x_len, y_len), tol_len);
  if (x_len <= 0 || y_len <= 0 || tol_len <= 0){
    // Avoid loop if any are length zero vectors
    n = 0;
  }
  double *p_x = REAL(x);
  double *p_y = REAL(y);
  double *p_t = REAL(tolerance);
  SEXP out = Rf_protect(Rf_allocVector(LGLSXP, n));
  int *p_out = LOGICAL(out);
  R_xlen_t xi, yi, ti;
  for (R_xlen_t i = 0; i < n; ++i) {
    xi = i % x_len;
    yi = i % y_len;
    ti = i % tol_len;
    p_out[i] = cpp_double_lt(p_x[xi], p_y[yi], p_t[ti]);
    // If either is NA, out is NA
    if (cpp_double_is_na(p_x[xi]) ||
        cpp_double_is_na(p_y[yi]) ||
        cpp_double_is_na(p_t[ti])){
      p_out[i] = NA_LOGICAL;
    }
  }
  Rf_unprotect(1);
  return out;
}

[[cpp11::register]]
SEXP cpp_double_lte_vectorised(SEXP x, SEXP y, SEXP tolerance) {
  // double tolerance = std::sqrt(std::numeric_limits<double>::epsilon());
  R_xlen_t x_len = Rf_xlength(x);
  R_xlen_t y_len = Rf_xlength(y);
  R_xlen_t tol_len = Rf_xlength(tolerance);
  R_xlen_t n = std::max(std::max(x_len, y_len), tol_len);
  if (x_len <= 0 || y_len <= 0 || tol_len <= 0){
    // Avoid loop if any are length zero vectors
    n = 0;
  }
  double *p_x = REAL(x);
  double *p_y = REAL(y);
  double *p_t = REAL(tolerance);
  SEXP out = Rf_protect(Rf_allocVector(LGLSXP, n));
  int *p_out = LOGICAL(out);
  R_xlen_t xi, yi, ti;
  for (R_xlen_t i = 0; i < n; ++i) {
    xi = i % x_len;
    yi = i % y_len;
    ti = i % tol_len;
    p_out[i] = cpp_double_lte(p_x[xi], p_y[yi], p_t[ti]);
    // If either is NA, out is NA
    if (cpp_double_is_na(p_x[xi]) ||
        cpp_double_is_na(p_y[yi]) ||
        cpp_double_is_na(p_t[ti])){
      p_out[i] = NA_LOGICAL;
    }
  }
  Rf_unprotect(1);
  return out;
}

[[cpp11::register]]
SEXP cpp_double_rel_diff_vectorised(SEXP x, SEXP y) {
  R_xlen_t x_len = Rf_xlength(x);
  R_xlen_t y_len = Rf_xlength(y);
  R_xlen_t n = std::max(x_len, y_len);
  if (x_len <= 0 || y_len <= 0){
    // Avoid loop if any are length zero vectors
    n = 0;
  }
  double *p_x = REAL(x);
  double *p_y = REAL(y);
  SEXP out = Rf_protect(Rf_allocVector(REALSXP, n));
  double *p_out = REAL(out);
  R_xlen_t xi;
  R_xlen_t yi;
  for (R_xlen_t i = 0; i < n; ++i) {
    xi = i % x_len;
    yi = i % y_len;
    p_out[i] = cpp_double_rel_diff(p_x[xi], p_y[yi]);
  }
  Rf_unprotect(1);
  return out;
}
[[cpp11::register]]
SEXP cpp_all_equal(SEXP x, SEXP y, SEXP tolerance, SEXP na_rm) {
  if (Rf_length(na_rm) != 1 || !Rf_isLogical(na_rm)){
    Rf_error("na.rm must be a logical vector of length 1");
  }
  bool skip_na = Rf_asLogical(na_rm);
  bool has_na;
  R_xlen_t x_len = Rf_xlength(x);
  R_xlen_t y_len = Rf_xlength(y);
  R_xlen_t tol_len = Rf_xlength(tolerance);
  R_xlen_t n = std::max(std::max(x_len, y_len), tol_len);
  if (x_len <= 0 || y_len <= 0 || tol_len <= 0){
    // Avoid loop if any are length zero vectors
    n = 0;
  }
  double *p_x = REAL(x);
  double *p_y = REAL(y);
  double *p_t = REAL(tolerance);
  SEXP out = Rf_protect(Rf_allocVector(LGLSXP, 1));
  LOGICAL(out)[0] = true;
  R_xlen_t xi, yi, ti;
  for (R_xlen_t i = 0; i < n; ++i) {
    xi = i % x_len;
    yi = i % y_len;
    ti = i % tol_len;
    has_na = cpp_double_is_na(p_x[xi]) ||
      cpp_double_is_na(p_y[yi]) ||
      cpp_double_is_na(p_t[ti]);
    if (has_na){
      if (skip_na){
        continue;
      } else {
        LOGICAL(out)[0] = NA_LOGICAL;
        break;
      }
    }
    if ( !cpp_double_equal(p_x[xi], p_y[yi], p_t[ti]) ){
      LOGICAL(out)[0] = false;
      break;
    }
  }
  Rf_unprotect(1);
  return out;
}
