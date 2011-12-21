package test.option;


import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.border.TitledBorder;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import test.Env;
import test.FrontEnd;
import test.funcgraph.FuncGraphPanel;
import test.gnuplot.GnuplotPanel;
import test.graph.PhaseViewer;
import test.robot.RobotFrame;
import test.runner.HyroseRunner;
import test.runner.extensiverunner.ERframe;
import test.runner.extensiverunner.ExtensiveRunner;
import test.util.FixFlowLayout;

public class TestOptions extends JPanel implements ActionListener,DocumentListener{

	public ERframe erframe;
	public RobotFrame robotFrame;

	String TestOptions[] = {
			"PhaseViewerOnAnotherPanel",
			"AutomatonTextOutput",
			"GnuplotOnAnotherPanel",
			"Extensive execution",
			"(Robo?)",
	};


	JButton ParopnteButton = new JButton("ぱろぷんて");
	JCheckBox optionCheckBox[] = new JCheckBox[TestOptions.length];


	TestOptions(){

		setLayout(new FixFlowLayout());
		setBorder(new TitledBorder("Test Option"));

		settingInit();

		for(int i=0;i<TestOptions.length;++i){
			optionCheckBox[i] = new JCheckBox(TestOptions[i]);
			add(optionCheckBox[i]);
		}


		for(int i=0;i<TestOptions.length;++i){
			optionCheckBox[i].addActionListener(this);
		}


		add(ParopnteButton);
		ParopnteButton.addActionListener(this);
		ParopnteButton.addMouseListener(new MouseAdapter(){
			public void mouseClicked(MouseEvent evt){
				if((ERframe.isERchecker() && RobotFrame.isRobochecker())){
					JOptionPane.showMessageDialog(null, "ぱろぷんて！!!");
				}else if(ERframe.isERchecker()){
					createERframe();
				}else if(RobotFrame.isRobochecker()){
					createRoboFrame();
				}else{
					JOptionPane.showMessageDialog(null, "ぱろぷんて！");
				}
			}
		});

	}



	void settingInit(){

	}

	public void actionPerformed(ActionEvent e) {
		PhaseViewer.setOnAnotherPanel(ocbIsTorF(0));
		if(optionCheckBox[1].isSelected()){/*automatonのテキスト出力*/}
		if(optionCheckBox[2].isSelected()){/*Gnuplotが他のパネルに表示される=true;*/}
		ERframe.setERchecker(ocbIsTorF(3));
		RobotFrame.setRoboChecker(ocbIsTorF(4));
	}



	public void changedUpdate(DocumentEvent e) {
		actionPerformed(null);
	}

	public void insertUpdate(DocumentEvent e) {
		actionPerformed(null);
	}

	public void removeUpdate(DocumentEvent e) {
		actionPerformed(null);
	}

	private void createERframe() {
		erframe = new ERframe();
	}

	private void createRoboFrame() {
		robotFrame = new RobotFrame();
	}

	private boolean ocbIsTorF(int n){
		return optionCheckBox[n].isSelected()?true:false;
	}
}

