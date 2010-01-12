#include <iostream>
#include "rp_expression_symbol_ext.h"

namespace rp {

inline
int rp_erep_bracketed(rp_erep f, rp_erep g)
{
  return( (rp_erep_type(f)==RP_EREP_NODE_OP) &&

	  ((rp_symbol_priority(rp_erep_symb(f)) <
	    rp_symbol_priority(rp_erep_symb(g))) ||

	   ((rp_symbol_priority(rp_erep_symb(f)) ==
	     rp_symbol_priority(rp_erep_symb(g))) &&
	    ((!rp_symbol_commutative(rp_erep_symb(g))) ||
	     (!rp_symbol_commutative(rp_erep_symb(f)))))) );
}

std::ostream& dump_erep(std::ostream& s, const rp_erep f,
                        const rp_vector_variable vars,
                        const int digits, const int mode)
{
  if(f==NULL) return s;

  char tmp[255];
  switch(rp_erep_type(f)){
  case RP_EREP_NODE_CST:
    if(rp_erep_cst(f)!=NULL){
      s << rp_erep_cst(f); //fprintf(out,"%s",rp_erep_cst(f));
    } else {
      rp_interval_print(tmp, rp_erep_val(f), digits, mode); //rp_interval_display(out,rp_erep_val(f),digits,mode);
      s << tmp;
    }
    break; /* RP_EREP_NODE_CST */
  case RP_EREP_NODE_VAR:
    if(vars==NULL){
      s << "_v(" << rp_erep_var(f) << ")"; //fprintf(out,"_v(%d)",rp_erep_var(f));
    } else {
      s << rp_variable_name(rp_vector_variable_elem(vars, rp_erep_var(f)));
      //fprintf(out,"%s",rp_variable_name(rp_vector_variable_elem(vars,rp_erep_var(f))));
    }
    break; /* RP_EREP_NODE_VAR */
  case RP_EREP_NODE_OP:
    if(rp_symbol_prefix(rp_erep_symb(f))){
      s << rp_symbol_name(rp_erep_symb(f)) << "("; //fprintf(out,"%s(",rp_symbol_name(rp_erep_symb(f)));
      rp::dump_erep(s, rp_erep_left(f),vars, digits, mode); //rp_erep_display(out,rp_erep_left(f),vars,digits,mode);
      // ª s‚É“ü‚Á‚Ä‚éH
      if(rp_symbol_binary(rp_erep_symb(f))){
        s << ","; //fprintf(out,",");
        rp::dump_erep(s, rp_erep_right(f), vars, digits, mode); //rp_erep_display(out,rp_erep_right(f),vars,digits,mode);
      }
      s << ")"; //fprintf(out,")");
    } else { /* infix */
      int lb = rp_erep_bracketed(rp_erep_left(f),f);
      if (lb) s << "("; //fprintf(out,"(");
      rp::dump_erep(s, rp_erep_left(f),vars, digits, mode); //rp_erep_display(out,rp_erep_left(f),vars,digits,mode);
      if (lb) s << ")"; //fprintf(out,")");
      s << rp_symbol_name(rp_erep_symb(f)); //fprintf(out,"%s",rp_symbol_name(rp_erep_symb(f)));

      if (rp_symbol_binary(rp_erep_symb(f))){
        int rb = rp_erep_bracketed(rp_erep_right(f),f);
        if (rb) s << "("; //fprintf(out,"(");
        rp::dump_erep(s, rp_erep_right(f), vars, digits, mode); //rp_erep_display(out,rp_erep_right(f),vars,digits,mode);
        if (rb) s << ")"; //fprintf(out,")");
      }
    }
    break; /* RP_EREP_NODE_OP */
  } /* switch */
  return s;
}

} // namespace rp
