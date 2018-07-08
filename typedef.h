#ifndef __typedef_H
#define __typedef_H

#include<stdio.h>
#include "constants.h"
#include "matrix.h"
#include "svm.h"

/*! Union storing integer OR float values*/
union int_or_float{
       /*! Integer value*/
       int intvalue;
       /*! Floating point value*/
       double floatvalue;
};

/*! Feature properties of the dataset*/
struct feature {
       /*! Type of the feature. Can be INTEGER for integer valued features, STRING for discrete features, REAL for continuous (with floating point values) features, CLASSDEF if the feature stores the class index, REGDEF if the feature stores the real regression value of the instance*/
       int type;
       /*! Old type of the feature. If the dataset is realized (integer features are converted to real features and discrete features are converted to 0/1 values), the type of each features changes. This property stores the old type of the feature before realization*/
       int oldtype;
       /*! The minimum value of the feature*/
       union int_or_float min;
       /*! The maximum value of the feature*/
       union int_or_float max;
       /*! If the feature is a discrete feature, this property stores the number of possible values the feature can attain. Size of the strings array*/
       int valuecount;
       /*! Has YES value if the feature is selected (for printing, for classification etc.), NO otherwise*/
       int selected;
       /*! If the features is a discrete feature or class definition feature, this property stores all possible values of that feature*/
       char *strings[MAXVALUES];
       };
       
typedef struct feature Feature;
typedef Feature* Featureptr;       

/*! Structure of a dataset. Datasets are stored as a link list*/
struct dataset {
       /*! Is the dataset active. If YES, learning algorithm can be applied on these datasets, NO otherwise.*/
       int active;
       /*! Name of the dataset*/
       char* name;
       /*! Number of classes in the dataset */
       int classno;
       /*! All possible names that a class feature can have. classes[i] is the name of the class i*/
       char* classes[MAXCLASSES];
       /*! Separator character. The features of each instance are separated via this character*/
       char separator;
       /*! Name of the dataset file*/
       char* file;
       /*! Number of features in the dataset (including the unused ones)*/
       int allfeatures;
       /*! Number of features in the dataset (only the used ones including the class or regression definition feature)*/
       int featurecount;
       /*! Number of features in the dataset after converting the discrete features into continuous ones (including the class or regression definition feature)*/
       int multifeaturecount;
       /*! Are the features available for classification or regression. availability[i] is TRUE if the feature is available, FALSE otherwise.*/
       int availability[MAXFEATURES];
       /*! Are the features selected for printing/exporting etc. selected[i] is YES if the feature is selected for printing and/or exporting*/
       int selected[MAXFEATURES];
       /*! Features of the dataset */
       Feature features[MAXFEATURES];
       /*! If the dataset is stored in a database, this is the data source name */
       char* dsnname;
       /*! If the dataset is stored in a database, this is the table name */
       char* tablename;
       /*! The name of the subdirectory where the dataset files are stored*/
       char* directory;
       /*! Number of instances in the dataset */
       int datacount;
       /*! Number of training instances in the dataset. This number is updated after the division of the dataset into train and test parts.*/
       int traincount;
       /*! Index of the class definition feature */
       int classdefno;
       /*! Sigma value of the dataset */
       double sigma;
       /*! When the discrete features of the dataset are converted into continuous ones, the class definition index (classdefno) may be changed (If the class definition feature is after some/all of the discrete features). In that case, this value stores the old classdefno*/
       int newclassdefno;
       /*! There are three types of database storings in the program. STORE_TEXT for default text based dataset storing, STORE_DB for storing the data in the database and STORE_TIME is used for time variant datasets such as HMM datasets*/
       STORAGE_TYPE storetype;
       /*! Number of instances in each class. classcounts[i] is the number of instances in the class with index i */
       int classcounts[MAXCLASSES];
       /*! Base error of the dataset. Error of SELECTMAX classifier*/
       double baseerror;
       /*! Kernel matrix. If the dataset is a kernel type dataset, this matrix stores the kernel*/
       matrixptr kernel;
       /*! Kernel matrix file names*/
       char* kernelfiles[MAXKERNELS];
       /*! Number of kernel matrices*/
       int kernelcount;
       /*! Pointer to the next dataset*/
       struct dataset *next;
       };

