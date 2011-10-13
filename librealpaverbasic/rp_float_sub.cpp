#include "rp_float.h"

#include <boost/math/special_functions/next.hpp>
#include <boost/math/special_functions/trunc.hpp>
#include <boost/math/special_functions/acosh.hpp>
#include <boost/math/special_functions/asinh.hpp>
#include <boost/math/special_functions/atanh.hpp>

#ifdef RP_SYSTEM_WIN32

/**
 * boostのnextafterは無限値(?)に対してエラーを返すので対策
 * POSIXのnextafterは無限値には無限値を返す(でも意味的にはエラーらしい)
 */
double nextafter(double x, double y) {
  if(x==RP_INFINITY || x==-RP_INFINITY) return x;
	else return boost::math::nextafter<double>(x, y);
}

double trunc(double x) {
	return boost::math::trunc<double>(x);
}

double acosh(double x) {
	return boost::math::acosh<double>(x);
}

double asinh(double x) {
	return boost::math::asinh<double>(x);
}

double atanh(double x) {
	return boost::math::atanh<double>(x);
}
#endif // RP_SYSTEM_WIN32
