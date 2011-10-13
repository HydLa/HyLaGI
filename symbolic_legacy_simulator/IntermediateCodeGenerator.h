#ifndef _INCLUDED_HYDLA_SYMBOLIC_LEGACY_SIMULATOR_INTERMEDIATE_CODE_GENERATOR_H_
#define _INCLUDED_HYDLA_SYMBOLIC_LEGACY_SIMULATOR_INTERMEDIATE_CODE_GENERATOR_H_

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "TreeVisitor.h"
#include "ParseTree.h"

using namespace hydla::parse_tree;

namespace hydla {
namespace symbolic_legacy_simulator {

class IntermediateCodeGenerator : 
    public TreeVisitor {
public:
  typedef boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree_sptr;

  IntermediateCodeGenerator();

  virtual ~IntermediateCodeGenerator();

  /**
   * ’†ŠÔƒR[ƒh‚ğì¬‚·‚é
   */
  std::string create(parse_tree_sptr parse_tree, std::string max_time);

  // ŒÄ‚Ño‚µ
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node);

  virtual void visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node);         

  // §–ñ®
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Constraint> node);            

  // Ask§–ñ
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);                   

  // Tell§–ñ
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);                  

  // ”äŠr‰‰Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Equal> node);                 

  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node);               

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Less> node);                  

  virtual void visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node);             

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Greater> node);               

  virtual void visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node);          

  // ˜_—‰‰Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);            

  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalOr> node);             
 
  // Zp“ñ€‰‰Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Plus> node);                  

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Subtract> node);              

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Times> node);                 

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Divide> node);                
  
  // Zp’P€‰‰Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);              

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);              
  
  // §–ñŠK‘w’è‹`‰‰Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Weaker> node);                
  
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parallel> node);              

  // ‘Š‰‰Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Always> node);                
  
  // ”÷•ª
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);          

  // ¶‹ÉŒÀ
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);              
  
  // •Ï”
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);              

  // ”š
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);                

private:
  parse_tree_sptr parse_tree_;

  int         in_guard_;
  std::string inter_str_;  
};

} // namespace symbolic_legacy_simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_SYMBOLIC_LEGACY_SIMULATOR_INTERMEDIATE_CODE_GENERATOR_H_
