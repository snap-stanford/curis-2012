#include "stdafx.h"

const TStr SAVED_QBDBGRAPH_FILE = "QBDBGraph_foredgedel.bin";
const TStr OUTPUT_DIRECTORY = "./deletededges_results";
const TSecTm END_DATE(2012, 1, 14);

int main(int argc, char *argv[]) {
  LogOutput Log;
  Log.SetupNewOutputDirectory(OUTPUT_DIRECTORY);

  THash<TStr, TStr> Arguments;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log);
  // default: compare partitioning methods
  bool CompareEdgeScores = Arguments.IsKey("edgescore");

  if (CompareEdgeScores) {
    fprintf(stderr, "Comparing edge scores!\n");
  }

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


  if (!CompareEdgeScores) {
    fprintf(stderr, "Calculating and logging percent edges deleted for different partitioning methods\n");
    TEdgesDel::ComparePartitioningMethods(&QB, &DB, QGraph, Log);
  } else {
    fprintf(stderr, "Calculating and logging percent edges deleted for different edge scores\n");
    TEdgesDel::CompareEdgeScores(&QB, &DB, QGraph, Log);
  }

  TSecTm EndDate(END_DATE);
  Log.WriteClusteringStatisticsToFile(EndDate);

  TStr Directory;
  Log.GetDirectory(Directory);
  Err("Done with analyzing percent edges deleted! Directory created at: %s\n", Directory.CStr());
}
