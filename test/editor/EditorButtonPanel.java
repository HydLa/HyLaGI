


package test.editor;

import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.IOException;

import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;

import test.*;
import test.funcgraph.FuncGraphPanel;
import test.funcgraph.FuncGraphRunner;
import test.graph.PhaseViewer;
import test.runner.*;

public class EditorButtonPanel extends JPanel implements ActionListener {

	private EditorPanel editorPanel;

	private HyroseRunner hyroseRunner;
	private AutomatonMakerRunner automatonMakerRunner;

	public PhaseViewer phaseviewer;

	private JPanel buttonPanel;
	public JButton hydlaButton;

	public JButton phaseviewerButton;
	public JButton automatonVisualizerButton;
	public JButton funcGraphButton;
	public JButton killButton;
	public JButton gnuplotButton;

	private JPanel labelPanel;
	private JLabel rowColumnStatus;
	private JLabel fileStatus;

	private int textRow;
	private int textColumn;

	private File outFile;
	private File mslFile;


	public EditorButtonPanel(EditorPanel editorPanel){
		this.editorPanel = editorPanel;

		setLayout(new BoxLayout(this,BoxLayout.Y_AXIS));

		buttonPanel = new JPanel(new GridLayout(2,4));

		hydlaButton = new JButton("HydLa");
		hydlaButton.addActionListener(this);
		buttonPanel.add(hydlaButton);

		funcGraphButton = new JButton("funcGraph");
		funcGraphButton.addActionListener(this);
		buttonPanel.add(funcGraphButton);

		phaseviewerButton = new JButton("PhaseViwer");
		phaseviewerButton.addActionListener(this);
		buttonPanel.add(phaseviewerButton);

		automatonVisualizerButton = new JButton("AutomatonVisualizer");
		automatonVisualizerButton.addActionListener(this);
		buttonPanel.add(automatonVisualizerButton);

		gnuplotButton = new JButton("gnuplot");
		gnuplotButton.addActionListener(this);
		buttonPanel.add(gnuplotButton);

		killButton = new JButton("Kill");
		killButton.addActionListener(this);
		buttonPanel.add(killButton);

		setAllEnable(true);
		add(buttonPanel);

		labelPanel = new JPanel(new GridLayout(1,2));

		fileStatus = new JLabel();
		updateFileStatus();
		labelPanel.add(fileStatus);

		rowColumnStatus = new JLabel();
		rowColumnStatus.setHorizontalAlignment(JLabel.RIGHT);
		setRowColumn(1,1);
		labelPanel.add(rowColumnStatus);

		add(labelPanel);

	}

	public void updateFileStatus(){
		String fileMark = "";
		if(editorPanel.isChanged()){
			fileMark = "edit";
		}
		fileStatus.setText(" "+fileMark+" "+editorPanel.getFileName());
	}

	public void setRowColumn(int r, int c){
		this.textRow=r;
		this.textColumn=c;
		String rowColumn = textRow + ":" + textColumn;
		rowColumnStatus.setText(rowColumn);
	}

	public void setAllEnable(boolean enable){
		hydlaButton.setEnabled(enable);
		funcGraphButton.setEnabled(enable);
		phaseviewerButton.setEnabled(enable);
		gnuplotButton.setEnabled(enable);
		automatonVisualizerButton.setEnabled(enable);
		killButton.setEnabled(!enable);
	}

	private void setButtonEnable(boolean enable){
		setAllEnable(enable);
	}

	private void ResetOptions(String options){
		options="";
	}

	private void FileSaveIfChanged(){
		if(editorPanel.isChanged()){
			editorPanel.fileSave();
		}
	}

