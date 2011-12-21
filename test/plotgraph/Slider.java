package test.plotgraph;

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.JSlider;
import java.awt.BorderLayout;
import java.awt.Container;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

public class Slider extends JPanel{

	static final int MAX_SL0 = 30;
	static final int MAX_SL1 = 100;
	static final int MIN_SL0 = 0;
	static final int MIN_SL1 = -10;
	static final int INT_SL0 = 1;
	static final int INT_SL1 = 0;

	static JSlider slider0 = new JSlider();
	static JSlider slider1 = new JSlider();
	static JLabel label0 = new JLabel();
	static JLabel label1 = new JLabel();

	public void height_slider(){
		//スライダ最大値・最小値・初期値・目盛り付け設定
		  slider0 = new JSlider(JSlider.HORIZONTAL,MIN_SL0, MAX_SL0, INT_SL0);
		  slider0.setMajorTickSpacing(5);
		  slider0.setMinorTickSpacing(1);  // 小目盛の設定
		  slider0.setPaintTicks(true);
		  slider0.setPaintLabels(true);
		  slider0.addChangeListener(
		   new ChangeListener() {
		    public void stateChanged(ChangeEvent e) {
		    	//MainFrame.canvas.repaint();//グラフキャンバスに対し再描画
		    	repaint();
		    	label0.setText("Value: " + value_of_height());
		    }
		   });
		  label0.setText("Value: " + value_of_height());
	}

	public void width_slider(){
		slider1 = new JSlider(JSlider.HORIZONTAL,MIN_SL1, MAX_SL1, INT_SL1);
		  slider1.setMajorTickSpacing(5);
		  slider1.setMinorTickSpacing(1);  // 小目盛の設定
		  slider1.setPaintTicks(true);
		  slider1.setPaintLabels(true);

		  slider1.addChangeListener(
		   new ChangeListener() {
		    public void stateChanged(ChangeEvent e) {
		    	//MainFrame.canvas.repaint();//※(2)グラフキャンバスに対し再描画
		    	repaint();
			  label1.setText("Value: " + value_of_width());
		    }
		   });
		  label1.setText("Value: " + value_of_width());
	}

	public static int value_of_height() {
		 int value = slider0.getValue();
		 return value;
		}

	 public static int value_of_width() {
		 int value = slider1.getValue();
		 return value;
		}
}
