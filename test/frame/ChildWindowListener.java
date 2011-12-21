
package test.frame;

import java.awt.Frame;
import java.awt.Window;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;

import javax.swing.JFrame;

import test.FrontEnd;

public class ChildWindowListener implements WindowListener {

	private Window window;

	public ChildWindowListener(Window window){
		this.window = window;
	}

	public void windowOpened(WindowEvent e) {
		if(FrontEnd.mainFrame!=null){
			FrontEnd.mainFrame.addChildWindow(window);
		}
	}

	public void windowClosed(WindowEvent e) {
		if(FrontEnd.mainFrame!=null){
			FrontEnd.mainFrame.removeChildWindow(window);
		}
	}

	public void windowActivated(WindowEvent e) {

	}

	public void windowDeactivated(WindowEvent e) {

	}

	public void windowDeiconified(WindowEvent e) {

	}

	public void windowIconified(WindowEvent e) {

	}

	public void windowClosing(WindowEvent e) {

	}

}
