//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "LocationNetwork.h"
#include "Toolkit.h"
#include "cluster.h"

////////////////////////// NetworkNode /////////////////////////////////////////////

NetworkNode::AddStatusType NetworkNode::add_connection(NetworkNode* pNetworkNode, double distance, bool recordDef) {
    if (getLocationIndex() == pNetworkNode->getLocationIndex())
        return NetworkNode::SelfReference;

    NodeDistance_t nodeConnection(pNetworkNode, distance, false, recordDef);
    ConnectionsContainer_t::iterator itrConnection = std::lower_bound(_connections.begin(), _connections.end(), nodeConnection, CompareNodeDistance());
    if (itrConnection == _connections.end() || itrConnection->get<0>()->getLocationIndex() != nodeConnection.get<0>()->getLocationIndex()) {
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
        if (visited.test(itr->get<0>()->getLocationIndex())) {
            if (!closetVisited)
                closetVisited = *itr;
            else if (itr->get<1>() < closetVisited->get<1>() || (itr->get<1>() == closetVisited->get<1>() && itr->get<0>()->getLocationIndex() < closetVisited->get<0>()->getLocationIndex()))
                closetVisited = *itr;
        }
    }
    return closetVisited;
}

//////////////////////////////////////// NetworkPathTree //////////////////////////////////////

NetworkPathTree::NetworkPathTree(const NetworkNode& root): _root(NetworkNode(root.getLocationIndex(), root.getLocation())) {
    _nodes_in_tree.insert(std::make_pair(root.getLocationIndex(), _root));
}

void NetworkPathTree::add_connection(const NetworkNode& networkNode, double distance, const boost::dynamic_bitset<>& visited) {
    NetworkNode::ClosestNode_t closedNode = networkNode.getClosestVisitedConnection(visited);
    if (closedNode) {
        std::pair<NodesContainer_t::iterator, bool> parentNode = _nodes_in_tree.insert(std::make_pair(closedNode->get<0>()->getLocationIndex(), NetworkNode(closedNode->get<0>()->getLocationIndex(), closedNode->get<0>()->getLocation())));
        std::pair<NodesContainer_t::iterator, bool> connectionNode = _nodes_in_tree.insert(std::make_pair(networkNode.getLocationIndex(), NetworkNode(networkNode.getLocationIndex(), networkNode.getLocation())));
        parentNode.first->second.add_connection(&(connectionNode.first->second), distance, false);
    }
}

////////////////////////// Network /////////////////////////////////////////////////

NetworkNode::AddStatusType Network::addConnection(unsigned int t1, const Location& location1, unsigned int t2, const Location& location2, double distance, bool recordDef) {
    NetworkContainer_t::iterator iterT1 = _nodes.emplace(std::make_pair(t1, NetworkNode(t1, location1))).first;
    NetworkContainer_t::iterator iterT2 = _nodes.emplace(std::make_pair(t2, NetworkNode(t2, location2))).first;
    return iterT1->second.add_connection(&(iterT2->second), distance, recordDef);
}

void Network::addNode(unsigned int t1, const Location& location) {
    _nodes.emplace(std::make_pair(t1, NetworkNode(t1, location)));
}

