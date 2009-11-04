#ifndef _INCLUDED_HYDLA_PARSE_TREE_NODE_H_
#define _INCLUDED_HYDLA_PARSE_TREE_NODE_H_

#include <string>
#include <vector>
#include <map>
#include <set>

#include <boost/shared_ptr.hpp>

namespace hydla { 
namespace parse_tree {
  
class Node;
class ConstraintDefinition;
class ProgramDefinition;

class TreeVisitor;

typedef boost::shared_ptr<Node> node_sptr;

typedef std::string                             difinition_name_t;
typedef int                                     bound_variable_count_t;
typedef std::pair<difinition_name_t, 
                  bound_variable_count_t>       difinition_type_t;

typedef boost::shared_ptr<ConstraintDefinition> constraint_def_map_value_t;
typedef std::map<difinition_type_t,
                 constraint_def_map_value_t>    constraint_def_map_t;

typedef boost::shared_ptr<ProgramDefinition>    program_def_map_value_t;
typedef std::map<difinition_type_t,
                 program_def_map_value_t>       program_def_map_t;

typedef std::set<difinition_type_t>             referenced_definition_t;

typedef std::map<std::string, node_sptr>        formal_arg_map_t;

typedef std::vector<node_sptr>                  actual_arg_list_t;

typedef std::map<std::string, int>              variable_map_t;


/**
 * Nodeのpreprocess関数の引数クラス
 */
typedef struct PreprocessArg_ {

  PreprocessArg_(variable_map_t&        variable_map,
                 program_def_map_t&     prog_def_map,
                 constraint_def_map_t&  cons_def_map,
                 formal_arg_map_t&      formal_arg_map) :
    in_guard_(false),
    in_constraint_(false),
    in_always_(false),
    differential_count_(0),
    variable_map_(variable_map),
    prog_def_map_(prog_def_map),
    cons_def_map_(cons_def_map),
    formal_arg_map_(formal_arg_map),
    refered_def_()
  {}

  PreprocessArg_(const PreprocessArg_& arg, 
                 formal_arg_map_t& formal_arg_map) :
    in_guard_(arg.in_guard_),
    in_constraint_(arg.in_constraint_),
    in_always_(arg.in_always_),
    differential_count_(arg.differential_count_),
    variable_map_(arg.variable_map_),
    prog_def_map_(arg.prog_def_map_),
    cons_def_map_(arg.cons_def_map_),
    formal_arg_map_(formal_arg_map),
    refered_def_(arg.refered_def_)
  {}


    // ガードの中かどうか
    bool in_guard_;
    
    // 制約式の中かどうか
    bool in_constraint_;

    // always制約の有効範囲内かどうか
    bool in_always_;

    // 微分記号を通過した回数
    // 変数に到達した際、この値がその変数に対する微分の最大回数
    int differential_count_;

    variable_map_t&         variable_map_;
    program_def_map_t&      prog_def_map_;
    constraint_def_map_t&   cons_def_map_;
    formal_arg_map_t&       formal_arg_map_;
    referenced_definition_t refered_def_;
} PreprocessArg;
typedef PreprocessArg preprocess_arg_t;

/**
 * パスツリーの基底ノード
 */
class Node {
public:
  Node(){}
  virtual ~Node(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor) = 0;
  virtual void preprocess(node_sptr& own, preprocess_arg_t& arg) = 0;
  virtual node_sptr clone() = 0;

  virtual std::string to_string() const {return "";}

  friend std::ostream& operator<< (std::ostream&, Node&);
};

/**
 * 1つの子ノードを持つノード
 */
class UnaryNode : public Node {
public:
  typedef boost::shared_ptr<UnaryNode> node_type_sptr;

  UnaryNode()
  {}
  
  UnaryNode(node_sptr &child) :
    child_(child)
  {}
  
  virtual ~UnaryNode()
  {}

  virtual void accept(node_sptr own, TreeVisitor* visitor) = 0;

  virtual void preprocess(node_sptr& own, preprocess_arg_t& arg) 
  {
    child_->preprocess(child_, arg);
  }
  
  virtual node_sptr clone() = 0;

  node_type_sptr clone(node_type_sptr n)
  {
    n->child_ = child_->clone();
    return n;
  }
  
  virtual std::string to_string() const
  {
    return "unary_node[" + child_->to_string() + "]";    
  }

  void set_child_node(node_sptr child)
  {
    child_ = child;
  }

  const node_sptr get_child_node() const
  {
    return child_;
  }

protected:
  node_sptr child_;
};

/**
 * 2つの子ノードを持つノード
 */
class BinaryNode : public Node{
public:
  typedef boost::shared_ptr<BinaryNode> node_type_sptr;

  BinaryNode()
  {}  
  
  BinaryNode(node_sptr &lhs, node_sptr &rhs) : 
    lhs_(lhs), rhs_(rhs)
  {}
    