typedef struct dataset Dataset;
typedef Dataset* Datasetptr;

/*! Possible values of a feature*/
union value{
       /*! Integer value*/
       int intvalue;
       /*! Floating point value*/
       double floatvalue;
       /*! Discrete values are stored as index values*/
       int stringindex;
       };

/*! Structure of an instance. Instances are stored as a (two-dimensional for time-variant datasets) link list*/
struct instance {
       /*! Feature values of the instance. values[i] is the value of the feature i*/
       union value* values;
       /*! Class Labels of the instances for a multi-labeled dataset. YES represents that instance is not labeled with that label, NO otherwise*/
       int* class_labels;
       /*! Pointer to the next instance */
       struct instance* next;
       /*! Index of the instance in the train (or test) sets*/
       int index;
       /*! Is the instance classified correctly? YES, if it is classified correctly, NO otherwise.*/
       int classified;
       /*! Is the instance an exception? YES, if it is, NO otherwise. */
       int isexception;
       /*! Pointer to the sublist of instances. Sublist stores the frames. The first frame is stored in the first node of the instance, the second frame is stored in the sublist of it, the third frame is stored in the sublist->next, the fourth frame in the sublist->next->next etc.*/
       struct instance* sublist;
       };

typedef struct instance Instance;
typedef Instance* Instanceptr;

/*! Results of the classification algorithm*/
struct classificationresult{
       /*! Number of correctly classified test instances.*/
       int performance;
       /*! Number of correctly classified test instances in each class. class_performance[i] is the number of correctly classified instances in class i.*/
       int *class_performance;
       /*! Number of test instances in each class. class_counts[i] is the number of instance in class i.*/
       int *class_counts;
       /*! Confusion matrix. confusion_matrix[i][j] is the number of test instances from class i which are classified as class j.*/
       int **confusion_matrix;
       /*! Number of classes in the dataset */
       int classno;
       /*! Posterior probabilities. posteriors[i][j] is the posterior probability of class j of instance i*/
       double **posteriors;
       /*! Number of test instances*/
       int datasize;
       /*! Total hinge loss*/
       double hingeloss;
       /*! Total knn loss*/
       double knnloss[3];
};

typedef struct classificationresult Classificationresult;
typedef Classificationresult* Classificationresultptr;

/*! Results of the regression algorithm*/
struct regressionresult{
       /*! Error of each instance. absoluteerors[i] = abs(predicted regression value - actual regression value)*/
       double* absoluteerrors;
       /*! Total square error */
       double regressionperformance;
       /*! Number of test instances*/
       int datasize;
       /*! Total epsilon sensitive loss*/
       double epsilonloss;
};

typedef struct regressionresult Regressionresult;
typedef Regressionresult* Regressionresultptr;

struct prediction{
       double regvalue;
       int classno;
       double hingeloss;
							double epsilonloss;
       double knnloss[3];
};

typedef struct prediction Prediction;
typedef Prediction* Predictionptr;

struct korderingsplit{
       int featurecount;
       int* featurelist;
       int* discretesplit;
       int** sortorder; 
};

typedef struct korderingsplit Korderingsplit;

