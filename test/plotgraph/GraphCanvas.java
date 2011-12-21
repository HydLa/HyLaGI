package test.plotgraph;
//******************************************************************************
//グラフ描画用のキャンバス　GraphCanvas
//フィールドのdata[]にデータを入れると描画されます
//data[]の要素数は横サイズと同じ
//******************************************************************************

//# time  ht      v       ht'     v'

//==============================================================================
//クラスライブラリのインポート
//==============================================================================
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.Graphics2D;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionListener;
import java.awt.Toolkit;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import javax.swing.JPanel;
import javax.swing.JFrame;
import javax.swing.JLabel;


//==============================================================================
//GraphCanvasクラス
//==============================================================================
public class GraphCanvas extends Canvas implements MouseMotionListener
{

 //=============================================================================
 //フィールド
 //=============================================================================
	static final int GRAPH_NAME_X_POS = 50;
	static final int GRAPH_NAME_Y_POS = 20;
	static final int X_SIDE_GAP = 20;
	static final int Y_SIDE_GAP = 210;
	static final int PLOT_GRAPH_TIME_SELECT = 1;
	static final int PLOT_GRAPH_SELECT = 1;
	static final int PLOT_GRAPH_MAG_RATIO = 10;
	//マウスの位置
	int mouse_x;
	int mouse_y;

	Sliderpanel slider = new Sliderpanel();
	FileLoader fileloader = new FileLoader();

	//キャンバスの寸法
	static Dimension dimension;

 //=============================================================================
 //コンストラクタ
 //=============================================================================

	public GraphCanvas(int width,int height)
	{

		//領域のサイズ設定
		setSize(width,height);

		//領域のサイズ取得
		dimension = getSize();

		//data[]のインスタンス生成
		FileLoader.data = new double[dimension.width];

		//グラフエリアを灰色に設定
		setBackground(Color.white);

		//マウスの位置初期化
		mouse_x = 0;
		mouse_y = 0;

		addMouseMotionListener(this);
	}

 //=============================================================================
 //メソッド
 //=============================================================================
//ダブルバッファリング

	public void update(Graphics g){
		Image offScreenImage=createImage(MainFrame.CANVAS_WIDHT,MainFrame.CANVAS_HEGHT); // オフスクリーンイメージを作成
		Graphics offScreenGraphics=offScreenImage.getGraphics(); // オフスクリーンイメージに描画するための Graphics オブジェクト
		paint(offScreenGraphics); // 次の画面のイメージを作る．
		g.drawImage(offScreenImage,0,0,this); // イメージを本物のスクリーンに書き込む
	}

	//ダブルバッファリングその２

//	public void update(Graphics g){
//		paint(g);
//	}



	public void paint(Graphics g)
	{
		//領域のサイズ取得
		dimension = getSize();

		String positionStr= "(" + mouse_x + "," + mouse_y + ")";
		g.drawString(positionStr, 5, 40);

		//軸の色は黒
		g.setColor(Color.black);
		//x軸
		g.drawLine(0,dimension.height-Y_SIDE_GAP,dimension.width-1,dimension.height-Y_SIDE_GAP);
		//y軸
		g.drawLine(X_SIDE_GAP,0,X_SIDE_GAP,dimension.height-1);

		//グラフ線の色は青に設定
		g.setColor(Color.red);

		int n = 1;
		if(n==0){
			FileLoader.func_data();
		}else if(n==1){
			FileLoader.plot_data();
		}else if(n==2){
			FileLoader.f_t_data();
		}

		//グラフ描画
		for(int i=0;i<=dimension.width-2;i++){
			if(n==0){
				//func_data
				g.setColor(Color.green);
				g.drawLine( i+X_SIDE_GAP+i*Slider.value_of_width()/10, (int)( -FileLoader.data[i]*Slider.value_of_height() + dimension.height-Y_SIDE_GAP ), (i+X_SIDE_GAP)+1+i*Slider.value_of_width()/10, (int)( -FileLoader.data[i+1]*Slider.value_of_height() + dimension.height-Y_SIDE_GAP ) );
			}else if(n==1){
				//plot_data
				g.setColor(Color.blue);
				if(i%FileLoader.element_num==PLOT_GRAPH_SELECT)	g.drawLine( (i+X_SIDE_GAP)+i*Slider.value_of_width()/10, (int)( -FileLoader.data[i]*Slider.value_of_height() + dimension.height-Y_SIDE_GAP ), (i+X_SIDE_GAP)+i*Slider.value_of_width()/10+FileLoader.element_num, (int)( -FileLoader.data[i+5]*Slider.value_of_height() + dimension.height-Y_SIDE_GAP ) );
			}else if(n==2){
				//f_t_data
				g.setColor(Color.pink);
				g.drawLine( i+X_SIDE_GAP+i*Slider.value_of_width()/100, (int)( -FileLoader.data[i]*Slider.value_of_height() + dimension.height-Y_SIDE_GAP ), (i+X_SIDE_GAP)+1+i*Slider.value_of_width()/100, (int)( -FileLoader.data[i+1]*Slider.value_of_height() + dimension.height-Y_SIDE_GAP ) );
			}

		}
		//グラフの軸の単位
		if(n==1){
			g.setColor(Color.blue);
			g.drawString(FileLoader.token[PLOT_GRAPH_TIME_SELECT][0],dimension.width-X_SIDE_GAP*2,dimension.height/2+GRAPH_NAME_Y_POS);
			g.drawString(FileLoader.token[PLOT_GRAPH_SELECT+1][0],X_SIDE_GAP/4,GRAPH_NAME_Y_POS);
		}

		if(n==2){
			g.setColor(Color.black);
			g.drawString(FileLoader.token[0][20],dimension.width-X_SIDE_GAP*2,dimension.height/2+GRAPH_NAME_Y_POS);
			g.drawString(FileLoader.token[0][21],X_SIDE_GAP/4,GRAPH_NAME_Y_POS);
		}

		int y_scale = (int)FileLoader.data[0];
//		System.out.println(y_scale);
		g.setColor(Color.blue);
//		g.drawString("10",5, -y_scale + dimension.height-20+(-1)*(-5+slider.f()));
//		g.drawString("5",5,-y_scale + dimension.height-20+(-1)*(-10+slider.f()));

	}

	public void mouseMoved(MouseEvent e) {
		mouse_x = e.getX();
		mouse_y = e.getY();
	//	repaint();
	}

	public void mouseDragged(MouseEvent e) {}




}//クラスの終端


