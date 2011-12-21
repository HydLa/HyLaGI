
package test;

import java.awt.Image;
import java.awt.Toolkit;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.LineNumberReader;
import java.io.OutputStreamWriter;
import java.net.URL;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

public class Env {

	static public final String APP_NAME =  "HIDE -HydLa Integrated Development Environment-";
	static public final String APP_VERSION = "0.5";
	static public final String APP_DATE = "2011/07/01";
	static public final String APP_HREF = "http://www.ueda.info.waseda.ac.jp/hydla/";

	static public final String HYDLA_VERSION = "HydLa : (2011/06/30)";

	static public final String DIR_NAME_HYDLA = "hydla_0.5x";//�ύX���܂���
	static public final String HYDLA_LIBRARY_DIR = "hydla";

	static public Env env = null;
    static private final String ENV_FILE = "env.txt";
//    static private final String ENV_DEFAULT_FILE = "env.txt";//"env_default.txt";

    static private Properties prop = new Properties();

    static public final String IMAGEFILE_ICON = "img/icon.png";

    static public final String FONT_SIZE_LIST[] = {"8","9","10","11","12","14","16","18","20","24","28","32","36","40","44","48","54","60","66","72","80","88","96","106"};

    public Env(){
    	env = this;

    	boolean firstMake = false;
    	try{
    		File e = new File(ENV_FILE);
    		if(!e.exists()){
    			System.out.println("make "+ENV_FILE);
    			e.createNewFile();
    			firstMake = true;
    		}
    	}catch (Exception e){
    		System.err.println(ENV_FILE+" make error. check "+(new File(".")).getAbsolutePath());
            System.exit(0);
    	}

    	try {
    		InputStream in = Env.getInputStreamOfFile(ENV_FILE);
			prop.load(in);
			in.close();
        } catch (Exception e) {
        	System.err.println("read error. check "+ENV_FILE);
            System.exit(0);
        }


        Properties default_prop = new Properties();
        try {
    		InputStream in = Env.getInputStreamOfFile(ENV_FILE);//ENV_DEFAULT_FILE����ύX
    		default_prop.load(in);
			in.close();
			for(Object k : default_prop.keySet()){
				String key = (String)k;
				String value = default_prop.getProperty(key);
				if(!prop.containsKey(key)){
					prop.setProperty(key, value);
					if(!firstMake){ System.out.println("auto update "+ENV_FILE+" : "+key+"="+value); }
				}
			}
        } catch (Exception e) {
        	System.err.println("read error. check "+ENV_FILE);//ENV_DEFAULT_FILE����ύX
        }
    }

