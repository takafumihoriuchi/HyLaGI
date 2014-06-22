#pragma once


#include "ParseTree.h"
#include "Simulator.h"
#include "CMMap.h"

namespace hydla{
namespace simulator{

class ConstraintAnalyzer 
{
public:
  typedef enum{
    CONDITIONS_TRUE,
    CONDITIONS_FALSE,
    CONDITIONS_VARIABLE_CONDITIONS
  } ConditionsResult;


  /**
   * 文字列(制約モジュール集合の名前)をキーに、そのモジュール集合に対応する条件(矛盾の条件もしくは無矛盾の条件)を持つ型
   */
  typedef std::map<std::string, hydla::symbolic_expression::node_sptr> conditions_map_t;
  //  typedef std::map<module_set_sptr, symbolic_expression::node_sptr> conditions_map_t;
  ConstraintAnalyzer(); 
  ConstraintAnalyzer(backend_sptr_t back);
  virtual ~ConstraintAnalyzer();

  virtual void set_backend(backend_sptr_t back);
  /**
   * 制約モジュール集合とそれに対する条件を出力する関数
   * analysis_fileで指定したファイルに書き出す
   * 指定がなければ標準出力に書き出す
   */
  virtual void print_conditions();

  //void add_continuity(const continuity_map_t& continuity_map, const PhaseType &phase);

  /**
   * msに対応する条件を算出する関数
   * bがTrueならmsが無矛盾となる条件を
   * bがFalseならmsが矛盾する条件を求める
   * 求めた条件はconditions_に入る
   * @return
   * CONDITIONS_TRUE : 得られた条件がTrueだった場合
   * CONDITIONS_FALSE : 得られた条件がFalseだった場合
   * CONDITIONS_VARIABLE_CONDITIONS : 得られた条件が何らかの条件だった場合
   */
//  virtual ConditionsResult find_conditions(const module_set_sptr& ms, bool b);


  /**
   * すべての解候補制約モジュール集合に対してfind_conditionsを適用させる関数
   */
  virtual void check_all_module_set(bool b);

  /**
   * cm_listの対応する場所にmsを入れる
   */
//  virtual void add_new_cm(const module_set_sptr& ms);

protected:

  /**
   * 制約モジュール集合の名前とそれに対応する条件のマップ
   */
  conditions_map_t conditions_;

  /**
   * 無矛盾の条件をキーに持ち、その条件がTrueのときに
   * 極大無矛盾となる制約モジュール集合を値に持つmapのlist
   * 順番は極大な方から順に持っている
   */
  //hierarchy::cm_map_list_t cm_list_;

  /**
   * 上のリストのルート
   * 多分いらないけど持っておく
   */
  //hierarchy::cm_map_sptr root_cm_;

  backend_sptr_t backend_;
};

}
}
