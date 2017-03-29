#pragma once

#include <boost/program_options.hpp>

namespace hydla {

/**
 * 実行時に指定するオプションを処理＆保持するクラス
 */
class ProgramOptions {
public:
  ProgramOptions();
  ProgramOptions(const ProgramOptions&);
  ProgramOptions& operator=(const ProgramOptions&); 
  
  ~ProgramOptions();
  /**
   * 実行時に与えられたオプションを解析する
   */
  void parse(int argc, char* argv[]);

  void parse(std::string str);

  template<typename T>
  T get(const char name[]) const 
    {
      return vm_[name].as<T>();
    }

  /**
   * 与えられた名前のオプションが，実行時に指定された数を返す
   */
  int count(const char name[]) const {return vm_.count(name);}

  bool defaulted(const char name[]) const{return vm_.count(name) > 0 && vm_[name].defaulted();}

  void help_msg(std::ostream& os) const {visible_desc_.print(os);}

  void init_descriptions();

  boost::program_options::variables_map vm_;
  boost::program_options::options_description visible_desc_;
  boost::program_options::options_description cmdline_desc_;
};

} //namespace hydla
