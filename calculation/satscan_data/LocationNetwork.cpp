//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "LocationNetwork.h"
#include "Toolkit.h"
#include "cluster.h"

////////////////////////// NetworkNode /////////////////////////////////////////////

NetworkNode::AddStatusType NetworkNode::add_connection(NetworkNode* pNetworkNode, double distance, bool recordDef) {
	if (getTractIndex() == pNetworkNode->getTractIndex())
		return NetworkNode::SelfReference;

	NodeDistance_t nodeConnection(pNetworkNode, distance, false, recordDef);
	ConnectionsContainer_t::iterator itrConnection = std::lower_bound(_connections.begin(), _connections.end(), nodeConnection, CompareNodeDistance());
	if (itrConnection == _connections.end() || itrConnection->get<0>()->getTractIndex() != nodeConnection.get<0>()->getTractIndex()) {
		itrConnection = _connections.insert(itrConnection, nodeConnection);
        return NetworkNode::Added;
    }

    // A node connection exists already for this node and connection but the distance matches -- no issue.
    if (itrConnection->get<1>() == nodeConnection.get<1>())
        return NetworkNode::Added;

    /* 
    A node connection exists already and the distance does not match - take the average of current value and 'distance'. 
    We only do this once to account for situations when the user created a network file with minor variation is values
    (e.g. A -> B = 2.0, B -> A = 2.000001).
    */

    // There are exceptions to above situation - user specified A -> B = 2.0 and A -> B = 2.000001; or value is already averaged.
    if (itrConnection->get<2>() || (itrConnection->get<3>() && recordDef))
        return NetworkNode::ConflictOther;

    // Update distance to average of values and mark as averaged.
    itrConnection->get<1>() = (itrConnection->get<1>() + distance) / 2.0;
    itrConnection->get<2>() = true; // marked as averaged
    return NetworkNode::Added;
}

NetworkNode::ClosestNode_t NetworkNode::getClosestVisitedConnection(const boost::dynamic_bitset<>& visited) const {
	NetworkNode::ClosestNode_t closetVisited = boost::none;
	NetworkNode::ConnectionsContainer_t::const_iterator end = getConnections().end(), itr = getConnections().begin();
	for (; itr != end; ++itr) {
		if (visited.test(itr->get<0>()->getTractIndex())) {
			if (!closetVisited)
				closetVisited = *itr;
			else if (itr->get<1>() < closetVisited->get<1>() || (itr->get<1>() == closetVisited->get<1>() && itr->get<0>()->getTractIndex() < closetVisited->get<0>()->getTractIndex()))
				closetVisited = *itr;
		}
	}
	return closetVisited;
}

//////////////////////////////////////// NetworkPathTree //////////////////////////////////////

NetworkPathTree::NetworkPathTree(const NetworkNode& root): _root(NetworkNode(root.getTractIndex())) {
	_nodes_in_tree.insert(std::make_pair(root.getTractIndex(), _root));
}

void NetworkPathTree::add_connection(const NetworkNode& networkNode, double distance, const boost::dynamic_bitset<>& visited) {
	NetworkNode::ClosestNode_t closedNode = networkNode.getClosestVisitedConnection(visited);
	if (closedNode) {
		std::pair<NodesContainer_t::iterator, bool> parentNode = _nodes_in_tree.insert(std::make_pair(closedNode->get<0>()->getTractIndex(), NetworkNode(closedNode->get<0>()->getTractIndex())));
		std::pair<NodesContainer_t::iterator, bool> connectionNode = _nodes_in_tree.insert(std::make_pair(networkNode.getTractIndex(), NetworkNode(networkNode.getTractIndex())));
		parentNode.first->second.add_connection(&(connectionNode.first->second), distance, false);
	}
}

////////////////////////// Network /////////////////////////////////////////////////

NetworkNode::AddStatusType Network::addConnection(tract_t t1, tract_t t2, double distance, bool recordDef) {
	NetworkContainer_t::iterator iterT1 = _nodes.emplace(std::make_pair(t1, NetworkNode(t1))).first;
	NetworkContainer_t::iterator iterT2 = _nodes.emplace(std::make_pair(t2, NetworkNode(t2))).first;
	return iterT1->second.add_connection(&(iterT2->second), distance, recordDef);
}

void Network::addNode(tract_t t1) {
	_nodes.emplace(std::make_pair(t1, NetworkNode(t1)));
}

