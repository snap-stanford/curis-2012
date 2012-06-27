#include "stdafx.h"
#include "data_loader.h"


// usage filelist directory
int main(int argc, char *argv[]) {
	TStr InFileName = "filename list";

	printf("Loading data from Spinn3r dataset to QuoteBase...\n");
	int NSkip = 0, fileCnt = 0;
	THash<TMd5Sig, TInt> MemeCntH(Mega(100), true);
	THashSet<TMd5Sig> SeenUrlSet(Mega(100), true);

	// Read files and count the quotes
	TDataLoader Memes;
	Memes.LoadFileList(InFileName);
	while (Memes.LoadNextFile()) {
		TVec<TMd5SigV> QtStrVV;
		TMd5SigV UrlSigV;

		while (Memes.LoadNextEntry()) {
			if (strstr(Memes.PostUrlStr.CStr(), "facebook.com") != NULL) continue;
			if (strstr(Memes.PostUrlStr.CStr(), "twitter.com") != NULL) continue;
			TMd5Sig UrlSig = TMd5Sig(Memes.PostUrlStr);
			if (SeenUrlSet.IsKey(UrlSig)) {NSkip++;continue;}
			SeenUrlSet.AddKey(UrlSig);
			for (int m = 0; m < Memes.MemeV.Len(); m++) {
				if (Memes.MemeV[m].CountCh('?') <= Memes.MemeV[m].Len()/2) {					// Filter non-English quote
					TStr QtStr = Memes.MemeV[m];
					QuoteFilter(QtStr);
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
