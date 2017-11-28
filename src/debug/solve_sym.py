from sympy import *


def add_equation(name, eq):
  #var("a:z")
  #var("zz")
  #x, y, z = symbols("x y z")
  exec("global " + name + ";" + name + " = " + eq)
  #exec("print(type("+name+"))")
  #print(type(expr0))
  #exec("print("+name+")")
  exec("print("+eq+")")
  #print((expr0))
  return str(1)

def solve_equation(name1, name2):
  #exec("sol = solve([" + name1 + ", " + name2 + "],[x, y, z, zz])")
  #print(ask_0_WALL)
  exec("sol = solve(" + name1 + "," + name2 + ")")
  return str(sol)
  
def solve_inequalities(name1, name2):
  #exec("sol = solve([" + name1 + ", " + name2 + "],[x, y, z, zz])")
  #print(ask_0_WALL)
  exec("sol = solve(" + name1 + "," + name2 + ")")
  return str(sol)

def add_var(name_var):
  var(name_var)
  return str(1)
