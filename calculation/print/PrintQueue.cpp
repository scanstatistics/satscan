#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "PrintQueue.h"

//for _sleep():
#if defined(__BORLANDC__)
#include <dos.h>
#else
#include <unistd.h>
#endif

//ClassDesc PrintQueue
// A PrintQueue is a BasePrint which operates on another BasePrint object (its
// "target").  The PrintQueue holds up to GetThreshold() printlines in a queue,
// forwarding them to "target".
// A print queue's threshold value can be "automated" by assigning a "threshold
// policy" to it.  The 'threshold_policy_i' contains one major function:
// GetRecommendedThresholdPolicy, which passes a "current threshold" and a
// "current size".  Every member function of PrintQueue calls this function
// and sets the threshold with the result (except for threshold manipulation
// functions).
// The function, SetThreshold, disregards the threshold policy.  If the new
// threshold is lower than the current threshold, lines are forwarded to target'
// until just 'new_threshold' lines remain in the queue.
//
//ClassDesc End PrintQueue


//constructor
//ensure
//  is_released : GetThreshold() == 0
PrintQueue::PrintQueue(BasePrint & Target)
 : BasePrint()
 , gTarget(Target)
 , gpThresholdPolicy(new default_threshold_policy())
 , glThreshold(0)
{
   try
   {
      SetThreshold(gpThresholdPolicy->GetRecommendedThresholdValue_OnConstruction());
   }
   catch (ZdException & e)
   {
      e.AddCallpath("constructor(BasePrint&)", "PrintQueue");
      throw;
   }
}

//constructor
//ensure
//  threshold_set : GetThreshold() == lThreshold
PrintQueue::PrintQueue(BasePrint & Target, threshold_policy_i const & ThresholdPolicy)
 : gTarget(Target)
 , gpThresholdPolicy(ThresholdPolicy.Clone())
 , glThreshold(0)
{
   try
   {
      SetThreshold(gpThresholdPolicy->GetRecommendedThresholdValue_OnConstruction());
   }
   catch (ZdException & e)
   {
      e.AddCallpath("constructor(BasePrint&, threshold_policy_i const &)", "PrintQueue");
      throw;
   }
}

//destructor
//Dump all queued lines to gTarget via Release().
PrintQueue::~PrintQueue()
{
   try
   {
      SetThreshold(gpThresholdPolicy->GetRecommendedThresholdValue_OnDestruction(glThreshold, gOutputLines.size()));
   }
   catch (ZdException & e)
   {
      e.AddCallpath("destructor", "PrintQueue");
      //log that an exception was thrown.
      //do not rethrow exception from this destructor.
   }
   catch (...)
   {
      //log that an exception was thrown.
      //do not rethrow exception from this destructor.
   }
}

//Set the threshold to 'lNewThreshold'.  If 'lNewThreshold' < GetThreshold() then
//forward lines to 'target' until 'lNewThreshold' lines remain queued.
void PrintQueue::SetThreshold(long lNewThreshold)
{
   try
   {
      if (lNewThreshold < 0)//make threshold "virtually infinite"
        lNewThreshold = std::numeric_limits<long>::max();
        
      while (gOutputLines.size() > lNewThreshold)
      {
         PrintWarningQualifiedLineToTarget(gOutputLines.front().first, gOutputLines.front().second.c_str());
         gOutputLines.pop_front();
      }
      glThreshold = lNewThreshold;
   }
   catch (ZdException & e)
   {
      e.AddCallpath("SetThreshold(long)", "PrintQueue");
      throw;
   }
}

//Print a (non-warning) line.
void PrintQueue::PrintLine(char *s)
{
   try
   {
      UpdateThreshold();
      PrintWarningQualifiedLine(false, s);
   }
   catch (ZdException & e)
   {
      e.AddCallpath("PrintLine(char *)", "PrintQueue");
      throw;
   }
}

