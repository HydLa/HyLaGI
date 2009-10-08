#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "HydLaParser.h"
#include "HydLaGrammar.h"
#include "CommentGrammar.h"

using namespace std;

namespace hydla {

HydLaParser::HydLaParser() 
{
}

HydLaParser::~HydLaParser()
{
}

bool HydLaParser::parse_flie(const char* filename) 
{
    ifstream in(filename);
    if (!in) {    
      throw std::logic_error(string("cannot open \"") + filename + "\"");
    }

    return parse(in);
}

bool HydLaParser::parse_string(const char* str)
{
    stringstream in(str);
    return parse(in);
}


bool HydLaParser::parse(std::istream& iter)
{
    pos_iter_t positBegin(make_multi_pass(istreambuf_iterator<char>(iter)), 
                          make_multi_pass(istreambuf_iterator<char>()));
    pos_iter_t positEnd;

    HydLaGrammar hg;
    CommentGrammar cg;
    ast_tree_ = ast_parse<node_val_data_factory_t>(positBegin, positEnd, hg, cg);

    return ast_tree_.full;
}

void HydLaParser::dump_tree(const tree_iter_t &iter, int nest) const 
{
    for(int i=0; i<nest; i++) {
        cout << "  "; // スペース２個分インデント
    }

    cout << "ID:" << iter->value.id().to_long();
    cout << " Node:" << string(iter->value.begin(), iter->value.end());
    size_t size = iter->children.size();
    if (size > 0) {
        cout << " Child Size:" << size << endl;
    } else {
        cout << endl;
    }

    for(size_t j=0; j<size; j++) {
        dump_tree(iter->children.begin()+j, nest+1);
    }
}

std::string HydLaParser::create_interlanguage(std::string max_time)
{
    std::string str;
    str += "HydLaMain[";
    str += create_interlanguage(ast_tree_.trees.begin());
    str += ", {";
    variable_map_t::iterator iter = variable_.begin();
    while(iter!=variable_.end()) {
        str += iter->first;
        if(++iter != variable_.end()) str += ", ";
    }
    str += "}," + max_time + "];";
    return str;
}

#define TransExpArg2(X) case HydLaGrammar::RI_##X: \
                            return #X "[" + create_interlanguage(ch) + ", " + \
                                      create_interlanguage(ch+1) + "]";


std::string HydLaParser::create_interlanguage(const tree_iter_t &iter) 
{                                            
    tree_iter_t ch = iter->children.begin();
    std::string val = string(iter->value.begin(), iter->value.end());

    switch(iter->value.id().to_long()) {
    case HydLaGrammar::RI_Equivalent:
        module_.insert(std::make_pair(
            string(ch->value.begin(), ch->value.end()), 
            "unit[" + create_interlanguage(ch+1) + "]"));
        //cout << "unit[" + create_interlanguage(ch+1) + "]" << endl;
        return "";

    case HydLaGrammar::RI_Equal:
        return "Equal[" + 
            create_interlanguage(ch) + ", " + 
            create_interlanguage(ch+1) + "]";

    case HydLaGrammar::RI_AskEqual:
        return "Equal[" + 
            create_interlanguage(ch) + ", " + 
            create_interlanguage(ch+1) + "]";

    case HydLaGrammar::RI_Unequal:
        return "Unequal[" + 
            create_interlanguage(ch) + ", " + 
            create_interlanguage(ch+1) + "]";

    case HydLaGrammar::RI_AskUnequal:
        return "Unequal[" + 
            create_interlanguage(ch) + ", " + 
            create_interlanguage(ch+1) + "]";

    case HydLaGrammar::RI_Entailment:
        return "ask[" + 
            create_interlanguage(ch) + ", " + 
            create_interlanguage(ch+1) +"]";

    case HydLaGrammar::RI_Tell:
        return "tell[" + create_interlanguage(ch) + "]";

    case HydLaGrammar::RI_LogicalAnd:
        return create_interlanguage(ch) + ", " +
            create_interlanguage(ch+1);    

    case HydLaGrammar::RI_Ask_LogicalAnd:
        return "And[" +
            create_interlanguage(ch) + ", " +
            create_interlanguage(ch+1) + "]";    

    case HydLaGrammar::RI_Previous:
        return "prev[" + create_interlanguage(ch) + "]";

        TransExpArg2(Plus);
        TransExpArg2(Subtract);
        TransExpArg2(Times);
        TransExpArg2(Divide);
        TransExpArg2(Less);
        TransExpArg2(LessEqual);
        TransExpArg2(Greater);
        TransExpArg2(GreaterEqual);

    case HydLaGrammar::RI_Positive:
        return create_interlanguage(ch);

    case HydLaGrammar::RI_Negative:
        return "Minus[" + create_interlanguage(ch) + "]";
        /*
        case HydLaGrammar::RI_LogicalAnd:
        return "And[" + create_interlanguage(ch) + "]";
        */
    case HydLaGrammar::RI_LogicalOr:
        return "Or[" + create_interlanguage(ch) + ", " + 
            create_interlanguage(ch+1) + "]";

    case HydLaGrammar::RI_Globally:
        {
            std::string str("always[");

            size_t size = iter->children.size();
            for(size_t i=0; i<size; i++) {
                str += create_interlanguage(ch+i);
                if(i < size-1) str += ", ";
            }
            str += "]";
            return str;
        }

    case HydLaGrammar::RI_Weaker:
        return "order[" + create_interlanguage(ch+1) + ", " + 
            create_interlanguage(ch) + "]";

    case HydLaGrammar::RI_Parallel:
        return "group[" + create_interlanguage(ch) + ", " + 
            create_interlanguage(ch+1) + "]";

    case HydLaGrammar::RI_Differential:
        return create_interlanguage(ch) + "'";

    case HydLaGrammar::RI_Variable:
        {
            variable_.insert(std::make_pair(val, 0));
            return val;
        }

    case HydLaGrammar::RI_Number:
        return val;

    case HydLaGrammar::RI_ConstraintName:
        {
            module_map_t::iterator itr = module_.find(val);
            if(itr != module_.end()) return module_.find(val)->second;
            return val;
        }

    case HydLaGrammar::RI_Statements:
        {        
            std::string str;
            for(size_t i=0; i<iter->children.size(); i++) {
                str += create_interlanguage(ch+i);
            }
            return str;
        }

    default:
        {
            std::string str(val);
            for(size_t i=0; i<iter->children.size(); i++) {
                str += create_interlanguage(ch+i);
            }
            return str;
        }
    }
}

}
