#include "ConstraintStore.h"

namespace hydla {
namespace bp_simulator {

  ConstraintStore::ConstraintStore(bool debug_mode) :
  debug_mode_(debug_mode)
  {}

  ConstraintStore::~ConstraintStore()
  {
    std::set<rp_constraint>::iterator it = this->exprs_.begin();
    while(it != this->exprs_.end()) {
      rp_constraint c = *it;
      rp_constraint_destroy(&c);
      it++;
    }
  }

  // TODO: ���̂�������
  // prev_var <- var �͂����ōs��
  void ConstraintStore::build(const variable_map_t& variable_map)
  {
  }

  /**
   * �X�g�A�̃R�s�[��Ԃ�
   * �R�s�[�łȂ��Ƒ���rp_**�\���̂ɏ����free����Ă��܂��\��������
   */
  std::set<rp_constraint> ConstraintStore::get_store_exprs_copy() const
  {
    std::set<rp_constraint> ans;
    std::set<rp_constraint>::const_iterator it = this->exprs_.begin();
    while(it != this->exprs_.end()) {
      rp_constraint c;
      // TODO: �Ԉ���Ă���\��
      rp_constraint_clone(&c, (*it));
      it++;
    }
    return ans;
  }

  /**
   * �X�g�A�ɐ����ǉ�����
   * @param c ����
   * @param vars ������̕ϐ��\
   */
  void ConstraintStore::add_constraint(rp_constraint c, const var_name_map_t& vars)
  {
    this->exprs_.insert(c);
    this->vars_.insert(vars.begin(), vars.end());
  }

  /**
   * �X�g�A�ɐ����ǉ�����
   * @param start start����end�ɂ��鐧���ǉ�����
   * @param end start����end�ɂ��鐧���ǉ�����
   * @param vars ������̕ϐ��\
   */
  void ConstraintStore::add_constraint(std::set<rp_constraint>::iterator start,
                                       std::set<rp_constraint>::iterator end,
                                       const var_name_map_t& vars)
  {
    this->exprs_.insert(start, end);
    this->vars_.insert(vars.begin(), vars.end());
  }

  /**
   * �X�g�A��\������
   * TODO: �_���v�`���ɂ���H
   */
  void ConstraintStore::display(const int digits) const
  {
    rp_vector_variable vec;
    rp_vector_variable_create(&vec);
    var_name_map_t::right_const_iterator it;
    for(it=this->vars_.right.begin(); it!=this->vars_.right.end(); it++){
      rp_variable v;
      rp_variable_create(&v, ((it->second).c_str()));
      rp_vector_insert(vec, v);
    }
    std::set<rp_constraint>::const_iterator ctr_it = this->exprs_.begin();
    while(ctr_it != this->exprs_.end()){
      rp_constraint_display(stdout, *ctr_it, vec, digits);
      std::cout << "\n";
      ctr_it++;
    }
    std::cout << "\n";
    rp_vector_destroy(&vec);
  }
} // namespace bp_simulator
} // namespace hydla
