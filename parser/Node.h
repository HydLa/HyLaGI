#ifndef _INCLUDED_HYDLA_PARSE_TREE_NODE_H_
#define _INCLUDED_HYDLA_PARSE_TREE_NODE_H_

#include <ostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

namespace hydla { 
namespace parse_tree {

class Node;
class ConstraintDefinition;
class ProgramDefinition;

class TreeVisitor;
class BaseNodeVisitor;

typedef unsigned int            node_id_t;
typedef boost::shared_ptr<Node> node_sptr;
typedef boost::shared_ptr<const Node> node_const_sptr;

/**
 * パスツリーの基底ノード
 */
class Node {
public:
  typedef boost::shared_ptr<Node> node_type_sptr;

  Node() : 
    id_(0)
  {}
  
  virtual ~Node()
  {}

  virtual void accept(node_sptr own, TreeVisitor* visitor) = 0;
  virtual void accept(node_sptr own, BaseNodeVisitor* visitor) = 0;
  
  /**
   * 子ノードを含めたノード（ツリー）の構造を複製する
   * 
   * 複製されたノードのIDはすべて0となる
   */
  virtual node_sptr clone() = 0;

  /**
   * 子ノードを含めたノード（ツリー）の構造の比較を行う
   * 
   * 構造比較時に，ノードのIDは考慮されない
   * 終端ノードにおいてはそのノードの具体的な値は比較時に考慮される
   * exactly_sameが真の場合，
   * x=1と1=xの様に対称性をもつツリーは同一であるとはみなされない
   * exactly_sameが偽の場合，
   * (x=1 & 2=y) & z=3 と x=1 & (y=2 & z=3) のようなHydLaプログラムとして
   * 同一の意味を持つ構造の場合は同一とみなされる
   */
  virtual bool is_same_struct(const Node& n, bool exactly_same) const;

  /**
   * ノードの型の名前
   */
  virtual std::string get_node_type_name() const {
    return "Node";
  }

  std::string get_string() const
  {
    std::stringstream sstr;
    dump(sstr);
    return sstr.str();
  }
  

  /**
   * ノードの状態を出力する
   */
  virtual std::ostream& dump(std::ostream& s) const 
  {
    return s << get_node_type_name()
             << "<" << get_id() << ">";
  }
  

  /**
   * ノードIDの設定
   */
  void set_id(node_id_t id)
  {
    id_ = id;
  }

  /**
   * ノードIDを得る
   */
  node_id_t get_id() const 
  {
    return id_;
  }

private:
  /**
   * ノードID
   */
  node_id_t id_;
};

std::ostream& operator<<(std::ostream&, const Node&);

class FactorNode : public Node {
public:
  typedef boost::shared_ptr<FactorNode> node_type_sptr;

  FactorNode()
  {}

  virtual ~FactorNode()
  {}

  virtual void accept(node_sptr own, TreeVisitor* visitor) = 0;
  virtual void accept(node_sptr own, BaseNodeVisitor* visitor);
  
  virtual node_sptr clone() = 0;

  virtual std::string get_node_type_name() const {
    return "FactorNode";
  }
};

/**
 * 1つの子ノードを持つノード
 */
class UnaryNode : public Node {
public:
  typedef boost::shared_ptr<UnaryNode> node_type_sptr;

  UnaryNode()
  {}
  
  UnaryNode(const node_sptr &child) :
    child_(child)
  {}
  
  virtual ~UnaryNode()
  {}

  virtual void accept(node_sptr own, TreeVisitor* visitor) = 0;
  virtual void accept(node_sptr own, BaseNodeVisitor* visitor);

  virtual node_sptr clone() = 0;

  virtual bool is_same_struct(const Node& n, bool exactly_same) const;

  node_type_sptr clone(node_type_sptr n)
  {
    n->child_ = child_->clone();
    return n;
  }

  virtual std::string get_node_type_name() const {
    return "UnaryNode";
  }

