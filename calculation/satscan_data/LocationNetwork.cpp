//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "LocationNetwork.h"
#include "Toolkit.h"
#include "cluster.h"

////////////////////////// NetworkNode /////////////////////////////////////////////

bool NetworkNode::add_connection(NetworkNode* pNetworkNode, double distance) {
	if (getTractIndex() == pNetworkNode->getTractIndex())
		throw prg_error("Network node cannot be added as connection to itself (tract=%u).", "add_connection()", getTractIndex());
	NodeDistance_t nodeConnection(pNetworkNode, distance);
	ConnectionsContainer_t::iterator itrConnection = std::lower_bound(_connections.begin(), _connections.end(), nodeConnection, CompareNodeDistance());
	if (itrConnection == _connections.end() || itrConnection->first->getTractIndex() != nodeConnection.first->getTractIndex())
		itrConnection = _connections.insert(itrConnection, nodeConnection);
	// Return indication that the distance matches any previously added connections distance.
	return itrConnection->second == nodeConnection.second;
}

NetworkNode::ClosestNode_t NetworkNode::getClosestVisitedConnection(const boost::dynamic_bitset<>& visited) const {
	NetworkNode::ClosestNode_t closetVisited = boost::none;
	NetworkNode::ConnectionsContainer_t::const_iterator end = getConnections().end(), itr = getConnections().begin();
	for (; itr != end; ++itr) {
		if (visited.test(itr->first->getTractIndex())) {
			if (!closetVisited)
				closetVisited = *itr;
			else if (itr->second < closetVisited->second || (itr->second == closetVisited->second && itr->first->getTractIndex() < closetVisited->first->getTractIndex()))
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
		std::pair<NodesContainer_t::iterator, bool> parentNode = _nodes_in_tree.insert(std::make_pair(closedNode->first->getTractIndex(), NetworkNode(closedNode->first->getTractIndex())));
		std::pair<NodesContainer_t::iterator, bool> connectionNode = _nodes_in_tree.insert(std::make_pair(networkNode.getTractIndex(), NetworkNode(networkNode.getTractIndex())));
		parentNode.first->second.add_connection(&(connectionNode.first->second), distance);
	}
}

////////////////////////// Network /////////////////////////////////////////////////

bool Network::addConnection(tract_t t1, tract_t t2, double distance) {
	NetworkContainer_t::iterator iterT1 = _nodes.emplace(std::make_pair(t1, NetworkNode(t1))).first;
	NetworkContainer_t::iterator iterT2 = _nodes.emplace(std::make_pair(t2, NetworkNode(t2))).first;
	return iterT1->second.add_connection(&(iterT2->second), distance);
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
		if (itrR->second < itrBestR->second || (itrR->second == itrBestR->second && itrR->first->getTractIndex() < itrBestR->first->getTractIndex())) {
			candidateNodes.push_back(*itrBestR);
			candidates.set(itrBestR->first->getTractIndex());
			candidate_length[itrBestR->first->getTractIndex()] = itrBestR->second;
			itrBestR = itrR;
		} else {
			candidateNodes.push_back(*itrR);
			candidates.set(itrR->first->getTractIndex());
			candidate_length[itrR->first->getTractIndex()] = itrR->second;
		}
	}
	// Add the node with shortest distance to return list, then add that nodes connections as candidates.
	locations.push_back(LocationDistance(itrBestR->first->getTractIndex(), itrBestR->second));
	visited.set(itrBestR->first->getTractIndex());
	if (pathTree) pathTree->add_connection(*itrBestR->first, itrBestR->second, visited);
	if (limitTo && limitTo.get() == locations.size())
		return locations;
	itrR = itrBestR->first->getConnections().begin(), endR = itrBestR->first->getConnections().end();
	for (; itrR != endR; ++itrR) {
		if (!visited.test(itrR->first->getTractIndex()) && !candidates.test(itrR->first->getTractIndex())) {
			candidateNodes.push_back(*itrR);
			candidates.set(itrR->first->getTractIndex());
			candidate_length[itrR->first->getTractIndex()] = itrBestR->second + itrR->second;
		}
	}
	// At this point the candidates will be all nodes off the starting node plus direct connections from that best node -- excluding the best node itself.
    // Now we process the candidates until we've exhauted the reachable network.
	double lengthBest, lengthCandidate;
	while (candidateNodes.size()) {
		// Search for the node with the shortest distance among all current candidates.
        NetworkNode::ConnectionsContainer_t::iterator itrBestC = candidateNodes.begin(), endC = candidateNodes.end(), itrC = candidateNodes.begin();
		for (++itrC; itrC != endC; ++itrC) {
			lengthCandidate = candidate_length[itrC->first->getTractIndex()];
			lengthBest = candidate_length[itrBestC->first->getTractIndex()];
			if (lengthCandidate < lengthBest || (lengthCandidate == lengthBest && itrC->first->getTractIndex() < itrBestC->first->getTractIndex()))
				itrBestC = itrC;
		}
		// At this point itrBest references the best candidate, so add it to the locations list along with it's distance from node center.
		double lengthBest = candidate_length[itrBestC->first->getTractIndex()];
		locations.push_back(LocationDistance(itrBestC->first->getTractIndex(), lengthBest));
		// Add all connections from itrBest as candidates as well - excluding any nodes already visited.
        NetworkNode::ConnectionsContainer_t::const_iterator itrB = itrBestC->first->getConnections().begin(), endB = itrBestC->first->getConnections().end();
		// But first set this node as visited and unset/remove from helper structures.
		visited.set(itrBestC->first->getTractIndex());
		if (pathTree) pathTree->add_connection(*itrBestC->first, itrBestC->second, visited);
		if (limitTo && limitTo.get() == locations.size())
			return locations;
		candidates.reset(itrBestC->first->getTractIndex());
		candidate_length.erase(itrBestC->first->getTractIndex());
		// Now remove current best node from candidates.
		candidateNodes.erase(itrBestC);
		// Add direct connections - being sure to include it's distance from node center.
		for (; itrB != endB; ++itrB) {
			if (!visited.test(itrB->first->getTractIndex()) && !candidates.test(itrB->first->getTractIndex())) {
				candidateNodes.push_back(*itrB);
				candidates.set(itrB->first->getTractIndex());
				candidate_length[itrB->first->getTractIndex()] = lengthBest + itrB->second;
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
			if (clusterTracts.find(itrC->first->getTractIndex()) != clusterTracts.end()) {
				connections.emplace(Connection_Detail_t(std::min(*itr, itrC->first->getTractIndex()), std::max(*itr, itrC->first->getTractIndex()), itrC->second));
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
		write << tracts.getIdentifier(node.getTractIndex()) << " ==> " << tracts.getIdentifier(itr->first->getTractIndex()) << std::endl;
		//std::cout << tracts.getIdentifier(node.getTractIndex()) << " ==> " << tracts.getIdentifier(itr->first->getTractIndex()) << std::endl;
		fprintf(AppToolkit::getToolkit().openDebugFile(), "%s", write.str().c_str());
	}
	end = node.getConnections().end(), itr = node.getConnections().begin();
	for (; itr != end; ++itr) {
		printTreePath(*itr->first, tracts);
	}
}

void Network::printTreePath(const TractHandler& tracts, const CCluster& cluster) const {
	const NetworkNode& clusterRoot = _nodes.at(cluster.GetCentroidIndex());
	LocationContainer_t locationPath;
	NetworkPathTree networkPathTree(clusterRoot);
	buildNeighborsAboutNode(clusterRoot, locationPath, &networkPathTree, static_cast<size_t>(cluster.GetNumTractsInCluster()));
	printTreePath(networkPathTree, tracts);
}
