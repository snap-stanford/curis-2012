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
  static void CalcAndLogOldBaseline(TQuoteBase *QB, TDocBase *DB, PNGraph& QGraph, LogOutput& Log);
  static void CalcForEdgeScore(TQuoteBase *QB, TDocBase *DB, const PNGraph& QGraph, LogOutput& Log, TStr& ToPrepend,
                               TFlt (*Fn)(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator), TInt NumTrials = 1,
                               TRnd *RandomGenerator = NULL);
  static TFlt CalcSumEdgeScoreOfGraph(TQuoteBase *QB, TDocBase *DB, const PNGraph& QGraph);
public:
  static void CalcAndLogPercentEdgesDel(TQuoteBase *QB, TDocBase *DB, PNGraph& QGraph, LogOutput& Log);
};

#endif
