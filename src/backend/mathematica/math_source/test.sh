#!/bin/sh

rlwrap math -run "optUseDebugPrint = True; timeOutS = Infinity; optIgnoreWarnings = True; timeDelta = 0; SetOptions[\$Output, PageWidth->Infinity]; <<./load_first/common.m; <<./math_source.m << approx.m << static_analysis.m << util.m"