/** Build locations path from node, following it's connections, their connections, etc. */
NetworkLocationContainer_t& Network::buildNeighborsAboutNode(const NetworkNode& node, NetworkLocationContainer_t& networkLocations, unsigned int num_locations, NetworkPathTree * pathTree, LimitTo_t limitTo) const {
    // Add node as first element in locations list.
    networkLocations.push_back(std::make_pair(&node, 0));
    if (node.getConnections().size() == 0)
        return networkLocations; // This node is isolated from rest of network.

    if (limitTo && limitTo.get() == networkLocations.size())
        return networkLocations;

    NetworkNode::CandidatesContainer_t candidateNodes; // collection of nodes that are candidates as network expands
    std::map<tract_t, double> candidate_length; // maps tract index of candidate nodes to their distance from network center
    boost::dynamic_bitset<> visited(num_locations), candidates(num_locations);

    visited.set(node.getLocationIndex()); // Mark the node as visited.
    // Find node with shortest distance from starting node - all other connections from node become candidates.
    NetworkNode::ConnectionsContainer_t::const_iterator itrBestR=node.getConnections().begin(), endR=node.getConnections().end(), itrR=node.getConnections().begin();
    for (++itrR; itrR != endR; ++itrR) {
        if (itrR->get<1>() < itrBestR->get<1>() || (itrR->get<1>() == itrBestR->get<1>() && itrR->get<0>()->getLocationIndex() < itrBestR->get<0>()->getLocationIndex())) {
            candidateNodes.push_back(*itrBestR);
            candidates.set(itrBestR->get<0>()->getLocationIndex());
            candidate_length[itrBestR->get<0>()->getLocationIndex()] = itrBestR->get<1>();
            itrBestR = itrR;
        } else {
            candidateNodes.push_back(*itrR);
            candidates.set(itrR->get<0>()->getLocationIndex());
            candidate_length[itrR->get<0>()->getLocationIndex()] = itrR->get<1>();
        }
    }
    // Add the node with shortest distance to return list, then add that nodes connections as candidates.
    networkLocations.push_back(std::make_pair(itrBestR->get<0>(), itrBestR->get<1>()));
    visited.set(itrBestR->get<0>()->getLocationIndex());
    if (pathTree) pathTree->add_connection(*itrBestR->get<0>(), itrBestR->get<1>(), visited);
    if (limitTo && limitTo.get() == networkLocations.size())
        return networkLocations;
    itrR = itrBestR->get<0>()->getConnections().begin(), endR = itrBestR->get<0>()->getConnections().end();
    for (; itrR != endR; ++itrR) {
        if (!visited.test(itrR->get<0>()->getLocationIndex()) && !candidates.test(itrR->get<0>()->getLocationIndex())) {
            candidateNodes.push_back(*itrR);
            candidates.set(itrR->get<0>()->getLocationIndex());
            candidate_length[itrR->get<0>()->getLocationIndex()] = itrBestR->get<1>() + itrR->get<1>();
        }
    }
    // At this point the candidates will be all nodes off the starting node plus direct connections from that best node -- excluding the best node itself.
    // Now we process the candidates until we've exhauted the reachable network.
    double lengthBest, lengthCandidate;
    while (candidateNodes.size()) {
        // Search for the node with the shortest distance among all current candidates.
        NetworkNode::ConnectionsContainer_t::iterator itrBestC = candidateNodes.begin(), endC = candidateNodes.end(), itrC = candidateNodes.begin();
        for (++itrC; itrC != endC; ++itrC) {
            lengthCandidate = candidate_length[itrC->get<0>()->getLocationIndex()];
            lengthBest = candidate_length[itrBestC->get<0>()->getLocationIndex()];
            if (lengthCandidate < lengthBest || (lengthCandidate == lengthBest && itrC->get<0>()->getLocationIndex() < itrBestC->get<0>()->getLocationIndex()))
                itrBestC = itrC;
        }
        // At this point itrBest references the best candidate, so add it to the locations list along with it's distance from node center.
        lengthBest = candidate_length[itrBestC->get<0>()->getLocationIndex()];
        networkLocations.push_back(std::make_pair(itrBestC->get<0>(), lengthBest));
        // Add all connections from itrBest as candidates as well - excluding any nodes already visited.
        NetworkNode::ConnectionsContainer_t::const_iterator itrB = itrBestC->get<0>()->getConnections().begin(), endB = itrBestC->get<0>()->getConnections().end();
        // But first set this node as visited and unset/remove from helper structures.
        visited.set(itrBestC->get<0>()->getLocationIndex());
        if (pathTree) pathTree->add_connection(*itrBestC->get<0>(), itrBestC->get<1>(), visited);
        if (limitTo && limitTo.get() == networkLocations.size())
            return networkLocations;
        candidates.reset(itrBestC->get<0>()->getLocationIndex());
        candidate_length.erase(itrBestC->get<0>()->getLocationIndex());
        // Now remove current best node from candidates.
        candidateNodes.erase(itrBestC);
        // Add direct connections - being sure to include it's distance from node center.
        for (; itrB != endB; ++itrB) {
            if (!visited.test(itrB->get<0>()->getLocationIndex()) && !candidates.test(itrB->get<0>()->getLocationIndex())) {
                candidateNodes.push_back(*itrB);
                candidates.set(itrB->get<0>()->getLocationIndex());
                candidate_length[itrB->get<0>()->getLocationIndex()] = lengthBest + itrB->get<1>();
            }
        }
    }
    return networkLocations;
}