/*! Structure for a condition in a rule. Conditions in a rule are stored as link list. A univariate condition is something like x_1 < 4.2, a multivariate condition is something like 1.4x_1 + 3.2x2 + ... < 4.5 */
struct decisioncondition{
       /*! Index of the best (selected) feature in the condition. 1 in the example above.*/
       int featureindex;
       /*! Comparison character. Can be <, >, * (represents intervals), for continuous features = for discrete features. \< in the example above*/
       char comparison;
       /*! Value of the best split. 4.2 in the example above*/
       double value;
       /*! Value of the upper limit of the interval*/
       double upperlimit;
       /*! Value of the lower limit of the interval*/
       double lowerlimit;
       /*! Weights of the features in the multivariate condition. Stored as one row matrix.*/
       matrix w;
       /*! Bias value for the multivariate condition*/
       double w0;
       /*! Number of instances that fall in this condition*/
       int datasize;
       /*! Number of free parameters. 2 for univariate condition, m + 1 for multivariate conditions where m is the number of dimensions after PCA*/
       int freeparam;
       /*! Type of condition. UNIVARIATE_CONDITION for univariate condition, MULTIVARIATE_CONDITION for multivariate condition, HYBRID_CONDITION for condition get by omnivariate rule inducer*/
       int conditiontype;
       /*! Pointer to the next condition*/
       struct decisioncondition* next;
							/*! If the condition is generated using K-ordering, this is the split*/
							Korderingsplit ksplit;
};

typedef struct decisioncondition Decisioncondition;
typedef Decisioncondition* Decisionconditionptr;

/*! Structure for a rule in a ruleset. Rules are stored as a link list*/
struct decisionrule{
       /*! Class index for this rule*/
       int classno;
       /*! Number of instances covered by this rule*/
       int covered;
       /*! Number of instances falsely covered by this rule. Number of instances whose class index is not classno*/
       int falsepositives;
       /*! Number of instances in each class that fall into this rule. counts[i] is the number of instances will class i*/
       int* counts;       
       /*! Header of the condition link list*/
       Decisionconditionptr start;
       /*! Tail of the condition link list*/
       Decisionconditionptr end;
       /*! Number of conditions in the rule*/
       int decisioncount;
       /*! Pointer to the next rule*/
       struct decisionrule* next;
};

typedef struct decisionrule Decisionrule;
typedef Decisionrule* Decisionruleptr;

/*! Structure for a ruleset*/
struct ruleset{
       /*! Header of the rule link list*/
       Decisionruleptr start;
       /*! Tail of the rule link list*/
       Decisionruleptr end;
       /*! Number of rules in the ruleset*/
       int rulecount;
       /*! Default class for this ruleset. If none of the rules cover an instance, the instance is assigned the default class*/
       int defaultclass;
       /*! Number of instances in each class that fall into default class. counts[i] is the number of instances will class i*/
       int* counts;       
       /*! Number of possible conditions in the dataset.*/
       int possibleconditioncount;
};

typedef struct ruleset Ruleset;
typedef Ruleset* Rulesetptr;

