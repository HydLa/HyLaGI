package test.phaseviewer.filecontrols;

import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;

public class PhaseList {

	 Map<String,String> phaseMap = new LinkedHashMap<String,String>();
	 String phaseName = "";
	 String constraintEQ = "";

	public PhaseList(){

	}



	public void add(String phaseName, String constraintEQ) {
		this.phaseName = phaseName;
		this.constraintEQ = constraintEQ;
		phaseMap.put(phaseName, constraintEQ);
	}


	public Map<String,String> getmap(){
		return phaseMap;
	}

	public String toString(){
		return "phaseName : "+phaseName +" "+ "constraintEQ : "+phaseMap.get(phaseName);
	}
}
