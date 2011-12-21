package test.runner.extensiverunner;



import java.awt.Color;
import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.Map;
import java.util.StringTokenizer;
import java.util.Timer;
import java.util.TimerTask;

import javax.swing.text.SimpleAttributeSet;
import javax.swing.text.StyleConstants;

import test.*;
import test.runner.RunnerOutputGetter;
import test.system.OutputPanel;
import test.util.OuterRunner;

public class ExtensiveRunner implements OuterRunner {

	private ThreadRunner runner;
	private RunnerOutputGetter output;
	private StringBuffer buffer;

	private String option;
	private File targetFile;
	private boolean success;

	private boolean quiet;

	private long time;
	private boolean isFileWriter;
	public String newFileName = "Nulltext.txt";
	private boolean isAutomatonTextOutput;
	private boolean isER;

	//無理やり作った。isAutomatonTextOutputのために。間違ってるよなあ
	public ExtensiveRunner(){
		this.isER = false;
		System.out.println("test0");
	}

	public ExtensiveRunner(String option){
		this(option,FrontEnd.mainFrame.editorPanel.getFile());
	}

	public ExtensiveRunner(String option,File targetFile){
		this.runner = new ThreadRunner();
		this.output = null;
		this.buffer = null;

		this.option = option;
		this.targetFile = targetFile;
		this.success = false;
		this.quiet = false;
		this.time = 0;
		this.isER = false;
	}



	public void run() {
		runner.start();
	}

	public void setOutputGetter(RunnerOutputGetter output){
		this.output = output;
	}

	public void setBuffering(boolean b){
		if(b){
			buffer = new StringBuffer();
		}else{
			buffer = null;
		}
	}

	public void setQuiet(boolean quiet){
		this.quiet = quiet;
	}

	public String getBufferString(){
		return buffer.toString();
	}

	public boolean isRunning() {
		if(runner==null) return false;
		return true;
	}

	public long getTime(){
		return time;
	}

	public void kill() {
		if (runner!=null) {
			runner.kill();
			runner.interrupt();
			runner=null;
		}
	}

	public void exit(){
		runner=null;
	}

	public boolean isSuccess(){
		return success;
	}

	public void isFileWriter(boolean b) {
		this.isFileWriter=b;
	}

	public void isAutomatonTextOutput(boolean b){
		this.isAutomatonTextOutput = b;
	}

	public void setFilename(String filename) {
		// TODO 自動生成されたメソッド・スタブ
		this.newFileName = filename;
	}

	private class ThreadRunner extends Thread {
		private Process proc;
		private String hyrose_path;

		ThreadRunner(){
			hyrose_path = Env.get("HYROSE_EXE_PATH");
		}

		public void run() {
			try {

				long startTimeMillis = System.currentTimeMillis();
				String file = Env.getSpaceEscape(targetFile.getAbsolutePath());

				FrontEnd.println("hydla_path : "+hyrose_path);
				FrontEnd.println("option : "+option);
				String testcmd = hyrose_path+" "+file+" "+option;
				ProcessBuilder pb = new ProcessBuilder(strList(testcmd));

				///////////////////////////////////////////

				File automatonfile = new File(newFileName);
				automatonfile.createNewFile();
				BufferedWriter bw = new BufferedWriter(new FileWriter(automatonfile));
//				}
				try {
			    proc = pb.start();
				InputStream stdIn = proc.getInputStream();
				BufferedInputStream in1 = new BufferedInputStream(proc.getInputStream());
				InputStream errIn = proc.getErrorStream();
				BufferedInputStream err1 = new BufferedInputStream(proc.getErrorStream());
				int c;
				output = FrontEnd.mainFrame.toolTab.systemPanel.outputPanel;
				while ((c = in1.read()) != -1) {
					if(isFileWriter){
						if(!newFileName.isEmpty()){
							bw.write((char)c);
						}
						if(isAutomatonTextOutput){
						output.outputChar((char)c);
						}
					}else{
						output.outputChar((char)c);
					}
				}
				in1.close();
				bw.close();
//				FrontEnd.println("isAutomaton:" + isAutomaton);
				stdIn.close();
				//hydla処理系側でのエラー
				String errStr="";
				while ((c = err1.read()) != -1) {
					String str = String.valueOf((char)c);
					errStr += str;
//				    System.out.print((char)c);
			        }
				if(!(errStr.isEmpty())) FrontEnd.errPrintln(errStr);

				errIn.close();
				int ret = proc.waitFor();
				if(!(ret==0)) FrontEnd.errPrintln("process exited with value : " + ret);
			    } catch (IOException e) {
			        // start()で例外が発生 または、Streamで例外発生

			    FrontEnd.errPrintln(String.valueOf(e));
				e.printStackTrace();
			    } catch (InterruptedException e) {
			        // waitFor()で例外が発生
			    FrontEnd.errPrintln(String.valueOf(e));
				e.printStackTrace();
			    } finally {
//			    	System.err.println("3つめのなぞ");
//			        stdIn.close();
//			        errIn.close();
			    }


				pb.directory(new File("."));


				try {
				}catch(Exception e){ if(!quiet) FrontEnd.printException(e); } //

				proc.waitFor();

				if(buffer==null){
					if(output==null){
						System.out.println("test9");
						output = FrontEnd.mainFrame.toolTab.systemPanel.outputPanel;
					}
					System.out.println("test10");

					output.outputEnd();
				}else{
					if(output==null){
						System.out.println("test12");
						output = FrontEnd.mainFrame.toolTab.systemPanel.outputPanel;
					}

				}


				time = System.currentTimeMillis() - startTimeMillis;
				success = true;


			}catch(Exception e){
				if(!quiet) FrontEnd.printException(e);
			}finally{
				exit();
			}
		}


		ArrayList<String> strList(String str){
			ArrayList<String> cmdList = new ArrayList<String>();
			StringTokenizer st = new StringTokenizer(str);
			while(st.hasMoreTokens()){
				String s = st.nextToken();
				if(s.length()>=2&&s.charAt(0)=='"'&&s.charAt(s.length()-1)=='"'){
					s = s.substring(1,s.length()-1);
				}
				cmdList.add(s);
			}
			return cmdList;
		}

		private void kill() {
			if(proc!=null) proc.destroy();
		}

	}


	public boolean isERchecker() {
		return isER;
	}

	public void setERchecker(boolean isER) {
		this.isER=isER;
	}


}
