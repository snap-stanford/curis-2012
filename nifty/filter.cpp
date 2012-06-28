#include "stdafx.h"
#include "data_loader.h"
#include "quote.h"
#include "doc.h"

THashSet<TMd5Sig> SeenUrlSet(Mega(100), true);

bool IsEnglish(TChA &quote) {
	return quote.CountCh('?') <= quote.Len()/2;
}

bool IsUrlInBlackList(TChA &Url) {
	if (strstr(Url.CStr(), "facebook.com") != NULL) { return true; }
	if (strstr(Url.CStr(), "twitter.com") != NULL) { return true; }
	return false;
}

bool IsDuplicateUrl(TChA &Url) {
	TMd5Sig UrlSig = TMd5Sig(Url);
	if (SeenUrlSet.IsKey(UrlSig)) { return true; }
		SeenUrlSet.AddKey(UrlSig);
	return false;
}

// usage filelist directory
int main(int argc, char *argv[]) {
	TStr InFileName = "filename list";

	printf("Loading data from Spinn3r dataset to QuoteBase...\n");
	int NSkip = 0, fileCnt = 0;
	THash<TMd5Sig, TInt> MemeCntH(Mega(100), true);

	// Read files and count the quotes
	TDataLoader Memes;
	Memes.LoadFileList(InFileName, "/lfs/hulk/0/datasets/spinn3r/spinn3r-full5/");
	while (Memes.LoadNextFile()) {
		TVec<TMd5SigV> QtStrVV;

		while (Memes.LoadNextEntry()) {
			if (IsUrlInBlackList(Memes.PostUrlStr)) continue;
			if (IsDuplicateUrl(Memes.PostUrlStr)) { NSkip++;continue; }
			for (int m = 0; m < Memes.MemeV.Len(); m++) {
				if (IsEnglish(Memes.MemeV[m])) {					// Filter non-English quote
					TStr QtStr = Memes.MemeV[m];
					TQuoteBase::QuoteFilter(QtStr);
					MemeCntH.AddDat(TMd5Sig(QtStr)) += 1;
				}
			}
		}
		printf("1: Complete %d file out of %d files HashSize = %d UrlSize = %d\n",
				++fileCnt, Memes.GetNumFiles(), MemeCntH.Len(), SeenUrlSet.Len());

		printf("Done \nAll quotes: %d\n", MemeCntH.Len());
		printf("  skip %d urls, keep %d\n", NSkip, SeenUrlSet.Len());

		printf("Meme2QtBs DONE!\n");
	}
	return 0;
}
