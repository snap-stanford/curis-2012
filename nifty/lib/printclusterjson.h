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

  static bool IsLeapYear(TInt Year);
  static TInt GetNumDaysInMonth(TSecTm& Date);
  static TInt QuarterDays(TInt NumDays, TInt Day);
  static TInt BucketDays(TSecTm& Day, TSecTm& StartDate, TStr Type);
  bool IsClusterDuplicate(TQuoteBase *QBCumulative, TClusterBase *CBCumulative, TStrSet& ExistingQuotes, TInt CandidateCluster);

public:
  static TSecTm RoundStartDate(TSecTm& StartDate, TStr& Type);
  static TSecTm CalculateEndPeriodDate(TSecTm& CurrentDate, TStr& Type);

  TPrintClusterJson();
  TPrintClusterJson(TStr& OutputJsonDir);
  void PrintClusterJsonForPeriod(TSecTm& StartDate, TSecTm& PresentTime, TStr& LogDirectory, TStr Type, TStr QBDBCDirectory);
};

#endif
