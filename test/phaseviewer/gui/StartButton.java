package test.phaseviewer.gui;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;

import test.phaseviewer.filecontrols.FilePreparation;
import test.phaseviewer.graph.DrawGraph;
import test.phaseviewer.graph.Rings;

public class StartButton {

	private DrawGraph drawgraph = new DrawGraph();
	private Rings rings;
	public JButton set() {
    	JButton start = new JButton("start");
        start.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
        		drawgraph.drawTreeGraph(new FilePreparation("pp_ip_output.txt"));
        		//êßñÒÇÃï\é¶
        		//new NodeLabel().putConstraintLabel(drawgraph.getmap());
        		rings = new Rings(getDrawgraph());
        		ControlPanel.getConsTextArea().println("*** DrawGraph ***");
            }
        });
    	return start;
	}
	public DrawGraph getDrawgraph() {
		return drawgraph;
	}

	public Rings getRings(){
		return rings;
	}

}
