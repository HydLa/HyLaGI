#ifndef _INCLUDED_HYDLA_PARAMETER_REPLACER_H_
#define _INCLUDED_HYDLA_PARAMETER_REPLACER_H_

#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "DefaultTreeVisitor.h"
#include "ValueVisitor.h"
#include "PhaseResult.h"

namespace hydla {
namespace simulator {

/**
 * VCS�ŉ���ID = -1�Ƃ����p�����[�^���C�K�؂Ȃ��̂ɒu��������i�u��������ƌ�����ID�ς��邾���j
 * TODO: ���̃N���X���̂��{���Ȃ�s�v�ȋC������D
 * �����̃c���[���ۂɂǂ��Ńp�����[�^���K�v�ɂȂ邩�͌����I�ɕ����肻��������C�ǂ��ɂ��������D
 * ������ɂ���CID = -1 ���}�W�b�N�i���o�[�ɂȂ��Ă��܂��Ă���D
 */
class ParameterReplacer : public parse_tree::DefaultTreeVisitor, hydla::simulator::ValueVisitor{
public:

  typedef std::map< std::pair<std::string, int>, int > parameter_id_map_t;

  ParameterReplacer();

  virtual ~ParameterReplacer();
  
  void add_mapping(const std::string& name, const int& derivative_count, const int& id);
  
  virtual void visit(hydla::simulator::symbolic::SymbolicValue&);
  
  void replace_value(value_t& val);
  
  // �L���萔
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parameter> node);
  
  private:
  parameter_id_map_t parameter_id_map_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_PARAMETER_REPLACER_H_
