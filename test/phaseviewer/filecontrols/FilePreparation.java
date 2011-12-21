package test.phaseviewer.filecontrols;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.Map;

public class FilePreparation extends FileControls{

	private SentenseBlock block;// = new SentenseBlock();
	PhaseList phaselist;// = new PhaseList();
	private String line;

	public FilePreparation(String filename) {
		super(filename);
	}


    public FilePreparation() {

	}


public void Fileloader(){
		try {
			BufferedReader reader = new BufferedReader(new FileReader(filename));
			block = new SentenseBlock();
			phaselist = new PhaseList();
			phaselist.getmap().clear();
			while((line = reader.readLine())!=null){
				 block.splitWhiteSpase(line);
				 if(getConstraintEQ()!=""){
					 phaselist.add(getPhaseName(), getConstraintEQ());
				 }else{
					 phaselist.add(getPhaseName(), "");
				 }
			}//while�̏I���
		reader.close();
		} catch (FileNotFoundException e) {
			System.out.println(filename + "��������܂���");
			String sPath = (new File(".")).getAbsoluteFile().getParentFile().toString();
			System.out.println("���݂̃f�B���N�g����" + sPath + "�ł�");
		} catch (IOException e) {
			System.out.println(e);
		}//try�̏I���
    }

   public String getPhaseName(){
	   return block.getBlock0(block.isBlockEmpty(1));
   }

   public String getConstraintEQ(){
	   return block.getBlock1(block.isBlockEmpty(2));
   }

	public Map<String,String> getmap(){
		return phaselist.getmap();
	}

	public String getFilename(){
		return filename;
	}

	public void setFilename(String filename){
		this.filename = filename;
	}
}
