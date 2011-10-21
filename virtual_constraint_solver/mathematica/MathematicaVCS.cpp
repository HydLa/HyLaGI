#include "MathematicaVCS.h"

#include <cassert>

#include "MathematicaVCSPoint.h"
#include "MathematicaVCSInterval.h"
#include "MathematicaExpressionConverter.h"
#include "PacketSender.h"

using namespace hydla::vcs;

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

bool MathematicaVCS::reset()
{
  return vcs_->reset();
}

bool MathematicaVCS::reset(const variable_map_t& vm)
{
  return vcs_->reset(vm);
}

bool MathematicaVCS::reset(const variable_map_t& vm, const parameter_map_t& pm)
{
  return vcs_->reset(vm, pm);
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
  return vcs_->add_constraint(constraints);
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
      ps.put_node(it->second.get_node(), PacketSender::VA_Time, true);
      ps.put_node(time.get_node(), PacketSender::VA_None, false);

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

std::string upward(std::string str){
  if(str.at(str.length()-1) == '.'){
    str = upward(str.substr(0,str.length()-1)) + '.';
  }else if(str.at(str.length()-1) != '9'){
    str = str.substr(0,str.length()-1)+(char)(str.at(str.length()-1)+1);
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
    ml_.put_function("ToString", 2);
    ml_.put_function("Interval", 1);    
    ml_.put_function("N", 2);  
    ps.put_node(val.get_node(), PacketSender::VA_None, true);
    ml_.put_integer(precision+5);     
    ml_.put_symbol("InputForm");
    ml_.skip_pkt_until(RETURNPKT);
    ret = ml_.get_string();
    /*
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
	sign = 'n';
      }else{
	sign = 'c';
      }
    }
    //std::cout<<ret<<std::endl;
    if(ret.find("}")-ret.find("{")-2>precision*2){
      for(int i=ret.find("{")+1;i<ret.find(",")-1;i++){
	if(ret.at(i)!='0' &&  ret.at(i)!='.') {
	  if(ret.substr(ret.find("{")+1,pre).find(".")!=-1) pre++;
	  lower = ret.substr(ret.find("{")+1,pre);
	  break;
	}else if(ret.at(i) == '0') pre++;
      }
      pre = precision;
      for(int i=ret.find(",")+2;i<ret.length();i++){
	if(ret.at(i)!='0' &&  ret.at(i)!='.') {
	  if(ret.substr(ret.find("{")+1,pre).find(".")!=-1) pre++;
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
    */
  } else  if(!val.is_undefined()) {
    ml_.put_function("ToString", 2);  
    ml_.put_function("N", 2);  
    ps.put_node(val.get_node(), PacketSender::VA_None, true);
    ml_.put_integer(precision);
    ml_.put_symbol("CForm");
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
  ps.put_node(lhs.get_node(), PacketSender::VA_None, true);
  ps.put_node(rhs.get_node(), PacketSender::VA_None, true);
  
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
  ps.put_node(time.get_node(), PacketSender::VA_None, true);
  ml_.skip_pkt_until(RETURNPKT);
  MathematicaExpressionConverter mec;
  time = mec.convert_math_string_to_symbolic_value(ml_.get_string());
}



  //SymbolicValue�̎��Ԃ����炷
hydla::vcs::SymbolicVirtualConstraintSolver::value_t MathematicaVCS::shift_expr_time(const value_t& val, const time_t& time){
  value_t tmp_val;
  ml_.put_function("exprTimeShift", 2);
  PacketSender ps(ml_);
  ps.put_node(val.get_node(), PacketSender::VA_None, true);
  ps.put_node(time.get_node(), PacketSender::VA_None, true);
  ml_.skip_pkt_until(RETURNPKT);
  MathematicaExpressionConverter mec;
  tmp_val = mec.convert_math_string_to_symbolic_value(ml_.get_string());
  return  tmp_val;
}


} // namespace mathematica
} // namespace vcs
} // namespace hydla 
