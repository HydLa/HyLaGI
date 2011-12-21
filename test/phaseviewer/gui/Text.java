package test.phaseviewer.gui;

import java.awt.Component;

public class Text {

	//égópñ@ê‡ñæ
	private String pvHelpText =
			"<html>PhaseViewer functional detail"+
			"<p>: shift+drag=>rotatory"+
			"<p>: ctrl+drag=>transform+" +
			"<p>: clicking node=>mathematical constraint indication" +
			"<p>: moving mousewheel=>scaling" +
			"<p>: drag=>translation"+
			"<p>: PICKING+node click+drag=>choosed node translation"+
			"<p>: PICKING+drag and choose+drag=>movement selected graph </html>";

	public String helptext() {
		return pvHelpText;
	}

}
