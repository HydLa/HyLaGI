#pragma once

#include <string>

#include "NodeTreeGenerator.h"
#include "HydLaAST.h"

hydla::symbolic_expression::node_sptr parse_arithmetic_string(const std::string &str);
