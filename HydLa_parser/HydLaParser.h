#ifndef _INCLUDED_HYDLA_PARSER_H_
#define _INCLUDED_HYDLA_PARSER_H_

#include <string>
#include <map>
#include <iterator>

#include <boost/spirit/iterator/classic_file_iterator.hpp>
#include <boost/spirit/iterator/classic_position_iterator.hpp>
#include <boost/spirit/tree/classic_ast.hpp>

namespace hydla {

class HydLaParser {
public:
  typedef boost::spirit::file_iterator<char> file_iter_t;
  typedef boost::spirit::position_iterator<file_iter_t> pos_iter_t;
  typedef boost::spirit::node_val_data_factory<> node_val_data_factory_t;
  typedef boost::spirit::tree_parse_info<pos_iter_t, node_val_data_factory_t> tree_info_t;
//  typedef boost::spirit::tree_parse_info<> tree_info_t;
  typedef boost::spirit::tree_match<pos_iter_t, node_val_data_factory_t>::tree_iterator	tree_iter_t;
//  typedef boost::spirit::tree_match<char const*>::tree_iterator	tree_iter_t;

  typedef std::map<std::string, std::string> module_map_t;
  typedef std::map<std::string, int>         variable_map_t;

  HydLaParser();
  ~HydLaParser();

  bool parse_flie(const char* filename);
  //bool parse_string(const char* str);

  void dump() {
    dump_tree(ast_tree_.trees.begin(), 0);
  }

  std::string create_interlanguage(std::string max_time, bool debug);

private:
  void dump_tree(const tree_iter_t &iter, int nest) const;

  std::string create_interlanguage(const tree_iter_t &iter);

  tree_info_t ast_tree_;

  module_map_t module_;
  variable_map_t variable_;
};
}
#endif //_INCLUDED_HYDLA_PARSER_H_
