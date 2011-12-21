package test.funcgraph;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;

public class FileToString {
	  // ファイル内容をを文字列化するメソッドです。
	  public String fileToString(File file) throws IOException {
	    BufferedReader br = null;
	    try {
	      // ファイルを読み込むバッファドリーダを作成します。
	      br = new BufferedReader(new InputStreamReader(new FileInputStream(file)));
	      // 読み込んだ文字列を保持するストリングバッファを用意します。
	      StringBuffer sb = new StringBuffer();
	      // ファイルから読み込んだ一文字を保存する変数です。
	      int c;
	      // ファイルから１文字ずつ読み込み、バッファへ追加します。
	      while ((c = br.read()) != -1) {

	        sb.append((char) c);
	      }
	      // バッファの内容を文字列化して返します。
	      String str = sb.toString();
	      str = str.replace("`","");

	      return str;
	    } finally {
	      // リーダを閉じます。
	      br.close();
	    }
	  }
}