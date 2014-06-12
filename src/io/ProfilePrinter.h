#pragma once

#include "Types.h"

namespace hydla{
namespace io{

/**
 * プロファイリング結果の出力を担当するクラス
 * とりあえず時間だけ
 */

class ProfilePrinter{
public:
  /**
   * 解軌道木全体を出力する関数
   */
  virtual void print_profile(const entire_profile_t&) const = 0;
};

}// output
}// hydla
