## Usage
/HyLaGIで```make test```すると統合テストが走る．  
ただしデフォルトでは約40並列でテストを行うので，  
ローカルで行う場合は/HyLaGI/system_test/system_test.shの```fnum```に適当な値を入れること．

## Files
- system_test.sh: テストを行う
- compare_hydat.py: 2つのhydat中の結果が等しいか比べる
- master.sh: テストの答えとしてmasterでの実行結果を作る
- /hydat/*.hydat.master: master.shの結果できる答え