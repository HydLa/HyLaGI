
#include "HAConverter.h"
#include "Timer.h"
#include "SymbolicTrajPrinter.h"
#include "SymbolicValue.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"
#include "../common/Logger.h"
#include "../parser/TreeInfixPrinter.h"
#include <limits.h>
#include <string>

using namespace std;

namespace hydla {
namespace simulator {

	HAConverter::HAConverter(Opts &opts):BatchSimulator(opts){}

	HAConverter::~HAConverter(){}

	phase_result_const_sptr_t HAConverter::simulate()
	{
		HYDLA_LOGGER_HA("* * * * * * * * * * * * *");
		cout << "This opt is not available." << endl;
		HYDLA_LOGGER_HA("* * * * * * * * * * * * *");
		phase_result_const_sptr_t result;
		return result;
	}
	
	void HAConverter::process_one_todo(simulation_todo_sptr_t& todo)
	{
	
	}

	bool HAConverter::compare_phase_result(phase_result_sptr_t r1, phase_result_sptr_t r2){
		// �t�F�[�Y
		if(!(r1->phase == r2->phase)) return false;
		// ���W���[���W��
		HYDLA_LOGGER_HA("compare :: id:", r1->id, " ", r1->module_set->get_name(), " <=> id:", r2->id, " ", r2->module_set->get_name());
		if(!(r1->module_set->compare(*r2->module_set) == 0)) return false;
		// positive_ask
		ask_set_t::iterator it_1 = r1->positive_asks.begin();
		ask_set_t::iterator it_2 = r2->positive_asks.begin();
		while(it_1 != r1->positive_asks.end() && it_2 != r2->positive_asks.end()) {
			if(!((*it_1)->is_same_struct(**it_2, true))) return false;
			it_1++;
			it_2++;
		}
		// �ǂ��炩�̃C�e���[�^���Ō�܂ŒB���Ă��Ȃ������瓙�����Ȃ�
		if(it_1 != r1->positive_asks.end() || it_2 != r2->positive_asks.end()) return false;
		
		return true;
	} 
	
	bool HAConverter::check_continue(current_condition_t *cc)
	{
		HYDLA_LOGGER_HA("****** check_continue ******");
		viewLs(*cc);
		std::vector<phase_result_sptrs_t>::iterator it_ls_vec = cc->ls.begin();		
		std::vector< std::vector<phase_result_sptrs_t>::iterator > remove_itr;
		while(it_ls_vec != cc->ls.end()){
			phase_result_sptrs_t::iterator it_ls = (*it_ls_vec).begin();
			phase_result_sptrs_t::iterator it_loop = cc->loop.begin();
			while(it_ls != (*it_ls_vec).end() && it_loop != cc->loop.end()) {
				// �������Ȃ���ԑJ�ڗ񂾂�����ls����폜
				if(!compare_phase_result(*it_ls, *it_loop)){
					HYDLA_LOGGER_HA("delete ls element : not equal loop");
					// while�ŃC�e���[�g����ls_�̒��g��erase�ł��Ȃ��̂ŁAerase����itr��ۑ����Č��ls_�̗v�f���폜����
					remove_itr.push_back(it_ls_vec);
					break;
				}
				it_ls++;
				it_loop++;
				// loop���Z����ԑJ�ڗ��ls����폜
				if(it_ls == (*it_ls_vec).end() && it_loop != cc->loop.end()){
					HYDLA_LOGGER_HA("delete ls element : less than loop");
					// while�ŃC�e���[�g����ls_�̒��g��erase�ł��Ȃ��̂ŁAerase����itr��ۑ����Č��ls_�̗v�f���폜����
					remove_itr.push_back(it_ls_vec);
				}
			}	
			it_ls_vec++;
			HYDLA_LOGGER_HA("* * * * * * * * * * * * *");
		}
		
		for (unsigned int i = 0 ; i < remove_itr.size() ; i++){
			cc->ls.erase(remove_itr[i]);
		}
		
		viewLs(*cc);
		
		if(cc->ls.empty()) {
			HYDLA_LOGGER_HA("****** end check_continue : false ******");
			return false;
		}
		HYDLA_LOGGER_HA("****** end check_continue : true ******");

		return true;
	}
  	
