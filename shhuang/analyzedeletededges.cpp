#include "stdafx.h"

const TStr SAVED_QBDBGRAPH_FILE = "QBDBGraph_foredgedel.bin";
const TStr OUTPUT_DIRECTORY = "./deletededges_results";
const TSecTm END_DATE(2012, 1, 14);

int main(int argc, char *argv[]) {
  // Load saved QB, DB, and graph (before edges are deleted) from file
  TFIn SavedQBDBG(SAVED_QBDBGRAPH_FILE);
  TQuoteBase QB;
  TDocBase DB;
  PNGraph QGraph;
  fprintf(stderr, "Loading QB\n");
  QB.Load(SavedQBDBG);
  fprintf(stderr, "Loading DB\n");
  DB.Load(SavedQBDBG);
  fprintf(stderr, "Loading Graph\n");
  QGraph = TNGraph::Load(SavedQBDBG);

  LogOutput Log;
  Log.SetupNewOutputDirectory(OUTPUT_DIRECTORY);

  fprintf(stderr, "Calculating and logging percent edges deleted for different edge scores\n");
  TEdgesDel::CalcAndLogPercentEdgesDel(&QB, &DB, QGraph, Log);

  TSecTm EndDate(END_DATE);
  Log.WriteClusteringStatisticsToFile(EndDate);

  TStr Directory;
  Log.GetDirectory(Directory);
  Err("Done with analyzing percent edges deleted! Directory created at: %s\n", Directory.CStr());
}
