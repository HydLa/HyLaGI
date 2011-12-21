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

package test.runner;

import java.awt.Color;
import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.StringTokenizer;
import java.util.Timer;
import java.util.TimerTask;

import test.*;
import test.system.OutputPanel;
import test.util.OuterRunner;

public class RebootRunner implements OuterRunner {

	private ThreadRunner runner;
	private String option;

	public RebootRunner(String option){
		this.option = option;
		this.runner = new ThreadRunner();
	}

	public void run() {
		runner.start();
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


//	@Override
	public boolean isSuccess() {
		return true;
	}


	private class ThreadRunner extends Thread {

		private Process p;

		public void run() {
			try {
				String cmd = "javaw "+option+" -jar HydLa.jar --reboot";

				ProcessBuilder pb = new ProcessBuilder(strList(cmd));
				Env.setProcessEnvironment(pb.environment());
				pb.redirectErrorStream(true);
				p = pb.start();
				BufferedReader in = new BufferedReader(new InputStreamReader(p.getInputStream()));

				String str;
				while ((str=in.readLine())!=null){
					System.out.println(str);
				}

				in.close();
				p.waitFor();

			}catch(Exception e){
				FrontEnd.printException(e);

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
			if(p!=null){
				p.destroy();
			}
		}

	}

}
