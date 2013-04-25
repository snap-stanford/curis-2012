#ifndef peaks_h
#define peaks_h

#include "stdafx.h"
#include "quote.h"
#include "doc.h"

typedef TTriple<TInt,TFlt,TSecTm> TFreqTriple;
typedef TVec<TFreqTriple> TFreqTripleV;
typedef TPair<TSecTm,TInt> TDateFreq;

class Peaks {
private:
  static const int K;

  static void GetPeaksEquationFunction(TFreqTripleV& FreqV, TFltV& FreqFltV);
  static TFlt CalcWindowAvg(TIntV& FreqV, TInt SlidingWindowSize);
public:
  static const uint NumSecondsInHour;
  static const uint NumSecondsInDay;
  static const uint NumSecondsInWeek;
  static const int NumHoursInDay;
  static const int NumDaysToGraph;
  static void GetPeaks(TDocBase *DocBase, TIntV& Sources, TFreqTripleV& PeakTimesV, TInt BucketSize, TInt SlidingWindowSize, TSecTm PresentTime);
  static void GetPeaks(TDocBase *DocBase, TIntV& Sources, TFreqTripleV& PeakTimesV, TFreqTripleV& FreqV, TInt BucketSize, TInt SlidingWindowSize, TSecTm PresentTime);
  static void GetFrequencyVector(TDocBase *DocBase, TIntV& Sources, TFreqTripleV& FreqV, TInt BucketSize, TInt SlidingWindowSize, TSecTm PresentTime, TInt NumDays = NumDaysToGraph);
  //void PlotClusterPeaks(TTriple<TInt, TInt, TIntV>& Cluster);
};

#endif
