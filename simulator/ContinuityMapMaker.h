#ifndef _INCLUDED_HYDLA_CONTINUITY_MAP_MAKER_H_
#define _INCLUDED_HYDLA_CONTINUITY_MAP_MAKER_H_

#include <vector>
#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "DefaultTreeVisitor.h"

namespace hydla {
namespace simulator {

typedef std::map<std::string, int>                               continuity_map_t;

/**
 * Tell�m�[�h�𒲂ׁC�A�����̍����ƂȂ�ϐ��i�Ƃ��̔����l�j�̏o���𐔂���N���X
 */
class ContinuityMapMaker : public parse_tree::DefaultTreeVisitor {
public:
  
  ContinuityMapMaker();

  virtual ~ContinuityMapMaker();
  
  /** 
   * 
   */
  void visit_node(boost::shared_ptr<parse_tree::Node> node, const bool& in_IP, const bool& negative)
  {
    in_interval_ = in_IP;
    negative_ = negative;
    differential_count_ = 0;
    accept(node);
  }

  /**
   * ������Ԃɖ߂�
   */
  void reset()
  {
    variables_.clear();
  }
  
  void set_continuity_map(const continuity_map_t& map){
    variables_ = map;
  }
  
  continuity_map_t get_continuity_map(){
    return variables_;
  }

  // Ask����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // ����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // ���Ɍ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);


  // �ϐ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

private:

  
  // �W�߂����񒆂ɏo������ϐ��Ƃ��̔����񐔂̃}�b�v
  continuity_map_t  variables_;
  int differential_count_;
  bool in_interval_;
  // ������ǉ����邩�ǂ���
  bool negative_;
};


std::ostream& operator<<(std::ostream& s, const continuity_map_t& continuity_map);

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_CONTINUITY_MAP_MAKER_H_
