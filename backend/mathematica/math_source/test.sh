#!/bin/sh

rlwrap math -run "optUseDebugPrint = True; timeOutS = Infinity; optOptimizationLevel = 0; optIgnoreWarnings = True; <<./common.m; <<./math_source.m << approx.m"