  virtual ~BinaryNode(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor) = 0;

  virtual void preprocess(node_sptr& own, preprocess_arg_t& arg)
  {
    lhs_->preprocess(lhs_, arg);
    rhs_->preprocess(rhs_, arg);
  }

  virtual node_sptr clone() = 0;

  node_type_sptr clone(node_type_sptr n)
  {
    n->lhs_ = lhs_->clone();
    n->rhs_ = rhs_->clone();
    return n;
  }

  virtual std::string to_string() const
  {
    return "binary_node[" + 
      lhs_->to_string() +  "," + 
      rhs_->to_string() + "]";
  }

  void set_lhs(node_sptr lhs) {lhs_ = lhs;}
  node_sptr get_lhs()         {return lhs_;}

  void set_rhs(node_sptr rhs) {rhs_ = rhs;}
  node_sptr get_rhs()         {return rhs_;}

protected:
  node_sptr lhs_;
  node_sptr rhs_;
};

/**
 * 制約やプログラムの呼び出しノードの共通クラス
 */
class Caller : public UnaryNode {
public:
  Caller(){}
  virtual ~Caller(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor) = 0;
  virtual void preprocess(node_sptr& own, preprocess_arg_t& arg) = 0;
  virtual node_sptr clone();
  virtual std::string to_string() const;

  // specific functions
  void        set_name(std::string& name) {name_ = name;}
  std::string get_name() const            {return name_;}

  void add_actual_arg(node_sptr a) {actual_arg_list_.push_back(a);}

protected:
  std::string name_;
  actual_arg_list_t actual_arg_list_;
};

/**
 * 制約呼び出し
 */
class ConstraintCaller : public Caller {
public:
  ConstraintCaller(){}
  virtual ~ConstraintCaller(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual void preprocess(node_sptr& own, preprocess_arg_t& arg);

  // specific functions

private:
};

/**
 * プログラム呼び出し
 */
class ProgramCaller : public Caller {
public:
  ProgramCaller(){}
  virtual ~ProgramCaller(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);
  virtual void preprocess(node_sptr& own, preprocess_arg_t& arg);

  // specific functions

private:
};

/**
 * 制約やプログラムの定義ノードの共通クラス
 */
class Definition : public UnaryNode {
public:
  typedef std::vector<std::string> bound_variables_t;

  Definition(){}
  virtual ~Definition(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor) = 0;

  virtual void preprocess(node_sptr& own, preprocess_arg_t& arg) 
  {}
  
  virtual void preprocess(node_sptr& own, 
                          preprocess_arg_t& arg, 
                          actual_arg_list_t& actual_arg_list);
  virtual node_sptr clone();
  virtual std::string to_string() const;

  // specific functions
  void        set_name(std::string& name) {name_ = name;}
  std::string get_name() const            {return name_;}

  void add_bound_variable(std::string& bound_variable) 
  {
    bound_variables_.push_back(bound_variable);
  }
 
  const bound_variables_t* get_bound_variables() const 
  {
    return &bound_variables_;
  }

private:
  std::string name_;
  bound_variables_t bound_variables_;
};

/**
 * プログラム定義
 */
class ProgramDefinition : public Definition {
public:
  ProgramDefinition(){}
  virtual ~ProgramDefinition(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

private:
};

/**
 * 制約定義
 */
class ConstraintDefinition : public Definition {
public:
  ConstraintDefinition(){}
  virtual ~ConstraintDefinition(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

private:
};

/**
 * 制約式
 */ 
class Constraint : public UnaryNode {
public:
  Constraint()
  {}
    
  virtual ~Constraint(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual void preprocess(node_sptr& own, preprocess_arg_t& arg)
  {
    // すでに制約式の中であった場合は自分自身を取り除く
    if(arg.in_constraint_) {
      child_->preprocess(child_, arg);
      own = child_;
    } else {
      preprocess_arg_t narg(arg);
      narg.in_constraint_ = true;
      child_->preprocess(child_, narg);
    }
  }

  virtual node_sptr clone()
  {
    boost::shared_ptr<Constraint> n(new Constraint());
    n->child_ = child_->clone();
    return n;
  }
  
  virtual std::string to_string() const 
  {
    return "constraint[" + child_->to_string() + "]";
  }

private:
};

/**
 * tell制約
 */ 
class Tell : public UnaryNode {
public:
  Tell()
  {}
    
  virtual ~Tell(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual void preprocess(node_sptr& own, preprocess_arg_t& arg)
  {
    child_->preprocess(child_, arg);
  }

  virtual node_sptr clone()
  {
    boost::shared_ptr<Tell> n(new Tell());
    n->child_ = child_->clone();
    return n;
  }
  
  virtual std::string to_string() const 
  {
    return "tell[" + child_->to_string() + "]";
  }

private:
};

/**
 * ask制約
 */ 
class Ask : public Node {
public:
  Ask()
  {}
    
