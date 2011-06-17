
#include <cassert>
#include <boost/algorithm/string/predicate.hpp>

#include "REDUCEVCSPoint.h"

// TODO RTreeVisitor‚Ìˆø‚Á‰z‚µ
//#include "../../parser/RTreeVisitor.h"
#include "REDUCEStringSender.h"

/*
#include "mathlink_helper.h"
#include "PacketErrorHandler.h"
#include "PacketChecker.h"
 */

#include "../mathematica/MathematicaExpressionConverter.h"

#include "Logger.h"

using namespace hydla::vcs;
using namespace hydla::parse_tree;
using namespace hydla::logger;

namespace hydla {
namespace vcs {
namespace reduce {

//REDUCEVCSPoint::REDUCEVCSPoint(MathLink* ml) :ml_(ml)
REDUCEVCSPoint::REDUCEVCSPoint(REDUCELink* cl) :
  cl_(cl)
{
  std::cout << "Begin REDUCEVCSPoint::REDUCEVCSPoint(REDUCEClient* cl)" << std::endl;

}

REDUCEVCSPoint::~REDUCEVCSPoint()
{
  std::cout << "Begin REDUCEVCSPoint::~REDUCEVCSPoint()" << std::endl;

}

// MathematicaVCSPoint‚æ‚è
bool REDUCEVCSPoint::reset()
{
  std::cout << "Begin REDUCEVCSPoint::reset()" << std::endl;
  // TODO: ƒ`ƒ‡ƒCl‚¦‚é
  assert(0);
  //   constraint_store_.first.clear();
  //   constraint_store_.second.clear();
  return true;
}

// MathematicaVCSPoint‚æ‚è
bool REDUCEVCSPoint::reset(const variable_map_t& variable_map)
{

  HYDLA_LOGGER_VCS_SUMMARY("#*** Reset Constraint Store ***");

  if(variable_map.size() == 0)
  {
    HYDLA_LOGGER_VCS_SUMMARY("no Variables");
    return true;
  }
  HYDLA_LOGGER_VCS_SUMMARY("------Variable map------\n", variable_map);

  std::set<MathValue> and_cons_set;

  MathematicaExpressionConverter mec;

  variable_map_t::variable_list_t::const_iterator it =
    variable_map.begin();
  variable_map_t::variable_list_t::const_iterator end =
    variable_map.end();
  for(; it!=end; ++it)
  {
    const MathVariable& variable = (*it).first;
    const value_t&    value = it->second;

    if(!value.is_undefined()) {
      std::ostringstream val_str;
      val_str << "Equal[";

      if(variable.derivative_count > 0)
      {
        val_str << "Derivative["
                << variable.derivative_count
                << "][prev["
                << PacketSender::var_prefix
                << variable.name
                << "]]";
      }
      else
      {
        val_str << "prev["
                << PacketSender::var_prefix
                << variable.name
                << "]";
      }

      val_str << ","
              << mec.convert_symbolic_value_to_math_string(value)
              << "]"; // Equal‚Ì•Â‚¶Š‡ŒÊ

      MathValue new_math_value;
      new_math_value.set(val_str.str());
      and_cons_set.insert(new_math_value);

      // §–ñƒXƒgƒA“à‚Ì•Ï”ˆê——‚ğì¬
      constraint_store_.second.insert(
        boost::make_tuple(variable.name,
                          variable.derivative_count,
                          true));
    }
  }

  constraint_store_.first.insert(and_cons_set);

  HYDLA_LOGGER_VCS(*this);

  return true;
}

// MathematicaVCSPoint‚æ‚è
bool REDUCEVCSPoint::reset(const variable_map_t& variable_map, const parameter_map_t& parameter_map){
//  assert(0);
//  return false;
  if(!reset(variable_map)){
    return false;
  }

  if(parameter_map.size() == 0)
  {
    HYDLA_LOGGER_VCS_SUMMARY("no Parameters");
    return true;
  }
  HYDLA_LOGGER_VCS_SUMMARY("------Parameter map------\n", parameter_map);


  std::set<MathValue> and_cons_set;
  par_names_.clear();

  parameter_map_t::variable_list_t::const_iterator it =
    parameter_map.begin();
  parameter_map_t::variable_list_t::const_iterator end =
    parameter_map.end();
  for(; it!=end; ++it)
  {
    const value_range_t&    value = it->second;
    if(!value.is_undefined()) {
      value_range_t::or_vector::const_iterator or_it = value.or_begin(), or_end = value.or_end();
      for(;or_it != or_end; or_it++){
        value_range_t::and_vector::const_iterator and_it = or_it->begin(), and_end = or_it->end();
        for(; and_it != and_end; and_it++){
          std::ostringstream val_str;

          // MathVariable‘¤‚ÉŠÖ‚·‚é•¶š—ñ‚ğì¬
          val_str << MathematicaExpressionConverter::get_relation_math_string(and_it->relation) << "[" ;

          val_str << PacketSender::par_prefix
                  << it->first.get_name();

          val_str << ","
                  << and_it->value
                  << "]"; // •Â‚¶Š‡ŒÊ
          MathValue new_math_value;
          new_math_value.set(val_str.str());
          and_cons_set.insert(new_math_value);
          par_names_.insert(it->first.get_name());
        }
      }
    }
  }
  parameter_store_.first.insert(and_cons_set);
  return true;
}
//TODO ’è”•Ô‚µ‚ÌC³
bool REDUCEVCSPoint::create_variable_map(variable_map_t& variable_map, parameter_map_t& parameter_map)
{
  assert(0);
  return false;
}

//TODO ’è”•Ô‚µ‚ÌC³
void REDUCEVCSPoint::create_max_diff_map(
    PacketSender& ps, max_diff_map_t& max_diff_map)
{
  assert(0);
}

//TODO ’è”•Ô‚µ‚ÌC³
void REDUCEVCSPoint::add_left_continuity_constraint(
    PacketSender& ps, max_diff_map_t& max_diff_map)
{
  assert(0);
}

//TODO ’è”•Ô‚µ‚ÌC³
VCSResult REDUCEVCSPoint::add_constraint(const tells_t& collected_tells, const appended_asks_t &appended_asks)
{

  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSPoint::add_constraint ***");
  REDUCEStringSender rss = REDUCEStringSender(*cl_);


//////////////////// ‘—Mˆ—

  // send_string‚Ìstring‚Í‚Ç‚Ì‚æ‚¤‚É‹æØ‚Á‚Ä‘—M‚µ‚Ä‚àOK

  // tell§–ñ‚ÌW‡‚©‚çexpr‚ğ“¾‚ÄREDUCE‚É“n‚·
  std::cout << "collected_tells" << std::endl;
  cl_->send_string("expr_:={");
//  cl_->send_string("expr_:={df(y,t,2) = -10,");
//  cl_->send_string("y = 10, df(y,t,1) = 0, prev(y) = y, df(prev(y),t,1) = df(y,t,1)};");

  tells_t::const_iterator tells_it  = collected_tells.begin();
  tells_t::const_iterator tells_end = collected_tells.end();
  for(; tells_it!=tells_end; ++tells_it) {
    if(tells_it != collected_tells.begin()) cl_->send_string(",");
    HYDLA_LOGGER_VCS("put node: ", *(*tells_it)->get_child());
//    std::cout << rtv.get_expr((*tells_it)->get_child()) << std::endl;
    rss.put_node((*tells_it)->get_child());
  }
  cl_->send_string("};");


  // appended_asks‚©‚çƒK[ƒh•”•ª‚ğ“¾‚ÄREDUCE‚É“n‚·
  std::cout << "appended_asks" << std::endl;
  cl_->send_string("pexpr_:={");
//  cl_->send_string("{}");

  appended_asks_t::const_iterator append_it  = appended_asks.begin();
  appended_asks_t::const_iterator append_end = appended_asks.end();
  for(; append_it!=append_end; ++append_it) {
    if(append_it != appended_asks.begin()) cl_->send_string(",");
    HYDLA_LOGGER_VCS("put node (guard): ", *(append_it->ask->get_guard()), "  entailed:", append_it->entailed);
    rss.put_node(append_it->ask->get_child());
  }
  cl_->send_string("};");


  // vars‚ğ“n‚·
  // vars_‚ÉŠÖ‚µ‚Äˆê”ÔŠO‘¤‚Ì"{}"•”•ª‚ÍAput_vars“à‚Å‘—‚Á‚Ä‚¢‚é
  cl_->send_string("vars_:=");
//  cl_->send_string("vars_:={y, prev(y), df(y,t,1), df(prev(y),t,1), df(y,t,2), y, prev(y), df(y,t,1), df(prev(y),t,1)};");
  rss.put_vars();
  cl_->send_string(";");


  cl_->send_string("symbolic redeval '(isconsistent vars_ pexpr_ expr_);");


/////////////////// óMˆ—
  HYDLA_LOGGER_VCS("--- receive ---");

  cl_->read_until_redeval();

  std::string ans = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("add_constraint_ans: ",
                   ans);


  // VCSR_FALSEŒãI—¹
  HYDLA_LOGGER_VCS("#*** End REDUCEVCSPoint::add_constraint ***");

  assert(0);

  return VCSR_FALSE;

}

//TODO ’è”•Ô‚µ‚ÌC³
VCSResult REDUCEVCSPoint::check_entailment(const ask_node_sptr& negative_ask, const appended_asks_t &appended_asks)
{

  assert(0);

  return VCSR_FALSE;
}

//TODO ’è”•Ô‚µ‚ÌC³
VCSResult REDUCEVCSPoint::integrate(
    integrate_result_t& integrate_result,
    const positive_asks_t& positive_asks,
    const negative_asks_t& negative_asks,
    const time_t& current_time,
    const time_t& max_time,
    const not_adopted_tells_list_t& not_adopted_tells_list,
    const appended_asks_t& appended_asks)
{

  assert(0);
  return VCSR_FALSE;
}

//TODO ’è”•Ô‚µ‚ÌC³
void REDUCEVCSPoint::send_cs() const
{
  assert(0);
}

//TODO ’è”•Ô‚µ‚ÌC³
void REDUCEVCSPoint::send_ps() const
{
  assert(0);
}

//TODO ’è”•Ô‚µ‚ÌC³
void REDUCEVCSPoint::send_pars() const{
  assert(0);
}

//TODO ’è”•Ô‚µ‚ÌC³
void REDUCEVCSPoint::send_cs_vars() const
{
   assert(0);
}

// MathematicaVCSPoint‚æ‚è
std::ostream& REDUCEVCSPoint::dump(std::ostream& s) const
{
  s << "#*** Dump REDUCEVCSPoint ***\n"
      << "--- constraint store ---\n";

  //
  std::set<std::set<MathValue> >::const_iterator or_cons_it =
      constraint_store_.first.begin();
  while((or_cons_it) != constraint_store_.first.end())
  {
    std::set<MathValue>::const_iterator and_cons_it =
        (*or_cons_it).begin();
    while((and_cons_it) != (*or_cons_it).end())
    {
      s << (*and_cons_it).get_string() << " ";
      and_cons_it++;
    }
    s << "\n";
    or_cons_it++;
  }

  // §–ñƒXƒgƒA“à‚É‘¶İ‚·‚é•Ï”‚Ìƒ_ƒ“ƒv
  s << "-- vars --\n";
  constraint_store_vars_t::const_iterator vars_it =
      constraint_store_.second.begin();
  while((vars_it) != constraint_store_.second.end())
  {
    s << *(vars_it) << "\n";
    vars_it++;
  }

  return s;
}

// MathematicaVCSPoint‚æ‚è
std::ostream& operator<<(std::ostream& s, const REDUCEVCSPoint& m)
{
  return m.dump(s);
}


} // namespace reduce
} // namespace simulator
} // namespace hydla

