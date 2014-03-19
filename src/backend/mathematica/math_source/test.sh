#!/bin/sh

rlwrap math -run "optUseDebugPrint = True; timeOutS = Infinity; optIgnoreWarnings = True; <<./must_be_first/common.m; <<./math_source.m << approx.m << static_analysis.m << util.m"