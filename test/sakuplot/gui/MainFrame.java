package test.sakuplot.gui;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.BufferedWriter;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSpinner;
import javax.swing.JTextField;
import javax.swing.SpinnerNumberModel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import test.sakuplot.model.CompoundGraph;
import test.sakuplot.model.EpsGraphics;
import test.sakuplot.model.GraphModel;

public class MainFrame extends JPanel//JFrame
{
	private static final long serialVersionUID = 1L;

	private PlotPanel panel;
	private JTextField expInputX;
	private JTextField expInputY;
	private JButton drawButton;
	private JSpinner division;
	private JCheckBox antialias;

	private ArrayList<GraphModel> _graphs = new ArrayList<GraphModel>();
	private List<CompoundGraph> _graphList = new ArrayList<CompoundGraph>();

	private int _division = 100;

	public MainFrame()
	{
		setLayout(new BorderLayout(5, 5));

		{
			JPanel p = new JPanel();
			p.setLayout(new BorderLayout());

			panel = new PlotPanel();
			//panel.setPreferredSize(new Dimension(500, 500));
			panel.setCenter(250, 250);
			p.add(panel, BorderLayout.CENTER);

			JScrollPane sp = new JScrollPane(p);
			add(sp, BorderLayout.CENTER);
		}

		{
			JPanel bottomPanel = new JPanel();
			bottomPanel.getPreferredSize().width = 500;

			JPanel inputPanel = new JPanel();
			inputPanel.setLayout(new GridLayout(2, 1));

			JPanel p;

			p = new JPanel();

			expInputX = new JTextField();
			expInputX.setColumns(20);
			expInputX.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					drawButton.doClick();
				}
			});
			p.add(expInputX);

			inputPanel.add(p);

			p = new JPanel();

			expInputY = new JTextField();
			expInputY.setColumns(20);
			expInputY.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					drawButton.doClick();
				}
			});
			p.add(expInputY);

			inputPanel.add(p);

			//bottomPanel.add(inputPanel);

			drawButton = new JButton("Output EPS");
			drawButton.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					JOptionPane.showMessageDialog(MainFrame.this, "output out.eps in current directory");
					EpsGraphics eps = panel.createEps();
					PrintWriter writer;
					try
					{
						writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream("out.eps"))));
						writer.write(eps.getContents());
						writer.close();
					}
					catch (FileNotFoundException ex)
					{
						ex.printStackTrace();
					}
				}
			});
			bottomPanel.add(drawButton);

			SpinnerNumberModel model = new SpinnerNumberModel(_division, 100, 5000, 100);
			division = new JSpinner(model);
			division.addChangeListener(new ChangeListener()
			{
				public void stateChanged(ChangeEvent e)
				{
					int div = (Integer)division.getValue();
					_division = div;
				}
			});
			bottomPanel.add(division);

			antialias = new JCheckBox("antialias");
			antialias.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					panel.setAntialias(antialias.isSelected());
					panel.repaint();
				}
			});
			bottomPanel.add(antialias);

			{
			JButton buttonGraphs = new JButton("Graph");
			buttonGraphs.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					GraphListDialog glf = new GraphListDialog(MainFrame.this);
					glf.setItems(_graphList);
					glf.setVisible(true);
				}
			});
			bottomPanel.add(buttonGraphs);
			}

			{
			JButton buttonHyd = new JButton("HydLaParse");
			buttonHyd.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					HydLaParseDialog.showDialog(MainFrame.this, _graphs);
				}
			});
			bottomPanel.add(buttonHyd);
			}

			add(bottomPanel, BorderLayout.SOUTH);
		}

		//pack();
		setSize(500, 500);
		//setLocationRelativeTo(null);
	}

	public void doPlot(List<CompoundGraph> compoundGraphs)
	{
		_graphList = compoundGraphs;
		panel.setCompoundGraph(compoundGraphs);
	}
}
