SetDirectory["./src/backend/mathematica/math_source/"];
$CharacterEncoding="UTF-8";
optUseDebugPrint = True;
timeOutS = Infinity;
optIgnoreWarnings = True;
SetOptions[$Output, PageWidth->Infinity];
<<"./load_first/common.m";
<<"./math_source.m";
<< "./approx.m";
<< "./static_analysis.m";
<< "util.m";



