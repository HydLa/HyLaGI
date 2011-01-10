#ifndef _SYMBOLIC_VALUE_H_
#define _SYMBOLIC_VALUE_H_

#include <string>
#include <vector>

namespace hydla {
namespace symbolic_simulator {

struct SymbolicValue {

  typedef enum{
    EQUAL,
    NOT_EQUAL,
    LESS_EQUAL,
    LESS,
    GREATER_EQUAL,
    GREATER,
    RELATION_NUMBER
  }Relation;
  
  typedef struct{
    Relation relation;
    std::string value;
  }Element;
  
  static const std::string relation_symbol_[RELATION_NUMBER];
  /**
   * ����`�l���ǂ���
   */
  bool is_undefined() const;

  /**
   * ������\�����擾����
   */
  std::string get_string() const;
  
  /**
   * �Ƃ肠������������Z�b�g
   */
  void set(std::string);

  /**
   * �f�[�^���_���v����
   */
  std::ostream& dump(std::ostream& s) const;
  
  void add(const std::vector<Element> &vec);
  
  void set(const std::vector<std::vector<Element> > &vec);
  

  private:
  

  std::string visit_all(const std::vector<Element> &vec, const std::string &delimiter) const;
  //������z��𑖍����ăf���~�^�ŘA�����ďo��
  
  std::vector<std::vector< Element > > value_; //�l�Ƃ̊֌W�̗�̗�i�������DNF�`���ɂȂ��Ă���D�ȂłȂ��ꂽ���̂��ɂłȂ��j
  std::string str_; // ������i�C�ӂ̎���������j
};

bool operator<(const SymbolicValue& lhs, const SymbolicValue& rhs);

std::ostream& operator<<(std::ostream& s, const SymbolicValue & v);


} // namespace simulator
} // namespace hydla

#endif // _SYMBOLIC_VALUE_H_
