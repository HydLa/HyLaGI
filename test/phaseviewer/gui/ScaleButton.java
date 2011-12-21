package test.phaseviewer.gui;

import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JPanel;

import edu.uci.ics.jung.visualization.VisualizationViewer;
import edu.uci.ics.jung.visualization.control.CrossoverScalingControl;
import edu.uci.ics.jung.visualization.control.ScalingControl;

public class ScaleButton {

	public JPanel set(VisualizationViewer<String,Integer> vv){
        JPanel scaleGrid = new JPanel(new GridLayout(0,1));
        scaleGrid.setBorder(BorderFactory.createTitledBorder("Scale"));
        scaleGrid.add(setPlusButton(vv));
        scaleGrid.add(setMinusButton(vv));
    		return scaleGrid;
	}

    private JButton setPlusButton(final VisualizationViewer<String,Integer> vv){
    	final ScalingControl scaler = new CrossoverScalingControl();
        JButton plus = new JButton("+");
        plus.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                scaler.scale(vv, 1.1f, vv.getCenter());
            }
        });
    	return plus;
    }

    private JButton setMinusButton(final VisualizationViewer<String,Integer> vv){
    	final ScalingControl scaler = new CrossoverScalingControl();
    	JButton minus = new JButton("-");
        minus.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                scaler.scale(vv, 1/1.1f, vv.getCenter());
            }
        });
    	return minus;
    }

}
