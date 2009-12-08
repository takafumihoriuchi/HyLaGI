#ifndef _INCLUDED_HYDLA_SYMBOLIC_VALUE_H_
#define _INCLUDED_HYDLA_SYMBOLIC_VALUE_H_

#include <iostream>

namespace hydla {
namespace symbolic_simulator {

struct SymbolicValue {
  bool rational; // 分数の形かどうか
  int numerator; // 分子
  int denominator; // 分母

  /**
   * データをダンプする
   */
  std::ostream& dump(std::ostream& s) const
  {
    if(rational) s << "Rational[numerator,denominator]";
    else s << numerator;
    return s;
  }

  friend std::ostream& operator<<(std::ostream& s, 
                                  const SymbolicValue & v)
  {
    return v.dump(s);
  }
};

} //namespace symbolic_simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_SYMBOLIC_VALUE_H_
