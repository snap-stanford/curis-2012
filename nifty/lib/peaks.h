#ifndef peaks_h
#define peaks_h

#include "stdafx.h"
#include "quote.h"
#include "doc.h"

typedef TTriple<TInt,TFlt,TSecTm> TFreqTriple;
typedef TVec<TFreqTriple> TFreqTripleV;

class Peaks {
private:
  static const int K;
  static const uint NumSecondsInHour;
  static const uint NumSecondsInDay;
  static const uint NumSecondsInWeek;

  static void GetPeaksEquationFunction(TFreqTripleV& FreqV, TFltV& FreqFltV);
  static void GetFrequencyVector(TDocBase *DocBase, TIntV& Sources, TFreqTripleV& FreqV, TInt BucketSize, TInt SlidingWindowSize);
  static TFlt CalcWindowAvg(TIntV& FreqV, TInt SlidingWindowSize);
public:
  static void GetPeaks(TDocBase *DocBase, TIntV& Sources, TFreqTripleV& PeakTimesV, TInt BucketSize, TInt SlidingWindowSize);
  static void GetPeaks(TDocBase *DocBase, TIntV& Sources, TFreqTripleV& PeakTimesV, TFreqTripleV& FreqV, TInt BucketSize, TInt SlidingWindowSize);
  //void PlotClusterPeaks(TTriple<TInt, TInt, TIntV>& Cluster);
};

#endif
