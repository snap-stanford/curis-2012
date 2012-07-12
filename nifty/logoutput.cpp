#include "logoutput.h"
#include "stdafx.h"

const TStr LogOutput::OutputDirectory = "/lfs/1/tmp/curis/output/";
const TStr LogOutput::PercentEdgesDeleted = "Percent of Edges Deleted";
const TStr LogOutput::NumOriginalEdges = "Number of Edges Prior to Deletion";
const TStr LogOutput::NumRemainingEdges = "Number of Remaining Edges";
const TStr LogOutput::NumQuotes = "Number of Unclustered Quotes";

void LogOutput::LogValue(const TStr Key, TStr Value) {
  OutputValues.AddDat(Key, Value);
}

void LogOutput::WriteOutputToFile() {
  TStrV Keys;
  OutputValues.GetKeyV(Keys);
  TSecTm Tm = TSecTm::GetCurTm();
  TStr DirName = TStr::Fmt("%02d-%02d%-%04d", Tm.GetMonthN()+1, Tm.GetDayN(), Tm.GetYearN()+1900);
  DirName += "_" + Tm.GetTmStr();
  TStr Command = "mkdir " + OutputDirectory + DirName;
  system(Command.CStr());
}
