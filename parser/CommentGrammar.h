#ifndef _INCLUDED_HYDLA_COMMENT_GRAMMAR_H_
#define _INCLUDED_HYDLA_COMMENT_GRAMMAR_H_

#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_confix.hpp>

namespace hydla {
namespace parser {

using namespace boost::spirit;

struct CommentGrammar : public grammar<CommentGrammar> {
  template <typename S> 
  struct definition {
    rule<S> comment;

    definition(CommentGrammar const& self) {
      comment = space_p |                               // while space
        comment_p("//") |       // '//'
        comment_p("/*", "*/");  // '/* */'
    }

    rule<S> const& start() const {return comment;}
  };
};

} // namespace parser
} // namespace hydla

#endif //_INCLUDED_HYDLA_COMMENT_GRAMMAR_H_

