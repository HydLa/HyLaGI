


package test.editor;

import javax.swing.SizeRequirements;
import javax.swing.text.AbstractDocument;
import javax.swing.text.BoxView;
import javax.swing.text.ComponentView;
import javax.swing.text.Element;
import javax.swing.text.IconView;
import javax.swing.text.LabelView;
import javax.swing.text.ParagraphView;
import javax.swing.text.StyleConstants;
import javax.swing.text.StyledEditorKit;
import javax.swing.text.View;
import javax.swing.text.ViewFactory;

//

public class NoWrapEditorKit extends StyledEditorKit{

	public ViewFactory getViewFactory() {
		return new NoWrapViewFactory();
	}

	class NoWrapViewFactory implements ViewFactory {
		public View create(Element elem) {
			String kind = elem.getName();
			if (kind != null) {
				if(kind.equals(AbstractDocument.ContentElementName)) {
					return new LabelView(elem);
				}else if (kind.equals(AbstractDocument.ParagraphElementName)) {
					return new NoWrapParagraphView(elem);
				}else if (kind.equals(AbstractDocument.SectionElementName)) {
					return new BoxView(elem, View.Y_AXIS);
				}else if (kind.equals(StyleConstants.ComponentElementName)) {
					return new ComponentView(elem);
				}else if (kind.equals(StyleConstants.IconElementName)) {
					return new IconView(elem);
				}
			}
			return new LabelView(elem);
		}

		class NoWrapParagraphView extends ParagraphView {
			public NoWrapParagraphView(Element elem) {
				super(elem);
			}
			protected SizeRequirements calculateMinorAxisRequirements(int axis, SizeRequirements r) {
				SizeRequirements req = super.calculateMinorAxisRequirements(axis, r);
				req.minimum = req.preferred;
				return req;
			}
			public int getFlowSpan(int index) {
				return Integer.MAX_VALUE;
			}
		}
	}

}
