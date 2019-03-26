#pragma once

#include <set>
#include <string>
#include <ostream>
#include <algorithm>

#include <boost/shared_ptr.hpp>

#include "Node.h"

namespace hydla {
namespace hierarchy {


/**
 * モジュールの集合を表すクラス
 *
 */
class ModuleSet
{
public:
  typedef std::pair<std::string,
                    hydla::symbolic_expression::node_sptr> module_t;
  typedef std::set<module_t>                      module_list_t;
  typedef module_list_t::const_iterator           module_list_const_iterator;
  typedef module_list_t::iterator           module_list_iterator;

  struct ModuleComp
  {
    bool operator()(const module_t& a, const module_t& b) const
    {
      return a.first < b.first;
    }
  };

  ModuleSet();
  ModuleSet(const std::string& name, hydla::symbolic_expression::node_sptr node);
  ModuleSet(ModuleSet& lhs, ModuleSet& rhs);

  ~ModuleSet();

  void replace(int x)const
  {
   bool flagA=false, flagB=false, flagC=false;

   for(auto it = module_list_.begin(); it != module_list_.end();)
   {
     if(it->first == "1<=a&a<=2")
     {
       it = module_list_.erase(it);
       flagA=true;
     }
     else if(it->first == "a=0")
     {
       it = module_list_.erase(it);
       flagB=true;
     }
     else if(it->first == "b=0")
     {
       it = module_list_.erase(it);
       flagC=true;
     }
     else
     {
       ++it;
     }
   }
  /*
    for(auto& m : module_list_)
    {
	if (m.first == "1<=a&a<=2")
	    {
	      using hydla::symbolic_expression::LogicalAnd;
	      using hydla::symbolic_expression::LessEqual;
	      using hydla::symbolic_expression::Equal;
	      using hydla::symbolic_expression::Variable;
	      using hydla::symbolic_expression::Number;
	      using hydla::symbolic_expression::Constraint;

	      using hydla::symbolic_expression::node_sptr;
	      
	      m.second =
		  boost::shared_ptr<Constraint>(new Constraint(
		      boost::shared_ptr<Equal>(new Equal(
			  boost::shared_ptr<Variable>(new Variable("a")),
			  boost::shared_ptr<Number>(new Number("2"))))));
	    }
    }
    */

using hydla::symbolic_expression::LogicalAnd;
              using hydla::symbolic_expression::LessEqual;
              using hydla::symbolic_expression::Equal;
              using hydla::symbolic_expression::Variable;
              using hydla::symbolic_expression::Number;
              using hydla::symbolic_expression::Constraint;

              using hydla::symbolic_expression::node_sptr;
	if(flagB)
	{

        module_list_.insert(module_t("a=2",
		  boost::shared_ptr<Constraint>(new Constraint(
		      boost::shared_ptr<Equal>(new Equal(
			  boost::shared_ptr<Variable>(new Variable("a")),
			  boost::shared_ptr<Number>(new Number(std::to_string(x)))))))));
			  }
	if(flagC)
	{
        module_list_.insert(module_t("b=1",
		  boost::shared_ptr<Constraint>(new Constraint(
		      boost::shared_ptr<Equal>(new Equal(
			  boost::shared_ptr<Variable>(new Variable("b")),
			  boost::shared_ptr<Number>(new Number(std::to_string(x)))))))));
			  }

  }

  /**
   * 集合(このクラス)の名前
   */ 
  std::string get_name() const;
  
  /**
   * 中値記法で出力
   */ 
  std::string get_infix_string() const;
  
  /**
   * 集合の最初の要素
   */
  module_list_const_iterator begin() const 
  {
    return module_list_.begin();
  }

 /**
   * 集合の最後の次の要素
   */
  module_list_const_iterator end() const 
  {
   return module_list_.end();
  }

  /**
   * 集合の最初の要素
   */
  module_list_iterator begin()
  {
    return module_list_.begin();
  }

 /**
   * 集合の最後の次の要素
   */
  module_list_iterator end()
  {
   return module_list_.end();
  }

  void clear() 
  {
    module_list_.clear();
  }


  /**
   * 集合の要素の数
   */
  size_t size() const 
  {
    return module_list_.size();
  }
  
  /**
   * find module
   */
  module_list_const_iterator find(const module_t& mod) const;

  /**
   * erase module from module_list_
   */
  int erase(const ModuleSet& ms);
  int erase(const module_t& m);
  module_list_const_iterator erase(const module_list_const_iterator& it);

  /**
   * モジュールを追加
   */
  void add_module(const module_t& mod) { module_list_.insert(mod); }
  void insert(const ModuleSet &ms) { module_list_.insert(ms.module_list_.begin(), ms.module_list_.end()); }

  bool is_super_set(const ModuleSet& subset_mod) const
  {
    return std::includes(module_list_.begin(), 
                         module_list_.end(),
                         subset_mod.module_list_.begin(), 
                         subset_mod.module_list_.end(),
                         ModuleComp());
  }

  /**
   * 集合のパースツリーの内容出力
   */
  std::ostream& dump(std::ostream& s) const;


  /**
   * このクラス同士の比較
   * 含まれるモジュール数が少ないほど小さい
   * モジュール数が同一の時は含まれているモジュール名により判断をおこなう
   */ 
  int compare(const ModuleSet& rhs) const;
  
  bool empty() const { return module_list_.size()==0; }
  /**
   * return whether this module_set includes given module_set or not
   */
  bool including(const ModuleSet& ms) const;

  bool disjoint(const ModuleSet& ms) const;

  /**
   * 集合の各制約モジュールに対してTreeVisitorの適用
   */ 
  void dispatch(hydla::symbolic_expression::TreeVisitor* visitor)
  {
    for (auto module :module_list_)
    {
      (module.second)->accept(module.second, visitor);
    }
  }
  
  bool operator<(const ModuleSet& rhs) const { return (compare(rhs) < 0); }

  friend std::ostream& operator<<(std::ostream &s, const ModuleSet::module_t& m);

private:
  mutable module_list_t module_list_;
};

std::ostream& operator<<(std::ostream& s, const ModuleSet& m);

class ModuleSetComparator
{
public:
  bool operator()(const ModuleSet &lhs, 
                  const ModuleSet &rhs) const
  {
    return lhs.compare(rhs) > 0;
  }
};

} // namespace hierarchy
} // namespace hydla
