#include "MathematicaVCS.h"

#include <cassert>

#include "MathematicaVCSPoint.h"
#include "MathematicaVCSInterval.h"
#include "MathematicaExpressionConverter.h"
#include "PacketSender.h"
#include "PacketChecker.h"

using namespace hydla::vcs;
using namespace hydla::parse_tree;

namespace hydla {
namespace vcs {
namespace mathematica {

void MathematicaVCS::change_mode(hydla::symbolic_simulator::Mode m, int approx_precision)
{
  mode_ = m;
  switch(m) {
    case hydla::symbolic_simulator::DiscreteMode:
      vcs_.reset(new MathematicaVCSPoint(&ml_));
      break;

    case hydla::symbolic_simulator::ContinuousMode:
      vcs_.reset(new MathematicaVCSInterval(&ml_, approx_precision));
      break;

    default:
      assert(0);//assert����
  }
}

MathematicaVCS::MathematicaVCS(const hydla::symbolic_simulator::Opts &opts)
{
  
  HYDLA_LOGGER_DEBUG("#*** init mathlink ***");
  //std::cout << opts.mathlink << std::endl;

  //TODO: ��O�𓊂���悤�ɂ���
  if(!ml_.init(opts.mathlink.c_str())) {
    std::cerr << "can not link" << std::endl;
    exit(-1);
  }

  // �o�͂����ʂ̉����̐ݒ�
  ml_.MLPutFunction("SetOptions", 2);
  ml_.MLPutSymbol("$Output"); 
  ml_.MLPutFunction("Rule", 2);
  ml_.MLPutSymbol("PageWidth"); 
  ml_.MLPutSymbol("Infinity"); 
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // �f�o�b�O�v�����g
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optUseDebugPrint"); 
  ml_.MLPutSymbol(opts.debug_mode ? "True" : "False");
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // �v���t�@�C�����[�h
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optUseProfile"); 
  ml_.MLPutSymbol(opts.profile_mode ? "True" : "False");
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // ���񃂁[�h
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optParallel"); 
  ml_.MLPutSymbol(opts.parallel_mode ? "True" : "False");
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // �o�͌`��
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optOutputFormat"); 
  switch(opts.output_format) {
    case hydla::symbolic_simulator::fmtTFunction:
      ml_.MLPutSymbol("fmtTFunction");
      break;

    case hydla::symbolic_simulator::fmtNumeric:
    default:
      ml_.MLPutSymbol("fmtNumeric");
      break;
  }
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  ml_.MLPutFunction("ToExpression", 1);
  ml_.MLPutString(vcs_math_source());  
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();
  
  MathematicaExpressionConverter::initialize();
}

MathematicaVCS::~MathematicaVCS()
{}

bool MathematicaVCS::reset(const variable_map_t& variable_map, const parameter_map_t& parameter_map)
{
  HYDLA_LOGGER_VCS("#*** MathematicaVCS::reset ***\n");
  

  ml_.put_function("resetConstraint", 0);
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  ml_.put_function("addConstraint", 4);
  PacketSender ps(ml_);
  {
    constraints_t constraints;
    HYDLA_LOGGER_VCS("------Variable map------\n", variable_map);
    variable_map_t::variable_list_t::const_iterator it = 
      variable_map.begin();
    variable_map_t::variable_list_t::const_iterator end = 
      variable_map.end();
    //��ɑ��M����K�v�̂��鐧��̌��𐔂���
    for(; it!=end; ++it){
      if(!it->second.is_undefined()){
        constraints.push_back(MathematicaExpressionConverter::make_equal(it->first, it->second.get_node(), true));
      }else if(mode_ == hydla::symbolic_simulator::ContinuousMode){
        // �l���Ȃ��Ȃ牽������̒萔������đ��M�D
        std::string name;
        name = PacketSender::par_prefix + it->first.name;
        for(int i=0;i<it->first.derivative_count;i++){
          //�Ƃ肠���������񐔕�d������
          name.append("d");
        }
        while(parameter_map.has_variable(parameter_t(name))){
          //�Ƃ肠�����d���񐔕�i������
          name.append("i");
        }
        constraints.push_back(MathematicaExpressionConverter::make_equal(it->first, node_sptr(new Parameter(name)), true));
      }
    }
    HYDLA_LOGGER_VCS_SUMMARY("size:", constraints.size());
    ps.put_nodes(constraints, PacketSender::VA_None);
  }
  ps.put_vars(PacketSender::VA_None);
  
  {
    constraints_t constraints;
    HYDLA_LOGGER_VCS_SUMMARY("------Parameter map------\n", parameter_map);

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
            node_sptr rel = MathematicaExpressionConverter::get_relation_node(and_it->relation, node_sptr(new Parameter(it->first.get_name())), and_it->get_value().get_node());
            constraints.push_back(rel);
          }
        }
      }
    }
    HYDLA_LOGGER_VCS_SUMMARY("size:", constraints.size());
    ps.put_nodes(constraints, PacketSender::VA_None);
  }
  ps.put_pars();
  

