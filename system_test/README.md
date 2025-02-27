## Usage

/HyLaGI で`make test`すると統合テストが走る．
ただしデフォルトでは逐次でテストを行うので，
並列で行う場合は`make test fnum=2`などとする．ただし，MathematicaやWolfram Engine等のライセンスによって同時実行数が制限されている場合があるため注意．

## Files
- system_test.sh:  
  テストを行う
- compare_hydat.py:  
  2つのhydat中の結果が等しいか比べる
- calc_hydat_master.sh:  
  テストで正とするhydatを作る  
  master更新時に実行する
- /hydat/*.hydat.master:  
  テストで正とするhydat