    static public void save(){
		try {

			FileOutputStream out = new FileOutputStream(ENV_FILE);
			prop.store(out, APP_NAME+" "+APP_VERSION);
	        out.close();


	        LineNumberReader reader = new LineNumberReader(new FileReader(ENV_FILE));
			ArrayList<String> lines = new ArrayList<String>();
			String line = null;
			while((line = reader.readLine())!=null){
				lines.add(line);
			}
			reader.close();
			Collections.sort(lines);
			BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(ENV_FILE)));
			for(String str : lines){
				writer.write(str+"\n");
			}
			writer.close();
		} catch (IOException e) {
			System.err.println("save error. check "+ENV_FILE);
		}
    }

    static public String get(String key){
    	if(!prop.containsKey(key)){
    		System.err.println("read error. check "+key+" in "+ENV_FILE);
    	}
    	return prop.getProperty(key);
    }

    static public int getInt(String key){
    	if(!prop.containsKey(key)){
    		System.err.println("read error. check "+key+" in "+ENV_FILE);
    	}
    	return Integer.valueOf(prop.getProperty(key));
    }

    static public boolean is(String key){
    	if(!prop.containsKey(key)){
    		System.err.println("read error. check "+key+" in "+ENV_FILE);
    	}
    	return Boolean.valueOf(prop.getProperty(key));
    }

    static public boolean isSet(String key){
    	if(Env.get(key)==null||Env.get(key).equals("")){
    		return false;
    	}else{
    		return true;
    	}
    }

    static public void setProcessEnvironment(Map<String,String> map){

    	map.put("LMNTAL_HOME",getLmntalLinuxPath());

    	if(Env.isWindows()){
    		String bin = "";
    		bin += get("WINDOWS_CYGWIN_DIR")+File.separatorChar+"bin;";
    		bin += get("WINDOWS_CYGWIN_DIR")+File.separatorChar+"usr"+File.separatorChar+"bin;";
    		bin += get("WINDOWS_CYGWIN_DIR")+File.separatorChar+"usr"+File.separatorChar+"local"+File.separatorChar+"bin;";
    		if(map.get("path")!=null) map.put("path",bin+map.get("path"));
    		if(map.get("Path")!=null) map.put("Path",bin+map.get("Path"));
    		if(map.get("PATH")!=null) map.put("PATH",bin+map.get("PATH"));
    		if(map.get("path")==null&&map.get("Path")==null&&map.get("PATH")==null){
    			map.put("PATH",bin);
    		}
		}

    }

    static public String getDirNameOfHydLa(){
    	if(isSet("DIR_NAME_HYDLA")){
    		return Env.get("DIR_NAME_HYDLA");
    	}else{
    		return DIR_NAME_HYDLA;
    	}
    }

    static public String getSpaceEscape(String path){
    	if(path.indexOf(" ")==-1){
    		return path;
    	}else{
    		return "\""+path+"\"";
    	}
    }

    static public String getSlimInstallPath(){
    	return HYDLA_LIBRARY_DIR+File.separatorChar+"installed";
    }
/*
    static public String getSlimInstallLibraryPath(){
    	return getSlimInstallPath()+File.separator+"share"+File.separator+"slim"+File.separator+"lib";
    }
*/
    static public String getLmntalLinuxPath(){
    	String path = (new File(HYDLA_LIBRARY_DIR)).getAbsolutePath();
    	path = getLinuxStylePath(path);
    	return path;
    }
