#ifndef __poly_H
#define __poly_H

#define MAXDIVISION 10000

/*! The structure that stores a single part of a polynomial expression such as 3x^5. Using this as a node, we can store a polynomial as a link list of nodes.*/
struct polynode{
                /*! Coefficient of the polynomial subexpression (3 for the example above)*/
                double coefficient;
                /*! Exponent part of the polynomial subexpression (5 for the example above)*/
                int power;
                /*! Pointer to the next node*/
                struct polynode* next;
               };

typedef struct polynode Polynode;
typedef Polynode* Polynodeptr;

/*! The link list structure storing the polynomial*/
struct polynom{
               /*! Head of the link list*/
               Polynodeptr start;
               /*! Tail of the link list*/
               Polynodeptr end;
              }; 

typedef struct polynom Polynom;
typedef Polynom* Polynomptr;

/*! Structure storing a single part of any function with one variable. The part of the function may contain multipliers of sinus, cosinus and/or usual exponents. An example function is -3x3sin4(4x)cos(2x+1)*/
struct functionnode{ 
                    /*! Coefficient of the functional subexpression (-3 for the example above)*/
	                   double coefficient;  
                    /*! Exponent part of the functional subexpression (3 for the example above)*/
																				int xpower;          
                    /*! Exponent part of the sinus multiplier of the functional subexpression (4 for the example above)*/
																				int sinpower;
                    /*! Polynom part of the sinus multiplier of the functional subexpression (4x for the example above)*/
																				Polynomptr sinus;    
                    /*! Exponent part of the cosinus multiplier of the functional subexpression (1 for the example above)*/
																				int cospower;        
                    /*! Polynom part of the cosinus multiplier of the functional subexpression (2x+1 for the example above)*/
																				Polynomptr cosinus;  
                    /*! Pointer to the next functional subexpression*/
																				struct functionnode* next;
                   };

typedef struct functionnode Functionnode;
typedef Functionnode* Functionnodeptr;

/*! The link list structure storing the function*/
struct function{
                /*! Head of the link list*/
	               Functionnodeptr start;
                /*! Tail of the link list*/
                Functionnodeptr end;
                /*! Minimum acceptable input value for the function*/
																double min;
                /*! Maximum acceptable input value for the function*/
																double max;
               };

typedef struct function Function;
typedef Function* Functionptr;

void        add_polynode_to_polynom(Polynomptr p,int power, double coefficient);
void        bias_variance_polynom(char* definitionfile, int maxdegree, int M, int N, int regression);
double      calculate_mse(double** data, int datasize, Polynomptr p);
double      calculate_msefile(char* fname, Polynomptr p);
double      calculate_poly_clserror(double** data, int datasize, Polynomptr p);
double      calculate_poly_loglikelihood(double** data, int datasize, Polynomptr p);
Function    create_function(char* st);
Polynomptr  create_polynom();
void        divide_function_into_parts(char* st, char* parts[6]);
void        find_function_bounds(Functionptr fx, int functioncount, double* min, double* max);
void        free_function(Function fx);
void        free_functionnode(Functionnodeptr fnode);
void        free_polynom(Polynomptr p);
double      function_value(Functionptr fx, int functioncount, double x);
double**    functiondatagenerate(int datasize, Functionptr fx, int functioncount, int noisy);
double**    functiondatageneratefile(int datasize, char* functiondefinition, int noisy);
Polynomptr  mean_of_polynoms(Polynomptr* polynoms, int polynomcount);
Polynomptr  read_polynom(char* st);
double**    polydatagenerate(int datasize, double start, double end, char* polynomial, int noisy);
Polynomptr  polyfit(double** data, int datasize, int degree);
Polynomptr  polyfitcls(double** data, int datasize, int degree);
Polynomptr  polyfitclsfile(char* fname, int degree, double* error);
Polynomptr  polyfitfile(char* fname,int degree,double* error);
Polynomptr  polylearn(char* trainfname, char* testfname, int maxdegree, double* trainerror, double* testerror, int regression);
int         polylearnvalidation(char* definitionfile, int maxdegree, int regression);
double      polynom_value(Polynomptr p,double value);
void        print_polynom(Polynomptr p, char* st);
Functionptr read_function_definition(char* functiondefinitionfile, int* functioncount);
double**    readpolydata(char* fname, int* datacount);
void        savepolydata(char* fname, double** polydata, int datasize);

#endif
