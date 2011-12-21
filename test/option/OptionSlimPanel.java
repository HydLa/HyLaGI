

package test.option;

import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JCheckBox;
import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JTextField;
import javax.swing.border.TitledBorder;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import test.*;

public class OptionSlimPanel extends JPanel implements ActionListener,DocumentListener {

	String majorOption[] = {
			"-h",
			"-c",
			"--nd",
			"-d",
			"--dump-module-set-list",
			"--dump-parse-tree",
			"--version",
			"--profile",
			"-p",
			"--dump-module-set-list-noinit",
			"--dump-module-set-graph",
			"--dump-module-set-graph-noinit",
			"--graphviz",
			"-i",
	};
	JCheckBox optionCheckBox[] = new JCheckBox[majorOption.length];
	JTextField optionField = new JTextField(15);
	JTextField timeField = new JTextField(15);//time
	JTextField precField = new JTextField(15);//precision
	JTextField modeField = new JTextField(15);//mode

	OptionSlimPanel(){

		setLayout(new FlowLayout());
		setBorder(new TitledBorder("HydLa Option"));

		for(int i=0;i<majorOption.length;++i){
			optionCheckBox[i] = new JCheckBox(majorOption[i]);
			add(optionCheckBox[i]);
		}
//		JLabel label1 = new JLabel("exe_mode：");
//		add(label1);
//		add(optionField);
		JLabel label4 = new JLabel("exe_mode：");
		add(label4);
		add(modeField);
		JLabel label2 = new JLabel("exe_time：");
		add(label2);
		add(timeField);
		JLabel label3 = new JLabel("exe_precision：");
		add(label3);
		add(precField);


		add(optionField);

		settingInit();

		for(int i=0;i<majorOption.length;++i){
			optionCheckBox[i].addActionListener(this);
		}
		optionField.getDocument().addDocumentListener(this);
		timeField.getDocument().addDocumentListener(this);
		precField.getDocument().addDocumentListener(this);
		modeField.getDocument().addDocumentListener(this);

	}

	void settingInit(){
		for(int i=0;i<majorOption.length;++i){
			optionCheckBox[i].setSelected(false);
		}
		optionField.setText("");
		timeField.setText("");
		precField.setText("");
		modeField.setText("");

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
		String field = optionField.getText();
		if(field.length()>0){
			newOptions += " " + optionField.getText();
		}
		String time = timeField.getText();
		if(time.length()>0){
			newOptions += " -t " + timeField.getText();
		}
		String prec = precField.getText();
		if(prec.length()>0){
			newOptions += " --output-precision " + precField.getText();
		}
		String mode = modeField.getText();
		if(mode.length()>0){
			newOptions += " " + "-m" + " " + modeField.getText();
		}
		Env.set("SLIM_OPTION",newOptions);
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
