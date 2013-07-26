#!/bin/sh

rlwrap math -run "optUseDebugPrint = True; timeOutS = Infinity; optOptimizationLevel = 0; optIgnoreWarnings = True; <<./virtual_constraint_solver/mathematica/math_source/vcs_math_source.m"