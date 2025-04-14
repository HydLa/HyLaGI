#pragma once

#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

#include <Python.h>

namespace hydla {
namespace debug {
class Solve_sym {
public:
  Solve_sym();
  ~Solve_sym();
  static void
  solve_main(std::vector<std::vector<std::vector<std::string>>> constraint_map,
             std::vector<std::vector<std::string>> v_map,
             std::vector<std::vector<std::string>> h_map);
  /// 前件の計算
  static std::map<std::vector<std::string>, std::map<std::string, std::string>>
  make_ask_map(
      std::vector<std::vector<std::string>> each_ask_cons_sets,
      std::vector<std::string> true_eq_list,
      std::vector<std::vector<std::vector<std::string>>> equation_name_map,
      std::map<std::string, std::string> range_map,
      std::vector<std::string> add_var_list);
  /// 後件の計算の際，前件の結果と合わせてマッピングするための関数
  static std::map<
      std::map<std::vector<std::string>, std::map<std::string, std::string>>,
      std::map<std::vector<std::string>, std::map<std::string, std::string>>>
  make_tell_map_over(
      std::map<std::vector<std::string>, std::map<std::string, std::string>>
          each_var_val_map,
      std::vector<std::vector<std::vector<std::string>>> equation_name_map,
      std::map<std::string, std::string> range_map,
      std::vector<std::string> add_var_list, std::vector<std::string> eq_list,
      std::vector<std::vector<std::vector<std::string>>> sym_prio_map);
  /// 後件の計算
  static std::map<std::vector<std::string>, std::map<std::string, std::string>>
  make_tell_map(
      std::map<std::vector<std::string>, std::map<std::string, std::string>>
          each_var_val_map,
      std::vector<std::vector<std::vector<std::string>>> equation_name_map,
      std::map<std::string, std::string> range_map,
      std::vector<std::string> add_var_list, std::vector<std::string> eq_list,
      std::vector<std::vector<std::vector<std::string>>> sym_prio_map);
  /// 結果の表示
  static void find_result(std::map<std::map<std::vector<std::string>,
                                            std::map<std::string, std::string>>,
                                   std::map<std::vector<std::string>,
                                            std::map<std::string, std::string>>>
                              ret_make_tell_map_2,
                          std::vector<std::vector<std::string>> v_map);

  /// solve.pyからの出力をマップにパースする(TODO:と、言いつつ全ての計算を行なっているので、この部分を関数化する)
  static std::map<std::string, std::string>
  make_val_map(std::map<std::string, std::string> range_map,
               std::vector<std::string> eq_list,
               std::vector<std::string> add_var_list);

  /// 得られたprev変数の解が一致するかどうかの確認(包含されている場合は分割を行う必要があるため)
  static std::map<std::vector<std::string>, std::map<std::string, std::string>>
  check_val_map(
      std::vector<std::string> var_val_map_first,
      std::map<std::string, std::string> var_val_map_second,
      std::map<std::string, std::string> var_val_after,
      std::vector<std::string> add_var_list,
      std::vector<std::vector<std::vector<std::string>>> equation_name_map,
      std::map<std::string, std::string> range_map,
      std::vector<std::vector<std::vector<std::string>>> sym_prio_map);

  ///////////////// 制約階層から制約集合を変更する関連の処理を行う関数
  ////////////////////////////
  /// unsat_coreの制約を制約階層に基づいて取り除いた制約集合を返す
  static std::vector<std::string> resolve_without_unsat_core(
      std::vector<std::string> var_val_map_first,
      std::map<std::string, std::string> var_val_after,
      std::vector<std::vector<std::vector<std::string>>> sym_prio_map);
  /// 矛盾する変数をもらって、制約階層に基づき制約を減らす(TODO:制約階層に基づきってところが実装できていない(とりあえず循環構造を持っている制約階層を考えないで実装してみた))
  static std::vector<std::string> make_resolve_cons_set(
      std::vector<std::string> discrete_val,
      std::vector<std::string> var_val_map_first,
      std::vector<std::vector<std::vector<std::string>>> sym_prio_map);
  /// 削除する制約を決める
  static std::vector<std::string> decide_erase_cons(
      std::vector<std::string> pre_erase,
      std::vector<std::vector<std::vector<std::string>>> sym_prio_map);

