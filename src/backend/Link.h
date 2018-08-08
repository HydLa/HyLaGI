#pragma once

#include <string>
#include <boost/bimap/bimap.hpp>
#include "Logger.h"

namespace hydla {
namespace backend {

class Link
{
  public:
  virtual ~Link() {}

  /// Type of Datas(used for receive)
  enum DataType
  {
    DT_FUNC,
    DT_STR,
    DT_INT,
    DT_SYM,
    DT_NONE
  };

  virtual void reset() {}

  virtual void put_symbol(const char *symbol) = 0;
  void put_symbol(const std::string& str) {put_symbol(str.c_str());}
  virtual void put_integer(int value) = 0;
  virtual void put_double(double value) = 0;
  virtual void put_number(const char *value) = 0;
  virtual void put_string(const char *value) = 0;
  virtual void put_string(const std::string &str) {put_string(str.c_str());}
  void put_number(const std::string &val) {put_number(val.c_str());}
  virtual void put_function(const char *name, int arg_cnt) = 0;
  void put_function(const std::string &name, int arg_cnt) {put_function(name.c_str(), arg_cnt);}
  virtual void put_parameter(const std::string& name, int diff_count, int id) = 0;

  virtual int get_integer() = 0;
  virtual void get_function(std::string& name, int& arg_cnt) = 0;
  virtual double get_double() = 0;
  virtual DataType get_type() = 0;
  virtual std::string get_symbol() = 0;
  virtual std::string get_string() = 0;
  virtual DataType get_next() = 0;

  virtual void pre_send() = 0;
  virtual void pre_receive() = 0;
  virtual void post_receive() = 0;

  virtual std::string backend_name() = 0;
  //std::string convert_function(const std::string& orig, bool hydla2back)
  //{
  //  bool converted;
  //  return convert_function(orig, hydla2back, converted);
  //}

  //std::string convert_function(const std::string& orig, bool hydla2back, bool &converted)
  //{
  //  converted = true;
  //  return orig;
  //}

  void put_converted_function(const std::string &orig, int arg_cnt)
  {
    put_function(orig, arg_cnt);
  }

  virtual std::string get_input_print() = 0;
  virtual std::string get_debug_print() = 0;
  virtual void check() = 0;

  bool trace = true;
  protected:
  // typedef boost::bimaps::bimap<std::string, std::string > function_map_t;
  // function_map_t function_map_;
};

} // namespace backend
} // namespace hydla
