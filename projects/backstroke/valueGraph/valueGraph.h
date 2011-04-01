#ifndef BACKSTROKE_VALUE_GRAPH
#define BACKSTROKE_VALUE_GRAPH

#include "valueGraphNode.h"
#include <ssa/staticSingleAssignment.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/bind.hpp>

namespace Backstroke
{


struct ValueGraphEdge
{
	ValueGraphEdge() : cost(0) {}
	ValueGraphEdge(int c) : cost(c) {}
	
	virtual void writeDotString(std::ostream& out) const
	{
		out << "[label=\"" << cost << "\"]";
	}

	int cost;
};

struct OrderedEdge : ValueGraphEdge
{
	OrderedEdge(int idx) : index(idx) {}
	
	int index;
};

//class ValueGraph : public boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
//		ValueGraphNode*, ValueGraphEdge*>
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
		ValueGraphNode*, ValueGraphEdge*> ValueGraph;

class EventReverser
{
public:
	typedef boost::graph_traits<ValueGraph>::vertex_descriptor VGVertex;
	typedef boost::graph_traits<ValueGraph>::edge_descriptor VGEdge;

	typedef StaticSingleAssignment SSA;
	typedef SSA::VarName VarName;

private:
	//! The SSA form of the function definition.
	SSA ssa_;

	//! The value graph object.
	ValueGraph valueGraph_;
	
	//! A map from SgNode to vertex of Value Graph.
	std::map<SgNode*, VGVertex> nodeVertexMap_;

	//! A map from variable with version to vertex of Value Graph.
	std::map<VersionedVariable, VGVertex> varVertexMap_;

	//! A map from variable with version to its reaching def object.
	//! This map is only for pseudo defs.
	std::map<VersionedVariable, SSA::ReachingDefPtr> varReachingDefMap_;

	//! All values which need to be restored (state variables).
	std::vector<VGVertex> valuesToRestore_;

	//! All available values, including constant and state variables (last version).
	std::set<VGVertex> availableValues_;

	//! All state variables.
	std::set<VarName> stateVariables_;

	////! All available nodes (the last version of state variables).
	//std::set<Vertex> availableNodes_;

	//! The start point of the search.
	VGVertex root_;

//	typedef CFG<VirtualCFG::InterestingNode,
//			VirtualCFG::InterestingEdge> CFG;
//	typedef CDG<CFG> CDG;
public:
	
	EventReverser()
	: ssa_(SageInterface::getProject())
	{
		ssa_.run(false);
	}

	//! Build the value graph for the given function.
	void buildValueGraph(SgFunctionDefinition* funcDef);

	//! Generate a dot file describing the value graph.
	void valueGraphToDot(const std::string& filename) const;

	void searchValueGraph();
	
private:

	/** This function set or add a def node to the value graph. If the variable defined is assigned by a
	 *  node with a temporary variable, just set the name and version to the temporary one.
	 *  Or else, build a new graph node and add an edge from this new node to the target node.
	 *
	 *  @param defNode The AST node which is defined.
	 *  @param useVertex The value graph vertex which defineds the new node.
	 */
	void setNewDefNode(SgNode* defNode, VGVertex useVertex);

	void writeValueGraphNode(std::ostream& out, const VGVertex& node) const
	{
		valueGraph_[node]->writeDotString(out);
	}

	void writeValueGraphEdge(std::ostream& out, const VGEdge& edge) const
	{
		valueGraph_[edge]->writeDotString(out);
	}

	/** Add a new vertex to the value graph.
	 *
	 *  @param newNode A value graph node which will be added.
	 *  @param sgNode The AST node which maps to the added value graph node.
	 *  @returns The new added vertex.
	 */
	VGVertex addValueGraphNode(ValueGraphNode* newNode, SgNode* sgNode = NULL);

	/** Add a new edge to the value graph.
	 *
	 *  @param src The source vertex.
	 *  @param tar The target vertex.
	 *  @param cost The weight of the edge.
	 *  @returns The new added edge.
	 */
	VGEdge addValueGraphEdge(VGVertex src, VGVertex tar, int cost = 0);

	/** Add a new ordered edge to the value graph.
	 *
	 *  @param src The source vertex.
	 *  @param tar The target vertex.
	 *  @param index The index of the edge (for example, for a binary operator,
	 * 0 means lhs operand, and 1 means rhs operand).
	 *  @returns The new added edge.
	 */
	VGEdge addValueGraphOrderedEdge(VGVertex src, VGVertex tar, int index);

	//! Add a phi node to the value graph.
	VGVertex addValueGraphPhiNode(VersionedVariable& var);

	//! Connect each variable node to the root with cost.
	void addStateSavingEdges();
	
	//! Given a type, return the cost of saving it.
	static int getCost(SgType* t);

	bool isStateVariable(const VarName& name)
	{
		return stateVariables_.count(name) > 0;
	}

	/** Given a SgNode, return its variable name and version.
	 * 
	 *  @param node A SgNode which should be a variable (either a var ref or a declaration).
	 *  //@param isUse Inidicate if the variable is a use or a def.
	 */
	VersionedVariable getVersionedVariable(SgNode* node, bool isUse = true);


	static VGVertex nullVertex()
	{ return boost::graph_traits<ValueGraph>::null_vertex(); }
};

} // End of namespace Backstroke



#endif // BACKSTROKE_VALUE_GRAPH