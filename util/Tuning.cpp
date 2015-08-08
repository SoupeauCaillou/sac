#include "Tuning.h"

Tuning* Tuning::instance = NULL;

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
	typeHints[h] = TuningType::Float;
	return result;
}

void Tuning::setF(hash_t h, float f) {
	dfp.set<float>(
		"",
		INV_HASH(h),
		&f);
	typeHints[h] = TuningType::Float;
}

int Tuning::i(hash_t h) {
	int result = 0;
	dfp.get<int>(
		DataFileParser::GlobalSection,
		h,
		&result);
	typeHints[h] = TuningType::Int;
	return result;
}

void Tuning::setI(hash_t h, int f) {
	dfp.set<int>(
		"",
		INV_HASH(h),
		&f);
	typeHints[h] = TuningType::Int;
}