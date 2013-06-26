#ifndef _SIMULATOR_VALUE_H_
#define _SIMULATOR_VALUE_H_

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "ValueVisitor.h"
#include "Node.h"

#include <boost/operators.hpp>

namespace hydla {
namespace simulator {

class ValueVisitor;

class Value:
  public boost::additive<Value>
{  
  public:
  
  virtual ~Value(){}
  
  /**
   * ����`�l���ǂ���
   */
  virtual bool undefined() const = 0;
  
  /**
   * �������g�̃N���[����V���ɍ쐬���C���̃|�C���^��Ԃ�
   * �Ԃ�l�̃������͌Ăяo�������ŉ������悤�ɒ��ӂ���D
   */
  virtual Value* clone() const = 0;

  /**
   * ������\�����擾����
   */
  virtual std::string get_string() const = 0;

  /**
   * Node�̌`���ɂ������̂��擾����
   */
  virtual hydla::parse_tree::node_sptr get_node() const = 0;
  
  virtual void accept(ValueVisitor &) = 0;
  
  /**
   * Value���m�̉��Z
   */
  Value& operator+=(const Value& rhs);

  /**
   * Value���m�̌��Z
   */
  Value& operator-=(const Value& rhs);
  
  /**
   * �f�[�^���_���v����
   */
  std::ostream& dump(std::ostream& s) const{
    if(undefined()) s << "UNDEF";
    else s << get_string();
    return s;
  }
};

bool operator<(const Value& lhs, const Value& rhs);

std::ostream& operator<<(std::ostream& s, const Value & v);


} // namespace simulator
} // namespace hydla

#endif // _SIMULATOR_VALUE_H_
