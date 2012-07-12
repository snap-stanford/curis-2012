void LogOutput::LogOutput(const TStr Key, TStr Value) {
  OutputValues.AddDat(Key, Value);
}

void LogOutput::WriteOutputToFile() {
  TStrV Keys;
  OutputValues.GetKeyV(Keys);
  TSecTm Tm = GetCurTm();
  TStr DirName = TStr::Fmt("%02d-%02d%-%04d", Tm.tm_mon+1, Tm.tm_mday, Tm.tm_year+1900);
  DirName += "_" + Tm.GetTmStr();
  TStr Command = "mkdir " + OutputDirectory + DirName;
  system(Command.CStr());
}
