

package test.runner;

import java.io.File;

public interface RunnerOutputGetter {
	public void outputStart(String command,String option,File target);
	public void outputLine(String str);
	public void outputChar(char c);
	public void outputEnd();
}
