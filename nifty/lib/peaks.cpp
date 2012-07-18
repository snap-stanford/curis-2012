#include "stdafx.h"
#include "peaks.h"
#include "quote.h"
#include "doc.h"

const int Peaks::K = 5;
const uint Peaks::NumSecondsInHour = 3600;
const uint Peaks::NumSecondsInWeek = 604800;

void Peaks::GetPeaks(TDocBase *DocBase, TIntV& Sources, TFreqTripleV& PeakTimesV, TInt BucketSize, TInt SlidingWindowSize) {
  TFreqTripleV FreqV;
  Peaks::GetPeaks(DocBase, Sources, PeakTimesV, FreqV, BucketSize, SlidingWindowSize);
}

void Peaks::GetPeaks(TDocBase *DocBase, TIntV& Sources, TFreqTripleV& PeakTimesV, TFreqTripleV& FreqV, TInt BucketSize, TInt SlidingWindowSize) {
  if (Sources.Len() == 0) return;

  Peaks::GetFrequencyVector(DocBase, Sources, FreqV, BucketSize, SlidingWindowSize);

  TFltV FreqFltV;
  Peaks::GetPeaksEquationFunction(FreqV, FreqFltV);

  // calculate statistics
  TMom M(FreqFltV);
  TFlt FreqMean = TFlt(M.GetMean());
  TFlt FreqStdDev = TFlt(M.GetSDev());

  // finds peak where if there is sequence with all > stdev, we assume all is one peak
  bool IsPeaking = false;
  int CurMax;
  for (int i = 0; i < FreqFltV.Len(); ++i) {
    if (FreqFltV[i] > FreqMean + FreqStdDev) {
      if (IsPeaking) {
        if (FreqV[i].Val2 > FreqV[CurMax].Val2) {
          CurMax = i;
        }
      } else {
        IsPeaking = true;
        CurMax = i;
      }
    } else if (IsPeaking) {
      IsPeaking = false;
      PeakTimesV.Add(FreqV[CurMax]);
    }
  }
  // final check so we don't miss the last peak
  if (IsPeaking) {
    PeakTimesV.Add(FreqV[CurMax]);
  }

  // If no peak satisfies the definition, then the "peak" time is
  // the hour with the highest quote frequency
  /*if (PeakTimesV.Len() == 0) {
    TFlt FreqMax = TFlt(FreqV[0].Val2);
    TInt FreqMaxIndex = 0;
    for (int i = 0; i < FreqV.Len(); ++i) {
      if (TFlt(FreqV[i].Val2) >= FreqMax) {
        FreqMax = TFlt(FreqV[i].Val2);;
        FreqMaxIndex = i;
      }
    }
    PeakTimesV.Add(HourOffsets[FreqMaxIndex]);
  }*/
}

void Peaks::GetPeaksEquationFunction(TFreqTripleV& FreqV, TFltV& FreqFltV) {
  for (int i = 0; i < FreqV.Len(); ++i) {
    // FreqFltV.Add(TFlt(FreqV[i].Val2));
    TFlt AvgDist = 0;
    TInt Count = 0;
    for (int j = i - K; j < i + K && j < FreqV.Len(); j++) {
      if (j >= 0 && j != i) {
        AvgDist += FreqV[i].Val2 - FreqV[j].Val2;
        Count++;
      }
    }
    FreqFltV.Add(AvgDist/Count);
  }
}

/* Sources = Sources that we want to calculate the distribution for.
 * FreqV = (X, Y, time) triple vector.
 * BucketSize = number of hours that will be lumped together
 * SlidingWindowSize = number of hours that a sliding window will occur.
 */
void Peaks::GetFrequencyVector(TDocBase *DocBase, TIntV& Sources, TFreqTripleV& FreqV, TInt BucketSize, TInt SlidingWindowSize) {
  // Sort the given document sources by time (ascending).
  TIntV SourcesSorted(Sources);
  SourcesSorted.SortCmp(TCmpDocByDate(true, DocBase));

  TDoc StartDoc;
  int StartDocIndex = 0;
  DocBase->GetDoc(SourcesSorted[StartDocIndex], StartDoc);
  TUInt StartTime = TUInt(StartDoc.GetDate().GetAbsSecs());

  TInt Frequency = 1;
  TInt HourNum = 0;
  uint BucketSizeSecs = NumSecondsInHour * BucketSize.Val;
  TIntV RawFrequencyCounts;

  for (int i = 0; i < SourcesSorted.Len(); ++i) {
    TDoc CurDoc;
    if (DocBase->GetDoc(SourcesSorted[i], CurDoc)) {
      TUInt CurTime = TUInt(CurDoc.GetDate().GetAbsSecs());
      if (CurTime - StartTime < BucketSizeSecs) { // still the same bucket? keep on incrementing.
        Frequency++;
      } else {
        RawFrequencyCounts.Add(Frequency);
        FreqV.Add(TFreqTriple(HourNum * BucketSize, CalcWindowAvg(RawFrequencyCounts, SlidingWindowSize), TSecTm(StartTime)));
        TInt NumHoursAhead = (CurTime - StartTime) / BucketSizeSecs;
        //printf("PrevDoc Date: %s, CurrDoc Date: %s, NumHoursAhead: %d\n", PrevDoc.GetDate().GetYmdTmStr().GetCStr(), CurrDoc.GetDate().GetYmdTmStr().GetCStr(), NumHoursAhead.Val);
        // Add frequencies of 0 if there are hours in between the two occurrences
        for (int j = 1; j < NumHoursAhead; ++j) {
          TUInt NewStartTime = StartTime + j * BucketSizeSecs;
          RawFrequencyCounts.Add(TInt(0));
          FreqV.Add(TFreqTriple((HourNum + j) * BucketSize, CalcWindowAvg(RawFrequencyCounts, SlidingWindowSize), TSecTm(NewStartTime)));
        }
        HourNum += NumHoursAhead;
        Frequency = 1;
        StartTime = StartTime + NumHoursAhead * BucketSizeSecs;
      }
    }
  }

  RawFrequencyCounts.Add(TInt(0));
  FreqV.Add(TFreqTriple(HourNum * BucketSize, CalcWindowAvg(RawFrequencyCounts, SlidingWindowSize), TSecTm(StartTime)));
}

TFlt Peaks::CalcWindowAvg(TIntV& FreqV, TInt SlidingWindowSize) {
  TInt StartIndex = FreqV.Len() - SlidingWindowSize;
  if (StartIndex < 0) {
    StartIndex = 0;
  }
  TFlt Avg = 0;
  for (int i = StartIndex; i < FreqV.Len(); i++) {
    Avg += FreqV[i];
  }
  Avg /= (FreqV.Len() - StartIndex);
  return Avg;
}
