package test.graph;

import java.awt.Menu;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

import test.FrontEnd;

public class PopupMenu extends JPopupMenu
{
	private static final long serialVersionUID = 1L;

	JMenuItem menuitem;
	JMenu menu;
	private PhaseViewer _pv;

	public PopupMenu(PhaseViewer pv)
	{
		_pv = pv;

		menuitem = new JMenuItem("Clear");
		menuitem.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				FrontEnd.println("Clear");
			}
		});
		add(menuitem);

			JMenu menuhelp = new JMenu("help");
			menuitem = new JMenuItem("Controll Instructions");
			menuitem.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					FrontEnd.println("ëÄçÏê‡ñæ");
				}
			});
			menuhelp.add(menuitem);

			JMenu menug = new JMenu("Glossary");
			menug.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					FrontEnd.println("ópåÍâê‡");
				}
			});
			menuhelp.add(menug);

				menuitem = new JMenuItem("Ç¢ÇÎÇ¢ÇÎÇ»Ç‡ÇÃ");
				menuitem.addActionListener(new ActionListener()
				{
					public void actionPerformed(ActionEvent e)
					{
						FrontEnd.println("Ç¢ÇÎÇ¢ÇÎÇ»Ç‡ÇÃ");
						_pv.consTextArea.println("Ç¢ÇÎÇ¢ÇÎÇ«Ç´Ç„ÇÒ");
					}
				});
				menug.add(menuitem);

				menuitem = new JMenuItem(_pv.getHelpText());
				menuitem.addActionListener(new ActionListener()
				{
					public void actionPerformed(ActionEvent e)
					{
						FrontEnd.println("Ç≥Ç‹Ç¥Ç‹Ç»Ç‡ÇÃ");
						_pv.consTextArea.errPrint("Ç«Ç±Ç…Ç«Å[Ç»ÇÒÇÃ");
					}
				});
				menug.add(menuitem);


			add(menuhelp);


	}
}
