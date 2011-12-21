package test.phaseviewer.filecontrols;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

public class FileControls{

	public String filename;

	public FileControls(String filename){
		this.setFilename(filename);
	}

	public FileControls(){

	}

	public String getFilename() {
		return filename;
	}
	public void setFilename(String filename) {
		this.filename = filename;
	}

	public String toString(){
		return filename;
	}

}
