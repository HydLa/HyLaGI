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
	//��s�̗v�f��
	static int element_num;
	//�O���t������f�[�^
	 static double[] data;
	 //��s�̊e�v�f
	 static String[][] token = new String[500][3000];

	//������Ŏ擾�����f�[�^�𐔒l�ɕϊ�
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
						//�f�[�^�o�͒��̉��s�𖳎�����
						break;
//					}else if(line.startsWith("#")){
					}else if(line.contains("end")){
					}else if(k==0){
						//�f�[�^�o�͂̕ϐ��o�͕��𖳎�����
					}else{
						data[n] = PLOT_GRAPH_MAG_RATIO*token(i,k);
						n++;
					}//if�̏I���
				}//for(i)�̏I���
				k++;
			}//while�̏I���
		reader.close();
		} catch (FileNotFoundException e) {
			System.out.println(plotfile + "��������܂���");
			String sPath = (new File(".")).getAbsoluteFile().getParentFile().toString();
			System.out.println("���݂̃f�B���N�g����" + sPath + "�ł�");

		} catch (IOException e) {
			System.out.println(e);
		}//try�̏I���

	}

	public static void f_t_data(){
		//�V�~�����[�V�����^�C��
		double t = 0;
		//�e�L�X�g�̍s��
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
				}//for(i)�̏I���
				k++;
				l_number=k;
			}//while�̏I���

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
			System.out.println(plotfile + "��������܂���");
			String sPath = (new File(".")).getAbsoluteFile().getParentFile().toString();
			System.out.println("���݂̃f�B���N�g����" + sPath + "�ł�");

		} catch (IOException e) {
			System.out.println(e);
		}//try�̏I���

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
	}//f_t_data�̏I���

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
