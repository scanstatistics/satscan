#ifndef __FIXTURE_SAMPLEDATA_H
#define __FIXTURE_SAMPLEDATA_H

// project files
#include "fixture_prm_source.h"

struct new_mexico_fixture : prm_sampledata_fixture {
    new_mexico_fixture() : prm_sampledata_fixture("nm.prm") { }
    virtual ~new_mexico_fixture() { }
};
struct nhumberside_fixture : prm_sampledata_fixture {
    nhumberside_fixture() : prm_sampledata_fixture("NHumberside.prm") { }
    virtual ~nhumberside_fixture() { }
};
struct nycfever_fixture : prm_sampledata_fixture {
    nycfever_fixture() : prm_sampledata_fixture("NYCfever.prm") { }
    virtual ~nycfever_fixture() { }
};
struct normalfake_fixture : prm_sampledata_fixture {
    normalfake_fixture() : prm_sampledata_fixture("NormalFake.prm") { }
    virtual ~normalfake_fixture() { }
};
struct survivalfake_fixture : prm_sampledata_fixture {
    survivalfake_fixture() : prm_sampledata_fixture("SurvivalFake.prm") { }
    virtual ~survivalfake_fixture() { }
};
struct marylandeducation_fixture : prm_sampledata_fixture {
    marylandeducation_fixture() : prm_sampledata_fixture("MarylandEducation.prm") { }
    virtual ~marylandeducation_fixture() { }
};
struct continuouspoissonfake_fixture : prm_sampledata_fixture {
    continuouspoissonfake_fixture() : prm_sampledata_fixture("ContinuousPoissonFake.prm") { }
    virtual ~continuouspoissonfake_fixture() { }
};
#endif
