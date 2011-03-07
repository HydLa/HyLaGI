#ifndef _SYMBOLIC_VALUE_H_
#define _SYMBOLIC_VALUE_H_

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "Node.h"

#include <boost/operators.hpp>

namespace hydla {
namespace symbolic_simulator {

class SymbolicValue:
    public boost::additive<SymbolicValue>
{

  typedef hydla::parse_tree::node_sptr node_sptr;
  node_sptr node_;  //�l��node_sptr
  
  bool is_unique_;  //�l����ӂ��ǂ����������ϐ��D�Ƃ肠�����O������ݒ肷��

  public:


  SymbolicValue(){}
  //�P�Ȃ镶����͐��l�ƌ��Ȃ��Ď󂯎��
  SymbolicValue(const std::string &str){node_.reset(new hydla::parse_tree::Number(str));}
  SymbolicValue(const node_sptr & node){node_ = node;}
  
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
   * �V���ȃm�[�h���Z�b�g
   */
  void set(const node_sptr&);

  /**
   * SymbolicValue���m�̉��Z. additive���p�����Ă���̂ł����+���g����悤�ɂȂ�
   */
  SymbolicValue& operator+=(const SymbolicValue& rhs);
  

  /**
   * SymbolicValue���m�̌��Z. additive���p�����Ă���̂ł����-���g����悤�ɂȂ�
   */
  SymbolicValue& operator-=(const SymbolicValue& rhs);
  
  /**
   * �f�[�^���_���v����
   */
  std::ostream& dump(std::ostream& s) const;
};

bool operator<(const SymbolicValue& lhs, const SymbolicValue& rhs);

std::ostream& operator<<(std::ostream& s, const SymbolicValue & v);


} // namespace simulator
} // namespace hydla

#endif // _SYMBOLIC_VALUE_H_