  virtual std::ostream& dump(std::ostream& s) const 
  {
    Node::dump(s);
    return s << "[" << *child_ << "]";
  }
  

  /**
   * setter of child node
   */
  void set_child(const node_sptr& child)  
  {
    child_ = child;
  }

  /**
   * getter of child node
   */
  const node_sptr& get_child() const
  {
    return child_;
  }

  /**
   * getter of child node
   */
  const node_sptr& get_child_node() const
  {
    return child_;
  }

protected:
  node_sptr child_;
};

#define DEFINE_UNARY_NODE(NAME)                             \
  class NAME : public UnaryNode {                           \
  public:                                                   \
  typedef boost::shared_ptr<NAME> node_type_sptr;           \
                                                            \
  NAME()                                                    \
    {}                                                      \
                                                            \
  NAME(const node_sptr& child) :                            \
    UnaryNode(child)                                        \
    {}                                                      \
                                                            \
  virtual ~NAME(){}                                         \
                                                            \
  virtual void accept(node_sptr own, TreeVisitor* visitor); \
                                                            \
  virtual node_sptr clone()                                 \
    {                                                       \
      node_type_sptr n(new NAME);                           \
      return UnaryNode::clone(n);                           \
    }                                                       \
                                                            \
  virtual std::string get_node_type_name() const {          \
    return #NAME;                                           \
  }                                                         \
  };

/**
 * 2つの子ノードを持つノード
 */
class BinaryNode : public Node{
public:
  typedef boost::shared_ptr<BinaryNode> node_type_sptr;

  BinaryNode()
  {}  
  
  BinaryNode(const node_sptr &lhs, const node_sptr &rhs) : 
    lhs_(lhs), rhs_(rhs)
  {}
    
  virtual ~BinaryNode(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor) = 0;
  virtual void accept(node_sptr own, BaseNodeVisitor* visitor);

  virtual bool is_same_struct(const Node& n, bool exactly_same) const;

  virtual node_sptr clone() = 0;

  node_type_sptr clone(node_type_sptr n)
  {
    n->lhs_ = lhs_->clone();
    n->rhs_ = rhs_->clone();
    return n;
  }

  virtual std::string get_node_type_name() const {
    return "BinaryNode";
  }
  

  virtual std::ostream& dump(std::ostream& s) const 
  {
    Node::dump(s);
    return s << "[" << *lhs_ << "," << *rhs_ << "]";
  }
   
  /**
   * setter of left-hand-side node
   */
  void set_lhs(const node_sptr& lhs) 
  {
    lhs_ = lhs;
  }

  /**
   * getter of left-hand-side node
   */
  const node_sptr& get_lhs() const         
  {
    return lhs_;
  }

  /**
   * setter of right-hand-side node
   */
  void set_rhs(const node_sptr& rhs) 
  {
    rhs_ = rhs;
  }

  /**
   * getter of right-hand-side node
   */  
  const node_sptr& get_rhs() const
  {
    return rhs_;
  }

protected:
  struct CheckInclude;
  typedef std::vector<std::pair<const Node*, bool> > child_node_list_t;

  bool is_exactly_same(const Node& n, bool exactly_same) const;
  void create_child_node_list(child_node_list_t& cnl, 
                              const Node* n) const;

