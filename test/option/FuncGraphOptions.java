package test.option;


import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.border.TitledBorder;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import test.Env;
import test.FrontEnd;
import test.funcgraph.FuncGraphPanel;
import test.funcgraph.FuncGraphRunner;
import test.gnuplot.GnuplotPanel;
import test.graph.PhaseViewer;
import test.runner.HyroseRunner;
import test.util.FixFlowLayout;

public class FuncGraphOptions extends JPanel implements ActionListener,DocumentListener{

	FuncGraphRunner funcGraphRunner;

	JTextField x_axis = new JTextField(15);
	JTextField y_axis = new JTextField(15);

	String TestOptions[] = {
			"FuncGraphOnAnotherPanel",
	};


	JCheckBox optionCheckBox[] = new JCheckBox[TestOptions.length];

	FuncGraphOptions(){

		setLayout(new FixFlowLayout());
		setBorder(new TitledBorder("FuncGraph Option"));

		settingInit();


		for(int i=0;i<TestOptions.length;++i){
			optionCheckBox[i] = new JCheckBox(TestOptions[i]);
			add(optionCheckBox[i]);
		}

		x_axis.getDocument().addDocumentListener(this);
		add(new JLabel("x_axis : "));
		add(x_axis);

		y_axis.getDocument().addDocumentListener(this);
		add(new JLabel("y_axis : "));
		add(y_axis);


		for(int i=0;i<TestOptions.length;++i){
			optionCheckBox[i].addActionListener(this);
		}

	}

	void settingInit(){
		String x_axis_val = Env.get("FUNC_X_AXIS");
		String y_axis_val = Env.get("FUNC_Y_AXIS");
		x_axis.setText(x_axis_val);
		y_axis.setText(y_axis_val);
	}

	public void actionPerformed(ActionEvent e) {

		FuncGraphPanel.setOnAnotherPanel(optionCheckBox[0].isSelected()?true:false);


		String newX_axis = x_axis.getText();
		Env.set("FUNC_X_AXIS",newX_axis);

		String newY_axis = y_axis.getText();
		Env.set("FUNC_Y_AXIS",newY_axis);
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

}

