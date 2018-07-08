#ifndef __svmprepare_H
#define __svmprepare_H
#include "svm.h"
#include "typedef.h"
#include "partition.h"

void prepare_svm_parameters(Svm_parameterptr param, SVM_TYPE probtype);
void prepare_svm_problem(Svm_problemptr prob, SVM_TYPE probtype, Instanceptr data, Partition p);

#endif