/*! Structure for a decision tree node*/
struct decisionnode {
       /*! Pointer to the left subtree (child)*/
       struct decisionnode* left;
       /*! Pointer to the right subtree (child)*/
       struct decisionnode* right;
       /*! If the best feature is a discrete feature, the decision node will have 2 or more subtrees (children). string[i] is i'th subtree child of this node*/
       struct decisionnode* string;
       /*! Pointer to the parent node*/
       struct decisionnode* parent;
       /*! Instances those fall in this decision node. Stored as a link list.*/
       Instanceptr instances;
       /*! If the split is a univariate split, this is the split threshold*/
       double split;
       /*! If the split is a univariate split, this is the index of the best feature*/
       int featureno;
       /*! Class index of this decision node. If the node is a leaf node, this classno will be used in classification*/
       int classno;
       /*! If the split is a multivariate (linear or quadratic) split, this is the weight vector*/
       matrix w;
       /*! If the split is a multivariate (linear or quadratic) split, this is the bias*/
       double w0;
       /*! If the decision tree is a K-class soft decision tree, this is output array at the leaf node*/
       double* w0s;
       /*! If this node is a leaf node, this is the rule generated for this leaf node*/
       Decisionconditionptr rule;
       /*! Type of decision leaves. CONSTANTLEAF if there is constant fit in the leaves, LINEARLEAF if there is a linear fit in the leaves*/
       LEAF_TYPE leaftype; 
       /*! If the decision tree is a soft decision tree, this is temporary output at the node*/
       double output;
       /*! If the decision tree is a K-class soft decision tree, this is temporary output array at the node*/
       double* outputs;
       /*! If the decision tree is a soft decision tree, this is weight of the node*/
       double weight; 
       /*! Number of conditions in the rule*/
       int conditioncount;
       /*! Number of instances that fall into this node. Size of the instances link list*/
       int covered;
       /*! Number of instances that fall into this node and is not in the correct class*/
       int falsepositives;
       /*! Number of possible conditions in the dataset.*/
       int possibleconditioncount;
       /*! Number of free parameters if this is a multivariate linear split. m + 1 when m is the number of dimensions after PCA*/
       int lineard;
       /*! Number of free parameters if this is a multivariate quadratic split. m + 1 when m is the number of dimensions after PCA*/
       int quadraticd;
       /*! Type of the split*/
       int conditiontype;
       /*! Number of instances in each class that fall into this node. counts[i] is the number of instances will class i*/
       int* counts;
       /*! If this is an SVMTREE node, the SVM split is stored in this field*/
       Svm_split svmsplit;
       /*! If this is a node of the tree generated by the PART algorithm, this field specifies if this node is already expanded*/
       BOOLEAN expanded;
       /*! Coordinates to draw the decision tree*/
       int x, y;
       Korderingsplit ksplit;       
};

typedef struct decisionnode Decisionnode;
typedef struct decisionnode* Decisionnodeptr;

/*! In the NODE-BOUNDED TREE algorithm, we need an heap (stored as a link list) to store the unexpanded decision nodes (leaves). This is the node structure for that link list*/
struct node_expanded{
                     /*! Pointer to the unexpanded decision node (leaf)*/
                     Decisionnodeptr node;
                     /*! Value of the unexpanded decision node (leaf)*/
                     double value;
                     /*! Pointer to the next node*/
                     struct node_expanded* next;
};

typedef struct node_expanded Node_expanded;
typedef Node_expanded* Node_expandedptr;

/*! Structure for a regression tree node*/
struct regressionnode {
       /*! Pointer to the left subtree (child)*/
       struct regressionnode* left;
       /*! Pointer to the right subtree (child)*/
       struct regressionnode* right;
       /*! If the best feature is a discrete feature, the decision node will have 2 or more subtrees (children). string[i] is i'th subtree child of this node*/
       struct regressionnode* string;
       /*! Pointer to the parent node*/
       struct regressionnode* parent;
       /*! Training instances those fall in this regression node. Stored as a link list.*/
       Instanceptr instances;
       /*! Validation instances those fall in this regression node. Stored as a link list.*/
       Instanceptr cvinstances;
       /*! If the function at the leaves is a linear fit, this is the weight vector*/
       matrix w;
       /*! If the regression tree is a soft regression tree, this is temporary output at the node*/
       double output;
       /*! If the regression tree is a soft regression tree, this is weight of the node*/
       double weight;
       /*! The split threshold*/
       double split;
       /*! Total square error for prune set*/
       double errorforprune; 
       /*! Total square error for training set*/
       double mseerror;
       /*! Index of the best feature*/
       int featureno;
       /*! Type of decision leaves. CONSTANTLEAF if there is constant fit in the leaves, LINEARLEAF if there is a linear fit in the leaves*/
       LEAF_TYPE leaftype;
       /*! If the fit is constant, this is the fit value*/
       double regressionvalue;
       /*! Number of instances that fall into this node. Size of the instances link list*/
       int covered;
       /*! Mean vector of the instances that fall into the left subtree. Stored as an instance*/
       Instanceptr leftcenter;
       /*! Mean vector of the instances that fall into the right subtree. Stored as an instance*/
       Instanceptr rightcenter;
       /*! Coordinates to draw the regression tree*/
       int x, y;       
};

