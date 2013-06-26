#ifndef _INCLUDED_HTDLA_PROGRAM_OPTIONS_H_
#define _INCLUDED_HTDLA_PROGRAM_OPTIONS_H_

#include <boost/program_options.hpp>
#include <ostream>

namespace hydla {

/**
 * singleton and noncopyable�i1�����C���X�^���X�𐶐��ł��Ȃ��j
 * ���s���Ɏw�肷��I�v�V�������������ێ�����N���X
 */
class ProgramOptions {
public:
    ~ProgramOptions();
    /**
     * ���s���ɗ^����ꂽ�I�v�V��������͂���
     */
    void parse(int argc, char* argv[]);

    template<typename T>
    T get(const char name[]) const 
    {
        return vm_[name].as<T>();
    }

    /**
     * �^����ꂽ���O�̃I�v�V�������C���s���Ɏw�肳�ꂽ����Ԃ�
     */
    int count(const char name[]) const {return vm_.count(name);}

    void help_msg(std::ostream& os) const {visible_desc_.print(os);}

    /**
     * �C���X�^���X�̐���
     * �N���X�S�̂ŋ���
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

#endif //_INCLUDED_HTDLA_PROGRAM_OPTIONS_H_

