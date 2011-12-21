

package test;

import java.io.File;
import java.io.IOException;
import java.util.HashSet;

import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;

import test.frame.*;
import test.runner.RebootRunner;
import test.util.CommonFontUser;

public class FrontEnd {

	static public FrontEnd frontEnd;
	static public MainFrame mainFrame;
	static public StartupFrame sf;

	static public HashSet<CommonFontUser> fontUsers = new HashSet<CommonFontUser>();

	public FrontEnd(String[] args){

		frontEnd    = this;

		mainFrame   = new MainFrame();
        mainFrame.editorPanel.firstFileOpen();

        sleep(500);

        loadArgs(args);

        println("(SYSTEM) Ready.");
	}

	void loadArgs(String[] args){
		for(int i=0;i<args.length;i++){
			if(args[i].length()==0){ continue; }
			if(args[i].charAt(0)!='-'){ continue; }

			if (args[i].equals("--stateviewer")) {
				if(i+1<args.length){
					File file = new File(args[i+1]);
					if(file.exists()){
						//mainFrame.jsp.setDividerLocation(0);
//						mainFrame.toolTab.statePanel.loadFile(file);
					}
				}
			}else{
				println("invalid option: " + args[i]);
			}
		}
	}

	static public void reboot(){
		if(!mainFrame.editorPanel.closeFile()){return;}
		mainFrame.exit();
		Env.save();
		mainFrame.dispose();
		System.out.println("HIDE reboot.");

		RebootRunner rebootRunner = new RebootRunner("-Xms16M -Xmx"+Env.get("REBOOT_MAX_MEMORY"));
		rebootRunner.run();
		while(rebootRunner.isRunning()){
			FrontEnd.sleep(200);
		}
		System.exit(0);
	}

	static public void exit(){
		if(!mainFrame.editorPanel.closeFile()){return;}
		mainFrame.editorPanel.setDefaultFile();
		mainFrame.exit();
		//if(Env.is("WATCH_DUMP")) Env.dumpWatch();
		Env.save();
		System.out.println("HIDE end.");
		System.exit(0);
	}

	static public void println(final String str){
		javax.swing.SwingUtilities.invokeLater(new Runnable(){public void run() {
			mainFrame.toolTab.systemPanel.logPanel.println(str);
		}});
	}

	static public void errPrintln(final String str){
		javax.swing.SwingUtilities.invokeLater(new Runnable(){public void run() {
			mainFrame.toolTab.systemPanel.logPanel.errPrintln(str);
		}});
	}

	static public void printException(final Exception e){
		javax.swing.SwingUtilities.invokeLater(new Runnable(){public void run() {
			mainFrame.toolTab.systemPanel.logPanel.printException(e);
		}});
		e.printStackTrace();
	}

	static public void sleep(long millis){
		try {
			Thread.sleep(millis);
		} catch (InterruptedException e) {
			FrontEnd.printException(e);
		}
	}

	static public void updateLookAndFeel(){
		try{
			if(Env.get("LookAndFeel").equals("Metal")){
				UIManager.setLookAndFeel("javax.swing.plaf.metal.MetalLookAndFeel");
			}else if(Env.get("LookAndFeel").equals("Motif")){
				UIManager.setLookAndFeel("com.sun.java.swing.plaf.motif.MotifLookAndFeel");
			}else if(Env.get("LookAndFeel").equals("Windows")){
				UIManager.setLookAndFeel("com.sun.java.swing.plaf.windows.WindowsLookAndFeel");
			}else if(Env.get("LookAndFeel").equals("WindowsClassic")){
				UIManager.setLookAndFeel("com.sun.java.swing.plaf.windows.WindowsClassicLookAndFeel");
			}else if(Env.get("LookAndFeel").equals("SystemDefault")){
				UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
			}
		}catch(Exception e){
			FrontEnd.printException(e);
		}

	}

	static public void addFontUser(CommonFontUser user){
		fontUsers.add(user);
	}

	static public void removeFontUser(CommonFontUser user){
		fontUsers.remove(user);
	}

	static public void loadAllFont(){
		for(CommonFontUser user : fontUsers){
			user.loadFont();
		}
	}

	/**
	 * @param args
	 */
	public static void main(final String[] args) {

		new Env();


		FrontEnd.updateLookAndFeel();
		try{
			FrontEnd.sf = new StartupFrame();
		}catch(Exception e){
			FrontEnd.printException(e);
			Env.set("LookAndFeel","Metal");
			FrontEnd.updateLookAndFeel();
			FrontEnd.sf = new StartupFrame();
		}

//		FrontEnd.sf.startEnvSet();

		javax.swing.SwingUtilities.invokeLater(new Runnable(){public void run(){
			try{
				new FrontEnd(args);
				FrontEnd.sf.setVisible(false);
			}catch(Exception e){
				FrontEnd.printException(e);
				e.printStackTrace();
			}
		}});
	}

}