	void HAConverter::set_possible_loops(phase_result_sptr_t result, current_condition_t *cc)
	{
		HYDLA_LOGGER_HA("****** set_possible_loops ******");
		phase_result_sptrs_t::iterator it_prs = cc->phase_results.begin();
		while(it_prs != cc->phase_results.end()){
			if(compare_phase_result(result, *it_prs)){
				phase_result_sptrs_t candidate_loop;
				copy(it_prs, cc->phase_results.end(), back_inserter(candidate_loop));
				cc->ls.push_back(candidate_loop);
			}
			it_prs++;
		}
		
		// ls�̒��g��\��
		viewLs(*cc);
		HYDLA_LOGGER_HA("****** end set_possible_loops ******");
	}
  	
	bool HAConverter::loop_eq_max_ls(current_condition_t cc)
	{
		HYDLA_LOGGER_HA("****** loop_eq_max_ls ******");
		// �Ƃ肠����ls�̍ŏ��̗v�f���ő�̂��̂Ɖ��聩���Ԃ񂱂�Ő�����
		// loop���Z����ԑJ�ڗ�ƁAloop�Ɠ������Ȃ���ԑJ�ڗ��check_continue��ls_����폜���Ă��邽��
		phase_result_sptrs_t::iterator it_ls = cc.ls[0].begin();
		phase_result_sptrs_t::iterator it_loop = cc.loop.begin();
		while(it_ls != cc.ls[0].end() && it_loop != cc.loop.end()) {
			if(!compare_phase_result(*it_ls, *it_loop)) {
				HYDLA_LOGGER_HA("****** end loop_eq_max_ls : false ******");
				return false;
			}
			it_ls++;
			it_loop++;
		}
		// �ǂ��炩�̃C�e���[�^���Ō�܂ŒB���Ă��Ȃ������瓙�����Ȃ�
		if(it_ls != cc.ls[0].end() || it_loop != cc.loop.end()) {
			HYDLA_LOGGER_HA("****** end loop_eq_max_ls : false ******");
			return false;
		}
		HYDLA_LOGGER_HA("****** end loop_eq_max_ls : true ******");
		return true;
	}
  	
	void HAConverter::checkNode(current_condition_t cc)
	{
		//TODO �G�b�W����X�e�b�v�̎���
		HYDLA_LOGGER_HA("****** checkNode ******");
		
		// �J�n����̃V�~�����[�V�������ʑS��
		phase_result_sptrs_t simulate_result = cc.phase_results;
		
		// ls���ɂȂ�m�[�h(IP)�����ɉ��A�G�b�W�̒ʉ߉\���𔻒肷��
		phase_result_sptrs_t::iterator it_ls = cc.ls[0].begin();
		// ���߂̃m�[�h�ƍŌ�̃m�[�h�͓������߁A���߂̃m�[�h���΂��d����h��
		it_ls++;
		while(it_ls != cc.ls[0].end()) {
			if ((*it_ls)->phase == IntervalPhase){
				checkEdge(*it_ls, cc);				
			}
			it_ls++;
		}
		
		HYDLA_LOGGER_HA("****** end checkNode ******");
	}
	