// Returns the network connections for this cluster.
Network::Connection_Details_t Network::getClusterConnections(const CCluster& cluster, const CSaTScanData& DataHub) const {
    Connection_Details_t connections;
    std::map<const Location *, unsigned int> _locations;
    std::vector<tract_t> clusterTracts;

    NetworkLocationContainer_t networkLocations;
    if (DataHub.GetParameters().GetMultipleCoordinatesType() != ONEPERLOCATION) {
        const Location& centroidLocation = dynamic_cast<const NetworkCentroidHandlerPassThrough*>(DataHub.GetGInfo())->getCentroidLocation(cluster.GetCentroidIndex());
        const NetworkNode& centroidNode = DataHub.refLocationNetwork().getNodes().find(DataHub.GetGroupInfo().getLocationsManager().getLocation(centroidLocation.name()).first.get())->second;
        buildNeighborsAboutNode(centroidNode, networkLocations, DataHub.GetGroupInfo().getLocationsManager().locations().size());
    }

    // Build the collection of atomic tract indexes for this cluster.
    cluster.getGroupIndexes(DataHub, clusterTracts, true);
    // Create a collection of locations that are within this cluster per the tracts/groups of this cluster.
    for (auto tTract: clusterTracts) {
        auto grouping = DataHub.GetGroupInfo().getObservationGroups()[tTract];
        LocationsManager::LocationIdx_t locationBest(boost::optional<unsigned int>(boost::none), 0);
        double dCurrent = -1;
        for (unsigned int i = 0; i < grouping->getLocations().size(); ++i) {
            double dDistance = -1;
            auto location = DataHub.GetGroupInfo().getLocationsManager().getLocation(grouping->getLocations()[i]->name());
            if (networkLocations.size()) {
                auto tractNode = &(DataHub.refLocationNetwork().getNodes().find(DataHub.GetGroupInfo().getLocationsManager().getLocation(location.second->name()).first.get())->second);
                double dDistance = -1;
                for (auto itr = networkLocations.begin(); itr != networkLocations.end(); ++itr) {
                    if (itr->first == tractNode) {
                        dDistance = itr->second;
                        break;
                    }
                }
                if (dDistance == -1) throw prg_error("Unable to determine distance between cluster centroid and '%s'.", "getClusterConnections()", location.second->name().c_str());
                //dDistance = DataHub.refLocationNetwork().getDistanceBetween(centroidLocation, *tractLocation, DataHub.GetGroupInfo());
                switch (DataHub.GetParameters().GetMultipleCoordinatesType()) {
                    case ATLEASTONELOCATION: if (locationBest.first == boost::none || dCurrent > dDistance) { locationBest = location; dCurrent = dDistance; } break; // Searching for the closest coordinate.
                    case ALLLOCATIONS: if (locationBest.first == boost::none || dCurrent < dDistance) { locationBest = location; dCurrent = dDistance; } break; //Searching for the farthest coordinate.
                    default: throw prg_error("Unknown multiple coordinates type '%d'.", "getClusterConnections()", DataHub.GetParameters().GetMultipleCoordinatesType());
                }
            } else
                _locations[location.second] = location.first.get();
        }
        if (locationBest.first != boost::none) {
            _locations[locationBest.second] = locationBest.first.get();
        }
    }

    /* Now that we know all of the locations in the cluster, we can construct the edges between them as defined by network. */
    for (auto itr = _locations.begin(); itr != _locations.end(); ++itr) {
        auto itrNode = _nodes.find(itr->second);
        if (itrNode != _nodes.end()) {
            const Location * nodeLocation = &itrNode->second.getLocation();
            // Define edge from this node to another connecting node if other is in cluster locations.
            for (auto itrC = itrNode->second.getConnections().begin(); itrC != itrNode->second.getConnections().end(); ++itrC) {
                const Location * connectionLocation = &(itrC->get<0>()->getLocation());
                if (_locations.find(connectionLocation) == _locations.end()) continue;
                connections.emplace(Connection_Detail_t(std::min(nodeLocation, connectionLocation), std::max(nodeLocation, connectionLocation)));
            }
        }
    }
    return connections;
}

