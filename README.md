# How to execute LTL model checking on HyLaGI

1. 検証式を ​LTL2BA で never claim に変換し、テキストファイルに保存する（検査式ファイル）

2. never claim 内の論理記号に対し、状態変数を用いた論理式を割り当てる

   - 以下は ```![]<>(y > 7)``` の記述をするにあたり, ```![]<>p``` をLTL2BAに入力して得られた出力の冒頭に ```p := y>7 ``` を加えたもの
  
```
p := y>7
never { /* !GFp */
T0_init :    /* init */
	if
	:: (1) -> goto T0_init
	:: (!p) -> goto accept_S2
	fi;
accept_S2 :    /* 1 */
	if
	:: (!p) -> goto accept_S2
	fi;
}
```
 
3. --fltl オプション、および適当なフェーズ数を指定し、標準入力から検査式ファイルを与えて実行する
   
   ``` hylagi model.hydla < models_ltl.txt -fltl -p15 ```
