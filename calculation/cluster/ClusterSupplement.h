//*****************************************************************************
#ifndef __CLUSTERSUPPLEMENT_H
#define __CLUSTERSUPPLEMENT_H
//*****************************************************************************
#include "SSException.h"
#include <boost/dynamic_bitset.hpp>

class CCluster;

class ClusterSupplementInfo {
private:
	class Info {
	private:
		unsigned int _report_number;
		boost::dynamic_bitset<> _overlapping;
	public:
		Info(unsigned int i) : _report_number(i) {}
		Info(unsigned int i, const boost::dynamic_bitset<> & bitset) : _report_number(i), _overlapping(bitset) {}
		const boost::dynamic_bitset<> & get_overlapping() const {return _overlapping;}
		unsigned int get_report_number() const {return _report_number;}
		void set_overlapping(const boost::dynamic_bitset<> & set) {_overlapping=set;}
	};
	typedef std::map<const CCluster*, Info> container_t;
	container_t _cluster_info;

	const Info& get_info(const CCluster& cluster) const {
		container_t::const_iterator itr = _cluster_info.find(&cluster);
		if (itr == _cluster_info.end())	throw prg_error("Unmapped cluster","get_info() const");
		return itr->second;
	}
	Info& get_info(const CCluster& cluster) {
		container_t::iterator itr = _cluster_info.find(&cluster);
		if (itr == _cluster_info.end()) throw prg_error("Unmapped cluster","get_info()");
		return itr->second;
	}

public:
	ClusterSupplementInfo() {}

	void addCluster(const CCluster& cluster, unsigned int rpt_idx) {
		_cluster_info.insert(std::make_pair(&cluster, Info(rpt_idx)));
	}
	void addCluster(const CCluster& cluster, unsigned int rpt_idx, const boost::dynamic_bitset<> & overlapping) {
		_cluster_info.insert(std::make_pair(&cluster, Info(rpt_idx, overlapping)));
	}
	unsigned int getClusterReportIndex(const CCluster& cluster) const {
		return get_info(cluster).get_report_number();
	}
	size_t size() const {return _cluster_info.size();}
	std::string& getOverlappingClusters(const CCluster& cluster, std::string& buffer) const {
		std::stringstream worker;
		const boost::dynamic_bitset<>& set = get_info(cluster).get_overlapping();
	    for (size_t clusteridx=0; clusteridx < set.size(); ++clusteridx) {
			if (set.test(clusteridx))
				worker << (worker.str().size() == 0 ? "" : ", ") << clusteridx + 1;
	    }
		buffer = worker.str();
		return buffer;
	}
	void setOverlappingClusters(const CCluster& cluster, const boost::dynamic_bitset<> & overlapping) {
		get_info(cluster).set_overlapping(overlapping);
	}
	bool test(const CCluster& cluster) const {
		container_t::const_iterator itr = _cluster_info.find(&cluster);
		return itr != _cluster_info.end();
	}
};
//*****************************************************************************
#endif
