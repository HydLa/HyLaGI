package test.frame;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;

import test.Env;
import test.FrontEnd;

public class LangSettingFrame extends JFrame {
	private SelectPanel panel;

	public LangSettingFrame(){

		setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
		setTitle("Language");
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

		private ButtonGroup group = new ButtonGroup();
		private String[] labels = {"English","japanese"};
		private String[] langs = {"en","jp"};
		private JRadioButton[] radios = new JRadioButton[labels.length];

		private JButton ok = new JButton("OK");

		SelectPanel(JFrame frame){
			this.frame = frame;

			setLayout(new BoxLayout(this,BoxLayout.Y_AXIS));

			JLabel label = new JLabel();
			label.setText("Please select your languare.");
			label.setPreferredSize(new Dimension(250, 40));
			label.setAlignmentX(Component.CENTER_ALIGNMENT);
			add(label);

			JPanel radioPanel = new JPanel();
			radioPanel.setLayout(new GridLayout(3,1));

			for(int i=0;i<labels.length;++i){
				radios[i] = new JRadioButton(labels[i]);
				radios[i].setMargin(new Insets(2,10,2,10));
				group.add(radios[i]);
				radioPanel.add(radios[i]);
			}
			radios[0].setSelected(true);

			add(radioPanel);

			JPanel buttonPanel = new JPanel();

			ok.addActionListener(this);
			buttonPanel.add(ok);

			add(buttonPanel);

		}

		public void actionPerformed(ActionEvent e) {
			for(int i=0;i<labels.length;++i){
				if(radios[i].isSelected()){
					Env.set("LANG",langs[i]);
					break;
				}
			}
			frame.dispose();
		}

	}

}
