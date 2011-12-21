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

public class GnuplotPathSettingFrame extends JFrame  {
	private SelectPanel panel;

	public GnuplotPathSettingFrame(){

		setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		setTitle("Gnuplot");
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
//		private JButton fileChooser = new JButton(Lang.w[0]);
		private JButton fileChooser = new JButton("éQè∆");

//		private JButton ok = new JButton(Lang.d[6]);
//		private JButton cancel = new JButton(Lang.d[2]);
		private JButton ok = new JButton("OK");
		private JButton cancel = new JButton("Cancel");

		SelectPanel(JFrame frame){
			this.frame = frame;

			setLayout(new BoxLayout(this,BoxLayout.Y_AXIS));

			JLabel label = new JLabel();
//			label.setText(Lang.w[1]);
			label.setText("GnuplotÇÃÉpÉXÇê›íËÇµÇƒÇ≠ÇæÇ≥Ç¢");
			label.setPreferredSize(new Dimension(350, 40));
			label.setAlignmentX(Component.CENTER_ALIGNMENT);
			add(label);

			JPanel inputPanel = new JPanel();

			pathInput.setPreferredSize(new Dimension(200,20));
			inputPanel.add(pathInput);

			fileChooser.addActionListener(this);
			inputPanel.add(fileChooser);

			add(inputPanel);


			JPanel buttonPanel = new JPanel();

			ok.addActionListener(this);
			buttonPanel.add(ok);

			cancel.addActionListener(this);
			buttonPanel.add(cancel);

			add(buttonPanel);

			if(getGnuplotPath().equals("")){
				pathInput.setText("C:\\gnuplot\\binary\\gnuplot.exe");
			}else{
				pathInput.setText(getGnuplotPath());
			}

		}

		public void actionPerformed(ActionEvent e) {
			Object src = e.getSource();
			if(src==fileChooser){
				JFileChooser chooser;
				String path = pathInput.getText();
				if((new File(path)).exists()){
					chooser = new JFileChooser((new File(path)).getParent());
				}else{
					chooser = new JFileChooser((new File(".")));
				}
				chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
				int res = chooser.showOpenDialog(this);
				if(res==JFileChooser.APPROVE_OPTION) {
					pathInput.setText(chooser.getSelectedFile().getAbsolutePath());
				}
			}else if(src==cancel){
				frame.dispose();
			}else if(src==ok){
				Env.set("GNUPLOT_PATH",pathInput.getText());
				frame.dispose();
			}
		}

		private String getGnuplotPath(){
			String path = Env.get("GNUPLOT_PATH");
			if(path==null){
				return "";
			}else{
				return path;
			}
		}

	}
}
