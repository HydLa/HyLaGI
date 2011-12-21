package test.phaseviewer.gui;

import java.awt.event.MouseEvent;

import javax.swing.JScrollBar;

import edu.uci.ics.jung.visualization.control.GraphMouseListener;

public class PvGraphMouseListener<V> implements GraphMouseListener<V>  {
	JScrollBar jscrollbar = new JScrollBar();
	@Override
    public void graphClicked(V v, MouseEvent me) {
    	//append�ɉ�ʂ��ǂ���
    	{
        jscrollbar = ControlPanel.scrollpane.getVerticalScrollBar();
        jscrollbar.setValue(jscrollbar.getMaximum());
    	}
    	ControlPanel.getConsTextArea().println(new TextChanger().changedText((String) v));
    }

	@Override
	public void graphPressed(V arg0, MouseEvent arg1) {
		// TODO �����������ꂽ���\�b�h�E�X�^�u

	}

	@Override
	public void graphReleased(V arg0, MouseEvent arg1) {
		// TODO �����������ꂽ���\�b�h�E�X�^�u

	}

}