/////////////////// ��M����
  //PacketChecker pc(ml_);
  //pc.check();
///////////////////

  HYDLA_LOGGER_EXTERN("-- math debug print -- \n");
  
  HYDLA_LOGGER_EXTERN( (ml_.skip_pkt_until(TEXTPKT), ml_.get_string()) );
  HYDLA_LOGGER_EXTERN( (ml_.skip_pkt_until(TEXTPKT), ml_.get_string()) );
  
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  HYDLA_LOGGER_VCS("#*** END MathematicaVCS::reset ***\n");
  return true;
}

void MathematicaVCS::set_continuity(const continuity_map_t& continuity_map){
  vcs_->set_continuity(continuity_map);
}

bool MathematicaVCS::create_maps(create_result_t &create_result)
{
  return vcs_->create_maps(create_result);
}

void MathematicaVCS::add_constraint(const constraints_t& constraints)
{
  
  PacketSender::VariableArg arg = (mode_==hydla::symbolic_simulator::DiscreteMode)?PacketSender::VA_None:PacketSender::VA_Time;
  HYDLA_LOGGER_VCS(
    "#*** Begin MathematicaVCS::add_constraint ***");

  ml_.put_function("addConstraint", 2);
  
  
  PacketSender ps(ml_);

  ps.put_nodes(constraints, arg);
  // vars��n��
  ps.put_vars(arg);

/////////////////// ��M����
  HYDLA_LOGGER_VCS( "--- receive ---");

  HYDLA_LOGGER_EXTERN(
    "-- math debug print -- \n",
    (ml_.skip_pkt_until(TEXTPKT), ml_.get_string()));
///////////////////

  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();
  
  HYDLA_LOGGER_VCS("\n#*** End MathematicaVCS::add_constraint ***");
  return;
}


VCSResult MathematicaVCS::check_consistency()
{
  return vcs_->check_consistency();
}


VCSResult MathematicaVCS::check_consistency(const constraints_t& constraints)
{
  return vcs_->check_consistency(constraints);
}

VCSResult MathematicaVCS::integrate(
  integrate_result_t& integrate_result,
  const constraints_t &constraints,
  const time_t& current_time,
  const time_t& max_time)
{
  return vcs_->integrate(integrate_result, 
                         constraints,
                         current_time, 
                         max_time);
}


void MathematicaVCS::apply_time_to_vm(const variable_map_t& in_vm, 
                                              variable_map_t& out_vm, 
                                              const time_t& time)
{
  HYDLA_LOGGER_VCS("--- apply_time_to_vm ---");

  PacketSender ps(ml_);

  variable_map_t::const_iterator it  = in_vm.begin();
  variable_map_t::const_iterator end = in_vm.end();
  for(; it!=end; ++it) {
    HYDLA_LOGGER_VCS("variable : ", it->first);

    // �l
    value_t    value;
    if(!it->second.is_undefined()) {
      ml_.put_function("applyTime2Expr", 2);
      ps.put_node(it->second.get_node(), PacketSender::VA_Time);
      ps.put_node(time.get_node(), PacketSender::VA_None);

    ////////////////// ��M����

      HYDLA_LOGGER_OUTPUT(
        "-- math debug print -- \n",
        (ml_.skip_pkt_until(TEXTPKT), ml_.get_string()));  

      ml_.skip_pkt_until(RETURNPKT);
      ml_.MLGetNext();ml_.MLGetNext();ml_.MLGetNext();

      int ret_code = ml_.get_integer();
      if(ret_code==0) {
        // TODO: �K�؂ȏ���������
        assert(0);
      }
      else {
        assert(ret_code==1);
        std::string tmp = ml_.get_string();
        MathematicaExpressionConverter mec;
        value = mec.convert_math_string_to_symbolic_value(tmp);
        HYDLA_LOGGER_OUTPUT("value : ", value.get_string());
      }
    }
    out_vm.set_variable(it->first, value);   
  }
}