/*
    static public String getSlimInstallLinuxPath(){
    	String path = (new File(getSlimInstallPath())).getAbsolutePath();
    	path = getLinuxStylePath(path);
    	return path;
    }
*/
    static public String getLinuxStylePath(String path){
    	if(File.separatorChar=='\\'){
    		path = path.replace('\\','/');
    		if(path.startsWith("c:/")||path.startsWith("C:/")){
    			path = "/cygdrive/c/"+path.substring(3);
    		}
        }
    	return path;
    }

    static public String getLmntalCmd(){
    	String cmd = "java";
		cmd += " -classpath ";
//		cmd += Env.LMNTAL_LIBRARY_DIR+File.separator+"bin"+File.separator+"lmntal.jar"+File.pathSeparator;
//		cmd += Env.LMNTAL_LIBRARY_DIR+File.separator+"lib"+File.separator+"std_lib.jar";
//		cmd += " -DLMNTAL_HOME="+Env.LMNTAL_LIBRARY_DIR;
		cmd += Env.HYDLA_LIBRARY_DIR+File.separator+"bin"+File.separator+"lmntal.jar"+File.pathSeparator;
		cmd += Env.HYDLA_LIBRARY_DIR+File.separator+"lib"+File.separator+"std_lib.jar";
		cmd += " -DLMNTAL_HOME="+Env.HYDLA_LIBRARY_DIR;

		cmd += " runtime.FrontEnd --interpret ";
		return cmd;
    }

    static public String getBinaryAbsolutePath(String cmd){
    	String usrLocalBin = File.separatorChar+"usr"+File.separatorChar+"local"+File.separatorChar+"bin"+File.separatorChar+cmd;
		String usrBin = File.separatorChar+"usr"+File.separatorChar+"bin"+File.separatorChar+cmd;
		String bin = File.separatorChar+"bin"+File.separatorChar+cmd;
    	if(Env.isWindows()){
    		usrLocalBin = get("WINDOWS_CYGWIN_DIR")+usrLocalBin+".exe";
    		usrBin = get("WINDOWS_CYGWIN_DIR")+usrBin+".exe";
    		bin = get("WINDOWS_CYGWIN_DIR")+bin+".exe";
    		if((new File(usrLocalBin)).exists()){
    			return usrLocalBin;
    		}else if((new File(usrBin)).exists()){
    			return usrBin;
    		}else if((new File(bin)).exists()){
    			return bin;
    		}else{
    			return cmd;
    		}
    	}else{
    		if((new File(usrLocalBin)).exists()){
    			return usrLocalBin;
    		}else if((new File(usrBin)).exists()){
    			return usrBin;
    		}else if((new File(bin)).exists()){
    			return bin;
    		}else{
    			return cmd;
    		}
    	}
    }

    static public String getSlimBinaryName(){
    	if(Env.isWindows()){
    		return "slim.exe";
		}else{
			return "slim";
		}
    }

    static public String getLtl2baBinaryName(){
    	if(Env.isWindows()){
    		return "ltl2ba.exe";
		}else{
			return "ltl2ba";
		}
    }

    static public double getPercentage(String key,double per){
    	String str = get(key);
		if(str.matches("^[0-9]{1,3}%$")){
			int t = Integer.parseInt(str.substring(0,str.indexOf('%')));
			if(t<0){ t=0; }else if(t>100){ t=100; }
			per = t/100.0;
        }
		return per;
	}

    static public void set(String key,String value){
    	prop.setProperty(key, value);
    }

    static public void set(String key,boolean value){
    	prop.setProperty(key, String.valueOf(value));
    }

    static public void set(String key,int value){
    	prop.setProperty(key, String.valueOf(value));
    }

	static public void setPercentage(String key,double per){
		int res = (int)(per*100);
		if(res<0){
			set(key,"0%");
		}else if(res>100){
			set(key,"100%");
		}else{
			set(key,res+"%");
		}
	}


    static public InputStream getInputStreamOfFile(String filename){
    	InputStream in = null;
    	File file = new File(filename);
    	try {
    		if(file.exists()){
    			in = new FileInputStream(file);
    		}else{
    			in = env.getClass().getResourceAsStream("/"+filename);
    		}
    	} catch (Exception e) {
    		e.printStackTrace();
    	}
    	return in;
    }


    static public Image getImageOfFile(String filename){
    	File file = new File(filename);
    	if(file.exists()){
    		return Toolkit.getDefaultToolkit().getImage(file.getPath());
    	}else{
    		URL fileUrl = env.getClass().getResource("/"+filename);
    		return Toolkit.getDefaultToolkit().getImage(fileUrl);
    	}
    }

    static public boolean isWindows(){
    	return File.pathSeparatorChar==';';
    }


    static HashMap<String,Long> watchNowTimes = new HashMap<String,Long>();
    static HashMap<String,Long> watchSumTimes = new HashMap<String,Long>();
    static HashMap<String,Integer> watchCount = new HashMap<String,Integer>();

    static public void startWatch(String key){
    	watchNowTimes.put(key, System.currentTimeMillis());
    }

    static public void stopWatch(String key){
    	long t;
    	if(watchNowTimes.containsKey(key)){
    		t = System.currentTimeMillis() - watchNowTimes.get(key);
    	}else{
    		t = 0;
    	}
    	if(watchSumTimes.containsKey(key)){
    		long sum = watchSumTimes.get(key);
    		watchSumTimes.put(key, sum+t);
    		watchCount.put(key, watchCount.get(key)+1);
    	}else{
    		watchSumTimes.put(key, t);
    		watchCount.put(key, 1);
    	}
    }

    static public void dumpWatch(){
    	DecimalFormat f = new DecimalFormat("####.##");
    	if(watchSumTimes.size()>0){
    		System.out.println("---- watch = "+watchSumTimes.size()+" ----");
    	}
    	for(String key : watchSumTimes.keySet()){
    		double t = watchSumTimes.get(key)/1000.0;
    		System.out.println("watch[" + key + "] : " + f.format(t) + " ("+watchCount.get(key)+")");
    	}
    	if(watchSumTimes.size()>0){
    		System.out.println();
    	}
    	watchSumTimes.clear();
    }

}