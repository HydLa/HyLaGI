#ifndef _INCLUDED_HYDLA_BACKEND_SYMBOLIC_LINK_H_
#define _INCLUDED_HYDLA_BACKEND_SYMBOLIC_LINK_H_

#include <string>

namespace hydla{
namespace backend{

class SymbolicLink
{
  public:
  virtual ~SymbolicLink(){}

  /// Type of Datas(used for receive)
  enum DataType
  {
    DT_FUNC,
    DT_STR,
    DT_INT,
    DT_SYM
  };



  virtual void put_symbol(const char *symbol) = 0;
  void put_symbol(const std::string& str){put_symbol(str.c_str());}
  virtual void put_integer(const int &value) = 0;
  virtual void put_number(const char *value) = 0;
  void put_number(const std::string &val){put_number(val.c_str());}
  virtual void put_function(const char *name, const int &arg_cnt) = 0;
  void put_function(const std::string &name, const int &arg_cnt){put_function(name.c_str(), arg_cnt);}

  virtual int get_integer() = 0;
  virtual int get_function(std::string& name, int& arg_cnt) = 0;
  virtual int get_arg_count(int &arg_cnt) = 0;
  virtual int get_type(DataType& type) = 0;
  virtual int get_symbol(std::string& name) = 0;
  virtual int get_string(std::string& str) = 0;
  virtual int get_next(DataType& type) = 0;
  virtual int get_next() = 0;

  virtual void pre_receive() = 0;
  virtual void post_receive() = 0;

  virtual std::string backend_name() = 0;
  virtual bool convert(const std::string& orig, const int& orig_cnt, const bool &hydla2back, std::string& ret, int& ret_cnt) = 0;
  virtual std::string get_input_print() = 0;
  virtual std::string get_debug_print() = 0;
  virtual void check() = 0;
};

} // namespace backend
} // namespace hydla

#endif // include guard
