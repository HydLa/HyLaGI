package test.phaseviewer.gui;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.GridLayout;

import javax.swing.BorderFactory;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;

import edu.uci.ics.jung.visualization.VisualizationViewer;

public class ControlPanel {
	//テキストブロック
	private static ConstraintTextArea consTextArea = new ConstraintTextArea("");
	static JScrollPane scrollpane = new JScrollPane(getConsTextArea());
	//テキストパネルの東側
	public JPanel eastPanel = new JPanel(new GridLayout(2,0));
	//テキストパネルの西側
	JPanel westPanel = new JPanel(new GridLayout(4,0));
	private StartButton startbutton = new StartButton();
	private ScaleButton scalebutton = new ScaleButton();
	RadialButton radialbutton = new RadialButton();
	PickedModeComboBox pickedmodecombobox = new PickedModeComboBox();


	public ControlPanel(){

	}

	public JPanel set(VisualizationViewer<String, Integer> vv) {
    	//制約表示コーナー
    	scrollpane.setBorder(BorderFactory.createTitledBorder("Constraint"));
    	JPanel controls = new JPanel(new BorderLayout());
    	controls.add(setEast(vv),BorderLayout.EAST);
    	controls.add(setWest(vv),BorderLayout.WEST);
    	controls.add(scrollpane,BorderLayout.CENTER);
		return controls;
	}

	private JPanel setEast(VisualizationViewer<String, Integer> vv){
		eastPanel.add(startbutton.set());
		eastPanel.add(scalebutton.set(vv));
		return eastPanel;
	}

	private JPanel setWest(VisualizationViewer<String, Integer> vv){
		westPanel.add(radialbutton.set(vv));
		westPanel.add(pickedmodecombobox.set(vv));
		westPanel.add(new JLabel("Label Pos"));
		westPanel.add(new NodeLabel().setPosBox(vv));
		return westPanel;
	}

	public static ConstraintTextArea getConsTextArea() {
		return consTextArea;
	}

	public static void setConsTextArea(ConstraintTextArea consTextArea) {
		ControlPanel.consTextArea = consTextArea;
	}

	public StartButton getStartbutton() {
		return startbutton;
	}





}