typedef struct regressionnode Regressionnode;
typedef struct regressionnode* Regressionnodeptr;

/*! Structure for a font in eps files. Also used in drawing lines*/
struct font{
  /*! Name of the font. Can be HELVETICA, COURIER, TIMES or ARIAL.*/
  FONT_TYPE fontname;
  /*! Size of the font*/
  double fontsize;
  /*! Color of the font. Can be BLACK, RED, GREEN, BLUE, YELLOW, PURPLE or LIGHTGREEN*/
  COLOR_TYPE fontcolor;
  /*! If this is the font of a line, this is the dashed type*/
  int dashedtype;
};

typedef struct font Font;

/*! Limits of the axes drawn*/
struct limits{
              /*! Minimum value of the axis*/
              double min;
              /*! Maximum value of the axis*/
              double max;
              /*! Number of parts visible on the axis*/
              int division;
              /*! Precision of the numbers on the axis*/
              int precision;
             };

typedef struct limits Limits;

/*! Point structure*/
struct point{
  /*! x axis value*/
  double x;
  /*! y axis value*/
  double y;
};

typedef struct point Point;

/*! Line structure*/
struct line{
      /*! First point of the line*/
      Point upleft;
      /*! Second point of the line*/
      Point downright;
};

typedef struct line Line;

/*! Line structure*/
struct circle{
      /*! First point of the line*/
      Point center;
      /*! Second point of the line*/
      double radius;
};

typedef struct circle Circle;

/*! String that will be written in the eps file*/
struct string{
      /*! Position of the string given as a point*/
      Point p;
      /*! String itself*/
      char* st;
};

typedef struct string String;

/*! Rectangle structure*/
struct rectangle{
      /*! Upleft corner of the rectangle*/
      Point upleft;
      /*! Downright corner of the rectagle*/
      Point downright;
};

typedef struct rectangle Rectangle;

/*! Mean and variance values of a result (accuracy, error, etc.)*/
struct meanvariance{
      /*! Mean value*/
      double mean;
      /*! Variance value*/
      double variance;
};

typedef struct meanvariance Meanvariance;

/*! Boxplot structure*/
struct boxplot{
      /*! Median of the data*/
      double median;
      /*! 25 quantile of the data*/
      double p25;
      /*! 75 quantile of the data*/
      double p75;
      /*! Smallest whisker value*/
      double swhisker;
      /*! Largest whisker value*/
      double lwhisker;
};

typedef struct boxplot Boxplot;

/*! Histogram plot structure*/
struct errorhistogramplot{
      /*! Minimum value of the sample*/
      double min;
      /*! Maximum value of the sample*/
      double max;
      /*! Number of instances in each bin. There are at most 10 bins*/
      int counts[10];
};

typedef struct errorhistogramplot Errorhistogramplot;

/*! Legend of an image*/
struct legend{
      /*! Where to put the legend. DOWN_RIGHT for down right, DOWN_LEFT for down left, UP_RIGHT for upper right, UP_LEFT for upper left, and NONE for not putting the legend*/
      LEGEND_POSITION_TYPE position;
      /*! Number of items in the legend*/
      int legendcount;
      /*! Colors of each item in the legend. colors[i] is the color of the item i. Can be BLACK, RED, GREEN, BLUE, YELLOW, PURPLE or LIGHTGREEN*/
      COLOR_TYPE* colors;
      /*! Type of each legend item.*/
      char* types;
      /*! Name of each legend item.*/
      char** names;
      /*! Font of the legend items*/
      Font fnt;
};

typedef struct legend Legend;

