#ifndef _SYMBOLIC_VALUE_H_
#define _SYMBOLIC_VALUE_H_

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "Node.h"
#include "../simulator/Value.h"

#include <boost/operators.hpp>

namespace hydla {
namespace simulator {
namespace symbolic {

class SymbolicValue: public hydla::simulator::Value
{  
  public:

  typedef hydla::parse_tree::node_sptr node_sptr;

  SymbolicValue();
  
  virtual ~SymbolicValue(){}
  
  virtual hydla::simulator::Value* clone() const
    {return new SymbolicValue(node_->clone());}
  
  /**
   * �P�Ȃ镶����͐��l�ƌ��Ȃ��Ď󂯎��
   */
  SymbolicValue(const std::string &str);
  
  /**
   * �n���ꂽ�m�[�h���Q�Ƃ���SymbolicValue�����
   */
  SymbolicValue(const node_sptr & node);
  
  /**
   * ����`�l���ǂ���
   */
  bool is_undefined() const;

  /**
   * ������\�����擾����
   */
  std::string get_string() const;
  /**
   * �m�[�h���擾����
   */
  node_sptr get_node() const;
  
  /**
   * �m�[�h��ݒ肷��
   */
  void set_node(node_sptr);
  
  /**
   * �V���ȃm�[�h���Z�b�g
   */
  void set(const node_sptr&);

  void accept(hydla::simulator::ValueVisitor& v){v.visit(*this);}
  
  private:
  
  node_sptr node_;  //�l��node_sptr
};

}
} // namespace simulator
} // namespace hydla

#endif // _SYMBOLIC_VALUE_H_