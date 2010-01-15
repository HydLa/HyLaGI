#include "ConsistencyCheckerInterval.h"

#define BP_USERVAR_PREFIX "userVar"

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace bp_simulator {

  ConsistencyCheckerInterval::ConsistencyCheckerInterval(){}

  ConsistencyCheckerInterval::~ConsistencyCheckerInterval(){}

  bool ConsistencyCheckerInterval::is_consistent(tells_t& collected_tells,
                                                 ConstraintStoreInterval& constraint_store)
  {
    // ƒXƒgƒA‚Ì•Ï”î•ñ‚ğƒRƒs[
    this->vars_ = constraint_store.get_store_vars();
    // tell‚ğÏ•ªCrp_constraintW‡‚ğ¶¬
    this->send_expression_str = "DSolve[{";
    tells_t::iterator it;
    for(it=collected_tells.begin(); it!=collected_tells.end(); it++) {
      this->accept(*it);
    }
    this->send_expression_str += "}";

    return true;
  }

  // Tell§–ñ
  void ConsistencyCheckerInterval::visit(boost::shared_ptr<Tell> node)
  {
    this->accept(node->get_child());
  }

  // ”äŠr‰‰Zq
  void ConsistencyCheckerInterval::visit(boost::shared_ptr<Equal> node)
  {
    this->accept(node->get_lhs());
    this->send_expression_str += " == ";
    this->accept(node->get_rhs());
  }
  void ConsistencyCheckerInterval::visit(boost::shared_ptr<UnEqual> node)
  {
    this->accept(node->get_lhs());
    this->send_expression_str += " != ";
    this->accept(node->get_rhs());
  }
  void ConsistencyCheckerInterval::visit(boost::shared_ptr<Less> node)
  {
    this->accept(node->get_lhs());
    this->send_expression_str += " < ";
    this->accept(node->get_rhs());
  }
  void ConsistencyCheckerInterval::visit(boost::shared_ptr<LessEqual> node)
  {
    this->accept(node->get_lhs());
    this->send_expression_str += " <= ";
    this->accept(node->get_rhs());
  }
  void ConsistencyCheckerInterval::visit(boost::shared_ptr<Greater> node)
  {
    this->accept(node->get_lhs());
    this->send_expression_str += " > ";
    this->accept(node->get_rhs());
  }
  void ConsistencyCheckerInterval::visit(boost::shared_ptr<GreaterEqual> node)
  {
    this->accept(node->get_lhs());
    this->send_expression_str += " >= ";
    this->accept(node->get_rhs());
}

  // Zp“ñ€‰‰Zq
  void ConsistencyCheckerInterval::visit(boost::shared_ptr<Plus> node)
  {
    this->send_expression_str += "(";
    this->accept(node->get_lhs());
    this->send_expression_str += " + ";
    this->accept(node->get_rhs());
    this->send_expression_str += ")";
  }
  void ConsistencyCheckerInterval::visit(boost::shared_ptr<Subtract> node)
  {
    this->send_expression_str += "(";
    this->accept(node->get_lhs());
    this->send_expression_str += " - ";
    this->accept(node->get_rhs());
    this->send_expression_str += ")";
  }
  void ConsistencyCheckerInterval::visit(boost::shared_ptr<Times> node)
  {
    this->send_expression_str += "(";
    this->accept(node->get_lhs());
    this->send_expression_str += " * ";
    this->accept(node->get_rhs());
    this->send_expression_str += ")";
  }
  void ConsistencyCheckerInterval::visit(boost::shared_ptr<Divide> node)
  {
    this->send_expression_str += "(";
    this->accept(node->get_lhs());
    this->send_expression_str += " / ";
    this->accept(node->get_rhs());
    this->send_expression_str += ")";
  }

  // Zp’P€‰‰Zq
  void ConsistencyCheckerInterval::visit(boost::shared_ptr<Negative> node)
  {
    this->send_expression_str += "-";
    this->accept(node->get_child());
  }
  void ConsistencyCheckerInterval::visit(boost::shared_ptr<Positive> node)
  {
    this->send_expression_str += "+";
    this->accept(node->get_child());
  }

  // ”÷•ª
  void ConsistencyCheckerInterval::visit(boost::shared_ptr<Differential> node)
  {
    this->derivative_count_++;
    this->accept(node->get_child());
    this->send_expression_str += "'";
    this->derivative_count_--;
  }

  // ¶‹ÉŒÀ
  // ¶‹ÉŒÀ‚ÍIP‚É‚¨‚¢‚Ä‰Šú’l‚Æ“¯‚¶
  void ConsistencyCheckerInterval::visit(boost::shared_ptr<Previous> node)
  {
    this->in_prev_ = true;
    this->accept(node->get_child());
    this->in_prev_ = false;
  }

  // •Ï”
  void ConsistencyCheckerInterval::visit(boost::shared_ptr<Variable> node)
  {
    // BP_USERVAR_PREFIX + node->get_name();
  }

  // ”š
  void ConsistencyCheckerInterval::visit(boost::shared_ptr<Number> node){}
}
}