  node_sptr lhs_;
  node_sptr rhs_;
};

#define DEFINE_BINARY_NODE(NAME)                            \
  class NAME : public BinaryNode {                          \
  public:                                                   \
  typedef boost::shared_ptr<NAME> node_type_sptr;           \
                                                            \
  NAME()                                                    \
    {}                                                      \
                                                            \
  NAME(const node_sptr& lhs, const node_sptr& rhs) :        \
    BinaryNode(lhs, rhs)                                    \
    {}                                                      \
                                                            \
  virtual ~NAME(){}                                         \
                                                            \
  virtual void accept(node_sptr own, TreeVisitor* visitor); \
                                                            \
  virtual node_sptr clone()                                 \
    {                                                       \
      node_type_sptr n(new NAME);                           \
      return BinaryNode::clone(n);                          \
    }                                                       \
  virtual std::string get_node_type_name() const {          \
    return #NAME;                                           \
  }                                                         \
  };

#define DEFINE_ASYMMETRIC_BINARY_NODE(NAME)                             \
  class NAME : public BinaryNode {                                      \
  public:                                                               \
  typedef boost::shared_ptr<NAME> node_type_sptr;                       \
                                                                        \
  NAME()                                                                \
    {}                                                                  \
                                                                        \
  NAME(const node_sptr& lhs, const node_sptr& rhs) :                    \
    BinaryNode(lhs, rhs)                                                \
    {}                                                                  \
                                                                        \
  virtual ~NAME(){}                                                     \
                                                                        \
  virtual void accept(node_sptr own, TreeVisitor* visitor);             \
                                                                        \
  virtual bool is_same_struct(const Node& n, bool exactly_same) const;  \
                                                                        \
  virtual node_sptr clone()                                             \
    {                                                                   \
      node_type_sptr n(new NAME);                                       \
      return BinaryNode::clone(n);                                      \
    }                                                                   \
                                                                        \
  virtual std::string get_node_type_name() const {                      \
    return #NAME;                                                       \
  }                                                                     \
  };


/**
 * 制約やプログラムの呼び出しノードの共通クラス
 */
class Caller : public UnaryNode {
public:
  typedef std::vector<node_sptr>        actual_args_t;
  typedef actual_args_t::iterator       actual_args_iterator;
  typedef actual_args_t::const_iterator actual_args_const_iterator;


  Caller(){}
  virtual ~Caller(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor) = 0;
  virtual node_sptr clone();

  virtual std::string get_node_type_name() const {
    return "Caller";
  }

  virtual std::ostream& dump(std::ostream& s) const;

  /**
   * 呼び出す定義名を設定する
   */
  void set_name(const std::string& name) 
  {
    name_ = name;
  }

  /**
   * 呼び出す定義名を返す
   */  
  std::string get_name() const
  {
    return name_;
  }

  /**
   * 実引数ノードの追加
   *
   * @param node 実引数のノード
   */
  void add_actual_arg(const node_sptr& node) 
  {
    actual_args_.push_back(node);
  }

  /**
   * 実引数ノードを返す
   *
   * @param index 実引数ノードの番号
   */
  node_sptr get_actual_arg(size_t index) const
  {
    return actual_args_[index];
  }

  /**
   * 実引数の数を返す
   */
  size_t actual_arg_size() const 
  {
    return actual_args_.size();
  }

  /**
   * 束縛変数のリストの最初の要素を指す
   * 読み書き可能なiteratorを返す
   */
  actual_args_iterator
  actual_arg_begin()
  {
    return actual_args_.begin();
  }

  /**
   * 束縛変数のリストの最初の要素を指す
   * 読み込みのみ可能なiteratorを返す
   */
  actual_args_const_iterator
  actual_arg_begin() const
  {
    return actual_args_.begin();
  }

  /**
   * 束縛変数のリストの最後の次の要素を指す
   * 読み書き可能なiteratorを返す
   */
  actual_args_iterator
  actual_arg_end()
  {
    return actual_args_.end();
  }

  /**
   * 束縛変数のリストの最後の次の要素を指す
   * 読み込みのみ可能なiteratorを返す
   */
  actual_args_const_iterator
  actual_arg_end() const
  {
    return actual_args_.end();
  }


protected:
  /// このノードが呼び出す定義名
  std::string name_;