  virtual ~Ask(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual void preprocess(node_sptr& own, preprocess_arg_t& arg)
  {
    preprocess_arg_t guard_arg(arg);
    guard_arg.in_guard_ = true;
    guard_->preprocess(guard_, guard_arg);
  
    preprocess_arg_t child_arg(arg);
    child_arg.in_always_ = false;    
    child_->preprocess(child_, child_arg);
  }

  virtual node_sptr clone()
  {
    boost::shared_ptr<Ask> n(new Ask());
    n->guard_ = guard_->clone();
    n->child_ = child_->clone();
    return n;
  }
  
  virtual std::string to_string() const 
  {
    return guard_->to_string() + "=>" + child_->to_string();
  }

  // specific functions
  void set_child_node(node_sptr child)    {child_ = child;}
  const node_sptr get_child_node() const  {return child_;}

  void set_guard_node(node_sptr guard)    {guard_ = guard;}
  const node_sptr get_guard_node() const  {return guard_;}

private:
  node_sptr guard_;
  node_sptr child_;
};

#define DEFINE_BINARY_OP_NODE(NAME, OPNAME)                 \
  class NAME : public BinaryNode {                          \
public:                                                     \
typedef boost::shared_ptr<NAME> node_type_sptr;             \
                                                            \
NAME()                                                      \
{}                                                          \
                                                            \
NAME(node_sptr& lhs, node_sptr& rhs) :                      \
  BinaryNode(lhs, rhs)                                      \
{}                                                          \
                                                            \
virtual ~NAME(){}                                           \
                                                            \
virtual void accept(node_sptr own, TreeVisitor* visitor);   \
                                                            \
virtual node_sptr clone()                                   \
{                                                           \
  node_type_sptr n(new NAME);                               \
  return BinaryNode::clone(n);                              \
}                                                           \
                                                            \
virtual std::string to_string() const                       \
{                                                           \
  return std::string(OPNAME) + "[" + lhs_->to_string() + "," + rhs_->to_string() + "]"; \
}                                                           \
}

#define DEFINE_BINARY_OP_NODE_WITH_PREPROCESS(NAME, OPNAME)       \
  class NAME : public BinaryNode {                                \
public:                                                           \
typedef boost::shared_ptr<NAME> node_type_sptr;                   \
                                                                  \
NAME()                                                            \
{}                                                                \
                                                                  \
NAME(node_sptr& lhs, node_sptr& rhs) :                            \
  BinaryNode(lhs, rhs)                                            \
{}                                                                \
                                                                  \
virtual ~NAME(){}                                                 \
                                                                  \
virtual void accept(node_sptr own, TreeVisitor* visitor);         \
                                                                  \
virtual node_sptr clone()                                         \
{                                                                 \
  node_type_sptr n(new NAME);                                     \
  return BinaryNode::clone(n);                                    \
}                                                                 \
                                                                  \
virtual void preprocess(node_sptr& own, preprocess_arg_t& arg);   \
                                                                  \
                                                                  \
virtual std::string to_string() const                             \
{                                                                 \
  return std::string(OPNAME) + "[" + lhs_->to_string() + "," + rhs_->to_string() + "]"; \
}                                                                 \
}


/**
 * 比較演算子「=」
 */
DEFINE_BINARY_OP_NODE(Equal, "=");

/**
 * 比較演算子「!=」
 */
DEFINE_BINARY_OP_NODE(UnEqual, "!=");

/**
 * 比較演算子「<」
 */
DEFINE_BINARY_OP_NODE(Less, "<");

/**
 * 比較演算子「<=」
 */
DEFINE_BINARY_OP_NODE(LessEqual, "<=");

/**
 * 比較演算子「>」
 */
DEFINE_BINARY_OP_NODE(Greater, ">");

/**
 * 比較演算子「>=」
 */
DEFINE_BINARY_OP_NODE(GreaterEqual, ">=");

/**
 * 算術演算子「+」
 */
DEFINE_BINARY_OP_NODE(Plus, "+");

/**
 * 算術演算子「-」
 */
DEFINE_BINARY_OP_NODE(Subtract, "-");

/**
 * 算術演算子「*」
 */
DEFINE_BINARY_OP_NODE(Times, "*");

/**
 * 算術演算子「/」
 */
DEFINE_BINARY_OP_NODE(Divide, "/");

/**
 * 論理演算子「/\」（連言）
 */
DEFINE_BINARY_OP_NODE_WITH_PREPROCESS(LogicalAnd, "/\\");

/**
 * 論理演算子「\/」（選言）
 */
DEFINE_BINARY_OP_NODE_WITH_PREPROCESS(LogicalOr, "\\/");

/**
 * 制約階層定義演算子
 * 並列制約「,」
 */ 
DEFINE_BINARY_OP_NODE_WITH_PREPROCESS(Parallel, ",");

/**
 * 制約階層定義演算子
 * 弱制約「<<」
 */ 
DEFINE_BINARY_OP_NODE_WITH_PREPROCESS(Weaker, "<<");

/**
 * 時相演算子「[]」(Always)
 */
class Always: public UnaryNode {
public:
  typedef boost::shared_ptr<Always> node_type_sptr;