//�ۂ߂��s���֐� get_real_val()�Ŏg��
std::string upward(std::string str){
  if(str.at(str.length()-1) == '.'){
    str = upward(str.substr(0,str.length()-1)) + '.';
  }else if(str.at(str.length()-1) != '9'){
    str = str.substr(0,str.length()-1)+(char)(str.at(str.length()-1)+1);
  }else if(str == "9"){
    str = "10";   
  }else {
    str = upward(str.substr(0,str.length()-1)) + '0';
  }
  return str;
}

//value_t���w�肳�ꂽ���x�Ő��l�ɕϊ�����
  std::string MathematicaVCS::get_real_val(const value_t &val, int precision, symbolic_simulator::OutputFormat opfmt){
  std::string ret;
  PacketSender ps(ml_);

  if(!val.is_undefined() && opfmt == symbolic_simulator::fmtNInterval) {
    
    //precision��2���傫���Ƃ���
    if(precision<2) precision=2;

    ml_.put_function("ToString", 2);
    ml_.put_function("Interval", 1);    
    ml_.put_function("N", 2);  
    ps.put_node(val.get_node(), PacketSender::VA_None);
    ml_.put_integer(precision+5);     
    ml_.put_symbol("InputForm");
    ml_.skip_pkt_until(RETURNPKT);
    ret = ml_.get_string();

    //usrVar�̏ꍇ�͂��̂܂�
    if(ret.find(PacketSender::var_prefix) != -1) 
      return ret.substr(ret.find("[")+1,ret.find("]")-ret.find("[")-1);

    //parameter������ꍇ�̑Ή� ex:Interval[1.6666666666666667`5.*pa]   Interval[pa]�i�s���S�j
    //TODO:Interval[-5.`5.*(-6.`5. + pa)]�Ȃǂւ̑Ή� ./hydla examples/sawtooth_wave_param.hydla -m s -t 10
    std::string parameter = "";
    if(ret.find("{") == -1){
      parameter = ret.substr(ret.find("[")+1,ret.find("]")-ret.find("[")-1);
      if(parameter.find("`") == -1) {
	return parameter;
      }else{
	//�����ł���`���ɕό`
	ret = "Interval[{"+parameter.substr(0,precision+5)+", "+parameter.substr(0,precision+5)+"}]";
	parameter = parameter.substr(parameter.find_last_of(".")+1);
      }
    }

    int loc;
    int pre = precision;
    char sign = 'p';
    std::string lower = "";
    std::string upper = "";
    //�w�肳�ꂽ���x�ɍ��킹��
    if(ret.find("-")!=-1) {
      ret.erase(ret.find("-"),1);
      if(ret.find("-")!=-1) {
	ret.erase(ret.find("-"),1);
	//���̋�� ��[�Ɖ��[�̂ǂ�����ۂ߂邩
	sign = 'n';
      }else{
	//[��,��]�̋��
	sign = 'c';
      }
    }
    //���[�����o��
    if(ret.find("}")-ret.find("{")-2>precision*2){
      for(int i=ret.find("{")+1;i<ret.find(",")-1;i++){
	if(ret.at(i)!='0' &&  ret.at(i)!='.') {
	  if(ret.substr(ret.find("{")+1,pre).find(".")!=-1) pre++;
	  lower = ret.substr(ret.find("{")+1,pre);
	  break;
	}else if(ret.at(i) == '0') pre++;
      }
      pre = precision;
      //��[�����o��
      for(int i=ret.find(",")+2;i<ret.length();i++){
	if(ret.at(i)!='0' &&  ret.at(i)!='.') {
	  if(ret.substr(ret.find(",")+2,pre).find(".")!=-1) pre++;
	  upper = ret.substr(ret.find(",")+2,pre);
	  break;
	}else if(ret.at(i) == '0') pre++;
      }

      //precision�{�P���ڂ����[�͐؂�̂āA��[�͐؂�グ
      switch (sign) {
      case 'n' : lower = upward(lower); break;
      case 'p' : upper = upward(upper); break;
      case 'c' : lower = "-" + upward(lower); upper = upward(upper); break;
      }
    
    }else{
      //ex:[0,0]�Ȃǂ̏ꍇ�ɑΉ�������
      lower = ret.substr(ret.find("{")+1,ret.find(",")-(ret.find("{")+1));
      upper = ret.substr(ret.find(",")+2,ret.find("}")-(ret.find(",")+2));
    }
    //���w�\�L�@(ex:*10^6)�̑Ή�
    std::string lowex = "";
    std::string upex = "";
    if((loc=ret.find("*^")) != -1){
      lowex = ret.substr(loc,ret.find(",")-loc);
      loc=ret.find_last_of("*^");
      upex = ret.substr(loc-1,lowex.length());
    }
    loc=0;
    
    //��v���Ă��镔�����ȗ����ĕ\������
    //TODO:"[0.9999,1.000]"��"0.999[9,10]"�̂悤�ɕ\��������
    while(true){
      if(lower.at(loc)!=upper.at(loc)) {
	ret = lower.substr(0,loc);
	lower = lower.substr(loc);
	upper = upper.substr(loc);

	if(lowex.compare(upex)==0){
	  ret = ret + "[" + lower + "," + upper + "]" + lowex;
	  break;
	}else ret = ret + "[" + lower + lowex + "," + upper + upex + "]";
	break;
      }
      loc++;
      //�������S�Ĉ�v���Ă���ꍇ�͂ǂ��炩�\��(���肦�Ȃ��͂�)
      if(loc > lower.length()-1){
	ret = lower;
	break;
      }else if(loc > upper.length()-1) {
	ret = upper;
	break;
      }
    }

    if(sign == 'n') ret = "-"+ret;
    ret = ret+parameter;
    
  } else  if(!val.is_undefined()) {
    ml_.put_function("ToString", 2);

    //ml_.put_function("Interval",1);

    ml_.put_function("N", 2);  
    ps.put_node(val.get_node(), PacketSender::VA_None);
    ml_.put_integer(precision);
    ml_.put_symbol("CForm");
    //ml_.put_symbol("InputForm");
    ml_.skip_pkt_until(RETURNPKT);
    ret = ml_.get_string();
  }  else {
    ret = "UNDEF";
  }

  return ret;
}