  /// 呼び出し時に使用する実引数のリスト
  actual_args_t actual_args_;
};

/**
 * 制約呼び出し
 */
class ConstraintCaller : public Caller {
public:
  ConstraintCaller(){}
  virtual ~ConstraintCaller(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual std::string get_node_type_name() const {
    return "ConstraintCaller";
  }
};

/**
 * プログラム呼び出し
 */
class ProgramCaller : public Caller {
public:
  ProgramCaller(){}
  virtual ~ProgramCaller(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual std::string get_node_type_name() const {
    return "ProgramCaller";
  }
};

/**
 * 制約やプログラムの定義ノードの共通クラス
 */
class Definition : public UnaryNode {
public:
  typedef std::vector<std::string>          bound_variables_t;
  typedef bound_variables_t::iterator       bound_variables_iterator;
  typedef bound_variables_t::const_iterator bound_variables_const_iterator;

  Definition(){}
  virtual ~Definition(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor) = 0;
  virtual node_sptr clone();

  virtual std::string get_node_type_name() const {
    return "Definition";
  }

  virtual std::ostream& dump(std::ostream& s) const;

  /**
   * 定義名を設定する
   */
  void set_name(const std::string& name) 
  {
    name_ = name;
  }
  
  /**
   * 定義名を返す
   */
  std::string get_name() const            
  {
    return name_;
  }

  /**
   * 束縛変数の追加
   *
   * @param variable_name 束縛変数の名前
   */
  void add_bound_variable(const std::string& variable_name) 
  {
    bound_variables_.push_back(variable_name);
  }

  /**
   * 束縛変数名を返す
   *
   * @param index 束縛変数の番号
   */
  std::string get_bound_variable(size_t index) const
  {
    return bound_variables_[index];
  }

  /**
   * 束縛変数の数を返す
   */
  size_t bound_variable_size() const 
  {
    return bound_variables_.size();
  }

  /**
   * 束縛変数のリストの最初の要素を指す
   * 読み書き可能なiteratorを返す
   */
  bound_variables_iterator 
  bound_variable_begin()
  {
    return bound_variables_.begin();
  }

  /**
   * 束縛変数のリストの最初の要素を指す
   * 読み込みのみ可能なiteratorを返す
   */
  bound_variables_const_iterator 
  bound_variable_begin() const
  {
    return bound_variables_.begin();
  }

  /**
   * 束縛変数のリストの最後の次の要素を指す
   * 読み書き可能なiteratorを返す
   */
  bound_variables_iterator 
  bound_variable_end()
  {
    return bound_variables_.end();
  }

  /**
   * 束縛変数のリストの最後の次の要素を指す
   * 読み込みのみ可能なiteratorを返す
   */
  bound_variables_const_iterator 
  bound_variable_end() const
  {
    return bound_variables_.end();
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

  virtual std::string get_node_type_name() const {
    return "ProgramDefinition";
  }
};

/**
 * 制約定義
 */
class ConstraintDefinition : public Definition {
public:
  ConstraintDefinition(){}
  virtual ~ConstraintDefinition(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual std::string get_node_type_name() const {
    return "ConstraintDefinition";
  }
};

/**
 * 制約式
 */ 
DEFINE_UNARY_NODE(Constraint);

/**
 * tell制約
 */ 
DEFINE_UNARY_NODE(Tell);

/**
 * ask制約
 */ 
class Ask : public BinaryNode {
public:
  Ask()
  {}

  Ask(const node_sptr& guard, const node_sptr& child) :
    BinaryNode(guard, child)
  {}
    
