

package test.option;

import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.Dimension;
import java.math.BigDecimal;

import javax.swing.event.ChangeListener;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JSpinner;
import javax.swing.SpinnerNumberModel;
import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JTextField;
import javax.swing.border.TitledBorder;
import javax.swing.event.ChangeEvent;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import test.*;

public class OptionHyrosePanel extends JPanel implements ActionListener,DocumentListener {

	String majorOption[] = {
			"--comprehensive",
			"--debug",
			"--dump-module-set-list",
			"--dump-parse-tree",
			"--dump-in-progress",
			"--dump-module-set-list-noinit",
			"--dump-module-set-graph",
			"--dump-module-set-graph-noinit",
			"--exclude-error",
			"--graphviz",
			"--help",
			"--interlanguage",
			"--in",
			"--nd",
			"--parallel",
			"--profile",
			"--version",
	};

	static String modeoption[] = {
			"SymbolicSimulator",
			"BandPSimulator",
			"SymbolicLegacySimulator"
	};

	static String formatoption[] = {
		"",
		"n",
		"t",
		"g"
	};

	static String solveroption[] = {
		"",
		"m",
		"r"
	};

	JCheckBox optionCheckBox[] = new JCheckBox[majorOption.length];//option
	JComboBox modeCheckBox = new JComboBox(modeoption);//mode
	JComboBox formatCheckBox = new JComboBox(formatoption);
	JComboBox solverCheckBox = new JComboBox(solveroption);
	SpinnerNumberModel timespinner = new SpinnerNumberModel(1,0,1000,1);//exe_time
	SpinnerNumberModel precspinner = new SpinnerNumberModel(5,1,10000,1);//precision
	JTextField approxOptionField = new JTextField(5);
	JTextField stepOptionField = new JTextField(5);
	JTextField varOptionField = new JTextField(10);
	JTextField mathlinkOptionField = new JTextField(5);
	JTextField areaOptionField = new JTextField(5);
	JTextField optionFreeField = new JTextField(15);//FreeCommand

	OptionHyrosePanel(){

		setLayout(new FlowLayout());
		setBorder(new TitledBorder("HydLa Option"));

		for(int i=0;i<majorOption.length;++i){
			optionCheckBox[i] = new JCheckBox(majorOption[i]);
			add(optionCheckBox[i]);
		}

		add(new JLabel("Exe_Prec:"));
		JSpinner prec = new JSpinner(precspinner);
		prec.setPreferredSize(new Dimension(80,20));
		add(prec);

		add(new JLabel("ModeOption:"));
		modeCheckBox.setEditable(true);
		add(modeCheckBox);

		add(new JLabel("FormatOption:"));
		formatCheckBox.setEditable(true);
		formatCheckBox.setPreferredSize(new Dimension(60,20));
		add(formatCheckBox);

		add(new JLabel("SolverOption:"));
		solverCheckBox.setEditable(true);
		solverCheckBox.setPreferredSize(new Dimension(60,20));
		add(solverCheckBox);

		add(new JLabel("Simulation_Time:"));
		JSpinner timer = new JSpinner(timespinner);
		timer.setPreferredSize(new Dimension(80,20));
		add(timer);

		add(new JLabel("Approx:"));
		add(approxOptionField);

		add(new JLabel("step:"));
		add(stepOptionField);

		add(new JLabel("output variables:"));
		add(varOptionField);

		add(new JLabel("area trace:"));
		add(areaOptionField);

		add(new JLabel("mathlink:"));
		add(mathlinkOptionField);

		add(new JLabel("Free_Command:"));
		add(optionFreeField);


		settingInit();

		for(int i=0;i<majorOption.length;++i){
			optionCheckBox[i].addActionListener(this);
		}
		optionFreeField.getDocument().addDocumentListener(this);
		approxOptionField.getDocument().addDocumentListener(this);
		stepOptionField.getDocument().addDocumentListener(this);
		varOptionField.getDocument().addDocumentListener(this);
		areaOptionField.getDocument().addDocumentListener(this);
		mathlinkOptionField.getDocument().addDocumentListener(this);
		modeCheckBox.addActionListener(this);
		formatCheckBox.addActionListener(this);
		timer.addChangeListener(listener);
		prec.addChangeListener(listener);

	}

