package test.runner;



import java.awt.Color;
import java.io.BufferedInputStream;
import java.io.BufferedReader;
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

import test.*;
import test.system.OutputPanel;
import test.util.OuterRunner;

public class HydLaRunner implements OuterRunner {

	private ThreadRunner runner;
	private RunnerOutputGetter output;
	private StringBuffer buffer;

	private String option;
	private File targetFile;
	private File symbolFile;
	private File ncFile;
	private boolean success;

	private boolean quiet;

	private long time;

	public HydLaRunner(String option){
		this(option,FrontEnd.mainFrame.editorPanel.getFile());
	}

	public HydLaRunner(String option,File targetFile){
		this.runner = new ThreadRunner();
		this.output = null;
		this.buffer = null;

		this.option = option;
		this.targetFile = targetFile;
		this.symbolFile = null;
		this.ncFile = null;
		this.success = false;
		this.quiet = false;
		this.time = 0;
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

	public void setSymbolFile(File symbolFile){
		this.symbolFile = symbolFile;
	}

	public void setNcFile(File ncFile){
		this.ncFile = ncFile;
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


	private class ThreadRunner extends Thread {
		private Process p1;
		private Process p2;
		private String slim_path;

		ThreadRunner(){
			System.out.println("test1");
			slim_path = Env.get("SLIM_EXE_PATH");
//			if(slim_path==null||slim_path.equals("")){ slim_path = Env.LMNTAL_LIBRARY_DIR+File.separator+"bin"+File.separator+Env.getSlimBinaryName(); }
		}

		public void run() {
			try {
				System.out.println("test2");

				//
				long startTimeMillis = System.currentTimeMillis();

				// LMNtal
//				String cmd1 = Env.getLmntalCmd()+" "+Env.get("SLIM_LMNTAL_COMPILE_OPTION")+" "+Env.getSpaceEscape(targetFile.getAbsolutePath());
				String test = "src"+File.separator+"img"+File.separator+"hydla.exe"; //"C:"+File.separator+"pleiades"+File.separator+"workspace"+File.separator+
				String file =/* " C:"+File.separator+"pleiades"+File.separator+"workspace"+File.separator+"test"+File.separator+*/"src"+File.separator+"img"+File.separator+"bouncing_particle.hydla"; //C:\\pleiades\\workspace\\
				//				if(!quiet) FrontEnd.println("(SLIM) "+cmd1);
//				if(!quiet) FrontEnd.println("(HydLa) "+test);

				//ProcessBuilder pb = new ProcessBuilder(strList(cmd1));
//				ProcessBuilder pb = new ProcessBuilder("image/slim.exe");
//				String test = "src\\img\\slim.exe";
//				FrontEnd.println(option);
				FrontEnd.println("hydla_path : "+slim_path);
				FrontEnd.println("option : "+option);
//				ProcessBuilder pb = new ProcessBuilder(test+" "+ "C:"+File.separator+"pleiades"+File.separator+"workspace"+File.separator+"test"+File.separator+"bouncing_particle.hydla",option);
//				ProcessBuilder pb = new ProcessBuilder("");
				String option1 = "-m";
				String timecaption = "-t";
				ProcessBuilder pb = new ProcessBuilder(test,file,"-m","s",timecaption,"2",option);
//				pb.directory(new File("."));
//				Env.setProcessEnvironment(pb.environment());
				//pb.redirectErrorStream(redirectErrorStream);
//				p1 = pb.start();
				/*
				InputStream testes = p1.getInputStream();
				int c;
				System.out.println("testes�̒l" + testes.read());
				while ((c = testes.read()) != -1) {
				    System.out.print(c);
				}
				testes.close();
				*/
				///////////////////////////////////////////
				try {
			    p1 = pb.start();
				InputStream stdIn = p1.getInputStream();
				BufferedInputStream in1 = new BufferedInputStream(p1.getInputStream());
				InputStream errIn = p1.getErrorStream();
				BufferedInputStream err1 = new BufferedInputStream(p1.getErrorStream());
				int c;
				output = FrontEnd.mainFrame.toolTab.systemPanel.outputPanel;
				while ((c = in1.read()) != -1) {
				    output.outputChar((char)c);
				}
				stdIn.close();
				while ((c = err1.read()) != -1) {
				    System.out.print((char)c);

			        }
				errIn.close();
				int ret = p1.waitFor();
				System.out.println("process exited with value : " + ret);
			    } catch (IOException e) {
			        // start()�ŗ�O������ �܂��́AStream�ŗ�O����
				e.printStackTrace();
			    } catch (InterruptedException e) {
			        // waitFor()�ŗ�O������
				e.printStackTrace();
			    } finally {
//			        stdIn.close();
//			        errIn.close();
			    }
			    ////////////////////////////////////////////////







//				BufferedInputStream in1 = new BufferedInputStream(p1.getInputStream());
//			    p1 = pb.start();
	//		    InputStream in1 = p1.getInputStream();
//				ErrorStreamPrinter err1 = new ErrorStreamPrinter(p1.getErrorStream());
//				err1.start();


				//String cmd2 = slim_path+" -Ilmntal"+File.separator+Env.getDirNameOfSlim()+File.separator+"lib"+File.separator+" "+option;
				//if(Env.is("SLIM_USE_LIBRARY")){
				//	cmd2 += " -I"+Env.getSlimInstallLibraryPath();
				//}
//				String cmd2 = slim_path+" "+option;
//				String view_option = option;
/*
				if(symbolFile!=null){
					System.out.println("test3");
					cmd2 += " --psym "+Env.getSpaceEscape(Env.getLinuxStylePath(symbolFile.getAbsolutePath()))+" ";
					view_option += " --psym "+symbolFile.getName();
				}
				if(ncFile!=null){
					System.out.println("test4");
					cmd2 += " --nc "+Env.getSpaceEscape(Env.getLinuxStylePath(ncFile.getAbsolutePath()))+" ";
					view_option += " --nc "+ncFile.getName();
				}
*/
//				cmd2 += " -";
				System.out.println("test5");
//				if(!quiet) FrontEnd.println("(SLIM) "+cmd2);

				//pb = new ProcessBuilder(strList(cmd2));;
//				pb = new ProcessBuilder(test);
				pb.directory(new File("."));
//				Env.setProcessEnvironment(pb.environment());
//				p2 = pb.start();
//				OutputStreamWriter out2 = new OutputStreamWriter(p2.getOutputStream());
//				OutputStreamWriter out2 = new OutputStreamWriter(p1.getOutputStream());
//				BufferedReader in2 = new BufferedReader(new InputStreamReader(p2.getInputStream()));
//				BufferedReader in2 = new BufferedReader(new InputStreamReader(p1.getInputStream()));
//				ErrorStreamPrinter err2 = new ErrorStreamPrinter(p2.getErrorStream());
				System.out.println("test6");

				//
				int b;
/*				while ((b = in1.read()) != -1) {
					out2.write(b);
//					System.out.println("test7,b="+(char)b);
				}
*/
				try {
//					out2.flush();
					System.out.println("test8");
				}catch(Exception e){ if(!quiet) FrontEnd.printException(e); } //

//				out2.close();
//				in1.close();
//				err1.join();
				p1.waitFor();

				if(buffer==null){
					if(output==null){
						System.out.println("test9");
						output = FrontEnd.mainFrame.toolTab.systemPanel.outputPanel;
					}
					System.out.println("test10");
//					output.outputStart("slim", view_option, targetFile);
//					err2.start();
					String str;
//					while ((str=in2.readLine())!=null) {
//						output.outputLine(str);
//						System.out.println("test11");
//					}
//					err2.join();
					output.outputEnd();
				}else{
					if(output==null){
						System.out.println("test12");
						output = FrontEnd.mainFrame.toolTab.systemPanel.outputPanel;
					}
//					err2.start();
					String str;
//					while ((str=in2.readLine())!=null) {
//						buffer.append(str+"\n");
//						System.out.println("test13");
//					}
//					err2.join();
				}

//				in2.close();
//				p2.waitFor();

				time = System.currentTimeMillis() - startTimeMillis;
				success = true;

				//System.out.println("SLIM_END  ="+System.currentTimeMillis());

				/*
				InputStream in2 = p2.getInputStream();
				output = FrontEnd.mainFrame.toolTab.systemPanel.outputPanel;
				output.outputStart("slim", option, targetFile);
				int str = 0;
				//while ((str=in2.read())!=-1) {
				//	output.outputLine(String.valueOf(str));
				//}
				while(true){
					output.outputLine(String.valueOf(in2.available()));
					if(str++>100){ break; }
					Thread.sleep(250);
				}
				output.outputEnd();
				*/


				/*
				String cmd1 = "java -DLMNTAL_HOME=lmntal -classpath lmntal"+File.separator+"bin"+File.separator+"lmntal.jar runtime.FrontEnd -O2 --interpret --slimcode \""+targetFile.getAbsolutePath()+"\"";
				FrontEnd.println("(SLIM) "+cmd1);

				ProcessBuilder pb = new ProcessBuilder(strList(cmd1));
				pb.redirectErrorStream(true);
				p1 = pb.start();
				BufferedInputStream in1 = new BufferedInputStream(p1.getInputStream());

				FileOutputStream fos = new FileOutputStream(targetFile.getAbsolutePath()+".slimcode");
				int b = -1;
				while ((b = in1.read()) != -1) {
					fos.write(b);
				}
				fos.flush();
				fos.close();
				in1.close();
				p1.waitFor();

				String cmd2 = Env.get("SLIM_EXE_PATH")+" "+option+" \""+targetFile.getAbsolutePath()+".slimcode\"";
				FrontEnd.println("(SLIM) "+cmd2);

				pb = new ProcessBuilder(strList(cmd2));
				pb.redirectErrorStream(true);
				p2 = pb.start();

				BufferedReader in2 = new BufferedReader(new InputStreamReader(p2.getInputStream()));
				if(buffer==null){
					if(output==null){
						output = FrontEnd.mainFrame.toolTab.systemPanel.outputPanel;
					}
					output.outputStart("slim", option, targetFile);
					String str;
					while ((str=in2.readLine())!=null) {
						output.outputLine(str);
					}
					output.outputEnd();
				}else{
					String str;
					while ((str=in2.readLine())!=null) {
						buffer.append(str+"\n");
					}
				}

				in2.close();
				p2.waitFor();

				exit();
				success = true;

				*/

			}catch(Exception e){
				if(!quiet) FrontEnd.printException(e);
				System.out.println("test14");

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
			if(p1!=null) p1.destroy();
			if(p2!=null) p2.destroy();
		}

	}
/* ���Ɏg���񂾂낤��
	static public boolean checkRun(){
		File f = new File("temp.lmn");
		try {
			FileWriter fp = new FileWriter(f);
			fp.write("slimruncheckatom.");
            fp.close();
		} catch (IOException e) {}

		final SlimRunner slimRunner = new SlimRunner("",f);
		slimRunner.setBuffering(true);
		slimRunner.setQuiet(true);
		slimRunner.run();

		int count = 0;
		while(slimRunner.isRunning()){
			FrontEnd.sleep(200);
			if(count++>10){
				slimRunner.kill();
				return false;
			}
		}
		if(slimRunner.getBufferString().indexOf("slimruncheckatom")>=0){
			return true;
		}else{
			return false;
		}
	}
*/
}
