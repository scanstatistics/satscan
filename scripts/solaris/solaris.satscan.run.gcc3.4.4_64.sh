#!/bin/sh

#temporarily set enviroment variables to point to gcc3.4.4 64-bit libraries
env -i LD_LIBRARY_PATH=/usr/openwin/lib:/opt/net/sybase/as11.9.2/lib:/opt/net/utils/gcc3.4.4/lib/sparcv9:/opt/net/utils/lib:/opt/net/pgsql/production/lib $1 $2 $3 $4
