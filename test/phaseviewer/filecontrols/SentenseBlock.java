package test.phaseviewer.filecontrols;

public class SentenseBlock {

	private String[] block;

	public SentenseBlock(){
		block=null;
	}
	public void splitWhiteSpase(String line){
		setBlock(line.trim().split("\\s+"));
	}
	public String[] getBlock() {
		return block;
	}
	public void setBlock(String[] block) {
		this.block = block;
	}

	public int getSize(){
		return block.length;
	}

	public String getBlock0(){
		return block[0];
	}

	public String getBlock0(boolean isfirstBlockEmpty){
		return isfirstBlockEmpty?"":block[0];
	}

	public String getBlock1(){
		return block[1];
	}

	public String getBlock1(boolean isSecondBlockEmpty){
		return isSecondBlockEmpty?"":block[1];
	}


	public boolean isBlockEmpty(int block) {
		return getSize()<block?true:false;
	}

	public String toString(){
		return "block[0]= "+ getBlock0(isBlockEmpty(1)) + " block[1]= "+ getBlock1(isBlockEmpty(2));
	}

}
