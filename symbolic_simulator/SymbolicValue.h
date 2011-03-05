#ifndef _SYMBOLIC_VALUE_H_
#define _SYMBOLIC_VALUE_H_

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "Node.h"

namespace hydla {
namespace symbolic_simulator {

struct SymbolicValue {

  typedef hydla::parse_tree::node_sptr node_sptr;


  SymbolicValue(){}
  SymbolicValue(const std::string &str){str_=str;}
  
  /**
   * ����`�l���ǂ���
   */
  bool is_undefined() const;
  
  /**
   * �l���i��ʓI�Ɂj��ӂɒ�܂邩
   */
  bool is_unique() const;
  
  void set_unique(const bool &u){is_unique_=u;}

  /**
   * ������\�����擾����
   */
  std::string get_string() const;
  /**
   * �m�[�h���擾����
   */
  node_sptr get_node() const;
  
  /**
   * �V���ȕ�������Z�b�g
   */
  void set(const std::string&);
  /**
   * �V���ȃm�[�h���Z�b�g
   */
  void set(const node_sptr&);

  
  /**
   * �f�[�^���_���v����
   */
  std::ostream& dump(std::ostream& s) const;

  private:
  std::string str_;  //�Ƃ肠�����l�͕������
  node_sptr node_;  //�l��node_sptr�ɂ��Ă݂���
  
  bool is_unique_;  //�l����ӂ��ǂ����������ϐ��D�Ƃ肠�����O������ݒ肷��
};

bool operator<(const SymbolicValue& lhs, const SymbolicValue& rhs);

std::ostream& operator<<(std::ostream& s, const SymbolicValue & v);


} // namespace simulator
} // namespace hydla

#endif // _SYMBOLIC_VALUE_H_
