// Loglikelihood.cpp

#include "loglikelihood.h"

double CalcLogLikelihood(count_t n, measure_t u,
                         count_t N, measure_t U)
{
   double nLogLikelihood;

   try
      {
      if (n != N && n != 0)
         nLogLikelihood = n*log(n/u) + (N-n)*log((N-n)/(U-u));
      else if (n == 0)
         nLogLikelihood = (N-n) * log((N-n)/(U-u));
      else
         nLogLikelihood = n*log(n/u);
      }
   catch (SSException & x)
      {
      x.AddCallpath("CalcLogLikelihood()", "Loglikelihood.cpp");
      throw;
      }
   return (nLogLikelihood);
}