	public void actionPerformed(ActionEvent e) {
		JButton src = (JButton)e.getSource();

		if (src == hydlaButton) {

			FileSaveIfChanged();

			setButtonEnable(false);

			FrontEnd.mainFrame.toolTab.setTab("Simulation");

			FrontEnd.println("(HydLa) Doing...");
			hyroseRunner = new HyroseRunner(Env.get("SLIM_OPTION"));
			hyroseRunner.run();
			(new Thread(new Runnable() { public void run() {
				while(hyroseRunner.isRunning()){
					FrontEnd.sleep(200);
				}
				FrontEnd.println("(Hyrose) Done! ["+(hyroseRunner.getTime()/1000.0)+"s]");
				hyroseRunner = null;
				javax.swing.SwingUtilities.invokeLater(new Runnable(){public void run() {
					setButtonEnable(true);
				}});
			}})).start();
///////////////////////////////////////////////////////////////////////////////
		}else if (src == funcGraphButton) {
			final String funcexeFile = "funcgraph.txt";
			String opt = "";
			FileSaveIfChanged();
			setButtonEnable(false);

			File funcgraphFile = new File(funcexeFile);
			try {
				funcgraphFile.createNewFile();
			} catch (IOException ee) {
				// TODO 自動生成された catch ブロック
				ee.printStackTrace();
			}
			final String funcPath = Env.getSpaceEscape(funcgraphFile.getAbsolutePath());
			System.out.println(funcPath);


			FrontEnd.println("Func Graph Doing...");

			FrontEnd.println("x_axis= "+Env.get("FUNC_X_AXIS")+" "+"y_axis= "+Env.get("FUNC_Y_AXIS"));

			ResetOptions(opt);

			opt += Env.get("SLIM_OPTION");

			if(!opt.contains("--nd")){
				System.out.println("opt0 : "+opt);
				opt += " "+"--nd"+" ";
				System.out.println("opt1 : "+opt);
			}

			if(opt.contains("-f t")||opt.contains("-ft")||opt.contains("-f n")||opt.contains("-fn")){
				opt = opt.replaceAll("-f t", "");
				opt += "-f"+" "+"g"+" ";
			}else{
				opt += "-f"+" "+"g"+" ";
			}

			hyroseRunner = new HyroseRunner(opt);
			hyroseRunner.isFileWriter(true);
			hyroseRunner.setFilename(funcPath);
			hyroseRunner.run();

			(new Thread(new Runnable() { public void run() {
				while(hyroseRunner.isRunning()){
					FrontEnd.sleep(200);
				}
				FrontEnd.println("(FuncGraph) Doing... ");//["+(slimRunner.getTime()/1000.0)+"s]");
				if(hyroseRunner.isSuccess()){

					FrontEnd.mainFrame.toolTab.funcgraphpanel.setFileName(funcPath);
					FrontEnd.mainFrame.toolTab.funcgraphpanel.restart();

					if(!FuncGraphPanel.isOnAnotherPanel()){
						if(!FuncGraphRunner.isErrFlag()){
							FrontEnd.mainFrame.toolTab.setTab("FuncGraph");
						}else{	
							FrontEnd.mainFrame.toolTab.setTab("Simulation");
						}
						FuncGraphRunner.setErrFlag(false);
					}
					FrontEnd.println("(Hyrose for FuncGraph) Done! ["+(hyroseRunner.getTime()/1000.0)+"s]");
				}else{
					FrontEnd.mainFrame.toolTab.setTab("Simulation");
					FrontEnd.errPrintln("FuncGraph failed");
				}

				hyroseRunner.isFileWriter(false);
				hyroseRunner = null;
				javax.swing.SwingUtilities.invokeLater(new Runnable(){public void run() {
					setButtonEnable(true);
				}});
			}})).start();


		}else if (src == phaseviewerButton) {

			final String ppipOutput = "pp_ip_output.txt";
			String opt = "";

			FileSaveIfChanged();

			setButtonEnable(false);

			File ppipFile = new File(ppipOutput);
			try{
			ppipFile.createNewFile();
			}catch(IOException e0){
				System.err.println(e0);
			}
			final String ppipPath = Env.getSpaceEscape(ppipFile.getAbsolutePath());

			FrontEnd.println("(PhaseViwer) Doing...");

			ResetOptions(opt);
			opt += Env.get("SLIM_OPTION");
			if(!opt.contains("--nd")){
				opt += " "+"--nd"+" ";
			}
			if(!opt.contains("-f")){
				opt += "-f"+" "+"g"+" ";
			}

			hyroseRunner = new HyroseRunner(opt);
			hyroseRunner.isFileWriter(true);
			hyroseRunner.setFilename(ppipOutput);
			hyroseRunner.run();

			(new Thread(new Runnable() { public void run() {
				while(hyroseRunner.isRunning()){
					FrontEnd.sleep(200);
				}
				FrontEnd.println("(PhaseViewer) Done! ");//["+(slimRunner.getTime()/1000.0)+"s]");
				if(hyroseRunner.isSuccess()){
					if(!PhaseViewer.isOnAnotherPanel()){
						FrontEnd.mainFrame.toolTab.setTab("PhaseViewer");
					}
					FrontEnd.mainFrame.toolTab.phaseviewer.setFileName(ppipPath);
					FrontEnd.mainFrame.toolTab.phaseviewer.restart();
					FrontEnd.println("PhaseViewerSuccess");
				}else{
					FrontEnd.println("PhaseViewerFailed");
				}
				FrontEnd.println("(Hyrose for PhaseViewer) Done! ["+(hyroseRunner.getTime()/1000.0)+"s]");
				hyroseRunner.isFileWriter(false);
				hyroseRunner = null;
				javax.swing.SwingUtilities.invokeLater(new Runnable(){public void run() {
					setButtonEnable(true);
				}});
			}})).start();

		}else if (src == automatonVisualizerButton) {
///////////////オートマトンを作成します//////////////
//４工程を一つのスレッドにした
//1.msl.txtの作成
//2.debug.txtの作成
//3.automaton_makerの動作
//4.automaton_visualizerの動作
///////////////msl.txtを作成する/////////////////////

			(new Thread(new Runnable() { public void run() {
				final String mslTextOut = "msl.txt";
				mslFile = new File(mslTextOut);
				try{
					mslFile.createNewFile();
				}catch(IOException e2){
				}
				String opt="";

				ResetOptions(opt);

				opt += Env.get("SLIM_OPTION");
				if(!opt.contains("--dump-module-set-list")){
					opt = " "+"--dump-module-set-list"+" ";
				}

				System.out.println("msl.txtのopt:"+opt);

				setButtonEnable(false);

				hyroseRunner = new HyroseRunner(opt);
				hyroseRunner.isFileWriter(true);
				hyroseRunner.setFilename(mslTextOut);
				hyroseRunner.run();
				while(hyroseRunner.isRunning()){
					FrontEnd.sleep(200);
				}
				//FrontEnd.println("module set start");
				FrontEnd.println("(Hyrose for making mslTextOut) Done! ["+(hyroseRunner.getTime()/1000.0)+"s]");
				//hyroseをストップする
				hyroseRunner.isFileWriter(false);
				hyroseRunner = null;

/*
 * out.txtを作成する
 */

				final String debugTextOut = "out.txt";

				FileSaveIfChanged();

				outFile = new File(debugTextOut);
				try{
					outFile.createNewFile();
				}catch(IOException e1){
				}

				setButtonEnable(false);
				FrontEnd.mainFrame.toolTab.setTab("Automaton");
				FrontEnd.println("Constraint Automaton start");

				//out.txtは通常実行とは独立に動作するためオプションは固定でOK
				ResetOptions(opt);

				//ログトレース
				opt += "-d"+" ";
				//シミュレーション時間
				opt += "-t"+" "+"1"+" ";
				//シミュレーションモード
				opt += "-m"+" "+"s";
				System.out.println("out.txtのオプション："+opt);
				/*
				 * hydlaの実行
				 */
				hyroseRunner = new HyroseRunner(opt);
				hyroseRunner.isFileWriter(true);
				hyroseRunner.setFilename(debugTextOut);
				hyroseRunner.run();

				while(hyroseRunner.isRunning()){
					FrontEnd.sleep(200);
				}
				FrontEnd.println("(Hyrose for making debugTextOut) Done! ["+(hyroseRunner.getTime()/1000.0)+"s]");
				hyroseRunner.isFileWriter(false);
				hyroseRunner = null;

/*
 * automaton_makerを作動します
 */

				automatonMakerRunner = new AutomatonMakerRunner(outFile,mslFile);
				automatonMakerRunner.run();
				while(automatonMakerRunner.isRunning()){
					FrontEnd.sleep(200);
				}
				automatonMakerRunner = null;

/*
 * ここからオートマトンを作成する
 */
				//if(automatonMakerRunner.isSuccess()){
				if(true){
					FrontEnd.mainFrame.toolTab.automatonpanel.restart(AutomatonMakerRunner.madeAutomatonFile);//"automaton_test.txt");
					FrontEnd.println("Automaton Success");
					FrontEnd.mainFrame.toolTab.setTab("AutomatonVisualizer");
				}else{
					FrontEnd.println("Automaton Failed");
				}


				javax.swing.SwingUtilities.invokeLater(new Runnable(){public void run() {
					setButtonEnable(true);
				}});
/*
 * thread化終わり
 */

			}})).start();
/*
 * automaton終わり
 */

		}else if (src == gnuplotButton) {
			final String gnuplotText = "gnuplottext.txt";

			FileSaveIfChanged();

			setButtonEnable(false);

			outFile = new File(gnuplotText);
			try{
				outFile.createNewFile();
			}catch(IOException e1){
			}

			FrontEnd.mainFrame.toolTab.setTab("GnuplotPanel");

			FrontEnd.println("(PlotGraph) Doing...");
			hyroseRunner = new HyroseRunner(Env.get("SLIM_OPTION"));
			hyroseRunner.isFileWriter(true);
			hyroseRunner.setFilename(gnuplotText);
			hyroseRunner.setBuffering(true);
			hyroseRunner.run();

			while(hyroseRunner.isRunning()){
				FrontEnd.sleep(200);
			}
			if(hyroseRunner.isSuccess()){
				FrontEnd.mainFrame.toolTab.gnuplotPanel.restart(gnuplotText);
				FrontEnd.println("Gnuplot Success");
			}else{
				FrontEnd.println("Gnuplot Failed");
			}
			hyroseRunner = null;
			javax.swing.SwingUtilities.invokeLater(new Runnable(){public void run() {
				setButtonEnable(true);
			}});

		}else if (src == killButton) {
			if(hyroseRunner!=null) hyroseRunner.kill();
			if(automatonMakerRunner!=null) automatonMakerRunner.kill();
			FrontEnd.mainFrame.toolTab.funcgraphpanel.kill();
			FrontEnd.errPrintln("Kill");

		}
	}

}
