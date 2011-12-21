


package test.frame;

import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.GraphicsEnvironment;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.BoxLayout;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JProgressBar;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;
import javax.swing.border.LineBorder;
import javax.swing.border.TitledBorder;

import test.Env;
import test.FrontEnd;
import test.Lang;

//import test.*;
import test.util.FixFlowLayout;

public class VersionFrame extends JFrame implements ActionListener{// implements ActionListener
	JButton button;

	String strTable[] = {
			"HydLa",
			"Version : "+Env.APP_VERSION,
			"Date : "+Env.APP_DATE,
			"",
//			Env.LMNTAL_VERSION,
//			Env.SLIM_VERSION,
//			Env.UNYO_VERSION
			Env.HYDLA_VERSION
	};

	VersionFrame(){
		ImageIcon image = new ImageIcon(Env.getImageOfFile("img/logo.png"));

		setIconImage(Env.getImageOfFile(Env.IMAGEFILE_ICON));
		setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
		setAlwaysOnTop(true);

	    JPanel panel = new JPanel();
	    panel.setBackground(new Color(255,255,255));
	    panel.setLayout(new BoxLayout(panel,BoxLayout.Y_AXIS));
		add(panel);

		JLabel icon = new JLabel(image);
		icon.setAlignmentX(Component.CENTER_ALIGNMENT);
//		panel.add(icon);

		JPanel lPanel = new JPanel();
		lPanel.setBorder(new LineBorder(Color.WHITE , 10));
		lPanel.setBackground(Color.WHITE);
		lPanel.setLayout(new GridLayout((int)(strTable.length/1),1));
		for(String s : strTable){
			lPanel.add(new JLabel(s));
		}
		panel.add(lPanel);

		button = new JButton("OK");
		button.addActionListener(this);
		button.setAlignmentX(Component.CENTER_ALIGNMENT);
		panel.add(button);

		addWindowListener(new ChildWindowListener(this));

		pack();
		setLocationRelativeTo(null);
//	    setVisible(true);

	}

//	@Override
	public void actionPerformed(ActionEvent e) {
		Object src = e.getSource();
		if(src==button){
			dispose();
		}

	}

}
