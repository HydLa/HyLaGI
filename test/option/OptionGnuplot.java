
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
import test.gnuplot.GnuplotPanel;
import test.util.FixFlowLayout;

public class OptionGnuplot extends JPanel implements ActionListener,DocumentListener{

	JTextField optionField = new JTextField(15);
	JCheckBox thDimension = new JCheckBox("3D");
	JCheckBox output = new JCheckBox("Output .png");
	JCheckBox PanelRatio = new JCheckBox("SquarePanel");

	OptionGnuplot(){

		setLayout(new FixFlowLayout());
		setBorder(new TitledBorder("gnuplot Option"));

		settingInit();

		thDimension.addActionListener(this);
		add(thDimension);

		output.addActionListener(this);
		add(output);

		PanelRatio.addActionListener(this);
		add(PanelRatio);

		optionField.getDocument().addDocumentListener(this);
		add(new JLabel("dataLine : "));
		add(optionField);


	}

	void settingInit(){
		String[] options = Env.get("GNUPLOT_OPTION").split(" ");
		for(String o : options){
				String text = optionField.getText();
				if(text.length()==0){
					optionField.setText(o);
				}else{
					optionField.setText(text+" "+o);
				}
		}
	}

	public void actionPerformed(ActionEvent e) {
		if(thDimension.isSelected()){
			GnuplotPanel.ThDimension=true;
		}else{
			GnuplotPanel.ThDimension=false;
		}
//		FrontEnd.println("GnuplotPanel.ThDimension : "+GnuplotPanel.ThDimension);
		if(output.isSelected()){
			GnuplotPanel.output=true;
		}else{
			GnuplotPanel.output=false;
		}

		if(PanelRatio.isSelected()){
			GnuplotPanel.PanelRatio=true;
		}else{
			GnuplotPanel.PanelRatio=false;
		}

		String newOptions = optionField.getText();
//		FrontEnd.println("newOptions : "+newOptions);
		Env.set("GNUPLOT_OPTION",newOptions);
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