  virtual ~Ask(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual bool is_same_struct(const Node& n, bool exactly_same) const;

  virtual node_sptr clone()
  {
    node_type_sptr n(new Ask);
    return BinaryNode::clone(n);
  }

  virtual std::string get_node_type_name() const {
    return "Ask";
  }

  /**
   * ガードノードを設定する
   */
  void set_guard(const node_sptr& guard) 
  {
    set_lhs(guard);
  }

  /**
   * ガードノードを得る
   */
  const node_sptr& get_guard() const     
  {
    return get_lhs();
  }
    
  /**
   * 子ノードを設定する
   */
  void set_child(const node_sptr& child) 
  {
    set_rhs(child);
  }
    
  /**
   * 子ノードを得る
   */
  const node_sptr& get_child() const     
  {
    return get_rhs();
  }
};

/**
 * 比較演算子「=」
 */
DEFINE_BINARY_NODE(Equal);

/**
 * 比較演算子「!=」
 */
DEFINE_BINARY_NODE(UnEqual);

/**
 * 比較演算子「<」
 */
DEFINE_ASYMMETRIC_BINARY_NODE(Less);

/**
 * 比較演算子「<=」
 */
DEFINE_ASYMMETRIC_BINARY_NODE(LessEqual);

/**
 * 比較演算子「>」
 */
DEFINE_ASYMMETRIC_BINARY_NODE(Greater);

/**
 * 比較演算子「>=」
 */
DEFINE_ASYMMETRIC_BINARY_NODE(GreaterEqual);


/**
 * 算術演算子「+」
 */
DEFINE_BINARY_NODE(Plus);


/**
 * 算術演算子「-」
 */

DEFINE_ASYMMETRIC_BINARY_NODE(Subtract);

/**
 * 算術演算子「*」
 */

DEFINE_BINARY_NODE(Times);

/**
 * 算術演算子「/」
 */
DEFINE_ASYMMETRIC_BINARY_NODE(Divide);

/**
 * 算術演算子「**」 「^」
 */
DEFINE_ASYMMETRIC_BINARY_NODE(Power);


/**
 * 論理演算子「/\」（連言）
 */
DEFINE_BINARY_NODE(LogicalAnd);

/**
 * 論理演算子「\/」（選言）
 */
DEFINE_BINARY_NODE(LogicalOr);

/**
 * 制約階層定義演算子
 * 並列制約「,」
 */ 
DEFINE_BINARY_NODE(Parallel);

/**
 * 制約階層定義演算子
 * 弱制約「<<」
 */ 
DEFINE_ASYMMETRIC_BINARY_NODE(Weaker);


/**
 * 時相演算子「[]」(Always)
 */
DEFINE_UNARY_NODE(Always);

/**
 * 算術単項演算子「+」
 */
DEFINE_UNARY_NODE(Positive);

/**
 * 算術単項演算子「-」
 */
DEFINE_UNARY_NODE(Negative);

/**
 * 微分「'」
 */
DEFINE_UNARY_NODE(Differential);


/**
 * 左極限「-」
 */
DEFINE_UNARY_NODE(Previous);


/**
 * 否定「!」
 */
DEFINE_UNARY_NODE(Not);

/**
 * 円周率
 */
class Pi : public FactorNode {
public:
  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual node_sptr clone()
  {
    boost::shared_ptr<Pi> n(new Pi());
    return n;
  }  
  virtual std::string get_node_type_name() const {
    return "Pi";
  }
};


/**
 * 自然対数の底
 */
class E : public FactorNode {
public:
  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual node_sptr clone()
  {
    boost::shared_ptr<E> n(new E());
    return n;
  }
  virtual std::string get_node_type_name() const {
    return "E";
  }
};


/**
 * 数字
 */ 
class Number : public FactorNode {
public:
  Number()
  {}  
  
  Number(const std::string& number) : 
    number_(number)
  {}
    
  virtual ~Number()
  {}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual bool is_same_struct(const Node& n, bool exactly_same) const;

  virtual node_sptr clone()
  {
    boost::shared_ptr<Number> n(new Number());
    n->number_ = number_;
    return n;
  }
  
  virtual std::string get_node_type_name() const {
    return "Number";
  }

  virtual std::ostream& dump(std::ostream& s) const 
  {
    Node::dump(s);
    return s <<"[" << number_ << "]";
  }
  
  void set_number(const std::string& number) 
  {
    number_ = number;
  }
  
  std::string get_number() const
  {
    return number_;
  }

private:
  std::string number_;
};

class False : public FactorNode{
public:
  False()
  {}
  
