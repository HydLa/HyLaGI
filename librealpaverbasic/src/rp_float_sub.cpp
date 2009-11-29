#include "rp_float.h"

#include <boost/math/special_functions/next.hpp>
#include <boost/math/special_functions/trunc.hpp>
#include <boost/math/special_functions/acosh.hpp>
#include <boost/math/special_functions/asinh.hpp>
#include <boost/math/special_functions/atanh.hpp>

double nextafter_next(double x) {
	return boost::math::nextafter<double>(x,RP_INFINITY);
}

double nextafter_prev(double x) {
	return boost::math::nextafter<double>(x,(-RP_INFINITY));
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
