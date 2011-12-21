package test.phaseviewer.graph;

import test.phaseviewer.filecontrols.FilePreparation;
import test.phaseviewer.gui.ControlPanel;

public class ClearGraph{
	public void clearTreeGraph(){
		new DrawGraph().drawTreeGraph(new FilePreparation("Nulltext.txt"));
	}

}