  Always()
  {}

  Always(node_sptr& child):
  UnaryNode(child)
  {}

  virtual ~Always(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual node_sptr clone()
  {
    node_type_sptr n(new Always);
    return UnaryNode::clone(n);
  }

  virtual void preprocess(node_sptr& own, preprocess_arg_t& arg);

  virtual std::string to_string() const
  {
    return "[](" + child_->to_string() + ")";
  }
};

/**
 * 算術単項演算子「+」
 */
class Positive: public UnaryNode {
public:
  typedef boost::shared_ptr<Positive> node_type_sptr;

  Positive()
  {}

  Positive(node_sptr& child):
  UnaryNode(child)
  {}

  virtual ~Positive(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual node_sptr clone()
  {
    node_type_sptr n(new Positive);
    return UnaryNode::clone(n);
  }

  virtual void preprocess(node_sptr& own, preprocess_arg_t& arg)
  {
    child_->preprocess(child_, arg);
  }

  virtual std::string to_string() const
  {
    return "+" + child_->to_string();
  }
};

/**
 * 算術単項演算子「-」
 */
class Negative: public UnaryNode {
public:
  typedef boost::shared_ptr<Negative> node_type_sptr;

  Negative()
  {}

  Negative(node_sptr& child):
    UnaryNode(child)
  {}

  virtual ~Negative(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual node_sptr clone()
  {
    node_type_sptr n(new Negative);
    return UnaryNode::clone(n);
  }

  virtual void preprocess(node_sptr& own, preprocess_arg_t& arg)
  {
    child_->preprocess(child_, arg);
  }

  virtual std::string to_string() const
  {
    return "-" + child_->to_string();
  }
};

/**
 * 微分「'」
 */
class Differential: public UnaryNode {
public:
  typedef boost::shared_ptr<Differential> node_type_sptr;

  Differential()
  {}

  Differential(node_sptr& child):
    UnaryNode(child)
  {}

  virtual ~Differential(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual node_sptr clone()
  {
    node_type_sptr n(new Differential);
    return UnaryNode::clone(n);
  }

  virtual void preprocess(node_sptr& own, preprocess_arg_t& arg)
  {
    preprocess_arg_t narg(arg);
    narg.differential_count_++;
    child_->preprocess(child_, narg);
  }

  virtual std::string to_string() const
  {
    return child_->to_string() + "'";
  }
};

/**
 * 左極限「-」
 */
class Previous: public UnaryNode {
public:
  typedef boost::shared_ptr<Previous> node_type_sptr;

  Previous()
  {}

  Previous(node_sptr& child):
    UnaryNode(child)
  {}

  virtual ~Previous(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual node_sptr clone()
  {
    node_type_sptr n(new Previous);
    return UnaryNode::clone(n);
  }

  virtual void preprocess(node_sptr& own, preprocess_arg_t& arg)
  {
    child_->preprocess(child_, arg);
  }

  virtual std::string to_string() const
  {
    return child_->to_string() + "-";
  }
};

/**
 * 小数
 */ 
class Number : public Node {
public:
  Number()
  {}  
  
  Number(std::string number) : 
    number_(number)
  {}
    
  virtual ~Number()
  {}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual void preprocess(node_sptr& own, preprocess_arg_t& arg)
  {
    //do nothing
  }

  virtual node_sptr clone()
  {
    boost::shared_ptr<Number> n(new Number());
    n->number_ = number_;
    return n;
  }
  
  virtual std::string to_string() const {return number_;}

  // specific functions
  void        set_number(std::string& number) {number_ = number;}
  std::string get_number() const              {return number_;}

private:
  std::string number_;
};

/**
 * 変数
 * 従属変数の場合もあり
 */ 
class Variable : public Node {
public:
  Variable()
  {}  
  
  Variable(std::string name) : 
    name_(name)
  {}
    
  virtual ~Variable(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual void preprocess(node_sptr& own, preprocess_arg_t& arg);

  virtual node_sptr clone()
  {
    boost::shared_ptr<Variable> n(new Variable());
    n->name_ = name_;
    return n;
  }
  
  virtual std::string to_string() const {return name_;}

  // specific functions
  void        set_name(std::string& name) {name_ = name;}
  std::string get_name() const            {return name_;}

private:
  std::string name_;
};

} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_NODE_H_
