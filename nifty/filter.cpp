#include "stdafx.h"
#include "dataloader.h"
#include "quote.h"
#include "doc.h"
#include <stdio.h>

const int MinMemeFreq = 5;
const int MinQtWrdLen = 3;
const int MaxQtWrdLen = 30;
const double MinCommonEnglishRatio = 0.25;

THashSet<TMd5Sig> SeenUrlSet(Mega(100), true);
THashSet<TStr> URLBlackList;
THashSet<TStr> CommonEnglishWordsList;

void LoadCommonEnglishWords() {
  PSIn EnglishWords = TFIn::New("common_english_words.txt");
  TStr Word;
  while (!EnglishWords->Eof() && EnglishWords->GetNextLn(Word)) {
    CommonEnglishWordsList.AddKey(Word);
  }
}

bool IsEnglish(const TChA &quote) {
  return quote.CountCh('?') <= quote.Len()/2;
}

/// Assumes lower case characters only format
bool IsRobustlyEnglish(TStr Quote) {
  TStrV Parsed;
  TQuote::ParseContentString(Quote, Parsed);
  TInt EnglishCount = 0;
  for (int i = 0; i < Parsed.Len(); ++i) {
    if (CommonEnglishWordsList.IsKey(Parsed[i])) {
      EnglishCount++;
    }
  }
  //printf("%f: %s\n", EnglishCount * 1.0 / Parsed.Len(), Quote.CStr());
  return EnglishCount * 1.0 / Parsed.Len() >= MinCommonEnglishRatio;
}

void LoadURLBlackList() {
  PSIn BlackListFile = TFIn::New("URLBlacklist");
  TStr BadURL;
  while (!BlackListFile->Eof() && BlackListFile->GetNextLn(BadURL)) {
    URLBlackList.AddKey(BadURL);
  }
}

bool IsUrlInBlackList(const TChA &Url) {
  TStr UrlStr(Url);
  TStrV PeriodVector;
  UrlStr.SplitOnAllAnyCh(".", PeriodVector);

  if (PeriodVector.Len() >= 2) {
    TStrV SlashVector;
    PeriodVector[PeriodVector.Len() - 1].SplitOnAllAnyCh("/", SlashVector);
    if (SlashVector.Len() >= 1) {
      TStr DomainName = PeriodVector[PeriodVector.Len() - 2] + "." + SlashVector[0];
      if (URLBlackList.IsKey(DomainName)) {
        return true;
      }
    }
  }

  return false;
}


// Removes all punctuation in the quotes and replace with spaces.
// Also converts upper case to lower case.
// Adapted (but modified) from memes.h because I want a white list, not a blacklist.
void FilterSpacesAndSetLowercase(TStr &QtStr) {
  // Three passes...hopefully this isn't too slow.
  TChA QtChA(QtStr);
  for (int i = 0; i < QtChA.Len(); ++i) {
    if (!(isalpha(QtChA[i]) || QtChA[i] == '\'')) {
      QtChA[i] = ' ';
    } else {
      QtChA[i] = tolower(QtStr[i]);
    }
  }
  QtStr = TStr(QtChA);
  TStrV WordV;
  QtStr.SplitOnAllAnyCh(" ", WordV);
  QtStr.Clr();
  for (int i = 0; i < WordV.Len(); ++i) {
    if (i > 0)  QtStr.InsStr(QtStr.Len()," ");
    QtStr.InsStr(QtStr.Len(), WordV[i]);
  }
}

bool IsDuplicateUrl(const TChA &Url) {
  TMd5Sig UrlSig = TMd5Sig(Url);
  if (SeenUrlSet.IsKey(UrlSig)) { return true; }
    SeenUrlSet.AddKey(UrlSig);
  return false;
}

bool IsPostTimeValid(TSecTm PostTm, TSecTm FileTm) {
  return (PostTm <= FileTm + 12 * 3600) && (PostTm >= FileTm - 12 * 3600);
}

void OutputQuoteInformation(TQuoteBase* QuoteBase, TStr FileName) {
  FILE *F = fopen(FileName.CStr(), "wt");
  TFOut QuotesFile(FileName);
  TIntV QuoteIds;
  QuoteBase->GetAllQuoteIds(QuoteIds);
  for (int i = 0; i < QuoteIds.Len(); ++i) {
    TQuote Quote;
    bool IsInQB = QuoteBase->GetQuote(QuoteIds[i], Quote);
    if (IsInQB) {
      TStr ContentString;
      Quote.GetContentString(ContentString);
      fprintf(F, "%d\t%s\n", Quote.GetNumSources().Val, ContentString.CStr());
    }
  }
  //Save(QuotesFile);
  fclose(F);
}

