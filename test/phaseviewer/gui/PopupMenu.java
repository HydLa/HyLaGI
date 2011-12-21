package test.phaseviewer.gui;

import java.awt.Menu;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

import test.phaseviewer.filecontrols.FilePreparation;
import test.phaseviewer.graph.ClearGraph;


public class PopupMenu extends JPopupMenu
{
	private static final long serialVersionUID = 1L;

	JMenuItem menuitem;
	JMenu menu;
	Text text = new Text();

	public PopupMenu()
	{

		menuitem = new JMenuItem("Clear");
		menuitem.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				new ClearGraph().clearTreeGraph();
				ControlPanel.getConsTextArea().println("*** Clear ***");
			}
		});
		add(menuitem);

			JMenu menuhelp = new JMenu("help");
			menuitem = new JMenuItem("Controll Instructions");
			menuitem.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					System.out.println("ëÄçÏê‡ñæ");
				}
			});
			menuhelp.add(menuitem);

			JMenu menug = new JMenu("Glossary");
			menug.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					System.out.println("ópåÍâê‡");
				}
			});
			menuhelp.add(menug);

				menuitem = new JMenuItem("Ç¢ÇÎÇ¢ÇÎÇ»Ç‡ÇÃ");
				menuitem.addActionListener(new ActionListener()
				{
					public void actionPerformed(ActionEvent e)
					{
						System.out.println("Ç¢ÇÎÇ¢ÇÎÇ»Ç‡ÇÃ");
					}
				});
				menug.add(menuitem);

				menuitem = new JMenuItem(text.helptext());
				menuitem.addActionListener(new ActionListener()
				{
					public void actionPerformed(ActionEvent e)
					{
						System.out.println("Ç≥Ç‹Ç¥Ç‹Ç»Ç‡ÇÃ");
					}
				});
				menug.add(menuitem);


			add(menuhelp);


	}
}
