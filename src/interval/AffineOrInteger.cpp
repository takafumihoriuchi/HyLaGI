#include "AffineOrInteger.h"

namespace hydla{
namespace interval{

std::ostream& operator<<(std::ostream &ost, const AffineOrInteger &val)
{
  if(val.is_integer)
  {
    ost << "integer: " << val.integer;
  }else
  {
    ost << "affine: " << val.affine_value;
  }
  return ost;
}

AffineOrInteger AffineOrInteger::operator+(const AffineOrInteger &rhs)
{
  AffineOrInteger ret;
  if(is_integer && rhs.is_integer)
  {
    ret.is_integer = true;
    ret.integer = integer + rhs.integer;
  }
  else
  {
    ret.is_integer = false;
    if(is_integer)
    {
      ret.affine_value = integer + rhs.affine_value;
    }
    else
    {
      if(rhs.is_integer)
      {
        ret.affine_value = affine_value + rhs.integer;
      }
      else
      {
        ret.affine_value = affine_value + rhs.affine_value;
      }
    }
  }
  return ret;
}
AffineOrInteger AffineOrInteger::operator-(const AffineOrInteger &rhs)
{
  AffineOrInteger ret;
  if(is_integer && rhs.is_integer)
  {
    ret.is_integer = true;
    ret.integer = integer - rhs.integer;
  }
  else
  {
    ret.is_integer = false;
    if(is_integer)
    {
      ret.affine_value = integer - rhs.affine_value;
    }
    else
    {
      if(rhs.is_integer)
      {
        ret.affine_value = affine_value - rhs.integer;
      }
      else
      {
        ret.affine_value = affine_value - rhs.affine_value;
      }
    }
  }
  return ret;
}
AffineOrInteger AffineOrInteger::operator*(const AffineOrInteger &rhs)
{
  AffineOrInteger ret;
  if(is_integer && rhs.is_integer)
  {
    ret.is_integer = true;
    ret.integer = integer * rhs.integer;
  }
  else
  {
    ret.is_integer = false;
    if(is_integer)
    {
      ret.affine_value = integer * rhs.affine_value;
    }
    else
    {
      if(rhs.is_integer)
      {
        ret.affine_value = affine_value * rhs.integer;
      }
      else
      {
        ret.affine_value = affine_value * rhs.affine_value;
      }
    }
  }
  return ret;
}


AffineOrInteger AffineOrInteger::operator/(const AffineOrInteger &rhs)
{
  AffineOrInteger ret;
  if(is_integer && rhs.is_integer &&
     (integer > rhs.integer && integer % rhs.integer == 0)
    )
  {
    ret.is_integer = true;
    ret.integer = integer / rhs.integer;
  }
  else
  {
    ret.is_integer = false;
    if(is_integer)
    {
      ret.affine_value = (double)integer * rhs.affine_value;
    }
    else
    {
      if(rhs.is_integer)
      {
        ret.affine_value = affine_value * (double)rhs.integer;
      }
      else
      {
        ret.affine_value = affine_value * rhs.affine_value;
      }
    }
  }
  return ret;
}

AffineOrInteger AffineOrInteger::operator-()
{
  AffineOrInteger ret;
  ret.is_integer = is_integer;
  if(is_integer)
  {
    ret.integer = -integer;
  }
  else
  {
    ret.affine_value = -affine_value;
  }
  return ret;
}

}
}