	void HAConverter::checkEdge(phase_result_sptr_t node, current_condition_t cc){
		HYDLA_LOGGER_HA("****** checkEdge ******");
		
		viewNode(node);
		
		// �܂��ʉ߂��Ă��Ȃ��G�b�W�̃K�[�h������T�����A�����𖞂������ǂ������肷��
		// ���ʂ�result_check_reachable_��reachable, unknown�ɓ����
		result_check_reachable_.reachable.clear();
		result_check_reachable_.unknown.clear();
		// �ʉߍς݃G�b�W�̒T�� + �ʉߍς݃m�[�h�̒T�� + �z���Ƃ̑Ώ�node��phase_result�����W
		// �ʉߍς݃G�b�W�ƒʉߍς݃m�[�h�͕����čl���� 
		// �G�b�W�́A�Ώۃm�[�h����ʉ߂��Ă���ꍇ�݂̂��l���邪�A�m�[�h�͎��s���ɒʉ߂����S�Ẵm�[�h���l����
		phase_result_sptrs_t node_transition;
		for(unsigned int i = 0 ; i < cc.phase_results.size() - 1 ; i++){
			if(cc.phase_results[i]->phase == IntervalPhase) {
					phase_t n(cc.phase_results[i]->module_set, cc.phase_results[i]->negative_asks, cc.phase_results[i]->positive_asks);
					passed_node.push_back(n);
			};
			if(compare_phase_result(node, cc.phase_results[i])){
				if(cc.phase_results[i]->id < cc.loop_start_id){
					// node�̎��̃t�F�[�Y������node����o�Ă���G�b�W
					phase_t e(cc.phase_results[i+1]->module_set, cc.phase_results[i+1]->negative_asks, cc.phase_results[i+1]->positive_asks);
					passed_edge.push_back(e);
				}
				node_transition.push_back(cc.phase_results[i]);
			}
		}
		
		HYDLA_LOGGER_HA("--- node transition ---");
		viewPrs(node_transition);
		
		for(unsigned int i = 0 ; i < passed_edge.size(); i++){
			HYDLA_LOGGER_HA("--- passed edge", i, " ---");
			viewEdge(passed_edge[i]);
		}
		HYDLA_LOGGER_HA("");
		
		// �S�ẴG�b�W��T��
		module_set_container_sptr msc = msc_no_init_;
		msc->reset();
		phase_t phase;
		while(msc->go_next()){
			vec_negative_asks.clear();
			vec_positive_asks.clear();
			module_set_sptr ms = msc->get_module_set();
			HYDLA_LOGGER_HA("--- Module Set: ",ms->get_name());
			phase.module_set = ms;
			
			GuardGetter gg;
			ms->dispatch(&gg);
			HYDLA_LOGGER_HA("- Guards - ");
			viewAsks(gg.asks);
			
			// negative_ask�͋�W������Apositive_ask�͑S�ẴK�[�h�������܂ޏ�Ԃ���X�^�[�g���A�p�W�������߂�
			// positive_asks�ɑS�ẴK�[�h������ǉ�����
			ask_set_t::iterator it = gg.asks.begin();
			positive_asks_t tmp_pos;
			while(it != gg.asks.end()){
				tmp_pos.insert((*it));
				it++;
			}
			vec_positive_asks.push_back(tmp_pos);
			
			// negative_ask�͋�W������
			negative_asks_t tmp_neg;
			vec_negative_asks.push_back(tmp_neg);
			
			it = gg.asks.begin();
			while(it != gg.asks.end()){
				create_asks_vec(*it);
				it++;
			}
			
			// debug print
			std::vector<negative_asks_t>::iterator it_neg_d = vec_negative_asks.begin();
			std::vector<positive_asks_t>::iterator it_pos_d = vec_positive_asks.begin();
			HYDLA_LOGGER_HA("****** beki");
			while(it_neg_d != vec_negative_asks.end() && it_pos_d != vec_positive_asks.end()){
				HYDLA_LOGGER_HA("negative ask:");
				viewAsks((*it_neg_d));
				HYDLA_LOGGER_HA("positive ask:");
				viewAsks((*it_pos_d));
				HYDLA_LOGGER_HA("");
				it_neg_d++;
				it_pos_d++;
			}
			HYDLA_LOGGER_HA("****** ******");
			// **

			std::vector<negative_asks_t>::iterator it_neg = vec_negative_asks.begin();
			std::vector<positive_asks_t>::iterator it_pos = vec_positive_asks.begin();
			while(it_neg != vec_negative_asks.end() && it_pos != vec_positive_asks.end()){				
				HYDLA_LOGGER_HA("--------------------");
				phase.negative_asks = *it_neg;
				phase.positive_asks = *it_pos;	
				
				HYDLA_LOGGER_HA("negative ask:");
				viewAsks(phase.negative_asks);
				HYDLA_LOGGER_HA("positive ask:");
				viewAsks(phase.positive_asks);
				
				if (!check_passed_phase(passed_edge, phase) && !check_passed_phase(passed_node, phase)){
					ResultCheckOccurrence res = check_occurrence(node, phase, cc);
					if(res == Reachable){						
						result_check_reachable_.reachable.insert(pair<phase_result_sptr_t, phase_t>(node ,phase));
					}else if(res == Unknown){
						result_check_reachable_.unknown.insert(pair<phase_result_sptr_t, phase_t>(node ,phase));
						
						HYDLA_LOGGER_HA("***** unknown *****");
						unknown_map_t::iterator it_unknown = result_check_reachable_.unknown.begin();
						while(it_unknown != result_check_reachable_.unknown.end()){
							HYDLA_LOGGER_HA("unknown...");
							it_unknown++;
						}
					}
					// unreachable�͉������Ȃ�
				}
				
				it_neg++;
				it_pos++;
				HYDLA_LOGGER_HA("--------------------");
			}
			
			msc->mark_current_node();
		}
		
		HYDLA_LOGGER_HA("****** end checkEdge ******");
	}
	
