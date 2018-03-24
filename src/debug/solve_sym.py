from sympy import *


def add_equation(name, eq):
  exec("global " + name + ";" + name + " = " + eq)
  return str(1)

def solve_equation(name1, name2):
  exec("sol = solve(" + name1 + "," + name2 + ")")
  return str(sol)

def solve_inequalities(name1, name2):
  exec("sol = solve(" + name1 + "," + name2 + ")")
  return str(sol)

def add_var(name_var):
  var(name_var)
  return str(1)
