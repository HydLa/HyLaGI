#ifndef _INCLUDED_HTDLA_PROGRAM_OPTIONS_H_
#define _INCLUDED_HTDLA_PROGRAM_OPTIONS_H_

#include <boost/program_options.hpp>
#include <ostream>

namespace hydla {

// singleton and noncopyable
class ProgramOptions {
public:
    ~ProgramOptions();

    void parse(int argc, char* argv[]);

    template<typename T>
    T get(const char name[]) const 
    {
        return vm_[name].as<T>();
    }

    int count(const char name[]) const {return vm_.count(name);}

    void help_msg(std::ostream& os) const {visible_desc_.print(os);}

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

#endif //_INCLUDED_HTDLA_PROGRAM_OPTIONS_H_

