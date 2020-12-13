## Usage
/HyLaGIで```make test```すると統合テストが走る．  
ただしデフォルトでは約40並列でテストを行うので，  
ローカルで行う場合は/HyLaGI/system_test/system_test.shの```fnum```に適当な値を入れること．

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
