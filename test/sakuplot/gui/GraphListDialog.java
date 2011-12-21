package test.sakuplot.gui;

import java.awt.Component;
import java.util.List;

import javax.swing.DefaultListModel;
import javax.swing.JDialog;
import javax.swing.JList;
import javax.swing.JScrollPane;

import test.sakuplot.model.CompoundGraph;
import test.sakuplot.model.GraphModel;

public class GraphListDialog extends JDialog
{
	private static final long serialVersionUID = 1L;

	private DefaultListModel _listModel = new DefaultListModel();
	private JList _graphList = new JList();

	public GraphListDialog(Component parent)
	{
		setTitle("Graph list");
		setSize(400, 230);
		setLocationRelativeTo(parent);

		_graphList.setModel(_listModel);
		add(new JScrollPane(_graphList));
	}

	public void setItems(List<CompoundGraph> graphList)
	{
		_listModel.clear();
		for (CompoundGraph compoundGraph : graphList)
		{
			int i = 0;
			for (GraphModel graph : compoundGraph.getGraphList())
			{
				_listModel.addElement("[" + compoundGraph.getName() + "] IP" + i + ": " + graph.toString());
				i++;
			}
		}
	}
}
