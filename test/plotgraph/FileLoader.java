package test.plotgraph;

import java.awt.*;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionListener;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;



public class FileLoader {

	static String exp;
	public static String plotfile;

	static final int PLOT_GRAPH_MAG_RATIO = 10;
	//一行の要素数
	static int element_num;
	//グラフ化するデータ
	 static double[] data;
	 //一行の各要素
	 static String[][] token = new String[500][3000];

	//文字列で取得したデータを数値に変換
	public static double token(int i,int k){
		if((token[i][k].indexOf(" "))!=0){
			String a = token[i][k];
			double value = Double.parseDouble(a);
			return value;
		}else{
			return -1;
		}
	}

	public static void plot_data(){
		int n=0;
		try {
			BufferedReader reader = new BufferedReader(new FileReader(plotfile));

			String line;
			int k=0;
			while((line = reader.readLine())!=null){
				String[] block = line.trim().split("\\s+");
				 element_num = block.length;

				for(int i = 0;i<element_num;i++) {
					token[i][k] = block[i];
					if(line.equals("")){
						//データ出力中の改行を無視する
						break;
//					}else if(line.startsWith("#")){
					}else if(line.contains("end")){
					}else if(k==0){
						//データ出力の変数出力部を無視する
					}else{
						data[n] = PLOT_GRAPH_MAG_RATIO*token(i,k);
						n++;
					}//ifの終わり
				}//for(i)の終わり
				k++;
			}//whileの終わり
		reader.close();
		} catch (FileNotFoundException e) {
			System.out.println(plotfile + "が見つかりません");
			String sPath = (new File(".")).getAbsoluteFile().getParentFile().toString();
			System.out.println("現在のディレクトリは" + sPath + "です");

		} catch (IOException e) {
			System.out.println(e);
		}//tryの終わり

	}

	public static void f_t_data(){
		//シミュレーションタイム
		double t = 0;
		//テキストの行数
		int l_number=0;

		String d1 = null,d2 = null;

		try {
			BufferedReader reader = new BufferedReader(new FileReader(plotfile));

			String line;
			int k=0;
			while((line = reader.readLine())!=null){
				String[] block = line.trim().split("\\s+");
				int element_num = block.length;

				for(int i = 0;i<element_num;i++) {
					token[i][k] = block[i];
				}//for(i)の終わり
				k++;
				l_number=k;
			}//whileの終わり

			for(int j=0;j<l_number;j++){
				if(token[0][j].equals("---------IP---------")){
					for(int l=0;l<5;l++){
						System.out.println("time_token["+l+"]["+(j+1)+"]"+token[l][j+1]);
						d1 = token[4][8];
					}

					for(int l=0;l<5;l++){
						System.out.println("ht_token["+l+"]["+(j+2)+"]"+token[l][j+2]);
						d2 = token[2][21];
					}
				}
			}
		reader.close();
		} catch (FileNotFoundException e) {
			System.out.println(plotfile + "が見つかりません");
			String sPath = (new File(".")).getAbsoluteFile().getParentFile().toString();
			System.out.println("現在のディレクトリは" + sPath + "です");

		} catch (IOException e) {
			System.out.println(e);
		}//tryの終わり

		System.out.println("d1:" + d1);
		System.out.println("d2:" + d2);
		exp = d2;

		for(int n=0;n<GraphCanvas.dimension.width-2;n++){
//			if(t < token(4,8)){
			if(0 <= t && t <= Math.sqrt(2)){
//				data[n] = d1;
				data[n] = (-5)*(-2+Math.pow(t, 2));
				t+=0.01;
//				System.out.println("t="+t);
//			}else if(token(4,8)<t && t<token(4,20)){
			}else if(Math.pow(2,1.0/2.0) <= t && t <= token(4,20)){
//				data[n] = d2;
				data[n] = (-26+18*Math.pow(2, 0.5)*(t)+(-5)*Math.pow(t, 2));

				t+=0.01;
//				System.out.println("t="+t);
			}else{
				break;
			}
		}
	}//f_t_dataの終わり

	public static void func_data(){
		double d = 0;
		for(int n=0;n<GraphCanvas.dimension.width-2;n++){

			if(d<Math.sqrt(2)){
				data[n] = (-5)*(-2+Math.pow(d, 2));
				d+=0.01;
				System.out.println("d="+d);
			}else if(Math.sqrt(2)<d && d<3){
				data[n] = (-26+18*Math.pow(2, 0.5)*(d)+(-5)*Math.pow(d, 2));
				d+=0.01;
				System.out.println("d="+d);
			}else{
				break;
			}
		}

	}


}
