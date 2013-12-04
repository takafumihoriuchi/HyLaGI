#ifndef _INCLUDED_HYDLA_BACKEND_INTERFACE_ERROR_H_
#define _INCLUDED_HYDLA_BACKEND_INTERFACE_ERROR_H_

namespace hydla{
namespace backend{

class InterfaceError : public std::runtime_error{
public:
  InterfaceError(const std::string& msg) : 
  std::runtime_error(init(msg))
  {}

private:
    std::string init(const std::string& msg)
  {
    std::stringstream s;
    s << "error of backend interface: " << msg << std::endl;
    return s.str();
  }
};

}
}

#endif