  virtual ~False(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual bool is_same_struct(const Node& n, bool exactly_same) const;
  
  virtual node_sptr clone()
  {
    boost::shared_ptr<False> n(new False());
    return n;
  }
    
  virtual std::string get_node_type_name() const {
    return "False";
  }
};
 
class True : public FactorNode{
public:
  True()
  {}
  
  virtual ~True(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual bool is_same_struct(const Node& n, bool exactly_same) const;
  
  virtual node_sptr clone()
  {
    boost::shared_ptr<True> n(new True());
    return n;
  }
    
  virtual std::string get_node_type_name() const {
    return "True";
  }
};
  
/**
 * 変数
 * 従属変数の場合もあり
 */ 
class Variable : public FactorNode {
public:
  Variable()
  {} 
  
  Variable(const std::string& name) : 
    name_(name)
  {}

  virtual ~Variable(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual bool is_same_struct(const Node& n, bool exactly_same) const;
  
  virtual node_sptr clone()
  {
    boost::shared_ptr<Variable> n(new Variable());
    n->name_ = name_;
    return n;
  }
    
  virtual std::string get_node_type_name() const {
    return "Variable";
  }

  virtual std::ostream& dump(std::ostream& s) const 
  {
    Node::dump(s);
    return s <<"[" << name_ << "]";
  }
  

  void set_name(const std::string& name) 
  {
    name_ = name;
  }

  std::string get_name() const
  {
    return name_;
  }


private:
  std::string name_;
};

/**
 * 記号定数
 */ 
class Parameter : public FactorNode {
public:
  Parameter(const std::string& name, const int& derivative_count, const int& id) : 
    name_(name), derivative_count_(derivative_count), phase_id_(id)
  {}
    
  virtual ~Parameter(){}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual bool is_same_struct(const Node& n, bool exactly_same) const;

  virtual node_sptr clone()
  {
    boost::shared_ptr<Parameter> n(new Parameter(name_, derivative_count_, phase_id_));
    return n;
  }

  virtual std::string get_node_type_name() const {
    return "Parameter";
  }

  virtual std::ostream& dump(std::ostream& s) const 
  {
    Node::dump(s);
    return s <<"[" << name_ << "]";
  }

  void set_name(const std::string& name) 
  {
    name_ = name;
  }

  std::string get_name() const
  {
    return name_;
  }

  
  void set_derivative_count(const int& dc)
  {
    derivative_count_ = dc;
  }

  
  int get_derivative_count() const
  {
    return derivative_count_;
  }

  
  void set_phase_id(const int& id)
  {
    phase_id_ = id;
  }

  
  int get_phase_id() const
  {
    return phase_id_;
  }


private:
  Parameter()
  {} 
  
  std::string name_;
  int derivative_count_;
  int phase_id_;
};



/**
 * 正の無限大を表すノード
 */

class Infinity : public FactorNode {
public:
  Infinity()
  {}
    
  virtual ~Infinity()
  {}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual node_sptr clone()
  {
    boost::shared_ptr<Infinity> n(new Infinity());
    return n;
  }
  
  virtual std::string get_node_type_name() const {
    return "Infinity";
  }

  virtual std::ostream& dump(std::ostream& s) const 
  {
    Node::dump(s);
    return s;
  }
};

/**
 * ｔ（時刻）を表すノード．変数の時刻に対する式の中に出現するやつ．数式処理用
 */

class SymbolicT : public FactorNode {
public:
  SymbolicT()
  {}
    
  virtual ~SymbolicT()
  {}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual node_sptr clone()
  {
    boost::shared_ptr<SymbolicT> n(new SymbolicT());
    return n;
  }
  
  virtual std::string get_node_type_name() const {
    return "SymbolicT";
  }

  virtual std::ostream& dump(std::ostream& s) const 
  {
    Node::dump(s);
    return s;
  }
};

class Print : public FactorNode{
public:
  
  Print()
  {}
  
  virtual ~Print()
  {}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual bool is_same_struct(const Node& n, bool exactly_same) const;

