#ifndef _INCLUDED_HYDLA_BACKEND_LINK_H_
#define _INCLUDED_HYDLA_BACKEND_LINK_H_

#include <string>
#include <boost/bimap/bimap.hpp>

namespace hydla{
namespace backend{

class Link
{
  public:
  virtual ~Link(){}

  /// Type of Datas(used for receive)
  enum DataType
  {
    DT_FUNC,
    DT_STR,
    DT_INT,
    DT_SYM,
    DT_NONE
  };



  virtual void put_symbol(const char *symbol) = 0;
  void put_symbol(const std::string& str){put_symbol(str.c_str());}
  virtual void put_integer(int value) = 0;
  virtual void put_number(const char *value) = 0;
  virtual void put_string(const char *value) = 0;
  virtual void put_string(const std::string &str){put_string(str.c_str());}
  void put_number(const std::string &val){put_number(val.c_str());}
  virtual void put_function(const char *name, int arg_cnt) = 0;
  void put_function(const std::string &name, int arg_cnt){put_function(name.c_str(), arg_cnt);}

  virtual int get_integer() = 0;
  virtual void get_function(std::string& name, int& arg_cnt) = 0;
  virtual DataType get_type() = 0;
  virtual std::string get_symbol() = 0;
  virtual std::string get_string() = 0;
  virtual DataType get_next() = 0;

  virtual void pre_send() = 0;
  virtual void pre_receive() = 0;
  virtual void post_receive() = 0;

  virtual std::string backend_name() = 0;
  inline bool convert(const std::string& orig, bool hydla2back, std::string& ret)
  {
    if(hydla2back){
        function_map_t::left_iterator it = function_map_.left.find(orig);
        if(it != function_map_.left.end()){
            ret = it->second;
            return true;
          }
        else{
            return false;
          }
      }else{
        function_map_t::right_iterator it = function_map_.right.find(orig);
        if(it != function_map_.right.end()){
            ret = it->second;
            return true;
          }
        else{
            return false;
          }  
      }
  }

  virtual std::string get_input_print() = 0;
  virtual std::string get_debug_print() = 0;
  virtual void check() = 0;
 protected:
  typedef boost::bimaps::bimap<std::string, std::string > function_map_t;
  function_map_t function_map_;
};

} // namespace backend
} // namespace hydla

#endif // include guard
