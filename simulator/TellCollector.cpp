#include "TellCollector.h"
#include "Logger.h"
#include "TreeInfixPrinter.h"

#include <assert.h>
#include <iostream>

namespace hydla {
namespace simulator {
using namespace hydla::logger;


namespace {
struct NodeDumper {
      
  template<typename T>
  NodeDumper(T it, T end) 
  {
    for(; it!=end; ++it) {
      ss << hydla::parse_tree::TreeInfixPrinter().get_infix_string(*it) << "\n";
    }
  }

  friend std::ostream& operator<<(std::ostream& s, const NodeDumper& nd)
  {
    s << nd.ss.str();
    return s;
  }

  std::stringstream ss;
};
}

TellCollector::TellCollector(const module_set_sptr& module_set) :
  module_set_(module_set)
{}

TellCollector::~TellCollector()
{}

void TellCollector::collected_tells(tells_t* collected_tells)
{
  collected_tells->clear();
  collected_tells->reserve(collected_tells_.size());
  collected_tells->insert(collected_tells->end(), 
    collected_tells_.begin(), collected_tells_.end());
}

void TellCollector::collect(tells_t*                 tells,
                            const expanded_always_t* expanded_always,                   
                            const positive_asks_t*   positive_asks)
{
  HYDLA_LOGGER_CLOSURE("#*** Begin TellCollector::collect ***\n");
  assert(expanded_always);
  assert(tells);
  assert(positive_asks);

  tells->clear();
  tells_          = tells;
  positive_asks_  = positive_asks;
  visited_always_.clear();

  // ModuleSet�̃m�[�h�̒T��
  in_positive_ask_    = false;
  in_negative_ask_    = false;
  in_expanded_always_ = false;
  module_set_->dispatch(this);

  // �W�J�ς�always�m�[�h�̒T��
  in_positive_ask_    = false;
  in_negative_ask_    = false;
  in_expanded_always_ = true;
  expanded_always_t::const_iterator it  = expanded_always->begin();
  expanded_always_t::const_iterator end = expanded_always->end();
  for(; it!=end; ++it) {
    // �̗p���Ă��郂�W���[���W�����ɓ����Ă��邩�ǂ���
    if(visited_always_.find(*it) != visited_always_.end()) {
      accept((*it)->get_child());
    }
  }

  HYDLA_LOGGER_CLOSURE(
    "#*** tell collector ***\n", 
    "--- collected tells ---\n", 
    NodeDumper(tells->begin(), tells->end()));
    
  HYDLA_LOGGER_CLOSURE("#*** End TellCollector::collect ***\n");
}

// ����
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Constraint> node)
{
  accept(node->get_child());
}

// Ask����
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{
  // ask���G���e�[���\�ł�������q�m�[�h���T������
  if(positive_asks_->find(node) != positive_asks_->end()) {
    in_positive_ask_ = true;
    accept(node->get_child());
    in_positive_ask_ = false;
  } else {
    in_negative_ask_ = true;
    accept(node->get_child());
    in_negative_ask_ = false;
  }
}

// Tell����
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Tell> node)
{
  if(!in_negative_ask_){
    // tell����̓o�^
    if(collect_all_tells_ || 
       collected_tells_.find(node) == collected_tells_.end()) 
    {
      tells_->push_back(node);
      collected_tells_.insert(node);
    }
  }
}

// �_����
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// �������Z�q
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Always> node)
{
  if(in_expanded_always_) {
    if(visited_always_.find(node) != visited_always_.end()) {
      accept(node->get_child());
    }
  } else {
    accept(node->get_child());
    visited_always_.insert(node);
  }
}

// ���W���[���̎㍇��
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Weaker> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// ���W���[���̕��񍇐�
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Parallel> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// ����Ăяo��
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node)
{
  accept(node->get_child());
}

// �v���O�����Ăяo��
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node)
{
  accept(node->get_child());
}