	void HAConverter::create_asks_vec(boost::shared_ptr<parse_tree::Ask> ask){
		hydla::parse_tree::TreeInfixPrinter tree_printer;
		// negative_ask��positive_ask�̙p�W�����擾����
		std::vector<negative_asks_t> tmp_vec_negative_asks;
		std::vector<positive_asks_t> tmp_vec_positive_asks;

		std::vector<negative_asks_t>::iterator it_neg = vec_negative_asks.begin();
		std::vector<positive_asks_t>::iterator it_pos = vec_positive_asks.begin();
		while(it_neg != vec_negative_asks.end() && it_pos != vec_positive_asks.end()){
			tmp_vec_negative_asks.push_back((*it_neg));
			(*it_neg).insert(ask);
			tmp_vec_negative_asks.push_back((*it_neg));
			
			tmp_vec_positive_asks.push_back((*it_pos));
			(*it_pos).erase(ask);
			tmp_vec_positive_asks.push_back((*it_pos));
			
			it_neg++;
			it_pos++;
		}
		vec_negative_asks = tmp_vec_negative_asks;
		vec_positive_asks = tmp_vec_positive_asks;
	}
	
	HAConverter::ResultCheckOccurrence 
		HAConverter::check_occurrence(phase_result_sptr_t node, phase_t edge, current_condition_t cc)
	{
		HYDLA_LOGGER_HA("****** check_occurrence ******");

		viewNode(node);
		viewEdge(edge);

		HYDLA_LOGGER_HA("****** end check_occurrence ******");
		return Unknown;
	}
	bool HAConverter::check_passed_phase(phases_t passed_phase, phase_t phase)
	{
		//HYDLA_LOGGER_HA("****** check_passed_phase ******");
		for(unsigned int i = 0 ; i < passed_phase.size(); i++){
			if(compare_phase(phase, passed_phase[i])){
				HYDLA_LOGGER_HA("****** check_passed_phase true ******");					
				return true;
			}
		}
		HYDLA_LOGGER_HA("****** check_passed_phase false ******");
		return false;
	}
	