// usage filelist directory
int main(int argc, char *argv[]) {
  printf("File name must be in the form: web-{year}-{month}-{day}T{hour}-{minute}-{second}Z.rar\n");
  TStr InFileName = "Spinn3rFileList.txt";
  TStr OutFileName = "QuoteFrequencies.txt";
  if (argc >= 2) {
    InFileName = TStr(argv[1]);
  }
  if (argc >= 3) {
    OutFileName = TStr(argv[2]);
  }
  LoadURLBlackList();
  LoadCommonEnglishWords();

  printf("Loading data from Spinn3r dataset to QuoteBase...\n");
  printf("1: Initial Filtering:\n");
  TQuoteBase *TmpQB = new TQuoteBase;
  TDocBase *TmpDB = new TDocBase;
  int NSkip = 0;
  // Read files and count the quotes
  TDataLoader Memes;
  Memes.LoadFileList(InFileName, "/lfs/1/tmp/curis/spinn3r/2012-01/");
  while (Memes.LoadNextFile()) {
    while (Memes.LoadNextEntry()) {
      if (IsUrlInBlackList(Memes.PostUrlStr)) { NSkip++;continue; }
      if (IsDuplicateUrl(Memes.PostUrlStr)) { NSkip++;continue; }
      if (!IsPostTimeValid(Memes.PubTm, Memes.GetCurrentFileTime())) { NSkip++;continue; }
      bool ContainValidQuote = false;
      for (int m = 0; m < Memes.MemeV.Len(); m++) {
        if (IsEnglish(Memes.MemeV[m]) &&
            TStrUtil::CountWords(Memes.MemeV[m]) >= MinQtWrdLen &&
            TStrUtil::CountWords(Memes.MemeV[m]) <= MaxQtWrdLen) {
          TStr QtStr = Memes.MemeV[m];
          FilterSpacesAndSetLowercase(QtStr);
          ContainValidQuote = true;
        } else {
          Memes.MemeV[m].Clr();
        }
      }
      if (ContainValidQuote) {
        TInt CurrentDocId = TmpDB->AddDoc(Memes.PostUrlStr, Memes.PubTm, Memes.ContentStr, Memes.LinkV);
        for (int m = 0; m < Memes.MemeV.Len(); m++) {
          if (!Memes.MemeV[m].Len() < 1) {
            TmpQB->AddQuote(Memes.MemeV[m], CurrentDocId);
          }
        }
      } else {
        NSkip++;
      }
    }
  }
  printf("Number of quotes: %d\n", TmpQB->Len());
  printf("Number of docs: %d\n", TmpDB->Len());
  printf("Skipped %d entries\n", NSkip);

  printf("\n2: Second Filtering:\n");
  TQuoteBase *QB = new TQuoteBase;
  TDocBase *DB = new TDocBase;
  TIntV QuoteIds;
  TmpQB->GetAllQuoteIds(QuoteIds);
  for (int i = 0; i < QuoteIds.Len(); i++) {
    TQuote Q;
    TmpQB->GetQuote(QuoteIds[i], Q);
    TStr QContentString;
    Q.GetContentString(QContentString);
    if (Q.GetNumSources() >= MinMemeFreq &&
        (Q.GetNumSources() <= 50 || Q.GetNumSources() >= 4 * Q.GetNumDomains(TmpDB)) &&
        IsRobustlyEnglish(QContentString)) {
      TIntV Sources;
      Q.GetSources(Sources);
      for (int j = 0; j < Sources.Len(); j++) {
        TDoc D;
        TmpDB->GetDoc(Sources[j], D);
        TInt NewSourceId = DB->AddDoc(D);
        QB->AddQuote(QContentString, NewSourceId);
      }
    }
    if (i % 50000 == 49999) {
      printf("%d out of %d quotes processed\n", i + 1, TmpQB->Len());
    }
  }

  printf("Number of quotes: %d\n", QB->Len());
  printf("Number of documents: %d\n", DB->Len());
  printf("\nLOADING DATA TO QUOTE BASE DONE!\n");
  printf("Writing quote frequencies...\n");
  OutputQuoteInformation(QB, OutFileName);
  printf("Done!\n");
  printf("Writing QuoteBase and DocBase\n");
  TFOut FOut("QBDB.bin");
  QB->Save(FOut);
  DB->Save(FOut);
  printf("Done!\n");
  delete QB;
  delete DB;
  delete TmpQB;
  delete TmpDB;
  return 0;
}
