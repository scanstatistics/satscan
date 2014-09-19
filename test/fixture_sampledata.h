#ifndef __FIXTURE_SAMPLEDATA_H
#define __FIXTURE_SAMPLEDATA_H

// project files
#include "fixture_prm_source.h"

struct new_mexico_fixture : prm_fixture {
    new_mexico_fixture() : prm_fixture("nm.prm") { }
    virtual ~new_mexico_fixture() { }
};
struct nhumberside_fixture : prm_fixture {
    nhumberside_fixture() : prm_fixture("NHumberside.prm") { }
    virtual ~nhumberside_fixture() { }
};
struct nycfever_fixture : prm_fixture {
    nycfever_fixture() : prm_fixture("NYCfever.prm") { }
    virtual ~nycfever_fixture() { }
};
struct normalfake_fixture : prm_fixture {
    normalfake_fixture() : prm_fixture("NormalFake.prm") { }
    virtual ~normalfake_fixture() { }
};
struct survivalfake_fixture : prm_fixture {
    survivalfake_fixture() : prm_fixture("SurvivalFake.prm") { }
    virtual ~survivalfake_fixture() { }
};
struct marylandeducation_fixture : prm_fixture {
    marylandeducation_fixture() : prm_fixture("MarylandEducation.prm") { }
    virtual ~marylandeducation_fixture() { }
};
struct continuouspoissonfake_fixture : prm_fixture {
    continuouspoissonfake_fixture() : prm_fixture("ContinuousPoissonFake.prm") { }
    virtual ~continuouspoissonfake_fixture() { }
};
#endif
