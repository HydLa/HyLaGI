#!/bin/sh

rlwrap math -run "optUseDebugPrint = True; timeOutS = Infinity; optIgnoreWarnings = True; <<./load_first/common.m; <<./math_source.m << approx.m << static_analysis.m << util.m"