  ///////////////// unsat core(変数 or 制約)を見つける関数
  ////////////////////////////
  /// 得られた変数の解が連続するかどうかの確認(微分値の暗黙の連続性を確認するため)一つの変数において2つ以上の階数で離散変化が発生する場合のみその変数の配列を返す
  static std::vector<std::string> check_continue_val(
      std::vector<std::string> var_val_map_first,
      std::map<std::string, std::string> var_val_map_second,
      std::map<std::string, std::string> var_val_after,
      std::vector<std::string> add_var_list,
      std::vector<std::vector<std::vector<std::string>>> equation_name_map,
      std::map<std::string, std::string> range_map);
  /// unsat coreとなる制約式を求める(複数の式の矛盾から、原因の制約を導く)
  static std::map<std::string, std::string>
  find_unsat_core(std::string error_m, std::vector<std::string> eq_list,
                  std::string solve_var, std::string renge_eq,
                  std::string each_in_val);

  ///////////////// 式変形用関数 /////////////////////////
  /// 配列をもらって、sympyに投げる時の形に変形する [a,b,c]のような感じ
  static std::string conv_list(std::vector<std::string> list);
  /// 配列をもらって、sympyに投げる時の代入式に変形する
  /// (x<0).subs([(x,-1),...])のような感じ
  static std::string make_sub_list(std::vector<std::string> list);
  /// 変数と階数のマップをもらって、変数名の配列を返す
  static std::vector<std::string>
  mv_to_v(std::vector<std::vector<std::string>> v_map);
  /// ガード付き制約名の配列をもらって、全組み合わせを返す
  static std::vector<std::vector<std::string>>
  make_cons_sets(std::vector<std::string> ask_eq_list);
  /// 式から使用している変数を抽出する
  static std::vector<std::string> get_sym(std::string eq);
  /// 第２引数で与えられた制約モジュールによって作成されたpython内の式の名前をリストで返す(ask制約とtell制約の式の名前を返すかを第３、４引数で指定する)
  static std::vector<std::string> make_eq_list(
      std::vector<std::vector<std::vector<std::string>>> constraint_map,
      std::vector<std::string> solve_module, bool add_ask, bool add_tell);
  /// 制約式全体の登録(中でadd_equation()を呼ぶ)
  /// 同時に制約階層と使用変数の情報を作成して返す
  static std::tuple<std::vector<std::vector<std::vector<std::string>>>,
                    std::map<std::string, std::string>,
                    std::vector<std::vector<std::vector<std::string>>>>
  add_equations(
      std::vector<std::vector<std::vector<std::string>>> constraint_map,
      std::string solve_var, std::vector<std::vector<std::string>> h_map);
  /// 2つの候補制約集合と解のマップを結合する
  static std::map<std::vector<std::string>, std::map<std::string, std::string>>
  connect_map(
      std::map<std::vector<std::string>, std::map<std::string, std::string>>
          base,
      std::map<std::vector<std::string>, std::map<std::string, std::string>>
          add);

  ///////////////// 判定用関数 /////////////////////
  /// 与えられた同じ変数に対する2つの式が意味的に等しいかの判定を行う
  static bool find_inc_rel(std::string val_before,
                           std::string var_map_after_second);
  /// 制約集合の答えとして得られた結果がすでに登録されている結果と重複するかの判定
  static bool check_duplication_ask(
      std::map<std::vector<std::string>, std::map<std::string, std::string>>
          each_var_val_map,
      std::map<std::string, std::string> var_val,
      std::vector<std::string> each_ask_cons_set);

  ///////////////// python接続用関数 /////////////////////
  /// 変数名の登録
  static int add_var(std::vector<std::string> name_var_list);
  /// 式の登録
  static int add_equation(std::string name, std::string eq);
  /// ガード式を解き、第2引数の変数に対しての解を返す
  static std::string solve_equation(std::string name, std::string eq);
  /// 不等式を解き、第2引数の変数に対しての解を返す(現在はsolve_equationと同じだが、返しを"|"が最大優先度になるようにしたい(ケース分岐に対応するため))
  static std::string solve_inequalities(std::string name, std::string eq);
  /// python関数の呼び出しが成功したかどうかの判定
  static int check_error(PyObject *pValue);
};
} // namespace debug
} // namespace hydla
