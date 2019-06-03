SetDirectory["./src/backend/mathematica/math_source/"];
$CharacterEncoding="UTF-8";
optUseDebugPrint = True;
timeOutS = Infinity;
optIgnoreWarnings = True;
optTimeConstraint = 1;
SetOptions[$Output, PageWidth->Infinity];
<<"./load_first/common.m";
<<"./math_source.m";
<< "./approx.m";
<< "./epsilon.m";
<< "./static_analysis.m";
<< "./findMinTimeWithRelaxation.m";
<< "./convex.m";
<< "util.m";
SetDirectory["../../../"];
