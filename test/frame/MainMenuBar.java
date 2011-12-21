/*
 *   Copyright (c) 2008, Ueda Laboratory LMNtal Group <lmntal@ueda.info.waseda.ac.jp>
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are
 *   met:
 *
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *    3. Neither the name of the Ueda Laboratory LMNtal Group nor the
 *       names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior
 *       written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

package test.frame;

import java.awt.Desktop;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;

import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.KeyStroke;

import test.*;

public class MainMenuBar extends JMenuBar implements ActionListener {

	private JMenu file;
    private JMenuItem iNew;
    private JMenuItem iOpen;
    private JMenuItem iSave;
    private JMenuItem iSaveAs;
//    private JMenuItem iSVOpen;
    private JMenuItem iExit;

    private JMenu edit;
    private JMenuItem iCopy;
    private JMenuItem iCut;
    private JMenuItem iPaste;
    public  JMenuItem iUndo;
    public  JMenuItem iRedo;

    private JMenu run;
    private JMenuItem iHydLa;
    private JMenuItem iLogtrace;
    private JMenuItem iHistory;
    private JMenuItem iInteractive;
//    private JMenuItem iLMNtalg;
//    private JMenuItem iUNYO;
//    private JMenuItem iSLIM;
    private JMenuItem iSViewer;
//    private JMenuItem iStateProfiler;
    private JMenuItem iKill;
    private JMenuItem iReboot;

    private JMenu setting;
    private JMenuItem iGnuplotPath;
    private JMenuItem iCygwinPath;
    private JMenuItem iSlimPath;
    private JMenuItem iGeneral;

    private JMenu verif;
    private JMenuItem iVerify;
    private JMenuItem iAssert;

    private JMenu help;
    private JMenuItem iVersion;
    private JMenuItem iRuntime;
    private JMenuItem iBrowse;

    public MainMenuBar(){

//    	file = new JMenu(Lang.m[0]);
    	file = new JMenu("File");
        add(file);
        file.setMnemonic(KeyEvent.VK_F);

//      iNew     = new JMenuItem(Lang.m[1]);
        iNew     = new JMenuItem("New");
        file.add(iNew);
        iNew.addActionListener(this);
        iNew.setMnemonic(KeyEvent.VK_N);
        iNew.setAccelerator(KeyStroke.getKeyStroke("control N"));

//      iOpen    = new JMenuItem(Lang.m[2]);
        iOpen    = new JMenuItem("Open");
        file.add(iOpen);
        iOpen.addActionListener(this);
        iOpen.setMnemonic(KeyEvent.VK_O);
        iOpen.setAccelerator(KeyStroke.getKeyStroke("control O"));

//        iSave    = new JMenuItem(Lang.m[3]);
        iSave    = new JMenuItem("Save");
        file.add(iSave);
        iSave.addActionListener(this);
        iSave.setMnemonic(KeyEvent.VK_S);
        iSave.setAccelerator(KeyStroke.getKeyStroke("control S"));

//        iSaveAs  = new JMenuItem(Lang.m[4]);
        iSaveAs  = new JMenuItem("SaveAs");
        file.add(iSaveAs);
        iSaveAs.addActionListener(this);
        iSaveAs.setMnemonic(KeyEvent.VK_A);

//        file.addSeparator();

//        iSVOpen  = new JMenuItem(Lang.m[30]);
//        iSVOpen  = new JMenuItem("SVOpen");
//        file.add(iSVOpen);
//        iSVOpen.addActionListener(this);

        file.addSeparator();

//        iExit    = new JMenuItem(Lang.m[5]);
        iExit    = new JMenuItem("Exit");
        file.add(iExit);
        iExit.addActionListener(this);
        iExit.setMnemonic(KeyEvent.VK_X);


//        edit = new JMenu(Lang.m[6]);
        edit = new JMenu("edit");
        add(edit);
        edit.setMnemonic(KeyEvent.VK_E);

//        iUndo = new JMenuItem(Lang.m[16]);
        iUndo = new JMenuItem("Undo");
        edit.add(iUndo);
        iUndo.setEnabled(false);
        iUndo.addActionListener(this);
        iUndo.setMnemonic(KeyEvent.VK_U);
        iUndo.setAccelerator(KeyStroke.getKeyStroke("control Z"));

//        iRedo = new JMenuItem(Lang.m[17]);
        iRedo = new JMenuItem("Redo");
        edit.add(iRedo);
        iRedo.setEnabled(false);
        iRedo.addActionListener(this);
        iRedo.setMnemonic(KeyEvent.VK_R);
        iRedo.setAccelerator(KeyStroke.getKeyStroke("control Y"));

        edit.addSeparator();

//        iCopy = new JMenuItem(Lang.m[7]);
        iCopy = new JMenuItem("Copy");
        edit.add(iCopy);
        iCopy.addActionListener(this);
        iCopy.setMnemonic(KeyEvent.VK_C);
        iCopy.setAccelerator(KeyStroke.getKeyStroke("control C"));

//        iCut = new JMenuItem(Lang.m[8]);
        iCut = new JMenuItem("Cut");
        edit.add(iCut);
        iCut.addActionListener(this);
        iCut.setMnemonic(KeyEvent.VK_T);
        iCut.setAccelerator(KeyStroke.getKeyStroke("control X"));

//        iPaste = new JMenuItem(Lang.m[9]);
        iPaste = new JMenuItem("Paste");
        edit.add(iPaste);
        iPaste.addActionListener(this);
        iPaste.setMnemonic(KeyEvent.VK_P);
        iPaste.setAccelerator(KeyStroke.getKeyStroke("control V"));


//        run = new JMenu(Lang.m[10]);
        run = new JMenu("run");
        add(run);
        run.setMnemonic(KeyEvent.VK_R);

//        iLMNtal = new JMenuItem(Lang.m[11]);
        iHydLa = new JMenuItem("HydLa");
        run.add(iHydLa);
        iHydLa.addActionListener(this);
        iHydLa.setMnemonic(KeyEvent.VK_H);
        iHydLa.setAccelerator(KeyStroke.getKeyStroke("F1"));

        iLogtrace = new JMenuItem("Logtrace");
        run.add(iLogtrace);
        iLogtrace.addActionListener(this);
        iLogtrace.setMnemonic(KeyEvent.VK_L);
        iLogtrace.setAccelerator(KeyStroke.getKeyStroke("F2"));

        iHistory = new JMenuItem("History");
        run.add(iHistory);
        iHistory.addActionListener(this);
        iHistory.setMnemonic(KeyEvent.VK_H);
        iHistory.setAccelerator(KeyStroke.getKeyStroke("F3"));

        iInteractive = new JMenuItem("Interactive");
        run.add(iInteractive);
        iInteractive.addActionListener(this);
        iInteractive.setMnemonic(KeyEvent.VK_I);
        iInteractive.setAccelerator(KeyStroke.getKeyStroke("F4"));


        iKill = new JMenuItem("Kill");
        run.add(iKill);
        iKill.addActionListener(this);
        iKill.setMnemonic(KeyEvent.VK_K);
        iKill.setAccelerator(KeyStroke.getKeyStroke("ESCAPE"));

        run.addSeparator();

//        iReboot = new JMenuItem(Lang.m[27]);
        iReboot = new JMenuItem("Reboot");
        run.add(iReboot);
        iReboot.addActionListener(this);


//        setting = new JMenu(Lang.m[22]);
        setting = new JMenu("setting");
        add(setting);
        setting.setMnemonic(KeyEvent.VK_S);

//        iCygwinPath = new JMenuItem(Lang.m[23]);
//        iCygwinPath = new JMenuItem("CygwinPath");
//        setting.add(iCygwinPath);
//        iCygwinPath.addActionListener(this);

        iGnuplotPath = new JMenuItem("GnuplotPath");
        setting.add(iGnuplotPath);
        iGnuplotPath.addActionListener(this);

//        iSlimPath = new JMenuItem(Lang.m[24]);
        iSlimPath = new JMenuItem("HydLaPath");
        setting.add(iSlimPath);
        iSlimPath.addActionListener(this);

//        iGeneral = new JMenuItem(Lang.m[29]);
        iGeneral = new JMenuItem("General");
        setting.add(iGeneral);
        iGeneral.addActionListener(this);

        verif = new JMenu("verification");
        add(verif);
        verif.setMnemonic(KeyEvent.VK_V);

        iVerify = new JMenuItem("Verify");
        verif.add(iVerify);
        iVerify.addActionListener(this);

        iAssert = new JMenuItem("Assertion");
        verif.add(iAssert);
        iAssert.addActionListener(this);



//        help = new JMenu(Lang.m[18]);
        help = new JMenu("help");
        add(help);
        help.setMnemonic(KeyEvent.VK_H);

//        iVersion = new JMenuItem(Lang.m[19]);
        iVersion = new JMenuItem("Version");
        help.add(iVersion);
        iVersion.addActionListener(this);

//        iRuntime = new JMenuItem(Lang.m[26]);
        iRuntime = new JMenuItem("Runtime");
        help.add(iRuntime);
        iRuntime.addActionListener(this);

//        iBrowse = new JMenuItem(Lang.m[28]);
        iBrowse = new JMenuItem("Browse");
        help.add(iBrowse);
        iBrowse.addActionListener(this);

    }

    public void updateUndoRedo(boolean undo,boolean redo){
    	iUndo.setEnabled(undo);
		iRedo.setEnabled(redo);
    }

	public void actionPerformed(ActionEvent e) {
		JMenuItem src = (JMenuItem)e.getSource();

		if (src == iNew) {
			FrontEnd.mainFrame.editorPanel.newFileOpen();
        }else if(src == iOpen) {
        	FrontEnd.mainFrame.editorPanel.fileOpen();
		}else if(src == iSave) {
        	FrontEnd.mainFrame.editorPanel.fileSave();
        }else if(src == iSaveAs) {
        	FrontEnd.mainFrame.editorPanel.fileSaveAs();
//        }else if(src == iSVOpen){
 //       	FrontEnd.mainFrame.toolTab.statePanel.loadFile();
        }else if(src == iExit) {
        	FrontEnd.frontEnd.exit();
        }else if(src == iCopy) {
        	FrontEnd.mainFrame.editorPanel.editor.copy();
        }else if(src == iCut) {
        	FrontEnd.mainFrame.editorPanel.editor.cut();
        }else if(src == iPaste) {
        	FrontEnd.mainFrame.editorPanel.editor.paste();
        }else if(src == iUndo) {
        	FrontEnd.mainFrame.editorPanel.editorUndo();
        }else if(src == iRedo) {
        	FrontEnd.mainFrame.editorPanel.editorRedo();
        }else if(src == iHydLa) {
        	FrontEnd.mainFrame.editorPanel.buttonPanel.hydlaButton.doClick();
//        }else if(src == iLMNtalg) {
//        	FrontEnd.mainFrame.editorPanel.buttonPanel.lmntalgButton.doClick();
//        }else if(src == iUNYO) {
//        	FrontEnd.mainFrame.editorPanel.buttonPanel.unyoButton.doClick();
//        }else if(src == iSLIM) {
//        	FrontEnd.mainFrame.editorPanel.buttonPanel.slimButton.doClick();
        }else if(src == iSViewer) {
        	FrontEnd.mainFrame.editorPanel.buttonPanel.phaseviewerButton.doClick();
        }else if(src == iKill) {
        	FrontEnd.mainFrame.editorPanel.buttonPanel.killButton.doClick();
        }else if(src == iReboot) {
        	new RebootFrame();
        }else if(src == iCygwinPath) {
        	new CygwinPathSettingFrame();
        }else if(src == iGnuplotPath) {
        	new GnuplotPathSettingFrame();
        }else if(src == iSlimPath) {
        	new HyrosePathSettingFrame();
        }else if(src == iGeneral) {
        	new GeneralSettingFrame();
        }else if(src == iVersion) {
        	new VersionFrame();

        	JOptionPane.showMessageDialog(
        			FrontEnd.mainFrame,
        			"HydLa\n"+
        			"Version : "+Env.APP_VERSION+"\n"+
        			"Date : "+Env.APP_DATE+"\n"+
        			"\n"+
        			/*
        			Env.LMNTAL_VERSION+"\n"+
        			Env.SLIM_VERSION+"\n"+
        			Env.UNYO_VERSION+"\n",
        			*/
        			Env.HYDLA_VERSION+"\n",
        			"HydLa",
        		JOptionPane.PLAIN_MESSAGE
        	);

        }else if(src == iRuntime) {
        	JOptionPane.showMessageDialog(
        			FrontEnd.mainFrame,
        			"Max Memory : "+(int)(Runtime.getRuntime().maxMemory()/(1024*1024))+" MB\n"+
        			"Use Memory : "+(int)(Runtime.getRuntime().totalMemory()/(1024*1024))+" MB\n"+
        			"Available Processors : "+Runtime.getRuntime().availableProcessors()+" \n"+
        			"Java Version : "+System.getProperty("java.version")+" \n"+
        			"Java Runtime Version : "+System.getProperty("java.runtime.version")+" \n"+
        			"Java VM Version : "+System.getProperty("java.vm.version")+" \n",
        			"Java Runtime Info",
        		JOptionPane.PLAIN_MESSAGE
        	);
        }else if(src == iBrowse){
        	if(!Desktop.isDesktopSupported()) return;
        	try{
        	  Desktop.getDesktop().browse(new URI(Env.APP_HREF));
        	}catch(IOException ioe) {
        	  ioe.printStackTrace();
        	}catch(URISyntaxException use) {
        	  use.printStackTrace();
        	}
        }

	}

}
