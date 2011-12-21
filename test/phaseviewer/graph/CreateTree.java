package test.phaseviewer.graph;

import org.apache.commons.collections15.Factory;

public class CreateTree extends Tree implements CreateGraph{

    private Factory<Integer> edgeFactory = new Factory<Integer>() {
        int i=0;
        public Integer create() {
                return i++;
        }
    };

	public CreateTree(String treeName){
		super(treeName);
	}

	@Override
	public boolean createGraph(String fst,String sec) {
		return getgraph().addEdge(edgeFactory.create(), fst, sec);
	}
}