/*! Axis structure*/
struct axis{
      /*! Display the axis? YES to display, NO not to display.*/
      int available;
      /*! Limits of the axis*/
      Limits lim;
      /*! Are limits set manually? YES if they are set manually, NO otherwise*/
      int limitsset;
      /*! If the axis legends are names, these are the names of these legends. names[i] is the legend of the part i*/
      char** names;
      /*! Are the axis legends names or numbers. YES if the axis legends are names, NO if they are numbers.*/
      int namesavailable;
      /*! Label of the axis*/
      char* label;
      /*! Font of the axis legends*/
      Font axisfnt;
      /*! Font of the label*/
      Font labelfnt;
};

typedef struct axis Axis;

/*! An object in an eps file*/
union obj{
          /*!Can be a line*/
          Line li;
          /*!Can be a circle*/
          Circle ci;
          /*!Can be a string*/
          String st;
          /*!Can be a rectangle*/
          Rectangle re;
          /*!Can be a mean variance box*/
          Meanvariance mv;
          /*!Can be a boxplot*/
          Boxplot box;
          /*!Can be an error histogram plot*/
          Errorhistogramplot errorhist;
};

/*! Object structure */
struct object{
       /*! Type of the object. OBJECT_LINE if the object is a line, OBJECT_STRING if the object is a string, OBJECT_RECTANGLE if the object is a rectangle, OBJECT_MVBOX if the object is a mean-variance box, OBJECT_BOXPLOT if the object is a boxplot, OBJECT_ERRORHISTOGRAM if the object is an histogram plot, OBJECT_NAMEDPOINT if the object is a point where there is a string over or under it.*/
       OBJECT_TYPE type;
       /*! Font of the object. Applies to OBJECT_STRING, OBJECT_NAMEDPOINT*/
       Font fnt;
       /*! If there are more than or equal to two group of objects such as two groups of histogram plots, this is the index of the group the object belongs to*/
       int groupindex;
       /*! The index of the object in its group. Look above for the definition of a group*/
       int index;
       /*! Object itself*/
       union obj obje;
};

typedef struct object Object;
typedef Object* Objectptr;

/*! Structure for an eps image*/
struct image{
       /*! Height of the eps image*/
       int height;
       /*! Width of the eps image*/
       int width;
       /*! x axis of the image*/
       Axis xaxis;
       /*! y axis of the image*/
       Axis yaxis;
       /*! Legend of the image*/
       Legend imagelegend;
       /*! Type of group coloring of the image*/
       GROUP_COLORING_TYPE groupcoloring;
       /*! Number of objects in the image. Size of the objects array*/
       int objectcount;
       /*! Array of objects in the image*/
       Objectptr objects;
};

typedef struct image Image;
typedef Image* Imageptr;

/*! Structure for a condition in a regression rule. Conditions in a rule are stored as link list. A condition is something like x_1 < 4.2*/
struct regressioncondition{
       /*! Index of the best (selected) feature in the condition. 1 in the example above.*/
       int featureindex;
       /*! Comparison character. Can be <, >, for continuos features = for discrete features. \< in the example above*/
       char comparison;
       /*! Value of the best split. 4.2 in the example above*/
       double value;
       /*! Mean vector of the instances that fall into the left subtree. Stored as an instance*/
       Instanceptr leftcenter;
       /*! Mean vector of the instances that fall into the right subtree. Stored as an instance*/
       Instanceptr rightcenter;
       /*! Pointer to the next regression condition*/
       struct regressioncondition* next;
       };

typedef struct regressioncondition Regressioncondition;
typedef Regressioncondition* Regressionconditionptr;

/*! Structure for a rule in a regression ruleset. Rules are stored as a link list*/
struct regressionrule{
       /*! If the function of the rule is a linear fit, this is the weight vector*/
       matrix w;
       /*! Type of functional fit for this rule. CONSTANTLEAF if there is constant fit for the rule, LINEARLEAF if there is a linear fit for the rule*/
       LEAF_TYPE leaftype;
       /*! If the fit is constant, this is the fit value*/
       double regressionvalue;
       /*! Header of the condition link list*/
       Regressionconditionptr start;
       /*! Tail of the condition link list*/
       Regressionconditionptr end;
       /*! Number of conditions in the rule*/
       int conditioncount;
       /*! Pointer to the next rule*/
       struct regressionrule* next;
       /*! Number of instances covered by this rule*/
       int covered;
       };

