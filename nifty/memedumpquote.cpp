#include "stdafx.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  // #### SETUP: Parse Arguments
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log);

  TStr OutputDirectory;
  TStr StartString = ArgumentParser::GetArgument(Arguments, "start", "2009-02-01");
  TStr QBDBDirectory = ArgumentParser::GetArgument(Arguments, "qbdb", QBDB_DIR_DEFAULT);
  TStr OutDirectory = ArgumentParser::GetArgument(Arguments, "out", "/lfs/1/tmp/curis/");
  TInt WindowSize = ArgumentParser::GetArgument(Arguments, "window", "14").GetInt();

  if (ArgumentParser::GetArgument(Arguments, "nolog", "") == "") {
    Log.DisableLogging();
  } else if (!Arguments.IsKeyGetDat("directory", OutputDirectory)) {
    Log.SetupNewOutputDirectory("");
  } else {
    Log.SetDirectory(OutputDirectory);
  }

  // #### DATA LOADING: Load ALL the things!
  TQuoteBase QB;
  TDocBase DB;
  fprintf(stderr, "Loading QB and DB from file for %d days, starting from %s...\n", WindowSize.Val, StartString.CStr());
  Err("%s\n", QBDBDirectory.CStr());
  TSecTm PresentTime = TDataLoader::LoadQBDBByWindow(QBDBDirectory, StartString, WindowSize, QB, DB);
  fprintf(stderr, "QBDB successfully loaded!\n");

  TVec<TSecTm> PubTmV;
  TVec<TStr> PostUrlV;
  TVec<TStr> QuoteV;

  fprintf(stderr, "Dumping quotes to file...\n");
  TIntV QuoteIds;
  QB.GetAllQuoteIds(QuoteIds);
  for (int i = 0; i < QuoteIds.Len(); i++) {
    TQuote Q;
    QB.GetQuote(QuoteIds[i], Q);
    TStr QContentString;
    Q.GetContentString(QContentString);

    TVec<TUInt64> Sources;
    Q.GetSources(Sources);
    for (int j = 0; j < Sources.Len(); j++) {
      TDoc D;
      DB.GetDoc(Sources[j], D);
      TStr PostUrl;
      D.GetUrl(PostUrl);
      TSecTm PostTime = D.GetDate();
      QuoteV.Add(QContentString);
      PubTmV.Add(PostTime);
      PostUrlV.Add(PostUrl);
    }
  }

  TFOut FOut(OutDirectory + "QuoteList" + ".bin");
  PubTmV.Save(FOut);
  PostUrlV.Save(FOut);
  QuoteV.Save(FOut);

  fprintf(stderr, "Done!\n");
  return 0;
}
