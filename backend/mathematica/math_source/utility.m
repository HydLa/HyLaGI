publicMethod[
  equivalent,
  lhs, rhs,
  Module[
    {},
    ToString[Reduce[Implies[lhs, rhs] && Implies[rhs, lhs], Reals] ]
  ]
];
