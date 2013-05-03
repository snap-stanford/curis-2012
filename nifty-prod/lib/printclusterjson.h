#ifndef printclusterjson_h
#define printclusterjson_h

#include "stdafx.h"
#include "quote.h"
#include "doc.h"
#include "cluster.h"
#include "printjson.h"
#include "dataloader.h"
#include "postcluster.h"

class TPrintClusterJson {
private:
  static const int NumDaysInMonth[];
  int NumTopClustersPerDay;
  TStr OutputJsonDir;

  void PrintFreqOverTime(THash<TSecTm, TFltV>& FreqOverTime);
  void UpdateDataForJsonPrinting(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB, THash<TSecTm, TFltV>& FreqOverTime, TVec<TSecTm>& Times, TIntV& ClustersToPrint, THash<TInt, TStr>& ClustersRepQuote, TSecTm& CurrentDate, TInt DaysPassed, TStr& Type);
  void PrintClustersInJson(THash<TSecTm, TFltV>& FreqOverTime, TVec<TSecTm>& Times, TIntV& ClustersToPrint, THash<TInt, TStr>& ClustersRepQuote, TStr& OutputFilename);
  void FilterDuplicateClusters(TQuoteBase *QBCumulative, TClusterBase *CBCumulative, TIntV& TopFilteredClusters, TIntV& TopFilteredClustersWoDups);
  static bool IsLeapYear(TInt Year);
  static TInt GetNumDaysInMonth(TSecTm& Date);

public:
  static TSecTm RoundStartDate(TSecTm& StartDate, TStr& Type);
  static TSecTm CalculateEndPeriodDate(TSecTm& CurrentDate, TStr& Type);

  TPrintClusterJson();
  TPrintClusterJson(TStr& OutputJsonDir);
  void PrintClusterJSONForPeriod(TStr& CurTimeString, TStr Type, TStr QBDBCDirectory);
  void PrintClusterJsonForPeriod(TStr& StartString, TStr& EndString, TStr& LogDirectory, TStr Type, TStr QBDBCDirectory);
};

#endif
