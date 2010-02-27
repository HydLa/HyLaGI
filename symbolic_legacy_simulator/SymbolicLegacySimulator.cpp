#include "SymbolicLegacySimulator.h"

#include <iostream>
#include <boost/xpressive/xpressive.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include "Logger.h"

#include "sls_math_source.h"

using namespace std;
using namespace boost;
using namespace boost::xpressive;

using namespace hydla::parse_tree;

namespace hydla {
namespace symbolic_legacy_simulator {

//struct rawchar_formatter
//{
//  string operator()(smatch const &what) const
//  {
//    char c[2] = {0, 0};
//    c[0] = (char)strtol(what.str(1).c_str(), NULL, 8);
//    return c;
//  }
//};
//sregex rawchar_reg = sregex::compile("\\\\(\\d\\d\\d)");
//std::ostream_iterator< char > out_iter( std::cout );
//rawchar_formatter rfmt;
//regex_replace( out_iter, str.begin(), str.end(), rawchar_reg, rfmt);	

SymbolicLegacySimulator::SymbolicLegacySimulator(const Opts& opts) :
  opts_(opts)
{
}

SymbolicLegacySimulator::~SymbolicLegacySimulator()
{
}

void SymbolicLegacySimulator::init_mathlink()
{
  HYDLA_LOGGER_DEBUG("#*** init mathlink ***");

  //TODO: ��O�𓊂���悤�ɂ���
  if(!ml_.init(opts_.mathlink.c_str())) {
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
  ml_.MLPutSymbol(opts_.debug_mode ? "True" : "False");
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // �v���t�@�C�����[�h
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optUseProfile"); 
  ml_.MLPutSymbol(opts_.profile_mode ? "True" : "False");
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // ���񃂁[�h
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optParallel"); 
  ml_.MLPutSymbol(opts_.parallel_mode ? "True" : "False");
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // �o�͌`��
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optOutputFormat"); 
  switch(opts_.output_format) {
    case fmtTFunction:
      ml_.MLPutSymbol("fmtTFunction");
      break;

    case fmtNumeric:
    default:
      ml_.MLPutSymbol("fmtNumeric");
      break;
  }
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // HydLa.m�̓��e���M
  //   ml_.MLPutFunction("Get", 1);
  //   ml_.MLPutString("symbolic_simulator/HydLa.m");
  ml_.MLPutFunction("ToExpression", 1);
  ml_.MLPutString(sls_math_source());  
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();
}

void SymbolicLegacySimulator::initialize(const parse_tree_sptr& parse_tree)
{
  pt_ = parse_tree;
  init_mathlink();
  
}

void SymbolicLegacySimulator::simulate()
{}

} // namespace symbolic_legacy_simulator
} // namespace hydla

