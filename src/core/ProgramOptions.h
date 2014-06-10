#pragma once

#include <boost/program_options.hpp>

namespace hydla {

/**
 * singleton and noncopyable（1つしかインスタンスを生成できない）
 * 実行時に指定するオプションを処理＆保持するクラス
 */
class ProgramOptions {
public:
    ~ProgramOptions();
    /**
     * 実行時に与えられたオプションを解析する
     */
    void parse(int argc, char* argv[]);

    template<typename T>
    T get(const char name[]) const 
    {
        return vm_[name].as<T>();
    }

    /**
     * 与えられた名前のオプションが，実行時に指定された数を返す
     */
    int count(const char name[]) const {return vm_.count(name);}

    void help_msg(std::ostream& os) const {visible_desc_.print(os);}

    /**
     * インスタンスの生成
     * クラス全体で共通
     */
    static ProgramOptions& instance();

private:
    ProgramOptions();
    ProgramOptions(const ProgramOptions&);
    ProgramOptions& operator=(const ProgramOptions&); 

    void init_descriptions();

    boost::program_options::variables_map vm_;
    boost::program_options::options_description visible_desc_;
    boost::program_options::options_description cmdline_desc_;
};

} //namespace hydla