void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Print> node)
{
  std::string str =  node->get_string();
  std::string args = node->get_args();

  args.erase(0,2);
  std::string sb(" ");
  std::string sa("");
  std::string::size_type n, nb = 0;
  //�X�y�[�X�̍폜
  while ((n = args.find(sb,nb)) != std::string::npos)
  {
    args.replace(n, sb.size(), sa);
    nb = n + sa.size();
  }
  //�����̕���
  int i = 0;
  std::string key(",");
  v_print.push_back(str);
  
  while(i <  (int)args.length())
  {
    int old_i = i;
    i = args.find(key, i);
    if( i != std::string::npos)
    {
      std::string item = args.substr(old_i, i - old_i);
      v_print.push_back(item);
    }else{
      std::string item = args.substr(old_i);
      v_print.push_back(item);
      break;
    }
    i += key.length();
  }

}
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::PrintPP> node)
{
  std::string str =  node->get_string();
  std::string args = node->get_args();

  args.erase(0,2);
  std::string sb(" ");
  std::string sa("");
  std::string::size_type n, nb = 0;
  //�X�y�[�X�̍폜
  while ((n = args.find(sb,nb)) != std::string::npos)
  {
    args.replace(n, sb.size(), sa);
    nb = n + sa.size();
  }
  //�����̕���
  int i = 0;
  std::string key(",");
  v_print_pp.push_back(str);
  
  while(i <  (int)args.length())
  {
    int old_i = i;
    i = args.find(key, i);
    if( i != std::string::npos)
    {
      std::string item = args.substr(old_i, i - old_i);
      v_print_pp.push_back(item);
    }else{
      std::string item = args.substr(old_i);
      v_print_pp.push_back(item);
      break;
    }
    i += key.length();
  }

}
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::PrintIP> node)
{
  std::string str =  node->get_string();
  std::string args = node->get_args();

  args.erase(0,2);
  std::string sb(" ");
  std::string sa("");
  std::string::size_type n, nb = 0;
  //�X�y�[�X�̍폜
  while ((n = args.find(sb,nb)) != std::string::npos)
  {
    args.replace(n, sb.size(), sa);
    nb = n + sa.size();
  }
  //�����̕���
  int i = 0;
  std::string key(",");
  v_print_ip.push_back(str);
  
  while(i <  (int)args.length())
  {
    int old_i = i;
    i = args.find(key, i);
    if( i != std::string::npos)
    {
      std::string item = args.substr(old_i, i - old_i);
      v_print_ip.push_back(item);
    }else{
      std::string item = args.substr(old_i);
      v_print_ip.push_back(item);
      break;
    }
    i += key.length();
  }

}

void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Scan> node)
{
  std::string str =  node->get_string();
  std::string args = node->get_args();

  std::string sb(" ");
  std::string sa("");
  std::string::size_type n, nb = 0;
  //�X�y�[�X�̍폜
  while ((n = args.find(sb,nb)) != std::string::npos)
  {
    args.replace(n, sb.size(), sa);
    nb = n + sa.size();
  }
  //�����̕���
  int i = 0;
  std::string key(",");
  
  while(i <  (int)args.length())
  {
    int old_i = i;
    i = args.find(key, i);
    if( i != std::string::npos)
    {
      std::string item = args.substr(old_i, i - old_i);
      v_scan.push_back(item);
    }else{
      std::string item = args.substr(old_i);
      v_scan.push_back(item);
      break;
    }
    i += key.length();
  }
}

void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Exit> node)
{
  std::string str =  node->get_string();
  std::string args = node->get_args();

  args.erase(0,2);
  std::string sb(" ");
  std::string sa("");
  std::string::size_type n, nb = 0;
  //�X�y�[�X�̍폜
  while ((n = args.find(sb,nb)) != std::string::npos)
  {
    args.replace(n, sb.size(), sa);
    nb = n + sa.size();
  }
  //�����̕���
  int i = 0;
  std::string key(",");
  v_print_ip.push_back(str);
  
  while(i <  (int)args.length())
  {
    int old_i = i;
    i = args.find(key, i);
    if( i != std::string::npos)
    {
      std::string item = args.substr(old_i, i - old_i);
      v_print_ip.push_back(item);
    }else{
      std::string item = args.substr(old_i);
      v_print_ip.push_back(item);
      break;
    }
    i += key.length();
  }

}
    
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Abort> node)
{
  std::string str =  node->get_string();
  std::string args = node->get_args();

  args.erase(0,2);
  std::string sb(" ");
  std::string sa("");
  std::string::size_type n, nb = 0;
  //�X�y�[�X�̍폜
  while ((n = args.find(sb,nb)) != std::string::npos)
  {
    args.replace(n, sb.size(), sa);
    nb = n + sa.size();
  }
  //�����̕���
  int i = 0;
  std::string key(",");
  v_print_ip.push_back(str);
  
  while(i <  (int)args.length())
  {
    int old_i = i;
    i = args.find(key, i);
    if( i != std::string::npos)
    {
      std::string item = args.substr(old_i, i - old_i);
      v_print_ip.push_back(item);
    }else{
      std::string item = args.substr(old_i);
      v_print_ip.push_back(item);
      break;
    }
    i += key.length();
  }

}


} //namespace simulator
} //namespace hydla 