	void settingInit(){
		for(int i=0;i<majorOption.length;++i){
			optionCheckBox[i].setSelected(false);
		}
		optionFreeField.setText("");

		Env.set("SLIM_OPTION",thisOptions);

		String[] options = Env.get("SLIM_OPTION").split(" ");
		for(String o : options){
			boolean exist = false;
			for(int i=0;i<majorOption.length;++i){
				if(majorOption[i].equals(o)){
					optionCheckBox[i].setSelected(true);
					exist = true;
				}
			}
/*			if(!exist){
				String text = optionField.getText();
				if(text.length()==0){
					optionField.setText(o);
				}else{
					optionField.setText(text+" "+o);
				}
			}
*/
		}
	}

	//オプションの値
	static String ExOptions0 = setTime(1) + setPrec(5);
	static String ExOptions1 = setMode(modeoption[0]);// + setFormat(formatoption[0]);
	static String thisOptions = ExOptions0+ExOptions1;

	ChangeListener listener = new ChangeListener() {
		public void stateChanged(ChangeEvent e){
			String newOptions = "";
//			double time = (Double)timespinner.getValue();
			Integer time = (Integer)timespinner.getValue();
			if(time>0){
//				double dec = Decimal2((Double)timespinner.getValue());
//				newOptions +=setTime(exeResultTime);
				newOptions +=setTime(time);
			}
			Integer prec = (Integer)precspinner.getValue();
			if(prec>0){
				newOptions += setPrec(prec);
			}
			ExOptions0 = newOptions;
			thisOptions = ExOptions0+ExOptions1;
			Env.set("SLIM_OPTION",thisOptions);
		}

		private double Decimal2(Double value) {
			BigDecimal exeIntime = new BigDecimal(value);
			BigDecimal bd = exeIntime.setScale(2, BigDecimal.ROUND_HALF_EVEN);
			double exeResultTime = bd.doubleValue();
			return exeResultTime;
		}
	};

	int mode = modeCheckBox.getItemCount();
	public void actionPerformed(ActionEvent e) {
		String newOptions = "";
		for(int i=0;i<majorOption.length;++i){
			if(optionCheckBox[i].isSelected()){
				if(newOptions.length()==0){
					newOptions += optionCheckBox[i].getText();
				}else{
					newOptions += " " + optionCheckBox[i].getText();
				}
			}
		}

		Object sol = solverCheckBox.getSelectedItem();
		if(!sol.equals("")){
			newOptions += setSolver(""+sol);
		}else{
//			if(newOptions.contains("-s")){
//				newOptions = newOptions.replaceAll("-s","");
//			}
		}

		if(mode>0){
			newOptions += setMode(""+modeCheckBox.getSelectedItem());
		}
		Object fom = formatCheckBox.getSelectedItem();
		if(!fom.equals("")){
			newOptions += setFormat(""+fom);
		}else{
			if(newOptions.contains("-f")){
				newOptions = newOptions.replaceAll("-f","");
			}
		}

		String field = optionFreeField.getText();
		if(field.length()>0){
			newOptions += " " + optionFreeField.getText();
		}

		String ap = approxOptionField.getText();
		if(ap.length()>0){
			newOptions += " "+"--approx"+" " + ap;
		}

		String st = stepOptionField.getText();
		if(st.length()>0){
			newOptions += " "+"--step"+" "+ st;
		}

		String va = varOptionField.getText();
		if(va.length()>0){
			newOptions += " "+"--output-variables"+" " + va;
		}

		String ar = areaOptionField.getText();
		if(ar.length()>0){
			newOptions += " "+"--area"+" " + ar;
		}

		String ma = mathlinkOptionField.getText();
		if(ma.length()>0){
			newOptions += " "+"--mathlink"+" " + ma;
		}

		ExOptions1 = newOptions;
		thisOptions = ExOptions0+ExOptions1;
		Env.set("SLIM_OPTION",thisOptions);
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

	static String setMode(String mode){
		mode = " "+"--method"+" "+mode;
		return mode;
	}

	static String setTime(int time){
		String timestr = " "+"--time"+" "+time;
		return timestr;
	}

	static String setPrec(int prec){
		String precstr = " "+"--output-precision"+" "+prec+" ";
		return precstr;
	}

	private static String setFormat(String format) {
		format = " "+"-f"+" "+format;
		return format;
	}

	private static String setSolver(String solver) {
		solver = " "+"-s"+" "+solver;
		return solver;
	}


}
