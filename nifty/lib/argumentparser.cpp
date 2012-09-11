#include "stdafx.h"
#include "argumentparser.h"

void ArgumentParser::ParseArguments(int argc, char *argv[], THash<TStr, TStr>& Arguments, LogOutput& Log) {
  for (int i = 1; i < argc; i++) {
    if (strlen(argv[i]) >= 2 && argv[i][0] == '-' && i + 1 < argc) {
      Arguments.AddDat(TStr(argv[i] + 1), TStr(argv[i + 1]));
      Log.LogValue(TStr(argv[i] + 1), TStr(argv[i + 1]));
      i++;
    } else {
      printf("Error: incorrect format. Usage: ./(program name) [-paramName parameter]");
      exit(1);
    }
  }
  if (Arguments.IsKey("nolog")) {
    Log.DisableLogging();
  }
}

TStr ArgumentParser::GetArgument(THash<TStr, TStr>& Arguments, TStr Key, TStr Default) {
  TStr Result;
  if (Arguments.IsKeyGetDat(Key, Result)) {
    return Result;
  } else {
    return Default;
  }
}
