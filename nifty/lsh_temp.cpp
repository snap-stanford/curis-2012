#include "stdafx.h"

const int BAND_SIZE = 5;
const int NUM_BANDS = 20;

THash<TMd5Sig, TIntSet> Shingles;

void  LSH() {
  for(int i = 0; i < NUM_BANDS; ++i) {
    THash<TInt, TIntV> Inverted; // (QuoteID, QuoteSignatureForBand)
    THash<TIntV, TIntSet> BandBuckets; // (BandSignature, QuoteIDs)
    for (int j = 0; j < BAND_SIZE; ++j) {
      // Create new signature
      TVec<TMd5Sig> Signature;
      Shingles.GetKeyV(Signature);
      Signature.Shuffle();

      // Place in bucket - not very efficient
      int SigLen = Signature.Len();
      for (int k = 0; k < SigLen; ++k) {
        TIntSet<TInt> CurSet = Shingles.GetDat(Signature[k]);
        for (TIter l = CurSet.BegI(); l < CurSet.EndI(); ++l) {
          TInt Key = l.GetKey();
          if (Inverted.IsKey(Key)) {
            TIntV CurSignature = Inverted.GetDat(Key);
            if (CurSignature.Len() < j) {
              CurSignature.Add(k);
              Inverted.AddDat(Key, CurSignature);
            }
          } else {
            TIntV NewSignature;
            NewSignature.Add(k);
            Inverted.AddDat(Key, NewSignature);
          }
        }
      }
    }

    TIntV InvertedKeys;
    Inverted.GetKeyV(InvertedKeys);
    TInt InvertedLen = InvertedKeys.Len();
    for (int k = 0; k < InvertedLen; ++k) {
      TIntSet Bucket;
      TIntSet Signature = Inverted.GetDat(InvertedKeys[k]);
      if (BandBuckets.IsKey(Signature)) {
        Bucket = BandBuckets.GetDat(InvertedKeys[k]);
      }
      Bucket.AddKey(InvertedKeys[k]);
      BandBuckets.AddDat(Signature, Bucket);
    }

    // edge creation here maybe?
  }
  // return hashes
}

// {1, 2, 3, 4}
// 1,2,3,4 <--