	bool HAConverter::compare_phase(phase_t p1, phase_t p2)
	{
		// module set
		if(!(p1.module_set->compare(*p2.module_set) == 0)) return false;
		// positive_ask
		ask_set_t::iterator it_1 = p1.positive_asks.begin();
		ask_set_t::iterator it_2 = p2.positive_asks.begin();
		while(it_1 != p1.positive_asks.end() && it_2 != p2.positive_asks.end()) {
			if(!((**it_1).is_same_struct(**it_2, true))) return false;
			it_1++;
			it_2++;
		}
		// �ǂ��炩�̃C�e���[�^���Ō�܂ŒB���Ă��Ȃ������瓙�����Ȃ�
		if(it_1 != p1.positive_asks.end() || it_2 != p2.positive_asks.end()) return false;
		
		return true;
	}
  
	bool HAConverter::check_contain(phase_result_sptr_t result, current_condition_t cc)
	{
		HYDLA_LOGGER_HA("****** check_contain ******");
		HYDLA_LOGGER_HA("�E�E�E�E�Ephase_result�E�E�E�E�E");
		viewPrs(cc.phase_results);
		HYDLA_LOGGER_HA("�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E");
		phase_result_sptrs_t::iterator it_prs = cc.phase_results.begin();
		while(it_prs != cc.phase_results.end()){
			if(compare_phase_result(result, *it_prs)) {
				HYDLA_LOGGER_HA("****** end check_contain : true ******");				
				return true;
			}
			it_prs++;
		}
		HYDLA_LOGGER_HA("****** end check_contain : false ******");				
		return false;
	}
	
	void HAConverter::viewEdge(phase_t edge)
	{
		HYDLA_LOGGER_HA("--- Edge: ");
		HYDLA_LOGGER_HA(edge.module_set->get_name());
		HYDLA_LOGGER_HA("negative ask:");
		viewAsks(edge.negative_asks);
		HYDLA_LOGGER_HA("positive_ask:");
		viewAsks(edge.positive_asks);
	}
	void HAConverter::viewNode(phase_result_sptr_t node)
	{
		HYDLA_LOGGER_HA("--- Node ---");
		HYDLA_LOGGER_HA(node->module_set->get_name());
		HYDLA_LOGGER_HA("negative ask:");
		viewAsks(node->negative_asks);
		HYDLA_LOGGER_HA("positive_ask:");
		viewAsks(node->positive_asks);
		HYDLA_LOGGER_HA("");
	}
	
	void HAConverter::viewLs(current_condition_t cc)
	{
		HYDLA_LOGGER_HA("�E�E�E�E�E ls �E�E�E�E�E");
		std::vector<phase_result_sptrs_t>::iterator it_ls_vec = cc.ls.begin();		
		while(it_ls_vec != cc.ls.end()){
			viewPrs(*it_ls_vec);
			HYDLA_LOGGER_HA("�E�E�E�E�E�E�E�E�E�E�E�E");
			it_ls_vec++;
		}
	}
	
	void HAConverter::viewPrs(phase_result_sptrs_t results)
	{
		phase_result_sptrs_t::iterator it_ls = results.begin();
		while(it_ls != results.end()) {
			viewPr(*it_ls);	
			it_ls++;
		}	
	}
	
	void HAConverter::viewPr(phase_result_sptr_t result)
	{
		if (logger::Logger::ha_converter_area_) {
	    hydla::output::SymbolicTrajPrinter printer;
			printer.output_one_phase(result);
			
			HYDLA_LOGGER_HA("negative ask:");
			viewAsks(result->negative_asks);
			HYDLA_LOGGER_HA("positive ask:");
			viewAsks(result->positive_asks);
		}
	}
	
	void HAConverter::viewAsks(ask_set_t asks)
	{
		hydla::parse_tree::TreeInfixPrinter tree_printer;
		ask_set_t::iterator it = asks.begin();
		string str = "";
		while(it != asks.end()){
			str += tree_printer.get_infix_string((*it)->get_guard()) + " ";
			it++;
		}
		HYDLA_LOGGER_HA(str);
	}
	
