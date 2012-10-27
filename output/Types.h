#ifndef _INCLUDED_HYDLA_OUTPUT_TYPES_H_
#define _INCLUDED_HYDLA_OUTPUT_TYPES_H_

namespace hydla{
namespace output{

typedef enum OutputFormat_ {
  fmtTFunction,
  fmtNumeric,
  fmtMathematica,
  fmtNInterval,
} OutputFormat;
  
typedef enum TimeOutputFormat_ {
  tFmtNot = 0,
  tFmtStd,
  tFmtCsv,
} TimeOutputFormat;

}// output
}// hydla

#endif // _INCLUDED_HYDLA_OUTPUT_TYPES_H_
