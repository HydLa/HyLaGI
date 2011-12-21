package test.sakuplot.gui;

import java.awt.BorderLayout;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.List;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

import test.Env;
import test.funcgraph.FileToString;
import test.sakuplot.model.CompoundGraph;
import test.sakuplot.model.GraphModel;
import test.sakuplot.model.SimulationCase;
import test.sakuplot.model.SimulationCaseParser;

public class HydLaParseDialog extends JDialog
{
	private static final long serialVersionUID = 1L;

	private static HydLaParseDialog _theInstance = null;

	private MainFrame _parent;
	private JTextArea _textArea;

	private HydLaParseDialog()
	{
		setTitle("HydLa Case-text parsing test");
		setSize(400, 400);
		setLocationRelativeTo(null);
		setLayout(new BorderLayout(2, 2));

		_textArea = new JTextArea();
		_textArea.setFont(new Font(Font.MONOSPACED, Font.PLAIN, 14));
		add(new JScrollPane(_textArea), BorderLayout.CENTER);

		JButton buttonWRY = new JButton("Set Graph");
		buttonWRY.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				doParse();
				setVisible(false);
			}
		});
		add(buttonWRY, BorderLayout.SOUTH);
	}

//	private void doParse()
//	{
//		String text = _textArea.getText();
//		String x_axis = Env.get("FUNC_X_AXIS");
//		String y_axis = Env.get("FUNC_Y_AXIS");
//		if (!text.isEmpty())
//		{
//			SimulationCaseParser scp = new SimulationCaseParser();
//			List<SimulationCase> cases = scp.parseCases(text);
//			List<CompoundGraph> graphList = new ArrayList<CompoundGraph>();
//			for (SimulationCase c : cases)
//			{
//				graphList.add(CompoundGraph.createFromSimulation(c, x_axis, y_axis));
//			}
//			_parent.doPlot(graphList);
//		}
//	}

	private void doParse()
	{
		new Thread(){
			public void run(){
		
		String text = _textArea.getText();
		String x_axis = Env.get("FUNC_X_AXIS");
		String y_axis = Env.get("FUNC_Y_AXIS");
		if (!text.isEmpty())
		{
			SimulationCaseParser scp = new SimulationCaseParser();
			List<SimulationCase> cases = scp.parseCases(text);
			List<CompoundGraph> graphList = new ArrayList<CompoundGraph>();
			for (SimulationCase c : cases)
			{
				graphList.add(CompoundGraph.createFromSimulation(c, x_axis, y_axis));
				try {
					Thread.sleep(500);
					_parent.doPlot(graphList);
				} catch (InterruptedException e) {
					// TODO é©ìÆê∂ê¨Ç≥ÇÍÇΩ catch ÉuÉçÉbÉN
					e.printStackTrace();
				}
			}
			
			//_parent.doPlot(graphList);
		}
		
			}
		}.start();
	}

	
	public static void showDialog(MainFrame parent, List<GraphModel> graphs)
	{
		if (_theInstance == null)
		{
			_theInstance = new HydLaParseDialog();
		}
		_theInstance._parent = parent;
		_theInstance.setVisible(true);
	}
}


