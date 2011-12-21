package test.plotgraph;
//******************************************************************************
//MainFrame.java
//グラフ描画のためのアプリケーションの外枠です。
//******************************************************************************
import java.awt.*;
import java.awt.List;
import java.util.*;
import java.io.*;
import javax.swing.*;

import java.awt.BorderLayout;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import java.awt.event.*;
import javax.swing.event.*;



//MainFrame
//==============================================================================
public class MainFrame extends JPanel
{
	static final int START_PANEL_X_POINT = 100;
	static final int START_PANEL_Y_POINT = 100;
	static final int CANVAS_WIDHT = 640;
	static final int CANVAS_HEGHT = 420;
	static final int FRAME_WIDTH = 640;
	static final int FRAME_HEGHT = 540;

	  //GraphCanvas作成
	static GraphCanvas canvas = new GraphCanvas(CANVAS_WIDHT,CANVAS_HEGHT);
	Sliderpanel sliderpanel = new Sliderpanel();


	public MainFrame(String plotfile){
		if (plotfile=="") {
			System.out.println("ファイルが見つかりません");
		}else{
			FileLoader.plotfile=plotfile;
		}

		  //setContentPane(new MainFrame());
		  setBounds(START_PANEL_X_POINT,START_PANEL_Y_POINT,FRAME_WIDTH,FRAME_HEGHT);
		  setSize(FRAME_WIDTH,FRAME_WIDTH);
		  //setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		  //setResizable(true);
		  //フレーム可視化
		  setVisible(true);
		  setLayout(new BorderLayout());
		  add(sliderpanel, BorderLayout.SOUTH);
		  add(canvas,BorderLayout.CENTER);

	}
}