//Print a "warning" line.
void PrintQueue::PrintWarningLine(char *s)
{
   try
   {
      UpdateThreshold();
      PrintWarningQualifiedLine(true, s);
   }
   catch (ZdException & e)
   {
      e.AddCallpath("PrintWarningLine(char *)", "PrintQueue");
      throw;
   }
}

//Print a line.  'bIsWarning' indicates whether or not 's' is a "warning" line.
void PrintQueue::PrintWarningQualifiedLine(bool bIsWarning, const char * s)
{
   std::pair<bool, std::string> arg_line(bIsWarning, std::string(s));
   try
   {
      if ((gOutputLines.size() < GetThreshold()) || (GetThreshold() < 0))
      {
         gOutputLines.push_back(arg_line);
      }
      else
      {
         if (gOutputLines.size() > 0)
         {
            std::pair<bool, std::string> dequeued_line(gOutputLines.front());
            gOutputLines.push_back(arg_line);
            try{ PrintWarningQualifiedLineToTarget(dequeued_line.first, dequeued_line.second.c_str());
            }catch(...){ gOutputLines.pop_back(); throw; }
            gOutputLines.pop_front();
         }
         else
         {
            PrintWarningQualifiedLineToTarget(arg_line.first, arg_line.second.c_str());
         }
      }
   }
   catch (ZdException & e)
   {
      e.AddCallpath("PrintWarningQualifiedLine(bool, const char *)", "PrintQueue");
      throw;
   }
}

//Send a line to gTarget. 'bIsWarning' indicates whether gTarget.PrintWarningLine(...)
// or gTarget.PrintLine(...) is called.
void PrintQueue::PrintWarningQualifiedLineToTarget(bool bIsWarning, const char * s)
{
   try
   {
      if (bIsWarning)
      {
         gTarget.PrintWarningLine(const_cast<char*>(s));
      }
      else
      {
      	 gTarget.PrintLine(const_cast<char*>(s));
      }
   }
   catch (ZdException & e)
   {
      e.AddCallpath("PrintWarningQualifiedLineToTarget(bool, const char *)", "PrintQueue");
      throw;
   }
}

//Set the threshold to the value recommended by the threshold policy.
void PrintQueue::UpdateThreshold()
{
   try
   {
      SetThreshold(gpThresholdPolicy->GetRecommendedThresholdValue(glThreshold, gOutputLines.size()));
   }
   catch (ZdException & e)
   {
      e.AddCallpath("UpdateThreshold()", "PrintQueue");
      throw;
   }
}



//ClassDesc Begin SatScanBatchThresholdPolicy
//This policy is:
// --until FinalizationTime is past, ThresholdValue is "infinite".
// --thereafter, ThresholdValue is 0.
//ClassDesc End

long TimedReleaseThresholdPolicy::GetRecommendedThresholdValue_OnConstruction()
{
   if (ZdTimestamp::Current(false) > gtsReleaseTime)
   {
      return 0;
   }
   else
   {
      return -1;
   }
}

long TimedReleaseThresholdPolicy::GetRecommendedThresholdValue(long lCurrentThreshold, long lCurrentSize)
{
   if (ZdTimestamp::Current(false) > gtsReleaseTime)
   {
      return 0;
   }
   else
   {
      return std::numeric_limits<long>::max();
   }
}

long TimedReleaseThresholdPolicy::GetRecommendedThresholdValue_OnDestruction(long lCurrentThreshold, long lCurrentSize)
{
   ZdTimestamp CurrentTime;
   CurrentTime.Now(false);
   if (CurrentTime < gtsReleaseTime)
   {
      unsigned long ulTimeDifference(gtsReleaseTime.GetTimeInMilliseconds() - CurrentTime.GetTimeInMilliseconds());
      //how many seconds to sleep?  round to the nearest:
      unsigned u((ulTimeDifference + 500) / 1000);
      //sleep...
      printf("sleeping %d seconds\n", u);
      #if defined(__BORLANDC__)
      _sleep(u);
      #else
      sleep(u);
      #endif
   }
   return 0;//whether or not we wait to return, we want the new threshold to be 0.
}



