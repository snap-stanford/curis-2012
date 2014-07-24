#include "stdafx.h"
#include "argumentparser.h"

THash<TStr, TStr> ArgumentParser::Arguments; // = THash<TStr, TStr>();

/* 
 * ParseArguments takes in the system artuments and returns a hash table (Arguments) of string
 * key-value pairs. Log is passed in just so logging can be automatically disabled if need be.
 */
void ArgumentParser::ParseArguments(int argc, char *argv[], THash<TStr, TStr>& Arguments, LogOutput& Log) {
  for (int i = 1; i < argc; i++) {
    if (strlen(argv[i]) >= 2 && argv[i][0] == '-' && i + 1 < argc) {
      Arguments.AddDat(TStr(argv[i] + 1), TStr(argv[i + 1]));
      Log.LogValue(TStr(argv[i] + 1), TStr(argv[i + 1]));
      i++;
    } else {
      printf("Error: incorrect format. You're probably missing an argument name. \nUsage: ./(program name) [-paramName parameter]");
      exit(1);
    }
  }
  if (Arguments.IsKey("nolog")) {
    Log.DisableLogging();
  }
  ArgumentParser::Arguments = Arguments;
}

/*
 * GetArgument takes in a hash table of Arguments (from ParseArguments), a key, and returns the
 * argument value passed in with that key. For laziness, "Default" is the default value that the
 * value should take on if the argument was not passed in in the command line.
 */
TStr ArgumentParser::GetArgument(THash<TStr, TStr>& Arguments, TStr Key, TStr Default) {
  TStr Result = Default;
  Arguments.IsKeyGetDat(Key, Result);
  return Result;
}

bool ArgumentParser::Contains(TStr Key) {
  return Arguments.IsKey(Key);
}
