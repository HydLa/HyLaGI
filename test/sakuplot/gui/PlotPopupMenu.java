package test.sakuplot.gui;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

public class PlotPopupMenu extends JPopupMenu
{
	private static final long serialVersionUID = 1L;
	
	private PlotPanel _plotpanel;
	
	public PlotPopupMenu(PlotPanel plotpanel)
	{
		_plotpanel = plotpanel;
		
		JMenuItem menu = new JMenuItem("Reset View");
		menu.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				_plotpanel.resetDisplayArea();
			}
		});
		add(menu);
		
		menu = new JMenuItem("Scroll to visible");
		menu.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				_plotpanel.setDisplayAreaToVisible();
			}
		});
		add(menu);
		
		menu = new JMenuItem("Preferences...");
		menu.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				PlotOptionDialog dialog = new PlotOptionDialog(_plotpanel);
				dialog.setVisible(true);
			}
		});
		add(menu);
	}
}
