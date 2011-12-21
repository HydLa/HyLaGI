package test.runner;

import java.io.BufferedInputStream;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.StringTokenizer;

import test.Env;
import test.FrontEnd;
//import test.runner.AutomatonMakerRunner.ThreadRunner;
import test.util.OuterRunner;

public class AutomatonMakerRunner implements OuterRunner{

	private ThreadRunner runner;
	private RunnerOutputGetter output;
	private StringBuffer buffer;
	private File debugFile;
	private File moduleSetListFile;
	private boolean success;
	private double time;

	public static String madeAutomatonFile = "automaton.txt";


	public AutomatonMakerRunner(File debugFile, File moduleSetListFile){
		this.runner = new ThreadRunner();
		this.output = null;
		this.buffer = null;

		this.debugFile = debugFile;
		this.moduleSetListFile = moduleSetListFile;
		this.success = false;
		this.time=0;
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

	public String getBufferString(){
		return buffer.toString();
	}

	public boolean isRunning() {
		if(runner==null) return false;
		return true;
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


	private class ThreadRunner extends Thread {
		private Process p1;
		private String automaton_maker_path;

		ThreadRunner(){
			automaton_maker_path = Env.get("AUTOMATON_EXE_PATH");
		}

		public void run() {
			try {

				long startTimeMillis = System.currentTimeMillis();
				FrontEnd.println("automaton_maker_path : "+automaton_maker_path);

				String automaton_exe_path = Env.get("AUTOMATON_EXE_PATH");
				FrontEnd.println("automaton_maker.exeのパス:"+automaton_exe_path);

				String testcmd = automaton_exe_path+" "+debugFile+" "+moduleSetListFile;
				ProcessBuilder pb = new ProcessBuilder(strList(testcmd));

				try{
					File automatonFile = new File(madeAutomatonFile);
					BufferedWriter bw = new BufferedWriter(new FileWriter(automatonFile));

					try {
					    p1 = pb.start();
						InputStream stdIn = p1.getInputStream();
						BufferedInputStream in1 = new BufferedInputStream(p1.getInputStream());
						InputStream errIn = p1.getErrorStream();
						BufferedInputStream err1 = new BufferedInputStream(p1.getErrorStream());
						int c;
						while ((c = in1.read()) != -1) {
						    bw.write(c);
						}
						stdIn.close();
						bw.close();
						while ((c = err1.read()) != -1) {
						    System.out.print((char)c);
					        }
						errIn.close();
						int ret = p1.waitFor();
							if(!(ret==0)){
								FrontEnd.println("process exited with value : " + ret);
							}
					    } catch (IOException e) {
					        // start()で例外が発生 または、Streamで例外発生
						e.printStackTrace();
					    } catch (InterruptedException e) {
					        // waitFor()で例外が発生
						e.printStackTrace();
					    } finally {
					    }
						p1.waitFor();

					}catch(Exception e){
					}finally{
						exit();
					}

					time = System.currentTimeMillis() - startTimeMillis;
					success = true;
				}finally{
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
			if(p1!=null) p1.destroy();
		}
	}


	public double getTime() {
		// TODO 自動生成されたメソッド・スタブ
		return time;
	}
}