  virtual node_sptr clone()
  {
    boost::shared_ptr<Print> n(new Print());
    n->string_ = string_;
    n->args_ = args_;
    return n;
  }
  
  virtual std::string get_node_type_name() const {
    return "Print";
  }

  virtual std::ostream& dump(std::ostream& s) const 
  {
    Node::dump(s);
    return s <<"[" << string_ << "]";
  }
  
  void set_string(const std::string& str) 
  {
    string_ = str;
  }
  
  std::string get_string() const
  {
    return string_;
  }
 
  void set_args(const std::string& str) 
  {
    args_ = str;
  }
  
  std::string get_args() const
  {
    return args_;
  }


private:
  std::string string_;
  std::string args_;
};


class IONode : public FactorNode{
public:
  
  IONode()
  {}
  
  virtual ~IONode()
  {}
  
  void set_string(const std::string& str) 
  {
    string_ = str;
  }
  
  std::string get_string() const
  {
    return string_;
  }
 
  void set_args(const std::string& str) 
  {
    args_ = str;
  }
  
  std::string get_args() const
  {
    return args_;
  }

protected:
  std::string string_;
  std::string args_;
};


class PrintPP : public IONode{
public:
  
  PrintPP()
  {}
  
  virtual ~PrintPP()
  {}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual bool is_same_struct(const Node& n, bool exactly_same) const;

  virtual node_sptr clone()
  {
    boost::shared_ptr<PrintPP> n(new PrintPP());
    n->string_ = string_;
    n->args_ = args_;
    return n;
  }
  
  virtual std::string get_node_type_name() const {
    return "PrintPP";
  }

  virtual std::ostream& dump(std::ostream& s) const 
  {
    Node::dump(s);
    return s <<"[" << string_ << "]";
  }
};

class PrintIP : public IONode{
public:
  
  PrintIP()
  {}
  
  virtual ~PrintIP()
  {}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual bool is_same_struct(const Node& n, bool exactly_same) const;

  virtual node_sptr clone()
  {
    boost::shared_ptr<PrintIP> n(new PrintIP());
    n->string_ = string_;
    n->args_ = args_;
    return n;
  }
  
  virtual std::string get_node_type_name() const {
    return "PrintIP";
  }

  virtual std::ostream& dump(std::ostream& s) const 
  {
    Node::dump(s);
    return s <<"[" << string_ << "]";
  }
};

class Scan : public IONode{
public:
  
  Scan()
  {}
  
  virtual ~Scan()
  {}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual bool is_same_struct(const Node& n, bool exactly_same) const;

  virtual node_sptr clone()
  {
    boost::shared_ptr<Scan> n(new Scan());
    n->string_ = string_;
    n->args_ = args_;
    return n;
  }
  
  virtual std::string get_node_type_name() const {
    return "Scan";
  }

  virtual std::ostream& dump(std::ostream& s) const 
  {
    Node::dump(s);
    return s <<"[" << args_ << "]";
  }
};
    
class Exit : public FactorNode{
public:
  
  Exit()
  {}
  
  virtual ~Exit()
  {}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual bool is_same_struct(const Node& n, bool exactly_same) const;

  virtual node_sptr clone()
  {
    boost::shared_ptr<Exit> n(new Exit());
    n->string_ = string_;
    n->args_ = args_;
    return n;
  }
  
  virtual std::string get_node_type_name() const {
    return "Exit";
  }

  virtual std::ostream& dump(std::ostream& s) const 
  {
    Node::dump(s);
    return s <<"[" << string_ << "]";
  }
  
  void set_string(const std::string& str) 
  {
    string_ = str;
  }
  
  std::string get_string() const
  {
    return string_;
  }
 
  void set_args(const std::string& str) 
  {
    args_ = str;
  }
  
  std::string get_args() const
  {
    return args_;
  }


private:
  std::string string_;
  std::string args_;
};

class Abort : public FactorNode{
public:
  
  Abort()
  {}
  
