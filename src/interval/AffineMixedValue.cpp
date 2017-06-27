#include "AffineMixedValue.h"
#include "Logger.h"

namespace hydla{
namespace interval{

AffineMixedValue::AffineMixedValue(){}
AffineMixedValue::AffineMixedValue(int i):integer(i), type(INTEGER){}
AffineMixedValue::AffineMixedValue(itvd itv):interval(itv), type(INTERVAL){}
AffineMixedValue::AffineMixedValue(affine_t a):affine_value(a), type(AFFINE){}

class ApproximateException:public std::runtime_error{
public:
  ApproximateException(const std::string& msg):
    std::runtime_error("error occurred in approximation: " + msg){}
};

std::ostream& operator<<(std::ostream &ost, const AffineMixedValue &val)
{
  if(val.type == INTEGER)
  {
    ost << "integer: " << val.integer;
  }else if(val.type == INTERVAL)
  {
    ost << "interval: " << val.interval;
  }else
  {
    ost << "affine: " << val.affine_value;
  }
  return ost;
}

AffineMixedValue AffineMixedValue::operator+(const AffineMixedValue &rhs) const
{
  AffineMixedValue ret;

  switch(type)
  {
  case INTEGER:
    switch(rhs.type)
    {
    case INTEGER:
      ret.type = INTEGER;
      ret.integer = integer + rhs.integer;
      break;
    case INTERVAL:
      ret.type = INTERVAL;
      ret.interval = itvd(integer) + rhs.interval;
      break;
    case AFFINE:
      ret.type = AFFINE;
      ret.affine_value = itvd(integer) + rhs.affine_value;
      break;
    }
    break;

  case INTERVAL:
    switch(rhs.type)
    {
    case INTEGER:
      ret.type = INTERVAL;
      ret.interval = interval + itvd(rhs.integer);
      break;
    case INTERVAL:
      ret.type = INTERVAL;
      ret.interval = interval + itvd(rhs.interval);
      break;
    case AFFINE:
      ret.type = AFFINE;
      ret.affine_value = affine_t(interval) + rhs.affine_value;
      break;
    }
    break;
  case AFFINE:
    ret.type = AFFINE;
    switch(rhs.type)
    {
    case INTEGER:
      ret.affine_value = affine_value + affine_t(rhs.integer);
      break;
    case INTERVAL:
      ret.affine_value = affine_value + affine_t(rhs.interval);
      break;
    case AFFINE:
      ret.affine_value = affine_value + rhs.affine_value;
      break;
    }
    break;
  }
  return ret;
}

AffineMixedValue AffineMixedValue::operator-(const AffineMixedValue &rhs) const
{
  return -(*this) + -(rhs);
}

AffineMixedValue AffineMixedValue::operator*(const AffineMixedValue &rhs) const
{
  AffineMixedValue ret;
  switch(type)
  {
  case INTEGER:
    switch(rhs.type)
    {
    case INTEGER:
      ret.type = INTEGER;
      ret.integer = integer * rhs.integer;
      break;
    case INTERVAL:
      ret.type = INTERVAL;
      ret.interval = itvd(integer) * rhs.interval;
      break;
    case AFFINE:
      ret.type = AFFINE;
      ret.affine_value = itvd(integer) * rhs.affine_value;
      break;
    }
    break;

  case INTERVAL:
    switch(rhs.type)
    {
    case INTEGER:
      ret.type = INTERVAL;
      ret.interval = interval * itvd(rhs.integer);
      break;
    case INTERVAL:
      ret.type = INTERVAL;
      ret.interval = interval * itvd(rhs.interval);
      break;
    case AFFINE:
      ret.type = AFFINE;
      ret.affine_value = affine_t(interval) * rhs.affine_value;
      break;
    }
    break;
  case AFFINE:
    ret.type = AFFINE;
    switch(rhs.type)
    {
    case INTEGER:
      ret.affine_value = affine_value * affine_t(rhs.integer);
      break;
    case INTERVAL:
      ret.affine_value = affine_value * affine_t(rhs.interval);
      break;
    case AFFINE:
      ret.affine_value = affine_value * rhs.affine_value;
      break;
    }
    break;
  }
  return ret;
}


AffineMixedValue AffineMixedValue::operator/(const AffineMixedValue &rhs) const
{
  AffineMixedValue ret;

  switch(type)
  {
  case INTEGER:
    switch(rhs.type)
    {
    case INTEGER:
      if(integer > rhs.integer && integer % rhs.integer == 0)
      {
        ret.type = INTEGER;
        ret.integer = integer / rhs.integer;
      }
      else
      {
        ret.type = INTERVAL;
        ret.interval = itvd(integer) / itvd(rhs.integer);
      }
      break;      
    case INTERVAL:
      ret.type = INTERVAL;
      ret.interval = itvd(integer) / rhs.interval;
      break;
    case AFFINE:
      ret.type = AFFINE;
      ret.affine_value = itvd(integer) / rhs.affine_value;
      break;
    }
    break;

  case INTERVAL:
    switch(rhs.type)
    {
    case INTEGER:
      ret.type = INTERVAL;
      ret.interval = interval / itvd(rhs.integer);
      break;
    case INTERVAL:
      ret.type = INTERVAL;
      ret.interval = interval / itvd(rhs.interval);
      break;
    case AFFINE:
      ret.type = AFFINE;
      ret.affine_value = affine_t(interval) / rhs.affine_value;
      break;
    }
    break;
  case AFFINE:
    ret.type = AFFINE;
    switch(rhs.type)
    {
    case INTEGER:
      ret.affine_value = affine_value / affine_t(rhs.integer);
      break;
    case INTERVAL:
      ret.affine_value = affine_value / affine_t(rhs.interval);
      break;
    case AFFINE:
      ret.affine_value = affine_value / rhs.affine_value;
      break;
    }
    break;
  }
  
  return ret;
}


AffineMixedValue AffineMixedValue::operator^(const
  AffineMixedValue &rhs) const
{
  AffineMixedValue ret;
  kv::interval<double> itv = to_interval();
  double l = itv.lower(), u = itv.upper();
  switch(type)
  {
  case INTEGER:
    switch(rhs.type)
    {
    case INTEGER:
      ret = AffineMixedValue(::pow(integer, rhs.integer));
      break;
    case INTERVAL:
      ret.type = INTERVAL;
      ret.interval = pow(integer, rhs.interval);
      break;
    case AFFINE:
      ret.type = AFFINE;
      ret.affine_value = pow(integer, rhs.affine_value);
      break;
    }
    break;

  case INTERVAL:
    switch(rhs.type)
    {
    case INTEGER:
      ret.type = INTERVAL;
      ret.interval = pow(interval, rhs.integer);
      break;
    case INTERVAL:
      if(u >= 0 && l >= 0)
      {
        ret.interval = exp(rhs.interval * log(interval));
      }
      ret.type = INTERVAL;
      break;
    case AFFINE:
      ret.type = AFFINE;
      if(u >= 0 && l >= 0)
      {
        ret.affine_value = exp(rhs.to_affine() * log(interval));
      }
      else
      {
        throw ApproximateException("noninteger power function for interval including zero");
      }
      break;
    }
    break;
  case AFFINE:
    ret.type = AFFINE;
    switch(rhs.type)
    {
    case INTEGER:
      ret.affine_value = pow(affine_value, rhs.integer);
      break;
    case INTERVAL:
    case AFFINE:
      if(u >= 0 && l >= 0)
      {
        ret.affine_value = exp(rhs.to_affine() * log(to_affine()));
      }
      else
      {
        throw ApproximateException("noninteger power function for interval including zero");
      }
      break;
    }
    break;
  }
  return ret;
}



AffineMixedValue AffineMixedValue::operator-() const
{
  AffineMixedValue ret;
  ret.type = type;
  switch(type)
  {
  case INTEGER:
    ret.integer = -integer;
    break;
  case INTERVAL:
    ret.interval = -interval;
    break;
  case AFFINE:
    ret.affine_value = -affine_value;
    break;
  }
  return ret;
}



itvd AffineMixedValue::to_interval() const
{
  if(type == INTEGER)return itvd(integer);
  else if(type == INTERVAL) return interval;
  else return affine_value.get_interval();
}

affine_t AffineMixedValue::to_affine() const
{
  if(type == INTEGER)return affine_t(integer);
  else if(type == INTERVAL) return affine_t(interval);
  else return affine_value;
}


}
}
