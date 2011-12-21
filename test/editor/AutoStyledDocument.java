
package test.editor;

import java.awt.Color;

import javax.swing.event.*;
import javax.swing.text.*;

import test.FrontEnd;

public class AutoStyledDocument extends DefaultStyledDocument{
	WordColorChanger wcc;

	public AutoStyledDocument(){
		wcc = new WordColorChanger(this);
		wcc.start();
	}

	public void colorUpdate(){
		wcc.update();
	}

	public void colorChange(){
		wcc.change();
	}

	public void end(){
		wcc.end();
	}

	public String getPlainText(){
		try {
			return getText(0,getLength()).replaceAll("\r\n","\n");
		}catch (BadLocationException e){
			FrontEnd.printException(e);
		}
		return "";
	}

	public void setCharacterAttributes(int offset, int length, AttributeSet s, boolean replace) {
		if (length == 0) { return; }
		try {
			//writeLock();
			InsignificantDocumentEvent changes = new InsignificantDocumentEvent(offset, length, DocumentEvent.EventType.CHANGE);
			buffer.change(offset, length, changes);
			AttributeSet sCopy = s.copyAttributes();
			int lastEnd = Integer.MAX_VALUE;
			for (int pos = offset; pos < (offset + length); pos = lastEnd) {
				Element run = getCharacterElement(pos);
				lastEnd = run.getEndOffset();
				if (pos == lastEnd) { break; }
				MutableAttributeSet attr = (MutableAttributeSet) run.getAttributes();
				changes.addEdit(new InsignificantUndoableEdit(run, sCopy, replace));
				if (replace) { attr.removeAttributes(attr); }
				attr.addAttributes(s);
			}
			changes.end();
			fireChangedUpdate(changes);
			fireUndoableEditUpdate(new UndoableEditEvent(this, changes));
		} finally {
			//writeUnlock();
		}
	}

	public void setColor(int offset, int length, Color c, boolean bold){
		SimpleAttributeSet attribute = new SimpleAttributeSet();
		attribute.addAttribute(StyleConstants.Foreground, c);
		attribute.addAttribute(StyleConstants.FontConstants.Bold,bold);
		if(offset+length<=getLength()){
			setCharacterAttributes(offset,length,attribute,false);
		}
	}

	public void startStyleEdit(){
		writeLock();
		SimpleAttributeSet attribute = new SimpleAttributeSet();
		setCharacterAttributes(0,getLength(),attribute,true);
	}

	public void endStyleEdit(){
		SimpleAttributeSet attribute = new SimpleAttributeSet();
		setCharacterAttributes(getLength(),0,attribute,true);
		writeUnlock();
	}

	public class InsignificantDocumentEvent extends DefaultDocumentEvent{

		public InsignificantDocumentEvent(int offs, int len, EventType type) {
			super(offs, len, type);
		}

		public boolean isSignificant(){
			return false;
		}

	}

	public class InsignificantUndoableEdit extends AttributeUndoableEdit{

		public InsignificantUndoableEdit(Element element, AttributeSet newAttributes, boolean isReplacing) {
			super(element, newAttributes, isReplacing);
		}

		public boolean isSignificant() {
			return false;
		}

	}

}