typedef struct regressionrule Regressionrule;
typedef Regressionrule* Regressionruleptr;

/*! Structure for a regression ruleset*/
struct regressionruleset{
       /*! Header of the rule link list*/
       Regressionruleptr start;
       /*! Tail of the rule link list*/
       Regressionruleptr end;
       /*! If the function of the rule is a linear fit, this is the weight vector*/
       matrix w;
       /*! Type of functional fit for this ruleset (for the instances not covered by any rule). CONSTANTLEAF if there is constant fit for the ruleset, LINEARLEAF if there is a linear fit for the ruleset*/
       LEAF_TYPE leaftype;
       /*! If the fit is constant, this is the fit value*/
       double regressionvalue;
       /*! Number of rules in the ruleset*/
       int rulecount;
       };

typedef struct regressionruleset Regressionruleset;
typedef Regressionruleset* Regressionrulesetptr;

/*! Structure storing the value of a variable in the ISELL language*/
union varvalue{
          /*! Value of the integer variable*/
          int intvalue;
          /*! Value of the floating point variable*/
          double floatvalue;  
          /*! Value of the string and file variable. For file variable this is the name of the file*/
          char* stringvalue;
          /*! Value of the matrix variable.*/  
          matrixptr matrixvalue; 
          /*! If the variable is an array, this value stores the values in the array*/
          struct variable* array;  
         };

/*! Variable in the ISELL language*/
struct variable{
                /*! Type of the variable. Can be INTEGER_TYPE for variables of integer type, REAL_TYPE for variables of floating point type, STRING_TYPE for variables of string type, ARRAY_TYPE for variables of array type, FILE_TYPE for variables of array type, MATRIX_TYPE for variables of matrix type */
                VARIABLE_TYPE type;
                /*! If this variable is used in a for loop in the language, this is the step size of this for loop */
                int step;
                /*! If this variable is used in a for loop in the language, this is the end value of this for loop */
                int end;
                /*! Stores the instruction pointer. Used when a go back is needed */
                int ip;
                /*! If the variable is a file variable, pointer to the stream */
                FILE* userfile;
                /*! Name of the variable */
                char* name;
                /*! If this variable is of array type, this is the size of that array. */
                int arraysize;
                /*! Value of this variable*/
                union varvalue value;
               };

typedef struct variable Variable;
typedef Variable* variableptr;

struct expnode{
                char* name;
                VARIABLE_TYPE type;
                union varvalue value;
                struct expnode* next;
};

typedef struct expnode Expnode;
typedef Expnode* Expnodeptr;

/*! Epsilon estimate link list node*/
struct epsilonestimate{
                       /*! Epsilon estimate*/
                       double epsilon;
                       /*! Pointer to the next epsilon estimate*/
                       struct epsilonestimate* next;
};

typedef struct epsilonestimate Epsilonestimate;
typedef Epsilonestimate* Epsilonestimateptr;

/*! VC estimate structure. Stored as a doubly link list*/
struct vcestimate{
                  /*! Number of random instances to create in the estimate*/
                  int n;
                  /*! Number of iterations to be done*/
                  int count;
                  int issaturated;
                  double contribution;
                  /*! Header of the epsilon estimates link list*/
                  Epsilonestimateptr epsilonestimates;
                  /*! Pointer to the next vc estimate experiment*/
                  struct vcestimate* next;
                  /*! Pointer to the vc estimate experiment before*/
                  struct vcestimate* before;
};

typedef struct vcestimate Vcestimate;
typedef Vcestimate* Vcestimateptr;

#endif
