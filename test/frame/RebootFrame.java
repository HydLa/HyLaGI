

package test.frame;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;

import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JTextField;

import test.Env;
import test.FrontEnd;
import test.Lang;

import test.*;
//import lavit.runner.SlimInstaller;

public class RebootFrame extends JFrame  {
	private SelectPanel panel;

	public RebootFrame(){

		setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		setTitle("HydLa Reboot");
		setIconImage(Env.getImageOfFile(Env.IMAGEFILE_ICON));
        setAlwaysOnTop(true);
        setResizable(false);

		panel = new SelectPanel(this);
		add(panel);

		addWindowListener(new ChildWindowListener(this));

		pack();
		setLocationRelativeTo(FrontEnd.mainFrame);
		setVisible(true);

	}

	private class SelectPanel extends JPanel implements ActionListener {
		private JFrame frame;

		private JTextField pathInput = new JTextField();

//		private JButton reboot = new JButton(Lang.m[27]);
//		private JButton cancel = new JButton(Lang.d[2]);
		private JButton reboot = new JButton("reboot");
		private JButton cancel = new JButton("cancel");

		SelectPanel(JFrame frame){
			this.frame = frame;

			setLayout(new BoxLayout(this,BoxLayout.Y_AXIS));

			JPanel inputPanel = new JPanel();

			JLabel label = new JLabel();
			label.setText("Memory");
			inputPanel.add(label);

			pathInput.setPreferredSize(new Dimension(100,20));
			inputPanel.add(pathInput);

			add(inputPanel);


			JPanel buttonPanel = new JPanel();

			reboot.addActionListener(this);
			buttonPanel.add(reboot);

			cancel.addActionListener(this);
			buttonPanel.add(cancel);

			add(buttonPanel);

			if(getMemory().equals("")){
				pathInput.setText("512M");
			}else{
				pathInput.setText(getMemory());
			}

		}

		public void actionPerformed(ActionEvent e) {
			Object src = e.getSource();
			if(src==cancel){
				frame.dispose();
			}else if(src==reboot){
				Env.set("REBOOT_MAX_MEMORY",pathInput.getText());
				frame.dispose();
				FrontEnd.reboot();
			}

		}

		private String getMemory(){
			String memory = Env.get("REBOOT_MAX_MEMORY");
			if(memory==null){
				return "";
			}else{
				return memory;
			}
		}

	}
}
