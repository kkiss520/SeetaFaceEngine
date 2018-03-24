/*
 *
 * This file is part of the open-source SeetaFace engine, which includes three modules:
 * SeetaFace Detection, SeetaFace Alignment, and SeetaFace Identification.
 *
 * This file is part of the SeetaFace Detection module, containing codes implementing the
 * face detection method described in the following paper:
 *
 *
 *   Funnel-structured cascade for multi-view face detection with alignment awareness,
 *   Shuzhe Wu, Meina Kan, Zhenliang He, Shiguang Shan, Xilin Chen.
 *   In Neurocomputing (under review)
 *
 *
 * Copyright (C) 2016, Visual Information Processing and Learning (VIPL) group,
 * Institute of Computing Technology, Chinese Academy of Sciences, Beijing, China.
 *
 * The codes are mainly developed by Shuzhe Wu (a Ph.D supervised by Prof. Shiguang Shan)
 *
 * As an open-source face recognition engine: you can redistribute SeetaFace source codes
 * and/or modify it under the terms of the BSD 2-Clause License.
 *
 * You should have received a copy of the BSD 2-Clause License along with the software.
 * If not, see < https://opensource.org/licenses/BSD-2-Clause>.
 *
 * Contact Info: you can send an email to SeetaFace@vipl.ict.ac.cn for any problems.
 *
 * Note: the above information must be kept whenever or wherever the codes are used.
 *
 */

#include "classifier/lab_boosted_classifier.h"

#include <memory>
#include <string>
#include <cstring>

namespace seeta {
namespace fd {

void LABBaseClassifier::SetWeights(const fixed_t* weights, int32_t num_bin) {
	if (weights_ != nullptr)
		delete weights_;
	weights_ = new fixed_t[num_bin + 1];
	std::memcpy(weights_, weights, sizeof(fixed_t)*(num_bin + 1));
  num_bin_ = num_bin;
  //std::copy(weights, weights + num_bin_ + 1, weights_);
}

bool LABBoostedClassifier::Classify(float* score, float* outputs) {
  bool isPos = true;
  float s = 0.0f;
  int32_t count = 0;
  seeta::fd::LABFeature* feat = feat_.data();
  std::shared_ptr<seeta::fd::LABBaseClassifier>* base = base_classifiers_.data();
  size_t size = base_classifiers_.size();
  //std::vector<seeta::fd::LABFeature>::iterator feat_it = feat_.begin();
  //std::vector<std::shared_ptr<seeta::fd::LABBaseClassifier> >::iterator classifier_it = base_classifiers_.begin();
  for(int i=0;i<size;i++) {
  //for (; feat_it != feat_.end() || classifier_it != base_classifiers_.end(); feat_it++,classifier_it++) {
	  uint8_t featVal = feat_map_->GetFeatureVal(feat[i].x, feat[i].y);
	  s += base[i]->weights(featVal);
	  count += 1;
	  if (count == kFeatGroupSize) {
		  count = 0;
		  if (s < base[i]->threshold()) {
			  isPos = false;
			  break;
		  }
	  }
  }
  /*for (size_t i = 0; isPos && i < base_classifiers_.size();) {
    for (int32_t j = 0; j < kFeatGroupSize; j++, i++) {
      uint8_t featVal = feat_map_->GetFeatureVal(feat_[i].x, feat_[i].y);
      s += base_classifiers_[i]->weights(featVal);
    }
    if (s < base_classifiers_[i - 1]->threshold())
      isPos = false;
  }
  */
  isPos = isPos && ((!use_std_dev_) || feat_map_->GetStdDev() > kStdDevThresh);

  if (score != nullptr)
    *score = s;
  if (outputs != nullptr)
    *outputs = s;

  return isPos;
}

void LABBoostedClassifier::AddFeature(int32_t x, int32_t y) {
  LABFeature feat;
  feat.x = x;
  feat.y = y;
  feat_.push_back(feat);
}

void LABBoostedClassifier::AddBaseClassifier(const fixed_t* weights,
	int32_t num_bin, fixed_t thresh) {
  std::shared_ptr<LABBaseClassifier> classifier(new LABBaseClassifier());
  classifier->SetWeights(weights, num_bin);
  classifier->SetThreshold(thresh);
  base_classifiers_.push_back(classifier);
}

}  // namespace fd
}  // namespace seeta
