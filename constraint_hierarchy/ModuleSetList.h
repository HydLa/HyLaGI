#ifndef _INCLUDED_HTDLA_CH_MODULE_SET_LIST_H_
#define _INCLUDED_HTDLA_CH_MODULE_SET_LIST_H_

#include <vector>

#include "ModuleSet.h"
#include "ModuleSetContainer.h"

namespace hydla {
namespace ch {

/**
 * ����⃂�W���[���W���̏W�������X�g�\���ŕ\���N���X
 * ����⃂�W���[���W���̏W���𓱏o����A���S���Y����
 * �u����K�w�ɂ��n�C�u���b�h�V�X�e���̃��f�����O��@(JSSST2009)�v
 * �Q�Ƃ̂���
 *
 */
class ModuleSetList : public ModuleSetContainer {
public:
  typedef std::vector<module_set_sptr> module_set_list_t;

  ModuleSetList();
  ModuleSetList(module_set_sptr m);
  virtual ~ModuleSetList();

  /**
   * ���񍇐��Ƃ��ďW������������
   */
  void add_parallel(ModuleSetList& parallel_module_set_list);

  /**
   * ���񍇐��Ƃ��ďW������������irequired���񈵂��j
   */
  void add_required_parallel(ModuleSetList& parallel_module_set_list);
  
  /**
   * �㍇���Ƃ��ďW������������
   */
  void add_weak(ModuleSetList& weak_module_set_list);

  /**
   * �W���̏W��(���̃N���X)�̖��O
   */ 
  std::string get_name() const;

  /**
   * �W���̏W���̃_���v
   */
  virtual std::ostream& dump(std::ostream& s) const;

  /**
   * ���O�\���ɂ��_���v
   */
  std::ostream& dump_node_names(std::ostream& s) const;
  
  /**
   * �c���[�\���ɂ��_���v
   */
  std::ostream& dump_node_trees(std::ostream& s) const;

  /**
   * �ɑ�Ȑ��񃂃W���[���W���𖳖����Ȃ��̂�������܂ł��߂�
   */
  virtual bool dispatch(boost::function<bool (hydla::ch::module_set_sptr)> callback_func, 
                        int threads = 1);

private:
  module_set_list_t module_set_list_;
};

} // namespace ch
} // namespace hydla

#endif //_INCLUDED_HTDLA_CH_MODULE_SET_LIST_H_
