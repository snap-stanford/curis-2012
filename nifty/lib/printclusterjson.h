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
  bool IsLeapYear(TInt Year);
  TSecTm RoundStartDate(TSecTm& StartDate, TStr& Type);
  TSecTm CalculateEndPeriodDate(TSecTm& CurrentDate, TStr& Type);

public:
  TPrintClusterJson();
  TPrintClusterJson(TStr& OutputJsonDir);
  void PrintClusterJsonForPeriod(TStr& StartString, TStr& EndString, LogOutput& Log, TStr& Type, TStr QBDBCDirectory = TStr("/lfs/1/tmp/curis/QBDBC/"));
};

#endif