/** Build locations path from node, following it's connections, their connections, etc. */
LocationContainer_t& Network::buildNeighborsAboutNode(const NetworkNode& node, LocationContainer_t& locations, NetworkPathTree * pathTree, LimitTo_t limitTo) const {

	// Add node as first element in locations list.
	locations.push_back(LocationDistance(node.getTractIndex(), 0));
	if (node.getConnections().size() == 0)
		return locations; // This node is isloated from reset of network.

	if (limitTo && limitTo.get() == locations.size())
		return locations;

	NetworkNode::CandidatesContainer_t candidateNodes; // collection of nodes that are candidates as network expands
	std::map<tract_t, double> candidate_length; // maps tract index of candidate nodes to their distance from network center
	boost::dynamic_bitset<> visited(_nodes.size()), candidates(_nodes.size());

	visited.set(node.getTractIndex()); // Mark the node as visited.
	// Find node with shortest distance from starting node - all other connections from node become candidates.
	NetworkNode::ConnectionsContainer_t::const_iterator itrBestR=node.getConnections().begin(), endR=node.getConnections().end(), itrR=node.getConnections().begin();
	for (++itrR; itrR != endR; ++itrR) {
		if (itrR->get<1>() < itrBestR->get<1>() || (itrR->get<1>() == itrBestR->get<1>() && itrR->get<0>()->getTractIndex() < itrBestR->get<0>()->getTractIndex())) {
			candidateNodes.push_back(*itrBestR);
			candidates.set(itrBestR->get<0>()->getTractIndex());
			candidate_length[itrBestR->get<0>()->getTractIndex()] = itrBestR->get<1>();
			itrBestR = itrR;
		} else {
			candidateNodes.push_back(*itrR);
			candidates.set(itrR->get<0>()->getTractIndex());
			candidate_length[itrR->get<0>()->getTractIndex()] = itrR->get<1>();
		}
	}
	// Add the node with shortest distance to return list, then add that nodes connections as candidates.
	locations.push_back(LocationDistance(itrBestR->get<0>()->getTractIndex(), itrBestR->get<1>()));
	visited.set(itrBestR->get<0>()->getTractIndex());
	if (pathTree) pathTree->add_connection(*itrBestR->get<0>(), itrBestR->get<1>(), visited);
	if (limitTo && limitTo.get() == locations.size())
		return locations;
	itrR = itrBestR->get<0>()->getConnections().begin(), endR = itrBestR->get<0>()->getConnections().end();
	for (; itrR != endR; ++itrR) {
		if (!visited.test(itrR->get<0>()->getTractIndex()) && !candidates.test(itrR->get<0>()->getTractIndex())) {
			candidateNodes.push_back(*itrR);
			candidates.set(itrR->get<0>()->getTractIndex());
			candidate_length[itrR->get<0>()->getTractIndex()] = itrBestR->get<1>() + itrR->get<1>();
		}
	}
	// At this point the candidates will be all nodes off the starting node plus direct connections from that best node -- excluding the best node itself.
    // Now we process the candidates until we've exhauted the reachable network.
	double lengthBest, lengthCandidate;
	while (candidateNodes.size()) {
		// Search for the node with the shortest distance among all current candidates.
        NetworkNode::ConnectionsContainer_t::iterator itrBestC = candidateNodes.begin(), endC = candidateNodes.end(), itrC = candidateNodes.begin();
		for (++itrC; itrC != endC; ++itrC) {
			lengthCandidate = candidate_length[itrC->get<0>()->getTractIndex()];
			lengthBest = candidate_length[itrBestC->get<0>()->getTractIndex()];
			if (lengthCandidate < lengthBest || (lengthCandidate == lengthBest && itrC->get<0>()->getTractIndex() < itrBestC->get<0>()->getTractIndex()))
				itrBestC = itrC;
		}
		// At this point itrBest references the best candidate, so add it to the locations list along with it's distance from node center.
		double lengthBest = candidate_length[itrBestC->get<0>()->getTractIndex()];
		locations.push_back(LocationDistance(itrBestC->get<0>()->getTractIndex(), lengthBest));
		// Add all connections from itrBest as candidates as well - excluding any nodes already visited.
        NetworkNode::ConnectionsContainer_t::const_iterator itrB = itrBestC->get<0>()->getConnections().begin(), endB = itrBestC->get<0>()->getConnections().end();
		// But first set this node as visited and unset/remove from helper structures.
		visited.set(itrBestC->get<0>()->getTractIndex());
		if (pathTree) pathTree->add_connection(*itrBestC->get<0>(), itrBestC->get<1>(), visited);
		if (limitTo && limitTo.get() == locations.size())
			return locations;
		candidates.reset(itrBestC->get<0>()->getTractIndex());
		candidate_length.erase(itrBestC->get<0>()->getTractIndex());
		// Now remove current best node from candidates.
		candidateNodes.erase(itrBestC);
		// Add direct connections - being sure to include it's distance from node center.
		for (; itrB != endB; ++itrB) {
			if (!visited.test(itrB->get<0>()->getTractIndex()) && !candidates.test(itrB->get<0>()->getTractIndex())) {
				candidateNodes.push_back(*itrB);
				candidates.set(itrB->get<0>()->getTractIndex());
				candidate_length[itrB->get<0>()->getTractIndex()] = lengthBest + itrB->get<1>();
			}
		}
	}
	return locations;
}