double Network::getDistanceBetween(const Location& location1, const Location& location2, const ObservationGroupingManager& groups) const {
    if (location1.name() == location2.name()) return 0.0;

    auto location1Node = _nodes.find(groups.getLocationsManager().getLocation(location1.name()).first.get())->second;
    auto location2Node = &(_nodes.find(groups.getLocationsManager().getLocation(location2.name()).first.get())->second);
    NetworkLocationContainer_t networkLocations;
    buildNeighborsAboutNode(location1Node, networkLocations, groups.getLocationsManager().locations().size());
    for (auto itr = networkLocations.begin(); itr != networkLocations.end(); ++itr) {
        if (itr->first == location2Node)
            return itr->second;
    }
    throw prg_error("Unable to determine distance between locations '%s' and '%s'.", "getDistanceBetween()", location1.name().c_str(), location2.name().c_str());
}

void Network::printPath(const NetworkLocationContainer_t& nodePath, const ObservationGroupingManager& groups) {
    double length = 0;
    for (auto itr= nodePath.begin(); itr != nodePath.end(); ++itr) {
        length += itr->second;
        std::cout << itr->first->getLocation().name() << " distance " << itr->second << " path length " << length << std::endl;
    }
}

void Network::printTreePath(const NetworkPathTree& treePath, const ObservationGroupingManager& groups) {
    std::string buffer;
    fprintf(AppToolkit::getToolkit().openDebugFile(), "Node tree path : %s\n", groups.getGroupname(treePath.getRoot().getLocationIndex(), buffer).c_str());
    printTreePath(treePath.getRoot(), groups);
}

void Network::printTreePath(const NetworkNode& node, const ObservationGroupingManager& groups) {
    NetworkNode::ConnectionsContainer_t::const_iterator end = node.getConnections().end(), itr = node.getConnections().begin();
    for (; itr != end; ++itr) {
        std::stringstream write;
        write << groups.getLocationsManager().locations()[node.getLocationIndex()]->name() << " ==> " << groups.getLocationsManager().locations()[itr->get<0>()->getLocationIndex()]->name() << std::endl;
        fprintf(AppToolkit::getToolkit().openDebugFile(), "%s", write.str().c_str());
    }
    end = node.getConnections().end(), itr = node.getConnections().begin();
    for (; itr != end; ++itr) {
        printTreePath(*itr->get<0>(), groups);
    }
}

void Network::printTreePath(const ObservationGroupingManager& groups, const CCluster& cluster) const {
    const NetworkNode& clusterRoot = _nodes.at(cluster.GetCentroidIndex());
    NetworkLocationContainer_t locationPath;
    NetworkPathTree networkPathTree(clusterRoot);
    buildNeighborsAboutNode(clusterRoot, locationPath, groups.getLocationsManager().locations().size(), &networkPathTree, static_cast<size_t>(cluster.getNumObservationGroups()));
    printTreePath(networkPathTree, groups);
}
