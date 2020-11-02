//*****************************************************************************
#ifndef __LOCATION_NETWORK_H
#define __LOCATION_NETWORK_H
//*****************************************************************************
#include "CentroidNeighbors.h"
#include <list>
#include <map>
#include <set>
#include <boost/optional.hpp>
#include "boost/tuple/tuple.hpp"
//*****************************************************************************

typedef std::vector<LocationDistance> LocationContainer_t;

class NetworkNode {
	public:
		typedef std::pair<const NetworkNode*, double>        NodeDistance_t;
		typedef std::list<NodeDistance_t>                    ConnectionsContainer_t;
		typedef std::list<NodeDistance_t>                    CandidatesContainer_t;
		typedef boost::optional<NetworkNode::NodeDistance_t> ClosestNode_t;

	protected:
		tract_t  _tract_index;
		ConnectionsContainer_t _connections;

	public:
		NetworkNode(tract_t tract_index) : _tract_index(tract_index) {}

		bool                             operator<(const NetworkNode& rhs) const { return getTractIndex() < rhs.getTractIndex(); }

		bool                             add_connection(NetworkNode* pNetworkNode, double distance);
		const ConnectionsContainer_t   & getConnections() const { return _connections; }
		tract_t                          getTractIndex() const { return _tract_index; }
		ClosestNode_t                    getClosestVisitedConnection(const boost::dynamic_bitset<>& visited) const;
};

class CompareNodeDistance {
	public:
		bool operator() (const NetworkNode::NodeDistance_t& lhs, const NetworkNode::NodeDistance_t& rhs) {
			return lhs.first->getTractIndex() < rhs.first->getTractIndex();
		}
};

class NetworkPathTree {
	public:
		typedef std::map<tract_t, NetworkNode> NodesContainer_t;

	private:
		NetworkNode             _root;
		NodesContainer_t        _nodes_in_tree;

	public:
		NetworkPathTree(const NetworkNode& root);

		void   add_connection(const NetworkNode& networkNode, double distance, const boost::dynamic_bitset<>& visited);

		const NetworkNode & getRoot() const { return _nodes_in_tree.at(_root.getTractIndex());  }
};

class CCluster;

class Network {
	public:
		typedef std::map<tract_t, NetworkNode>         NetworkContainer_t;
		typedef boost::optional<unsigned int>          LimitTo_t;
		typedef boost::tuple<tract_t, tract_t, double> Connection_Detail_t;

		class ConnectionDetailCompare {
			public:
				bool operator () (const Network::Connection_Detail_t& lhs, const Network::Connection_Detail_t& rhs) const {
					return lhs.get<0>() == rhs.get<0>() ? lhs.get<1>() < rhs.get<1>() : lhs.get<0>() < rhs.get<0>();
				}
		};

		typedef std::set<Connection_Detail_t, ConnectionDetailCompare>  Connection_Details_t;


	protected:
		NetworkContainer_t _nodes;

	public:
		bool                         addConnection(tract_t t1, tract_t t2, double distance);
		void				         addNode(tract_t t1);
		LocationContainer_t        & buildNeighborsAboutNode(const NetworkNode& node, LocationContainer_t& locations, NetworkPathTree * pathTree = 0, LimitTo_t limitTo = boost::none) const;
		const NetworkContainer_t   & getNodes() const { return _nodes; }

		Connection_Details_t         getClusterConnections(const CCluster& cluster, const CSaTScanData& DataHub) const;
		static void                  printPath(const LocationContainer_t& nodePath, const TractHandler& tracts);
		static void                  printTreePath(const NetworkPathTree& treePath, const TractHandler& tracts);
		static void                  printTreePath(const NetworkNode& node, const TractHandler& tracts);
		void                         printTreePath(const TractHandler& tracts, const CCluster& cluster) const;
};
#endif