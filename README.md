# How to execute LTL model checking on HyLaGI

## ビルド時の注意事項

- sss

## ビルド成功後の実行手順

1. 検証式を ​LTL2BA で never claim に変換し、テキストファイルに保存する（検査式ファイル）

- LTL2BAでは, y > 10 などの式を記述することは出来ないので, ```[](y > 10)``` に対応するBAを得たい時は ```[]p``` を入力する. ```p``` を ```y > 10``` に置き換える処理は後述

2. never claim 内の論理記号に対し、状態変数を用いた論理式をテキストファイルの冒頭に記述する

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

4. dot 言語の形式でオートマトンが出力される. 受理状態のサイクルがあれば, パスが赤色で表示される

### 使用するHydLa モデルに関する注意事項

- LTL モードによるオートマトンの生成を停止させるには, HydLa モデルの初期値に関して適切に抽象化を施し, フェーズ間の包含を成立させる必要がある.
  - 初期値 ```y=10``` で床を跳ねるボールの一回跳ねた後の放物線の軌道には, 二回跳ねた後の放物線の軌道が含まれない. それぞれの軌道に対応するオートマトンの状態は区別される
  - 初期値 ```0 < y < 10``` で床を跳ねるボールの一回跳ねた後の放物線の軌道には, 二回跳ねた後の放物線の軌道が含まれる. この時, 二回跳ねた後の放物線の軌道に対応する状態は新しく生成されず, 一回跳ねた後の放物線の軌道に対応する状態に包含される.