bool MathematicaVCS::less_than(const time_t &lhs, const time_t &rhs)
{
  ml_.put_function("ToString", 1);  
  ml_.put_function("Less", 2);  
  PacketSender ps(ml_);
  ps.put_node(lhs.get_node(), PacketSender::VA_None);
  ps.put_node(rhs.get_node(), PacketSender::VA_None);
  
  ml_.skip_pkt_until(RETURNPKT);
  std::string ret = ml_.get_string();
  return  ret == "True";
}

void MathematicaVCS::simplify(time_t &time) 
{
  HYDLA_LOGGER_DEBUG("SymbolicTime::send_time : ", time);
  ml_.put_function("ToString", 1);
  ml_.put_function("FullForm", 1);
  ml_.put_function("Simplify", 1);
  PacketSender ps(ml_);
  ps.put_node(time.get_node(), PacketSender::VA_None);
  ml_.skip_pkt_until(RETURNPKT);
  MathematicaExpressionConverter mec;
  time = mec.convert_math_string_to_symbolic_value(ml_.get_string());
}



  //SymbolicValue�̎��Ԃ����炷
hydla::vcs::SymbolicVirtualConstraintSolver::value_t MathematicaVCS::shift_expr_time(const value_t& val, const time_t& time){
  value_t tmp_val;
  ml_.put_function("exprTimeShift", 2);
  PacketSender ps(ml_);
  ps.put_node(val.get_node(), PacketSender::VA_None);
  ps.put_node(time.get_node(), PacketSender::VA_None);
  ml_.skip_pkt_until(RETURNPKT);
  MathematicaExpressionConverter mec;
  tmp_val = mec.convert_math_string_to_symbolic_value(ml_.get_string());
  return  tmp_val;
}


} // namespace mathematica
} // namespace vcs
} // namespace hydla 
