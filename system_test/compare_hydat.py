import subprocess
import sys
import json
from pprint import pprint
from collections import defaultdict
from collections import deque

g = None
idx = None

def same_expr(e1, e2, assum='True'):
  if e1 == e2:
    return True
  
  code = 'Simplify[Simplify[{}]==Simplify[{}],{}]'.format(e1, e2, assum)
  command = ['wolframscript', '-code', code]
  res = subprocess.check_output(command)
  return res == b'True\n'

def paramaps2assum(paramaps):
  assum = 'False'
  for paramap in paramaps:
    for k, v in paramap.items():
      tmp = 'True'
      if 'unique_value' in v:
        tmp += ' && {} == {}'.format(k, v['unique_value'])
      else:
        for lb in v['lower_bounds']:
          tmp += ' && {} <{} {}'.format(lb['value'], '=' if lb['closed'] else '', k)
        for ub in v['upper_bounds']:
          tmp += ' && {} <{} {}'.format(k, '=' if ub['closed'] else '', ub['value'])
      assum += ' || ' + tmp
  if assum == 'False':
    assum = 'True'
  return assum

def time2assum(p):
  if p['type'] == 'PP':
    return 't == {}'.format(p['time']['time_point'])
  else:
    return '{} < t < {}'.format(p['time']['start_time'], p['time']['end_time'])

def same_phase(p1, p2):
  #print('start check new phase')

  p1assum = paramaps2assum(p1['parameter_maps'])
  p2assum = paramaps2assum(p2['parameter_maps'])
  #print(' check assum:\n  {}\n  {}'.format(p1assum, p2assum))
  if not same_expr(p1assum, p2assum):
    #print('different phase: different assum')
    return False
  assum = p1assum

  if p1['type'] == 'PP':
    #print(' check time:\n  {}\n  {}\n  with {}'.format(p1['time']['time_point'], p2['time']['time_point'], assum))
    if not same_expr(p1['time']['time_point'], p2['time']['time_point'], assum):
      #print('different phase: different time_point')
      return False
  else:
    #print(' check start time:\n  {}\n  {}\n  with {}'.format(p1['time']['start_time'], p2['time']['start_time'], assum))
    if not same_expr(p1['time']['start_time'], p2['time']['start_time'], assum):
      #print('different phase: different start_time')
      return False
    #print(' check end time:\n  {}\n  {}\n  with {}'.format(p1['time']['end_time'], p2['time']['end_time'], assum))
    if not same_expr(p1['time']['end_time'], p2['time']['end_time'], assum):
      #print('different phase: different end_time')
      return False
  tassum = time2assum(p1)
  assum = '({}) && {}'.format(assum, tassum)

  if p1['variable_map'].keys() != p2['variable_map'].keys():
    #print('different phase: different variable set')
    return False
  for k, v1 in p1['variable_map'].items():
    v2 = p2['variable_map'][k]
    # TODO: 'unique_value'とは？
    #print(' check variable {}:\n  {}\n  {}\n  with {}'.format(k, v1['unique_value'], v2['unique_value'], assum))
    if not same_expr(v1['unique_value'], v2['unique_value'], assum):
      #print('different phase: different value:', k)
      return False
  #print('same phase')
  return True

def init_graph(firsts):
  global g
  global idx
  g = defaultdict(list)
  idx = defaultdict(dict)
  for p in firsts:
    g[0].append(p['id'])

def dfs(phases):
  global g
  global idx
  for p in phases:
    idx[p['id']] = p
    
    for ch in p['children']:
      g[p['id']].append(ch['id'])
    
    dfs(p['children'])

def iso(g1, idx1, g2, idx2):
  q1 = deque()
  q2 = deque()
  q1.append(0)
  q2.append(0)

  while q1:
    p1 = q1.pop()
    p2 = q2.pop()
    #print('children len: ', len(g1[p1]), len(g2[p2]))
    if len(g1[p1]) != len(g2[p2]):
      #print('differen children size')
      return False
    for n1 in g1[p1]:
      if idx1[n1]['simulation_state'] == 'NOT_SIMULATED':
        continue
      f = True
      for n2 in g2[p2]:
        if idx2[n2]['simulation_state'] == 'NOT_SIMULATED':
          continue
        #print('start find same child')
        if same_phase(idx1[n1], idx2[n2]):
          #print('find same child: ', n1, n2)
          q1.append(n1)
          q2.append(n2)
          f = False
          break
      if f:
        #print('can\'t find same child')
        return False
  return True

if __name__=='__main__':
  args = sys.argv

  gs = []
  idxs = []
  msg = 'compare target:\n'
  for i in range(1, len(args)):
    msg += ' ' + args[i] + '\n'
    with open(args[i]) as f:
      s = f.read()
      d = json.loads(s)
      
      init_graph(d['first_phases'])
      dfs(d['first_phases'])
      gs.append(g)
      idxs.append(idx)

  g = gs.pop()
  idx = idxs.pop()
  for i in range(len(gs)):
    if not iso(g, idx, gs[i], idxs[i]):
      print(msg + '\033[31mdifferent result\033[0m')
      sys.exit(1)
  print(msg + '\033[32msame result\033[0m')