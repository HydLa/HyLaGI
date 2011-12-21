package test.plotgraph;
//******************************************************************************
//�O���t�`��p�̃L�����o�X�@GraphCanvas
//�t�B�[���h��data[]�Ƀf�[�^������ƕ`�悳��܂�
//data[]�̗v�f���͉��T�C�Y�Ɠ���
//******************************************************************************

//# time  ht      v       ht'     v'

//==============================================================================
//�N���X���C�u�����̃C���|�[�g
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
//GraphCanvas�N���X
//==============================================================================
public class GraphCanvas extends Canvas implements MouseMotionListener
{

 //=============================================================================
 //�t�B�[���h
 //=============================================================================
	static final int GRAPH_NAME_X_POS = 50;
	static final int GRAPH_NAME_Y_POS = 20;
	static final int X_SIDE_GAP = 20;
	static final int Y_SIDE_GAP = 210;
	static final int PLOT_GRAPH_TIME_SELECT = 1;
	static final int PLOT_GRAPH_SELECT = 1;
	static final int PLOT_GRAPH_MAG_RATIO = 10;
	//�}�E�X�̈ʒu
	int mouse_x;
	int mouse_y;

	Sliderpanel slider = new Sliderpanel();
	FileLoader fileloader = new FileLoader();

	//�L�����o�X�̐��@
	static Dimension dimension;

 //=============================================================================
 //�R���X�g���N�^
 //=============================================================================

	public GraphCanvas(int width,int height)
	{

		//�̈�̃T�C�Y�ݒ�
		setSize(width,height);

		//�̈�̃T�C�Y�擾
		dimension = getSize();

		//data[]�̃C���X�^���X����
		FileLoader.data = new double[dimension.width];

		//�O���t�G���A���D�F�ɐݒ�
		setBackground(Color.white);

		//�}�E�X�̈ʒu������
		mouse_x = 0;
		mouse_y = 0;

		addMouseMotionListener(this);
	}

 //=============================================================================
 //���\�b�h
 //=============================================================================
//�_�u���o�b�t�@�����O

	public void update(Graphics g){
		Image offScreenImage=createImage(MainFrame.CANVAS_WIDHT,MainFrame.CANVAS_HEGHT); // �I�t�X�N���[���C���[�W���쐬
		Graphics offScreenGraphics=offScreenImage.getGraphics(); // �I�t�X�N���[���C���[�W�ɕ`�悷�邽�߂� Graphics �I�u�W�F�N�g
		paint(offScreenGraphics); // ���̉�ʂ̃C���[�W�����D
		g.drawImage(offScreenImage,0,0,this); // �C���[�W��{���̃X�N���[���ɏ�������
	}

	//�_�u���o�b�t�@�����O���̂Q

//	public void update(Graphics g){
//		paint(g);
//	}



	public void paint(Graphics g)
	{
		//�̈�̃T�C�Y�擾
		dimension = getSize();

		String positionStr= "(" + mouse_x + "," + mouse_y + ")";
		g.drawString(positionStr, 5, 40);

		//���̐F�͍�
		g.setColor(Color.black);
		//x��
		g.drawLine(0,dimension.height-Y_SIDE_GAP,dimension.width-1,dimension.height-Y_SIDE_GAP);
		//y��
		g.drawLine(X_SIDE_GAP,0,X_SIDE_GAP,dimension.height-1);

		//�O���t���̐F�͐ɐݒ�
		g.setColor(Color.red);

		int n = 1;
		if(n==0){
			FileLoader.func_data();
		}else if(n==1){
			FileLoader.plot_data();
		}else if(n==2){
			FileLoader.f_t_data();
		}

		//�O���t�`��
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
		//�O���t�̎��̒P��
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




}//�N���X�̏I�[


