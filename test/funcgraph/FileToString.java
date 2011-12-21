package test.funcgraph;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;

public class FileToString {
	  // �t�@�C�����e���𕶎��񉻂��郁�\�b�h�ł��B
	  public String fileToString(File file) throws IOException {
	    BufferedReader br = null;
	    try {
	      // �t�@�C����ǂݍ��ރo�b�t�@�h���[�_���쐬���܂��B
	      br = new BufferedReader(new InputStreamReader(new FileInputStream(file)));
	      // �ǂݍ��񂾕������ێ�����X�g�����O�o�b�t�@��p�ӂ��܂��B
	      StringBuffer sb = new StringBuffer();
	      // �t�@�C������ǂݍ��񂾈ꕶ����ۑ�����ϐ��ł��B
	      int c;
	      // �t�@�C������P�������ǂݍ��݁A�o�b�t�@�֒ǉ����܂��B
	      while ((c = br.read()) != -1) {

	        sb.append((char) c);
	      }
	      // �o�b�t�@�̓��e�𕶎��񉻂��ĕԂ��܂��B
	      String str = sb.toString();
	      str = str.replace("`","");

	      return str;
	    } finally {
	      // ���[�_����܂��B
	      br.close();
	    }
	  }
}