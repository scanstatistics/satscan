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

//typedef std::vector<LocationDistance> LocationContainer_t;

class NetworkNode {
    public:
        enum AddStatusType { Added=0, Averaged, SelfReference, ConflictOther };

	public:
		typedef boost::tuple<const NetworkNode*, double, bool, bool>  NodeDistance_t; // connection node, distance, averaged, record source
		typedef std::list<NodeDistance_t>                             ConnectionsContainer_t;
		typedef std::list<NodeDistance_t>                             CandidatesContainer_t;
		typedef boost::optional<NetworkNode::NodeDistance_t>          ClosestNode_t;

	protected:
		const Location & _location;
		unsigned int  _location_index;
		ConnectionsContainer_t _connections;

	public:
		NetworkNode(tract_t location_index, const Location& location) : _location_index(location_index), _location(location) {}

		bool                             operator<(const NetworkNode& rhs) const { return getLocationIndex() < rhs.getLocationIndex(); }

        AddStatusType                    add_connection(NetworkNode* pNetworkNode, double distance, bool recordDef);
		const ConnectionsContainer_t   & getConnections() const { return _connections; }
		tract_t                          getLocationIndex() const { return _location_index; }
		const Location                 & getLocation() const { return _location; }
		ClosestNode_t                    getClosestVisitedConnection(const boost::dynamic_bitset<>& visited) const;

        static const char              * getStatusMessage(AddStatusType e) {
            switch (e) {
            case SelfReference: return "location references self as connection";
            case ConflictOther: return "connection conflicts with a previous connection definition";
            case Averaged:
            default: return "";
            }
        }
};

typedef std::vector<std::pair<const NetworkNode*, double> > NetworkLocationContainer_t;

class CompareNodeDistance {
	public:
		bool operator() (const NetworkNode::NodeDistance_t& lhs, const NetworkNode::NodeDistance_t& rhs) {
			return lhs.get<0>()->getLocationIndex() < rhs.get<0>()->getLocationIndex();
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

		const NetworkNode & getRoot() const { return _nodes_in_tree.at(_root.getLocationIndex());  }
};

class CCluster;

class Network {
	public:
		typedef std::map<unsigned int, NetworkNode>    NetworkContainer_t;
		typedef boost::optional<unsigned int>          LimitTo_t;
        typedef boost::tuple<const Location*, const Location*> Connection_Detail_t;

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
        NetworkNode::AddStatusType   addConnection(unsigned int t1, const Location& location1, unsigned int t2, const Location& location2, double distance, bool recordDef);
		void				         addNode(unsigned int t1, const Location& location);
        NetworkLocationContainer_t & buildNeighborsAboutNode(const NetworkNode& node, NetworkLocationContainer_t& networkLocations, unsigned int num_locations, NetworkPathTree * pathTree = 0, LimitTo_t limitTo = boost::none) const;
        Connection_Details_t         getClusterConnections(const CCluster& cluster, const CSaTScanData& DataHub) const;
        double                       getDistanceBetween(const Location& location1, const Location& location2, const ObservationGroupingManager& groups) const;
        const NetworkContainer_t   & getNodes() const { return _nodes; }
        bool                         locationIndexInNetwork(unsigned int locationIdx) { return _nodes.find(locationIdx) != _nodes.end(); }
        static void                  printPath(const NetworkLocationContainer_t& nodePath, const ObservationGroupingManager& groups);
		static void                  printTreePath(const NetworkPathTree& treePath, const ObservationGroupingManager& groups);
		static void                  printTreePath(const NetworkNode& node, const ObservationGroupingManager& groups);
		void                         printTreePath(const ObservationGroupingManager& groups, const CCluster& cluster) const;
};
#endif