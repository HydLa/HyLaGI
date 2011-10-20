#include "Logger.h"

#include <iostream>

namespace hydla {
namespace logger {

bool Logger::parsing_area_;
bool Logger::calculate_closure_area_;
bool Logger::module_set_area_;
bool Logger::vcs_area_;
bool Logger::external_area_; //Mathematica�Ƃ�REDUCE�Ƃ��̊O���\�t�g����̏o��
bool Logger::output_area_;
bool Logger::rest_area_;  //�ǂ��ŏo������ǂ���������Ȃ��悤�Ȃ�

Logger::Logger() :
  log_level_(Warn)
{
  area_.push(std::cerr);
  debug_.push(std::cerr);
  summary_.push(std::cerr);//��ǓI�f�o�b�O���[�h
  warn_.push(std::cerr);
  error_.push(std::cerr);
  fatal_.push(std::cerr);
}

Logger::~Logger()
{}

Logger& Logger::instance() {
  static Logger inst;
  return inst;
}

} // namespace logger
} // namespace hydla
