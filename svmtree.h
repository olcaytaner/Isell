#ifndef __svmtree_H
#define __svmtree_H

struct splitpoint{
                  double x;
                  int count;
};

typedef struct splitpoint Splitpoint;
typedef Splitpoint* Splitpointptr;

int         create_svmtreechildren(Decisionnodeptr node, Svmtree_parameterptr p, int positive);
Svm_split   find_best_svm_split_for_partition(Instanceptr data, Partition p);
int         find_best_svmtree_feature(Instanceptr instances, double* split, int positive, double C);
int         optimize_C_for_svmtreenode(Instanceptr* traindata, int positive, double* split);
int         support_vector_length(Svm_nodeptr sv);

#endif
