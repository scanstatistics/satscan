#ifndef __FIXTURE_SAMPLEDATA_H
#define __FIXTURE_SAMPLEDATA_H

// project files
#include "fixture_prm_source.h"

struct new_mexico_fixture : prm_sampledata_fixture {
    new_mexico_fixture() : prm_sampledata_fixture("NewMexicoSpaceTimePoisson.prm") { }
    virtual ~new_mexico_fixture() { }
};
struct nhumberside_fixture : prm_sampledata_fixture {
    nhumberside_fixture() : prm_sampledata_fixture("NHumbersideSpatialBernoulli.prm") { }
    virtual ~nhumberside_fixture() { }
};
struct nycfever_fixture : prm_sampledata_fixture {
    nycfever_fixture() : prm_sampledata_fixture("NYCSpaceTimePermutation.prm") { }
    virtual ~nycfever_fixture() { }
};
struct normalfake_fixture : prm_sampledata_fixture {
    normalfake_fixture() : prm_sampledata_fixture("FakeSpatialNormal.prm") { }
    virtual ~normalfake_fixture() { }
};
struct survivalfake_fixture : prm_sampledata_fixture {
    survivalfake_fixture() : prm_sampledata_fixture("FakeSpaceTimeExponential.prm") { }
    virtual ~survivalfake_fixture() { }
};
struct marylandeducation_fixture : prm_sampledata_fixture {
    marylandeducation_fixture() : prm_sampledata_fixture("MarylandSpatialOrdinal.prm") { }
    virtual ~marylandeducation_fixture() { }
};
struct continuouspoissonfake_fixture : prm_sampledata_fixture {
    continuouspoissonfake_fixture() : prm_sampledata_fixture("PolygonSpatialContinuousPoisson.prm") { }
    virtual ~continuouspoissonfake_fixture() { }
};
#endif