  virtual ~Abort()
  {}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual bool is_same_struct(const Node& n, bool exactly_same) const;

  virtual node_sptr clone()
  {
    boost::shared_ptr<Abort> n(new Abort());
    n->string_ = string_;
    n->args_ = args_;
    return n;
  }
  
  virtual std::string get_node_type_name() const {
    return "Abort";
  }

  virtual std::ostream& dump(std::ostream& s) const 
  {
    Node::dump(s);
    return s <<"[" << string_ << "]";
  }
  
  void set_string(const std::string& str) 
  {
    string_ = str;
  }
  
  std::string get_string() const
  {
    return string_;
  }
 
  void set_args(const std::string& str) 
  {
    args_ = str;
  }
  
  std::string get_args() const
  {
    return args_;
  }


private:
  std::string string_;
  std::string args_;
};

/**
 * システム変数のノード
 */
class SystemVariableNode : public FactorNode{
public:
  
  SystemVariableNode()
  {}
  
  virtual ~SystemVariableNode()
  {}
  
};

/**
 * timer変数　
 * シミュレーション開始からの時刻を保持しているシステム変数
 */
class SVtimer : public SystemVariableNode{
public:
  SVtimer()
  {}

  virtual ~SVtimer()
  {}

  virtual void accept(node_sptr own, TreeVisitor* visitor);

  virtual node_sptr clone()
  {
    boost::shared_ptr<SVtimer> n(new SVtimer());
    return n;
  }

  virtual std::string get_node_type_name() const {
    return "$timer";
  }

  virtual std::ostream& dump(std::ostream& s) const {
    Node::dump(s);
    return s;
  }
};

/**
 * 任意個の引数を持つノード
 */

class ArbitraryNode : public Node {
public:
  typedef boost::shared_ptr<ArbitraryNode> node_type_sptr;

  ArbitraryNode()
  {}

  virtual ~ArbitraryNode()
  {}

  virtual void accept(node_sptr own, BaseNodeVisitor* visitor);
  virtual void accept(node_sptr own, TreeVisitor* visitor) = 0;

  virtual std::string get_node_type_name() const {
    return "ArbitraryNode";
  }
  
  void add_argument(node_sptr node);
  void set_argument(node_sptr node, int i);
  
  virtual std::ostream& dump(std::ostream& s) const;
  
  int get_arguments_size();
  virtual std::string get_string() const = 0;
  node_sptr get_argument(int number);
  
  protected:
  std::vector<node_sptr> arguments_;
};



/*
 * 関数
 */

class Function : public ArbitraryNode {
public:
  typedef boost::shared_ptr<Function> node_type_sptr;

  Function()
  {}
  
  Function(const std::string& str) : 
    string_(str)
  {}

  virtual ~Function()
  {}

  virtual void accept(node_sptr own, TreeVisitor* visitor);
  
  virtual node_sptr clone();

  virtual std::string get_node_type_name() const {
    return "Function";
  }
  
  void set_string(const std::string& str){string_ = str;}
  
  virtual std::string get_string() const{return string_;}

private:
  std::string string_;
};


/*
 * サポート外関数．
 * ソルバ間差異などに対応できないが，一応ソルバ特有の関数を任意で呼び出せるようにしておく．
 * 動作保証はできない．
 */

class UnsupportedFunction : public ArbitraryNode {
public:
  typedef boost::shared_ptr<UnsupportedFunction> node_type_sptr;

  UnsupportedFunction()
  {}
  
  UnsupportedFunction(const std::string& str) : 
    string_(str)
  {}

  virtual ~UnsupportedFunction()
  {}

  virtual void accept(node_sptr own, TreeVisitor* visitor);
  
  virtual node_sptr clone();

  virtual std::string get_node_type_name() const {
    return "UnsupportedFunction";
  }
  
  
  void set_string(const std::string& str){string_ = str;}
  
  virtual std::string get_string() const{return string_;}

private:
  std::string string_;
};


} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_NODE_H_
