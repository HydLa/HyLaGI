package test.phaseviewer.gui;

import java.awt.Dimension;
import java.text.SimpleDateFormat;

import javax.swing.JTextArea;
import javax.swing.text.DefaultStyledDocument;

public class ConstraintTextArea extends JTextArea{
	/**
	 *
	 */
	private static final long serialVersionUID = 1L;

	public ConstraintTextArea(String s){
		super(s);
		this.setMinimumSize(new Dimension(100,200));
		this.setEditable(false);
	}

	public void println(String str){
		append(str+"\n");
	}

	public void errPrint(String str){

	}
}