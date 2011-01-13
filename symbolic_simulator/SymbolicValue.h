#ifndef _SYMBOLIC_VALUE_H_
#define _SYMBOLIC_VALUE_H_

#include <string>
#include <vector>

namespace hydla {
namespace symbolic_simulator {

typedef std::string symbolic_element_value_t;   //simbolic_value_t�̍\���v�f�DRelation�Ƒg�ނ���

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
    symbolic_element_value_t value;
  }Element;
  
  SymbolicValue();
  
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
   * �Ƃ肠�����ŏ���element_value��Ԃ�
   */
  symbolic_element_value_t get_first_value() const;
  
  /**
   * �Ƃ肠�����ŏ���relation��Ԃ�
   */
  Relation get_first_relation() const;

  /**
   * �Ƃ肠�����ŏ���relation�̋L����Ԃ�
   */
  std::string get_first_symbol() const;
  
  /**
   * �Ƃ肠������������Z�b�g
   */
  void set(std::string);

  /**
   * �V���Ȃ��̂��Z�b�g
   */
  void set(const std::vector<std::vector<Element> > &vec);
  
  /**
   * add�̑Ώۂ�����or�ɁD
   */
  void go_next_or();

  /**
   * �V���ȗv�f��ǉ�
   */
  void add(const Element &ele);

  /**
   * �f�[�^���_���v����
   */
  std::ostream& dump(std::ostream& s) const;
  
  
  std::vector<std::vector< Element > > value_; //�l�Ƃ̊֌W�̗�̗�iDNF�`���D�ȂłȂ��ꂽ���̂��ɂłȂ��j

  private:
  

  std::vector <Element> *current_or_;
  std::string visit_all(const std::vector<Element> &vec, const std::string &delimiter) const;
  //�z��𑖍����ăf���~�^�ŘA�����ďo��
  
  std::string str_; // ������i�C�ӂ̎���������j
};

bool operator<(const SymbolicValue& lhs, const SymbolicValue& rhs);

std::ostream& operator<<(std::ostream& s, const SymbolicValue & v);


} // namespace simulator
} // namespace hydla

#endif // _SYMBOLIC_VALUE_H_
