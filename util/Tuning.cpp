#include "Tuning.h"


void Tuning::load(const char* assetName) {
    FileBuffer fb = assetAPI->loadAsset(assetName);
    if (fb.size) {
		dfp.load(fb, assetName);
		registerNewAsset(assetName);
		delete[] fb.data;
    }
}

void Tuning::reload(const char* assetName) {
    FileBuffer fb = assetAPI->loadAsset(assetName);
    if (fb.size) {
		dfp.load(fb, assetName);
		delete[] fb.data;
    }
}

float Tuning::f(hash_t h) {
	float result = 0.0f;
	dfp.get<float>(
		DataFileParser::GlobalSection,
		h,
		&result);
	return result;
}


int Tuning::i(hash_t h) {
	int result = 0;
	dfp.get<int>(
		DataFileParser::GlobalSection,
		h,
		&result);
	return result;
}