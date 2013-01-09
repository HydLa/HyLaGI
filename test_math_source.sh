#!/bin/sh

rlwrap math -run "optUseDebugPrint = True; timeOutS = Infinity; optOptimizationLevel = 0; <<./virtual_constraint_solver/mathematica/vcs_math_source.m"