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
  
  typedef struct Element{
    Relation relation;
    symbolic_element_value_t value;
    Element(const symbolic_element_value_t& val, const Relation& rel);
    std::string get_symbol()const;
    std::string get_value()const;
  }element_t;
  

  typedef std::vector< Element > and_vector;
  typedef std::vector< and_vector > or_vector;
  typedef and_vector::iterator         and_iterator;
  typedef and_vector::const_iterator   and_const_iterator;
  typedef or_vector::iterator         or_iterator;
  typedef or_vector::const_iterator   or_const_iterator;

  SymbolicValue();
  
  static const std::string relation_symbol_[RELATION_NUMBER];

  /**
   * ����`�l���ǂ���
   */
  bool is_undefined() const;
  
  /**
   * �l���i��ʓI�Ɂj��ӂɒ�܂邩
   */
  bool is_unique() const;

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
   * �V���Ȃ��̂��Z�b�g
   */
  void set(const std::vector<std::vector<Element> > &vec);
  
  /**
   * �V���ȗv�f���P�Z�b�g����i����܂ł̂��͎̂̂Ă�j
   */
  void set(const Element &ele);

  /**
   * �v�f��S�Ď̂āC����������
   */
  void clear();
  
  /**
   * add�̑Ώۂ�����or�ɁD
   */
  void go_next_or();

  /**
   * �V���ȗv�f��ǉ�
   */
  void add(const Element &ele);
  
  or_const_iterator or_begin() const {return value_.begin();}
  or_iterator or_begin() {return value_.begin();}
  or_const_iterator or_end() const {return value_.end();}
  or_iterator or_end() {return value_.end();}

  
  /**
   * �f�[�^���_���v����
   */
  std::ostream& dump(std::ostream& s) const;
  
  
  or_vector value_; //�l�Ƃ̊֌W�̗�̗�iDNF�`���D�ȂłȂ��ꂽ���̂��ɂłȂ��j

  private:
  

  std::vector <Element> *current_or_;
  std::string visit_all(const std::vector<Element> &vec, const std::string &delimiter) const;
  //�z��𑖍����ăf���~�^�ŘA�����ďo��
  
};

bool operator<(const SymbolicValue& lhs, const SymbolicValue& rhs);

std::ostream& operator<<(std::ostream& s, const SymbolicValue & v);


} // namespace simulator
} // namespace hydla

#endif // _SYMBOLIC_VALUE_H_