	void HAConverter::push_result(current_condition_t cc)
	{
		phase_result_sptrs_t result;
		for(unsigned int i = 0 ; i < cc.phase_results.size() ; i++){
			result.push_back(cc.phase_results[i]);
			if(cc.phase_results[i]->id == cc.loop_start_id) break;
		}
		HYDLA_LOGGER_HA("�E�E�E�E�E ha_result ", ha_results_.size(), " �E�E�E�E�E");
		viewPrs(result);
		HYDLA_LOGGER_HA("�E�E�E�E�E�E�E�E�E�E�E�E�E�E");
		ha_results_.insert(pair<phase_result_sptrs_t, unknown_map_t>(result, cc.unknown_map));
	}
	
	void HAConverter::output_ha()
	{
		cout << "-�E-�E-�E-�EResult Convert�E-�E-�E-�E-" << endl;
		ha_result_t::iterator it_ha_res = ha_results_.begin();		
		while(it_ha_res != ha_results_.end()){
			convert_phase_results_to_ha((*it_ha_res).first, (*it_ha_res).second);
			cout << "-�E-�E-�E-�E-�E-�E-�E-�E-�E-�E-�E-�E-" << endl;
			it_ha_res++;
		}
	}
	
	void HAConverter::convert_phase_results_to_ha(phase_result_sptrs_t result, unknown_map_t unknown_map)
	{
		cout << "digraph g{" << endl;
		cout << "edge [dir=forward];" << endl;
		cout << "\"start\" [shape=point];" << endl;
			cout << "\"start\"->\"" << result[1]->module_set->get_name() << "\\n(" << get_asks_str(result[1]->positive_asks) 
			<< ")\" [label=\"" << result[0]->module_set->get_name() << "\\n(" << get_asks_str(result[0]->positive_asks)
			   << ")\", labelfloat=false,arrowtail=dot];" << endl;
		for(unsigned int i = 2 ; i < result.size() ; i++){
			if(result[i]->phase == IntervalPhase){
				cout << "\"" << result[i-2]->module_set->get_name() << "\\n(" << get_asks_str(result[i-2]->positive_asks) 
					<< ")\"->\"" << result[i]->module_set->get_name() << "\\n(" << get_asks_str(result[i]->positive_asks) 
						<< ")\" [label=\"" << result[i-1]->module_set->get_name() << "\\n(" << get_asks_str(result[i-1]->positive_asks) 
				     << ")\", labelfloat=false,arrowtail=dot];" << endl;
			}
		}
		
		unknown_map_t::iterator it_un_map = unknown_map.begin();
			while(it_un_map != unknown_map.end()){
				cout << "\"" << (*it_un_map).first->module_set->get_name() << "\\n(" << get_asks_str((*it_un_map).first->positive_asks) 
					<< ")\"->\"unknown\" [label=\"" << (*it_un_map).second.module_set->get_name() << "\\n(" << get_asks_str((*it_un_map).second.positive_asks) 
				     << ")\", labelfloat=false,arrowtail=dot, style=\"dashed\"];" << endl;
				it_un_map++;
			}
		cout << "}" << endl;
	}
	std::string HAConverter::get_asks_str(ask_set_t asks)
	{
		std::string res = "";
		hydla::parse_tree::TreeInfixPrinter tree_printer;
		ask_set_t::iterator it = asks.begin();
		while(it != asks.end()){
			res += tree_printer.get_infix_string((*it)->get_guard()) + " ";
			it++;
		}
		return res;
	}
		
	GuardGetter::GuardGetter(){}
	GuardGetter::~GuardGetter(){}
	
	void GuardGetter::accept(const boost::shared_ptr<parse_tree::Node>& n){
		n->accept(n, this);
	}
	
	void GuardGetter::visit(boost::shared_ptr<parse_tree::Ask> node){
		asks.insert(node);
	}



}//namespace hydla
}//namespace simulator 

