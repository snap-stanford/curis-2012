#ifndef edgesdel_h
#define edgesdel_h

#include "stdafx.h"
#include "quote.h"
#include "doc.h"
#include "logoutput.h"

class TEdgesDel {
private:
  static const TStr PercentEdgesDeleted;
  static const TStr NumEdgesRemaining;
  static const TStr NumClusters;
  static const TStr PercentEdgesDeletedNFS;
  static const TStr PercentEdgeScoreDeletedNFS;
  static const TStr PercentEdgeScoreDeleted;
  static const TStr IncEdgeDelMaxEdgeScore;
  static const TStr IncEdgeDelMaxNumEdges;
  static void CalcAndLogOldBaseline(TQuoteBase *QB, TDocBase *DB, PNGraph& QGraph, LogOutput& Log);
  static void UpdatePercentages(TQuoteBase *QB, TDocBase *DB, const PNGraph& QGraph, const PNGraph& QGraphPartitioned,
                                TFlt& AvgPercentEdgeScoreDeletedNFS, TFlt& AvgPercentEdgeScoreDeleted, 
                                TFlt& AvgPercentEdgesDeletedNFS, TFlt& AvgPercentEdgesDeleted, LogOutput& Log, const TStr& ToPrepend);
  static void TryPartitioningMethod(TQuoteBase *QB, TDocBase *DB, const PNGraph& QGraph, LogOutput& Log, const TStr& ToPrepend,
                               TFlt (*Fn)(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator), TInt NumTrials = 1,
                               TRnd *RandomGenerator = NULL);
  static void TryEdgeScore(TQuoteBase *QB, TDocBase *DB, const PNGraph& QGraph, LogOutput& Log, const TStr& ToPrepend,
                             TFlt (*Fn)(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator), TInt NumTrials = 1,
                             TRnd *RandomGenerator = NULL);
  static TFlt CalcSumEdgeScoreOfGraph(TQuoteBase *QB, TDocBase *DB, const PNGraph& QGraph);
public:
  static void ComparePartitioningMethods(TQuoteBase *QB, TDocBase *DB, PNGraph& QGraph, LogOutput& Log);
  static void CompareEdgeScores(TQuoteBase *QB, TDocBase *DB, PNGraph& QGraph, LogOutput& Log);
};

#endif
