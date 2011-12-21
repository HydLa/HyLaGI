package test.phaseviewer.gui;

import java.awt.Point;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import edu.uci.ics.jung.visualization.VisualizationViewer;



public class RightClick {

	private boolean _drag;
	private Point _mouse;

	   public void set(VisualizationViewer<String, Integer> vv){
	        vv.addMouseListener(new MouseAdapter()
			{
				public void mousePressed(MouseEvent e)
				{
					if (e.isPopupTrigger())
					{
						PopupMenu popup = new PopupMenu();
						popup.show(e.getComponent(), e.getX(), e.getY());
					}
					else if (e.getButton() == MouseEvent.BUTTON1)
					{
						_mouse = e.getPoint();
						_drag = true;
					}
				}

				public void mouseReleased(MouseEvent e)
				{
					if (e.isPopupTrigger())
					{
						PopupMenu popup = new PopupMenu();
						popup.show(e.getComponent(), e.getX(), e.getY());
					}
					_drag = false;
				}
			});
	    }
}