Network::Connection_Details_t Network::getClusterConnections(const CCluster& cluster, const CSaTScanData& DataHub) const {
	Connection_Details_t connections;
	std::set<tract_t> clusterTracts;

	// Create collections of all locations in cluster.
	for (tract_t i=1; i <= cluster.GetNumTractsInCluster(); ++i) {
		tract_t tTract = DataHub.GetNeighbor(cluster.GetEllipseOffset(), cluster.GetCentroidIndex(), i);
		if (!DataHub.GetIsNullifiedLocation(tTract)) {
			clusterTracts.emplace(tTract);
		}
	}

	// Create collection of all connections between locations of cluster.
	std::set<tract_t>::const_iterator itr = clusterTracts.begin();
	for (; itr != clusterTracts.end(); ++itr) {
		const NetworkNode& node = _nodes.at(*itr);
		NetworkNode::ConnectionsContainer_t::const_iterator itrC = node.getConnections().begin();
		for (; itrC != node.getConnections().end(); ++itrC) {
			if (clusterTracts.find(itrC->get<0>()->getTractIndex()) != clusterTracts.end()) {
				connections.emplace(Connection_Detail_t(std::min(*itr, itrC->get<0>()->getTractIndex()), std::max(*itr, itrC->get<0>()->getTractIndex()), itrC->get<1>()));
			}
		}
	}

	return connections;
}

void Network::printPath(const LocationContainer_t& nodePath, const TractHandler& tracts) {
	double length = 0;
	LocationContainer_t::const_iterator itr = nodePath.begin(), end = nodePath.end();
	for (; itr != end; ++itr) {
		length += itr->GetDistance();
		std::cout << tracts.getIdentifier(itr->GetTractNumber()) << " distance " << itr->GetDistance() << " path length " << length << std::endl;
	}
}

void Network::printTreePath(const NetworkPathTree& treePath, const TractHandler& tracts) {
	fprintf(AppToolkit::getToolkit().openDebugFile(), "Node tree path : %s\n", tracts.getIdentifier(treePath.getRoot().getTractIndex()));
	printTreePath(treePath.getRoot(), tracts);
}

void Network::printTreePath(const NetworkNode& node, const TractHandler& tracts) {
	NetworkNode::ConnectionsContainer_t::const_iterator end = node.getConnections().end(), itr = node.getConnections().begin();
	for (; itr != end; ++itr) {
		std::stringstream write;
		write << tracts.getIdentifier(node.getTractIndex()) << " ==> " << tracts.getIdentifier(itr->get<0>()->getTractIndex()) << std::endl;
		//std::cout << tracts.getIdentifier(node.getTractIndex()) << " ==> " << tracts.getIdentifier(itr->first->getTractIndex()) << std::endl;
		fprintf(AppToolkit::getToolkit().openDebugFile(), "%s", write.str().c_str());
	}
	end = node.getConnections().end(), itr = node.getConnections().begin();
	for (; itr != end; ++itr) {
		printTreePath(*itr->get<0>(), tracts);
	}
}

void Network::printTreePath(const TractHandler& tracts, const CCluster& cluster) const {
	const NetworkNode& clusterRoot = _nodes.at(cluster.GetCentroidIndex());
	LocationContainer_t locationPath;
	NetworkPathTree networkPathTree(clusterRoot);
	buildNeighborsAboutNode(clusterRoot, locationPath, &networkPathTree, static_cast<size_t>(cluster.GetNumTractsInCluster()));
	printTreePath(networkPathTree, tracts);
}
