#include "distributions.h"
#include "hmm.h"
#include "instance.h"
#include "instancelist.h"
#include "loadmodel.h"
#include "libarray.h"
#include "libmath.h"
#include "libmemory.h"
#include "libmisc.h"
#include "librandom.h"
#include "math.h"
#include "statistics.h"

extern Datasetptr current_dataset;

int complexity_of_hmm(Hmmptr hmm)
{
 /*!Last Changed 02.04.2007*/
 int complexity = 0, statecomplexity, numrealfeatures, componentcount, i;
 complexity += hmm->statecount; /*pi*/
 switch (hmm->hmmtype) /*a_dot_j and a_i_dot*/
  {
   case    FULL_MODEL:complexity += hmm->statecount * hmm->statecount;
                      break;
   case      LR_MODEL:complexity += 2 * hmm->statecount - 1;
                      break;
   case LR_LOOP_MODEL:complexity += 2 * hmm->statecount;
                      break;
  }
 numrealfeatures = current_dataset->multifeaturecount - 1;
 switch (hmm->statetype)
  {
   case          STATE_DISCRETE:statecomplexity = 0;
                                for (i = 0; i < current_dataset->featurecount; i++) /*prob*/
                                  if (current_dataset->features[i].type == STRING)
                                    statecomplexity += current_dataset->features[i].valuecount;
                                complexity += statecomplexity * hmm->statecount;
                                break;
   case          STATE_GAUSSIAN:complexity += (numrealfeatures + numrealfeatures * numrealfeatures) * hmm->statecount; /*mean and covariance*/
                                break;
   case  STATE_GAUSSIAN_MIXTURE:for (i = 0; i < hmm->statecount; i++)
                                 {
                                  componentcount = ((Gaussianmixtureptr) hmm->states[i].b)->componentcount;
                                  complexity += componentcount * (numrealfeatures + numrealfeatures * numrealfeatures); /*mean and covariance*/
                                 }
                                break;
   case         STATE_DIRICHLET:complexity += numrealfeatures * hmm->statecount; /*alpha*/
                                break;
   case STATE_DIRICHLET_MIXTURE:for (i = 0; i < hmm->statecount; i++)
                                 {
                                  componentcount = ((Dirichletmixtureptr) hmm->states[i].b)->componentcount;
                                  complexity += componentcount * numrealfeatures; /*alpha*/
                                 }
                                break;
  }
 return complexity;
}

Hmmptr empty_hmm(int statecount, HMM_STATE_TYPE statetype, HMM_MODEL_TYPE hmmtype)
{
 /*!Last Changed 22.01.2007*/
 Hmmptr result;
 result = safemalloc(sizeof(Hmm), "empty_hmm", 2);
 result->states = safemalloc(statecount * sizeof(State), "empty_hmm", 3);
 result->statecount = statecount;
 result->statetype = statetype;
 result->hmmtype = hmmtype;
 return result;
}

Hmmptr random_hmm(int statecount, HMM_STATE_TYPE statetype, int componentcount, HMM_MODEL_TYPE hmmtype, Instanceptr traindata)
{
 /*!Last Changed 27.03.2007*/
 int i, j;
 Hmmptr result;
 double* myarray, randvalue;
 result = empty_hmm(statecount, statetype, hmmtype);
 myarray = random_normalized_array(statecount);
 for (i = 0; i < statecount; i++)
  {
   result->states[i].pi = myarray[i];
   result->states[i].a_dot_j = safemalloc(sizeof(double) * statecount, "random_hmm", 9);
   switch (hmmtype)
    {
     case    FULL_MODEL:result->states[i].a_i_dot = random_normalized_array(statecount);
                        break;
     case      LR_MODEL:result->states[i].a_i_dot = safecalloc(statecount, sizeof(double), "random_hmm", 14);
                        randvalue = produce_random_value(0.0, 1.0);
                        if (i != statecount - 1)
                         {
                          result->states[i].a_i_dot[i] = randvalue;
                          result->states[i].a_i_dot[i + 1] = 1 - randvalue;
                         } 
                        else
                          result->states[i].a_i_dot[i] = 1.0;
                        break;
     case LR_LOOP_MODEL:result->states[i].a_i_dot = safecalloc(statecount, sizeof(double), "random_hmm", 24);
                        randvalue = produce_random_value(0.0, 1.0);
                        if (i != statecount - 1)
                         {
                          result->states[i].a_i_dot[i] = randvalue;
                          result->states[i].a_i_dot[i + 1] = 1 - randvalue;
                         } 
                        else
                         {
                          result->states[i].a_i_dot[statecount - 1] = randvalue;
                          result->states[i].a_i_dot[0] = 1 - randvalue;
                         }
                        break;
    }
   result->states[i].b = random_state_model(statetype, componentcount, traindata);
  } 
 for (i = 0; i < statecount; i++)
   for (j = 0; j < statecount; j++)
     result->states[j].a_dot_j[i] = result->states[i].a_i_dot[j];
 safe_free(myarray);
 return result;
}

void free_hmm(Hmmptr hmm)
{
 /*!Last Changed 25.02.2007 added free_state_model*/
 /*!Last Changed 22.01.2007*/
 int i;
 for (i = 0; i < hmm->statecount; i++)
  {
   free_state_model(hmm->states[i].b, hmm->statetype);
   safe_free(hmm->states[i].a_dot_j);
   safe_free(hmm->states[i].a_i_dot);
  }  
 safe_free(hmm->states);
 safe_free(hmm);
}

Hmmptr alloc_hmm(int statecount, HMM_STATE_TYPE statetype, int componentcount, HMM_MODEL_TYPE hmmtype)
{
 /*!Last Changed 15.03.2007*/
 int i;
 Hmmptr desthmm = empty_hmm(statecount, statetype, hmmtype);
 for (i = 0; i < statecount; i++)
  {
    desthmm->states[i].pi = 0;
    desthmm->states[i].a_dot_j = safemalloc(sizeof(double) * statecount, "init_hmm", 6);
    desthmm->states[i].a_i_dot = safemalloc(sizeof(double) * statecount, "init_hmm", 7);
    desthmm->states[i].b = alloc_state_model(statetype, componentcount);
  } 
 return desthmm;
}

Hmmptr calloc_hmm(int statecount, HMM_STATE_TYPE statetype, int componentcount, HMM_MODEL_TYPE hmmtype)
{
 /*!Last Changed 15.03.2007*/
 int i;
 Hmmptr desthmm = empty_hmm(statecount, statetype, hmmtype);
 for (i = 0; i < statecount; i++)
  {
    desthmm->states[i].pi = 0;
    desthmm->states[i].a_dot_j = safecalloc(statecount, sizeof(double), "init_hmm", 6);
    desthmm->states[i].a_i_dot = safecalloc(statecount, sizeof(double), "init_hmm", 7);
    desthmm->states[i].b = calloc_state_model(statetype, componentcount);
  } 
 return desthmm;
}

Hmmptr clone_hmm(Hmmptr srchmm)
{
 /*!Last Changed 15.03.2007*/
 int i;
 Hmmptr desthmm = empty_hmm(srchmm->statecount, srchmm->statetype, srchmm->hmmtype);
 for (i = 0; i < srchmm->statecount; i++)
  {
    desthmm->states[i].pi = srchmm->states[i].pi;
    desthmm->states[i].a_dot_j = clone_array(srchmm->states[i].a_dot_j, srchmm->statecount);
    desthmm->states[i].a_i_dot = clone_array(srchmm->states[i].a_i_dot, srchmm->statecount);
    desthmm->states[i].b = clone_state_model(srchmm->states[i].b, srchmm->statetype);
  } 
 return desthmm;
}

void init_hmm(Hmmptr srchmm)
{
 /*!Last Changed 15.03.2007*/
 int i;
 for (i = 0; i < srchmm->statecount; i++)
  {
    srchmm->states[i].pi = 0;
    init_array(srchmm->states[i].a_dot_j, srchmm->statecount);
    init_array(srchmm->states[i].a_i_dot, srchmm->statecount);
    init_state_model(srchmm->states[i].b, srchmm->statetype);
  } 
}

void copy_hmm(Hmmptr desthmm, Hmmptr srchmm)
{
 /*!Last Changed 15.03.2007*/
 int i;
 desthmm->hmmtype = srchmm->hmmtype;
 desthmm->statetype = srchmm->statetype;
 for (i = 0; i < srchmm->statecount; i++)
  {
    desthmm->states[i].pi = srchmm->states[i].pi;
    memcpy(desthmm->states[i].a_dot_j, srchmm->states[i].a_dot_j, srchmm->statecount * sizeof(double));
    memcpy(desthmm->states[i].a_i_dot, srchmm->states[i].a_i_dot, srchmm->statecount * sizeof(double));
    copy_state_model(desthmm->states[i].b, srchmm->states[i].b, srchmm->statetype);
  } 
}

void sum_hmm(Hmmptr desthmm, Hmmptr srchmm)
{
 /*!Last Changed 15.03.2007*/
 int i;
 for (i = 0; i < srchmm->statecount; i++)
  {
    desthmm->states[i].pi += srchmm->states[i].pi;
    add_arrays(desthmm->states[i].a_dot_j, srchmm->states[i].a_dot_j, srchmm->statecount);
    add_arrays(desthmm->states[i].a_i_dot, srchmm->states[i].a_i_dot, srchmm->statecount);
    sum_state_model(desthmm->states[i].b, srchmm->states[i].b, srchmm->statetype);
  } 
}

int check_hmm(char* name, Hmmptr hmm)
{
 /*!Last Changed 11.04.2007*/
 int i, j;
 double sum;
 Gaussianptr g;
 Gaussianmixtureptr gm;
 int numrealfeatures = current_dataset->multifeaturecount - 1;
 printf("Checking %s. ", name);
 if (!between(hmm->hmmtype, 0, 2))
  {
   printf("Hmm type %d wrong\n", hmm->hmmtype);
   return 0;
  }
 if (!between(hmm->statetype, 0, 4))
  {
   printf("Hmm state type %d wrong\n", hmm->statetype);
   return 0;
  }
 if (hmm->statecount <= 1)
  {
   printf("Hmm state count %d must be larger than 1\n", hmm->statecount);
   return 0;
  }
 if (!(hmm->states))
  {
   printf("Hmm states not allocated\n");
   return 0;
  }
 sum = 0.0;
 for (i = 0; i < hmm->statecount; i++)
   sum += hmm->states[i].pi;
 if (!dequal(sum - 1, 0))
  {
   printf("Sum of pi %.6f is not equal to 1\n", sum);
   return 0;
  }
 for (i = 0; i < hmm->statecount; i++)
  {
   for (j = 0; j < hmm->statecount; j++)
     if (hmm->states[i].a_i_dot[j] != hmm->states[j].a_dot_j[i])
      {
       printf("Outgoing probability %.6f and incoming probability %.6f from state %d to state %d do not match\n", hmm->states[i].a_i_dot[j], hmm->states[j].a_dot_j[i], i, j);
       return 0;
      }
   sum = sum_of_list_double(hmm->states[i].a_i_dot, hmm->statecount);
   if (!dequal(sum - 1, 0))
    {
     printf("Sum of a_i_dot %.6f of state %d is not equal to 1\n", sum, i);
     return 0;
    }
   switch (hmm->statetype)
    {
     case         STATE_GAUSSIAN:g = (Gaussianptr) hmm->states[i].b;
                                 if (!(g->mean))
                                  {
                                   printf("Gaussian mean of state %d not allocated\n", i);
                                   return 0;
                                  }
                                 if (g->covariance.row != numrealfeatures || g->covariance.col != numrealfeatures)
                                  {
                                   printf("Covariance matrix dimensions %d %d of state %d is not true\n", g->covariance.row, g->covariance.col, i);
                                   return 0;
                                  }
                                 break;
     case STATE_GAUSSIAN_MIXTURE:gm = (Gaussianmixtureptr) hmm->states[i].b;
                                 if (gm->componentcount <= 0)
                                  {
                                   printf("Number of components %d must be larger than 0\n", gm->componentcount);
                                   return 0;
                                  }
                                 sum = sum_of_list_double(gm->weights, gm->componentcount);
                                 if (!dequal(sum - 1, 0))
                                  {
                                   printf("Sum of component weights %.6f must be equal to 1.0", sum);
                                   return 0;
                                  }
                                 if (!(gm->components))
                                  {
                                   printf("Gaussian components not allocated\n");
                                   return 0;
                                  }
                                 for (j = 0; i < gm->componentcount; i++)
                                  {
                                   if (!(gm->components[i].mean))
                                    {
                                     printf("Gaussian mean of mixture component %d of state %d not allocated\n", j, i);
                                     return 0;
                                    }
                                   if (gm->components[i].covariance.row != numrealfeatures || gm->components[i].covariance.col != numrealfeatures)
                                    {
                                     printf("Covariance matrix dimensions %d %d of mixture component %d of state %d is not true\n", g->covariance.row, g->covariance.col, j, i);
                                     return 0;
                                    }
                                  }
                                 break;
     default                    :break;
    }
  }
 printf("Ok...\n");
 return 1;
}

double* add_probability_conserving(double* prob, int count, int added)
{
 /*!Last Changed 05.04.2007*/
 double* result;
 int i;
 result = alloc(prob, (count + added) * sizeof(double), count + added);
 for (i = 0; i < count; i++)
   result[i] *= (count + 0.0) / (count + added);
 for (i = count; i < count + added; i++)
   result[i] = 1.0 / (count + added);
 return result;
}

void add_component(Hmmptr hmm, int count, Instanceptr traindata)
{
 /*!Last Changed 06.04.2007*/
 int i, j;
 Gaussianmixtureptr gm;
 Dirichletmixtureptr dm;
 for (i = 0; i < hmm->statecount; i++)
   switch (hmm->statetype)
    {
     case  STATE_GAUSSIAN_MIXTURE:gm = (Gaussianmixtureptr) hmm->states[i].b;
                                  gm->weights = add_probability_conserving(gm->weights, gm->componentcount, count);
                                  gm->components = alloc(gm->components, (gm->componentcount + count) * sizeof(Gaussian), gm->componentcount + count);
                                  for (j = gm->componentcount; j < gm->componentcount + count; j++)
                                    random_gaussian(&(gm->components[j]), traindata);
                                  gm->componentcount += count;
                                  break;
     case STATE_DIRICHLET_MIXTURE:dm = (Dirichletmixtureptr) hmm->states[i].b;
                                  dm->weights = add_probability_conserving(dm->weights, dm->componentcount, count);
                                  dm->components = alloc(dm->components, (dm->componentcount + count) * sizeof(Dirichlet), dm->componentcount + count);
                                  for (j = dm->componentcount; j < dm->componentcount + count; j++)
                                    random_dirichlet(&(dm->components[j]), traindata);
                                  dm->componentcount += count;
                                  break;
     default                     :break;
    }
}

void add_state(Hmmptr hmm, int count, Instanceptr traindata)
{
 /*!Last Changed 06.04.2007*/
 int i, j, componentcount;
 double* pi, randvalue;
 hmm->states = alloc(hmm->states, (hmm->statecount + count) * sizeof(State), hmm->statecount + count);
 /*Reshuffle pi's*/
 pi = safemalloc(hmm->statecount * sizeof(double), "add_state", 5);
 for (i = 0; i < hmm->statecount; i++)
   pi[i] = hmm->states[i].pi;
 pi = add_probability_conserving(pi, hmm->statecount, count);
 for (i = 0; i < hmm->statecount + count; i++)
   hmm->states[i].pi = pi[i];   
 safe_free(pi);
 /*Reshuffle a_i_dot*/
 switch (hmm->hmmtype)
  {
   case    FULL_MODEL:for (i = 0; i < hmm->statecount; i++)
                        hmm->states[i].a_i_dot = add_probability_conserving(hmm->states[i].a_i_dot, hmm->statecount, count);
                      for (i = hmm->statecount; i < hmm->statecount + count; i++)
                        hmm->states[i].a_i_dot = random_normalized_array(hmm->statecount + count);
                      break;
   case      LR_MODEL:
   case LR_LOOP_MODEL:for (i = 0; i < hmm->statecount; i++)
                       {
                        hmm->states[i].a_i_dot = alloc(hmm->states[i].a_i_dot, (hmm->statecount + count) * sizeof(double), hmm->statecount + count);
                        for (j = hmm->statecount; j < hmm->statecount + count; j++)
                          hmm->states[i].a_i_dot[j] = 0.0;
                       }
                      for (i = hmm->statecount; i < hmm->statecount + count; i++)
                        hmm->states[i].a_i_dot = safecalloc(hmm->statecount + count, sizeof(double), "add_state", 27);
                      for (i = hmm->statecount - 1; i < hmm->statecount + count - 1; i++)
                       {
                        randvalue = produce_random_value(0.0, 1.0);
                        hmm->states[i].a_i_dot[i] = randvalue;
                        hmm->states[i].a_i_dot[i + 1] = 1 - randvalue;
                       }
                      if (hmm->hmmtype == LR_MODEL)
                        hmm->states[hmm->statecount + count - 1].a_i_dot[hmm->statecount + count - 1] = 1.0;
                      else
                       {
                        hmm->states[hmm->statecount - 1].a_i_dot[0] = 0.0;
                        randvalue = produce_random_value(0.0, 1.0);
                        hmm->states[hmm->statecount + count - 1].a_i_dot[hmm->statecount + count - 1] = randvalue;
                        hmm->states[hmm->statecount + count - 1].a_i_dot[0] = 1 - randvalue;
                       }
                      break;
  }
 /*Reshuffle a_dot_j*/
 for (i = 0; i < hmm->statecount; i++)
   hmm->states[i].a_dot_j = alloc(hmm->states[i].a_dot_j, (hmm->statecount + count) * sizeof(double), hmm->statecount + count);
 for (i = hmm->statecount; i < hmm->statecount + count; i++)
   hmm->states[i].a_dot_j = safemalloc((hmm->statecount + count) * sizeof(double), "add_state", 49);
 for (i = 0; i < hmm->statecount + count; i++)
   for (j = 0; j < hmm->statecount + count; j++)
     hmm->states[j].a_dot_j[i] = hmm->states[i].a_i_dot[j];
 /*Add b's*/
 for (i = hmm->statecount; i < hmm->statecount + count; i++)
  {
   componentcount = get_component_count(hmm->states[0], hmm->statetype);
   hmm->states[i].b = random_state_model(hmm->statetype, componentcount, traindata);
  }
 /*Update statecount*/
 hmm->statecount += count;
}

void free_state_model(void* model, HMM_STATE_TYPE statetype)
{
 /*!Last Changed 26.02.2007*/
 Gaussianptr g;
 Gaussianmixtureptr gm;
 Multinomialptr m;
 Dirichletptr d;
 Dirichletmixtureptr dm;
 int i;
 switch (statetype)
  {
   case          STATE_DISCRETE:m = (Multinomialptr) model;
                                for (i = 0; i < current_dataset->featurecount - 1; i++)
                                  safe_free(m->prob[i]);
                                safe_free(m->prob);
                                break;
   case          STATE_GAUSSIAN:g = (Gaussianptr) model;
                                safe_free(g->mean);
                                matrix_free(g->covariance);
                                break;
   case  STATE_GAUSSIAN_MIXTURE:gm = (Gaussianmixtureptr) model;
                                safe_free(gm->weights);
                                for (i = 0; i < gm->componentcount; i++)
                                 {
                                  safe_free(gm->components[i].mean);
                                  matrix_free(gm->components[i].covariance);
                                 }
                                safe_free(gm->components);
                                break;
   case         STATE_DIRICHLET:d = (Dirichletptr) model;
                                safe_free(d->alpha);
                                break;
   case STATE_DIRICHLET_MIXTURE:dm = (Dirichletmixtureptr) model;
                                safe_free(dm->weights);
                                for (i = 0; i < dm->componentcount; i++)
                                  safe_free(dm->components[i].alpha);
                                safe_free(dm->components);
                                break;
  }
 safe_free(model);
}

void* random_state_model(HMM_STATE_TYPE statetype, int componentcount, Instanceptr traindata)
{
 /*!Last Changed 27.03.2007*/
 Gaussianptr g;
 Gaussianmixtureptr gm;
 Multinomialptr m;
 Dirichletptr d;
 Dirichletmixtureptr dm;
 int i, j;
 switch (statetype)
  {
   case          STATE_DISCRETE:m = safemalloc(sizeof(Multinomial), "random_state_model", 10);
                                m->prob = safemalloc(sizeof(double*) * (current_dataset->featurecount - 1), "random_state_model", 11);
                                j = 0;
                                for (i = 0; i < current_dataset->featurecount; i++)
                                  if (current_dataset->features[i].type == STRING)
                                   {
                                    m->prob[j] = random_normalized_array(current_dataset->features[i].valuecount);
                                    j++;
                                   }
                                return m;
                                break;
   case          STATE_GAUSSIAN:g = safemalloc(sizeof(Gaussian), "random_state_model", 21);
                                random_gaussian(g, traindata);
                                return g;
                                break;
   case  STATE_GAUSSIAN_MIXTURE:gm = safemalloc(sizeof(Gaussianmixture), "random_state_model", 26);
                                gm->componentcount = componentcount;
                                gm->weights = random_normalized_array(gm->componentcount);
                                gm->components = safemalloc(componentcount * sizeof(Gaussian), "random_state_model", 29);
                                for (i = 0; i < gm->componentcount; i++)
                                  random_gaussian(&(gm->components[i]), traindata);
                                return gm;
                                break;
   case         STATE_DIRICHLET:d = safemalloc(sizeof(Dirichlet), "random_state_model", 37);
                                random_dirichlet(d, traindata);
                                return d;
                                break;
   case STATE_DIRICHLET_MIXTURE:dm = safemalloc(sizeof(Dirichletmixture), "random_state_model", 41);
                                dm->componentcount = componentcount;
                                dm->weights = random_normalized_array(dm->componentcount);
                                dm->components = safemalloc(componentcount * sizeof(Dirichlet), "random_state_model", 44);
                                for (i = 0; i < dm->componentcount; i++)
                                  random_dirichlet(&(dm->components[i]), traindata);
                                return dm;
                                break;
   default                     :return NULL;
  }
}

void* alloc_state_model(HMM_STATE_TYPE statetype, int componentcount)
{
 /*!Last Changed 15.03.2007*/
 Gaussianptr g;
 Gaussianmixtureptr gm;
 Multinomialptr m;
 Dirichletptr d;
 Dirichletmixtureptr dm;
 int i, j;
 switch (statetype)
  {
   case          STATE_DISCRETE:m = safemalloc(sizeof(Multinomial), "alloc_state_model", 10);
                                m->prob = safemalloc(sizeof(double*) * (current_dataset->featurecount - 1), "alloc_state_model", 10);
                                j = 0;
                                for (i = 0; i < current_dataset->featurecount; i++)
                                  if (current_dataset->features[i].type == STRING)
                                   {
                                    m->prob[j] = safemalloc(sizeof(double) * current_dataset->features[i].valuecount, "alloc_state_model", 16);
                                    j++;
                                   }
                                return m;
                                break;
   case          STATE_GAUSSIAN:g = safemalloc(sizeof(Gaussian), "alloc_state_model", 21);
                                alloc_gaussian(g);
                                return g;
                                break;
   case  STATE_GAUSSIAN_MIXTURE:gm = safemalloc(sizeof(Gaussianmixture), "alloc_state_model", 26);
                                gm->componentcount = componentcount;
                                gm->weights = safemalloc(sizeof(double) * gm->componentcount, "alloc_state_model", 28);
                                gm->components = safemalloc(gm->componentcount * sizeof(Gaussian), "alloc_state_model", 29);
                                for (i = 0; i < gm->componentcount; i++)
                                  alloc_gaussian(&(gm->components[i]));
                                return gm;
                                break;
   case         STATE_DIRICHLET:d = safemalloc(sizeof(Dirichlet), "alloc_state_model", 37);
                                alloc_dirichlet(d);
                                return d;
                                break;
   case STATE_DIRICHLET_MIXTURE:dm = safemalloc(sizeof(Dirichletmixture), "alloc_state_model", 41);
                                dm->componentcount = componentcount;
                                dm->weights = safemalloc(sizeof(double) * dm->componentcount, "alloc_state_model", 43);
                                dm->components = safemalloc(componentcount * sizeof(Dirichlet), "alloc_state_model", 44);
                                for (i = 0; i < dm->componentcount; i++)
                                  alloc_dirichlet(&(dm->components[i]));
                                return dm;
                                break;
      default                  :return NULL;
  }
}

void* calloc_state_model(HMM_STATE_TYPE statetype, int componentcount)
{
 /*!Last Changed 15.03.2007*/
 Gaussianptr g;
 Gaussianmixtureptr gm;
 Multinomialptr m;
 Dirichletptr d;
 Dirichletmixtureptr dm;
 int i, j;
 switch (statetype)
  {
   case          STATE_DISCRETE:m = safemalloc(sizeof(Multinomial), "calloc_state_model", 10);
                                m->prob = safemalloc(sizeof(double*) * (current_dataset->featurecount - 1), "calloc_state_model", 11);
                                j = 0;
                                for (i = 0; i < current_dataset->featurecount; i++)
                                 if (current_dataset->features[i].type == STRING)
                                  {
                                   m->prob[j] = safecalloc(current_dataset->features[i].valuecount, sizeof(double), "calloc_state_model", 16);
                                   j++;
                                  }
                                return m;
                                break;
   case          STATE_GAUSSIAN:g = safemalloc(sizeof(Gaussian), "calloc_state_model", 21);
                                alloc_gaussian(g);
                                initialize_matrix(&(g->covariance), 0.0);
                                return g;
                                break;
   case  STATE_GAUSSIAN_MIXTURE:gm = safemalloc(sizeof(Gaussianmixture), "calloc_state_model", 27);
                                gm->componentcount = componentcount;
                                gm->weights = safecalloc(gm->componentcount, sizeof(double), "calloc_state_model", 29);
                                gm->components = safemalloc(componentcount * sizeof(Gaussian), "calloc_state_model", 30);
                                for (i = 0; i < gm->componentcount; i++)
                                 {
                                  alloc_gaussian(&(gm->components[i]));
                                  initialize_matrix(&(gm->components[i].covariance), 0.0);
                                 }
                                return gm;
                                break;
   case         STATE_DIRICHLET:d = safemalloc(sizeof(Dirichlet), "calloc_state_model", 38);
                                alloc_dirichlet(d);
                                return d;
                                break;
   case STATE_DIRICHLET_MIXTURE:dm = safemalloc(sizeof(Dirichletmixture), "calloc_state_model", 42);
                                dm->componentcount = componentcount;
                                dm->weights = safecalloc(dm->componentcount, sizeof(double), "calloc_state_model", 44);
                                dm->components = safemalloc(componentcount * sizeof(Dirichlet), "calloc_state_model", 45);
                                for (i = 0; i < dm->componentcount; i++)
                                  alloc_dirichlet(&(dm->components[i]));
                                return dm;
                                break;
      default                  :return NULL;
  }
}

void* clone_state_model(void* model, HMM_STATE_TYPE statetype)
{
 /*!Last Changed 15.03.2007*/
 Gaussianptr g, g1;
 Gaussianmixtureptr gm, gm1;
 Multinomialptr m, m1;
 Dirichletptr d, d1;
 Dirichletmixtureptr dm, dm1;
 int i, j;
 int numrealfeatures = current_dataset->multifeaturecount - 1;
 switch (statetype)
  {
   case          STATE_DISCRETE:m = (Multinomialptr) model;
                                m1 = safemalloc(sizeof(Multinomial), "clone_state_model", 11);
                                m1->prob = safemalloc(sizeof(double*) * (current_dataset->featurecount - 1), "clone_state_model", 11);
                                j = 0;
                                for (i = 0; i < current_dataset->featurecount; i++)
                                  if (current_dataset->features[i].type == STRING)
                                   {
                                    m1->prob[j] = clone_array(m->prob[j],  current_dataset->features[i].valuecount);
                                    j++;
                                   }
                                return m1;
                                break;
   case          STATE_GAUSSIAN:g = (Gaussianptr) model;
                                g1 = safemalloc(sizeof(Gaussian), "clone_state_model", 23);
                                g1->mean = clone_array(g->mean, numrealfeatures);
                                g1->covariance = matrix_copy(g->covariance);
                                return g1;
                                break;
   case  STATE_GAUSSIAN_MIXTURE:gm = (Gaussianmixtureptr) model;
                                gm1 = safemalloc(sizeof(Gaussianmixture), "clone_state_model", 29);
                                gm1->componentcount = gm->componentcount;
                                gm1->weights = clone_array(gm->weights, gm->componentcount);
                                gm1->components = safemalloc(gm->componentcount * sizeof(Gaussian), "clone_state_model", 32);
                                for (i = 0; i < gm->componentcount; i++)
                                 {
                                  gm1->components[i].mean = clone_array(gm->components[i].mean, numrealfeatures);
                                  gm1->components[i].covariance = matrix_copy(gm->components[i].covariance);
                                 }
                                return gm1;
                                break;
   case         STATE_DIRICHLET:d = (Dirichletptr) model;
                                d1 = safemalloc(sizeof(Dirichlet), "clone_state_model", 40);
                                d1->alpha = clone_array(d->alpha, numrealfeatures);
                                return d1;
                                break;
   case STATE_DIRICHLET_MIXTURE:dm = (Dirichletmixtureptr) model;
                                dm1 = safemalloc(sizeof(Dirichletmixture), "clone_state_model", 45);
                                dm1->componentcount = dm->componentcount;
                                dm1->weights = clone_array(dm->weights, dm->componentcount);
                                dm1->components = safemalloc(dm->componentcount * sizeof(Dirichlet), "clone_state_model", 48);
                                for (i = 0; i < dm->componentcount; i++)
                                  dm1->components[i].alpha = clone_array(dm->components[i].alpha, numrealfeatures);
                                return dm1;
                                break;
      default                  :return NULL;
  }
}

void init_state_model(void *model, HMM_STATE_TYPE statetype)
{
 /*!Last Changed 15.03.2007*/
 Gaussianptr g;
 Gaussianmixtureptr gm;
 Multinomialptr m;
 Dirichletptr d;
 Dirichletmixtureptr dm;
 int i, j;
 int numrealfeatures = current_dataset->multifeaturecount - 1;
 switch (statetype)
  {
   case          STATE_DISCRETE:m = (Multinomialptr) model;
                                j = 0;
                                for (i = 0; i < current_dataset->featurecount; i++)
                                  if (current_dataset->features[i].type == STRING)
                                   {
                                    init_array(m->prob[j], current_dataset->features[i].valuecount);
                                    j++;
                                   }
                                break;
   case          STATE_GAUSSIAN:g = (Gaussianptr) model;
                                init_array(g->mean, numrealfeatures);
                                initialize_matrix(&(g->covariance), 0.0);
                                break;
   case  STATE_GAUSSIAN_MIXTURE:gm = (Gaussianmixtureptr) model;
                                init_array(gm->weights, gm->componentcount);
                                for (i = 0; i < gm->componentcount; i++)
                                 {
                                  init_array(gm->components[i].mean, numrealfeatures);
                                  initialize_matrix(&(gm->components[i].covariance), 0.0);
                                 }
                                break;
   case         STATE_DIRICHLET:d = (Dirichletptr) model;
                                init_array(d->alpha, numrealfeatures);
                                break;
   case STATE_DIRICHLET_MIXTURE:dm = (Dirichletmixtureptr) model;
                                init_array(dm->weights, dm->componentcount);
                                for (i = 0; i < dm->componentcount; i++)
                                  init_array(dm->components[i].alpha, numrealfeatures);
                                break;
  }
}

void copy_state_model(void *destmodel, void* srcmodel, HMM_STATE_TYPE statetype)
{
 /*!Last Changed 15.03.2007*/
 Gaussianptr g, g1;
 Gaussianmixtureptr gm, gm1;
 Multinomialptr m, m1;
 Dirichletptr d, d1;
 Dirichletmixtureptr dm, dm1;
 int i, j;
 int numrealfeatures = current_dataset->multifeaturecount - 1;
 switch (statetype)
  {
   case          STATE_DISCRETE:m = (Multinomialptr) destmodel;
                                m1 = (Multinomialptr) srcmodel;
                                j = 0;
                                for (i = 0; i < current_dataset->featurecount; i++)
                                  if (current_dataset->features[i].type == STRING)
                                   {
                                    memcpy(m->prob[j], m1->prob[j], current_dataset->features[i].valuecount * sizeof(double));
                                    j++;
                                   }
                                break;
   case          STATE_GAUSSIAN:g = (Gaussianptr) destmodel;
                                g1 = (Gaussianptr) srcmodel;
                                memcpy(g->mean, g1->mean, numrealfeatures * sizeof(double));
                                matrix_identical(&(g->covariance), g1->covariance);
                                break;
   case  STATE_GAUSSIAN_MIXTURE:gm = (Gaussianmixtureptr) destmodel;
                                gm1 = (Gaussianmixtureptr) srcmodel;
                                memcpy(gm->weights, gm1->weights, gm->componentcount * sizeof(double));
                                for (i = 0; i < gm->componentcount; i++)
                                 {
                                  memcpy(gm->components[i].mean, gm1->components[i].mean, numrealfeatures * sizeof(double));
                                  matrix_identical(&(gm->components[i].covariance), gm1->components[i].covariance);
                                 }
                                break;
   case         STATE_DIRICHLET:d = (Dirichletptr) destmodel;
                                d1 = (Dirichletptr) srcmodel;
                                memcpy(d->alpha, d1->alpha, numrealfeatures * sizeof(double));
                                break;
   case STATE_DIRICHLET_MIXTURE:dm = (Dirichletmixtureptr) destmodel;
                                dm1 = (Dirichletmixtureptr) srcmodel;
                                memcpy(dm->weights, dm1->weights, dm->componentcount * sizeof(double));
                                for (i = 0; i < dm->componentcount; i++)
                                  memcpy(dm->components[i].alpha, dm1->components[i].alpha, numrealfeatures * sizeof(double));
                                break;
  }
}

void sum_state_model(void *destmodel, void* srcmodel, HMM_STATE_TYPE statetype)
{
 /*!Last Changed 15.03.2007*/
 Gaussianptr g, g1;
 Gaussianmixtureptr gm, gm1;
 Multinomialptr m, m1;
 Dirichletptr d, d1;
 Dirichletmixtureptr dm, dm1;
 int i, j;
 int numrealfeatures = current_dataset->multifeaturecount - 1;
 switch (statetype)
  {
   case          STATE_DISCRETE:m = (Multinomialptr) destmodel;
                                m1 = (Multinomialptr) srcmodel;
                        		      j = 0;
                                for (i = 0; i < current_dataset->featurecount; i++)
                                  if (current_dataset->features[i].type == STRING)
                                   {
                                    add_arrays(m->prob[j], m1->prob[j], current_dataset->features[i].valuecount);
                              		    j++;
                                   }
                                break;
   case          STATE_GAUSSIAN:g = (Gaussianptr) destmodel;
                                g1 = (Gaussianptr) srcmodel;
                                add_arrays(g->mean, g1->mean, numrealfeatures);
                                matrix_add(&(g->covariance), g1->covariance);
                                break;
   case  STATE_GAUSSIAN_MIXTURE:gm = (Gaussianmixtureptr) destmodel;
                                gm1 = (Gaussianmixtureptr) srcmodel;
                                add_arrays(gm->weights, gm1->weights, gm->componentcount);
                                for (i = 0; i < gm->componentcount; i++)
                                 {
                                  add_arrays(gm->components[i].mean, gm1->components[i].mean, numrealfeatures);
                                  matrix_add(&(gm->components[i].covariance), gm1->components[i].covariance);
                                 }
                                break;
   case         STATE_DIRICHLET:d = (Dirichletptr) destmodel;
                                d1 = (Dirichletptr) srcmodel;
                                add_arrays(d->alpha, d1->alpha, numrealfeatures);
                                break;
   case STATE_DIRICHLET_MIXTURE:dm = (Dirichletmixtureptr) destmodel;
                                dm1 = (Dirichletmixtureptr) srcmodel;
                                add_arrays(dm->weights, dm1->weights, dm->componentcount);
                                for (i = 0; i < dm->componentcount; i++)
                                  add_arrays(dm->components[i].alpha, dm1->components[i].alpha, numrealfeatures);
                                break;
  }
}

void multiply_constant_state_model(void *model, double mul, HMM_STATE_TYPE statetype)
{
 /*!Last Changed 15.03.2007*/
 Gaussianptr g;
 Gaussianmixtureptr gm;
 Multinomialptr m;
 Dirichletptr d;
 Dirichletmixtureptr dm;
 int i, j;
 int numrealfeatures = current_dataset->multifeaturecount - 1;
 switch (statetype)
  {
   case          STATE_DISCRETE:m = (Multinomialptr) model;
                                j = 0;
                                for (i = 0; i < current_dataset->featurecount; i++)
                                  if (current_dataset->features[i].type == STRING)
                                   {
                                    multiply_array(m->prob[j], mul, current_dataset->features[i].valuecount);
                                    j++;
                                   }
                                break;
   case          STATE_GAUSSIAN:g = (Gaussianptr) model;
                                multiply_array(g->mean, mul, numrealfeatures);
                                matrix_multiply_constant(&(g->covariance), mul);
                                break;
   case  STATE_GAUSSIAN_MIXTURE:gm = (Gaussianmixtureptr) model;
                                multiply_array(gm->weights, mul, numrealfeatures);
                                for (i = 0; i < gm->componentcount; i++)
                                 {
                                  multiply_array(gm->components[i].mean, mul, numrealfeatures);
                                  matrix_multiply_constant(&(gm->components[i].covariance), mul);
                                 }
                                break;
   case         STATE_DIRICHLET:d = (Dirichletptr) model;
                                multiply_array(d->alpha, mul, numrealfeatures);
                                break;
   case STATE_DIRICHLET_MIXTURE:dm = (Dirichletmixtureptr) model;
                                multiply_array(dm->weights, mul, dm->componentcount);
                                for (i = 0; i < dm->componentcount; i++)
                                  multiply_array(dm->components[i].alpha, mul, numrealfeatures);
                                break;
  }
}

void update_state_model(void *model, double accum, double *accumMix, HMM_STATE_TYPE statetype)
{
 /*!Last Changed 27.03.2007*/
 Gaussianptr g;
 Gaussianmixtureptr gm;
 Multinomialptr m;
 Dirichletptr d;
 Dirichletmixtureptr dm;
 int i, j;
 double mul = (1.0 / accum);
 int numrealfeatures = current_dataset->multifeaturecount - 1;
 matrix covprior;
 switch (statetype)
  {
   case          STATE_DISCRETE:m = (Multinomialptr) model;
                                j = 0;
                                for (i = 0; i < current_dataset->featurecount; i++)
                                  if (current_dataset->features[i].type == STRING)
                                   {
                                    multiply_array(m->prob[j], mul, current_dataset->features[i].valuecount);
                                    j++;
                                   }
                                break;
   case          STATE_GAUSSIAN:g = (Gaussianptr) model;
                                multiply_array(g->mean, mul, numrealfeatures);
                                matrix_multiply_constant(&(g->covariance), mul);
                                covprior = identity(g->covariance.row);
                                matrix_multiply_constant(&(covprior), PRIOR);
                                matrix_add(&(g->covariance), covprior);
                                matrix_free(covprior);
                                break;
   case  STATE_GAUSSIAN_MIXTURE:gm = (Gaussianmixtureptr) model;
                                multiply_array(gm->weights, mul, gm->componentcount);
                                for (i = 0; i < gm->componentcount; i++)
                                 {
                                  multiply_array(gm->components[i].mean, (1.0 / accumMix[i]), numrealfeatures);
                                  matrix_multiply_constant(&(gm->components[i].covariance), (1.0 / accumMix[i]));
                                  covprior = identity(gm->components[i].covariance.row);
                                  matrix_multiply_constant(&(covprior), PRIOR);
                                  matrix_add(&(gm->components[i].covariance), covprior);
                                  matrix_free(covprior);
                                 }
                                break;
   case         STATE_DIRICHLET:d = (Dirichletptr) model;
                                multiply_array(d->alpha, mul, numrealfeatures);
                                break;
   case STATE_DIRICHLET_MIXTURE:dm = (Dirichletmixtureptr) model;
                                multiply_array(dm->weights, mul, dm->componentcount);
                                for (i = 0; i < dm->componentcount; i++)
                                  multiply_array(dm->components[i].alpha, (1.0 / accumMix[i]), numrealfeatures);
                                break;
  }
}

void alloc_gaussian(Gaussianptr g)
{
 /*!Last Changed 07.04.2007*/
 int numrealfeatures = current_dataset->multifeaturecount - 1;
 g->mean = safecalloc(numrealfeatures, sizeof(double), "alloc_gaussian", 2);
 g->covariance = matrix_alloc(numrealfeatures, numrealfeatures);
}

void alloc_dirichlet(Dirichletptr d)
{
 /*!Last Changed 07.04.2007*/
 int numrealfeatures = current_dataset->multifeaturecount - 1;
 d->alpha = safecalloc(numrealfeatures, sizeof(double), "alloc_dirichlet", 2);
}

void random_gaussian(Gaussianptr g, Instanceptr traindata)
{
 /*!Last Changed 07.04.2007*/
 int numrealfeatures = current_dataset->multifeaturecount - 1;
 int size, index, seqlen, i, t;
 matrix m;
 Instanceptr datapoint, tmp;
 size = data_size(traindata);
 index = myrand() % size;
 datapoint = data_x(traindata, index);
 seqlen = frame_count(datapoint);
 m = matrix_alloc(seqlen, numrealfeatures);
 tmp = datapoint;
 t = 0;
 while (tmp)
  {
   for (i = 0; i < numrealfeatures; i++)
     m.values[t][i] = real_feature(tmp, i);
   if (t == 0)
     tmp = tmp->sublist;
   else
     tmp = tmp->next;
   t++;
  }
 g->mean = matrix_average_columns(m);
 g->covariance = identity(numrealfeatures);
 matrix_free(m);
}

void random_dirichlet(Dirichletptr d, Instanceptr traindata)
{
 /*!Last Changed 07.04.2007*/
 int numrealfeatures = current_dataset->multifeaturecount - 1;
 d->alpha = random_array_double(numrealfeatures, 0.0, 1.0);
}

int get_component_count(State s, HMM_STATE_TYPE statetype)
{
 /*!Last Changed 06.04.2007*/
 if (statetype == STATE_GAUSSIAN_MIXTURE)
   return ((Gaussianmixtureptr) s.b)->componentcount;
 else 
   if (statetype == STATE_DIRICHLET_MIXTURE)
     return ((Dirichletmixtureptr) s.b)->componentcount;
   else
     return 0;
}

double multinomial_multivariate(Instanceptr inst, Multinomialptr m)
{
 /*!Last Changed 26.02.2007*/
 double prod = 1.0;
 int i, j = 0;
 for (i = 0; i < current_dataset->featurecount; i++)
   if (current_dataset->features[i].type == STRING)
    {
     prod *= m->prob[j][inst->values[i].stringindex];
     j++; 
    } 
 return prod;
}

void generate_gaussian_mixture_data(Gaussianmixtureptr gm, int* classes, int datacount, char* outfilename)
{
 /*!Last Changed 28.04.2007*/
 FILE* outfile;
 char pst[READLINELENGTH];
 int i, dimension, j, componentindex;
 matrix a, z, result;
 double value;
 set_precision(pst, NULL, " "); 
 dimension = gm->components[0].covariance.row;
 outfile = fopen(outfilename, "w");
 a = matrix_alloc(1, dimension);
 for (i = 0; i < datacount; i++)
  {
   for (j = 0; j < dimension; j++)
     a.values[0][j] = produce_normal_value(0.0, 1.0);
   value = produce_random_value(0, 1);
   componentindex = find_bin(gm->weights, gm->componentcount, value);
   cholesky_decomposition(gm->components[componentindex].covariance, &z);
   result = matrix_multiply(a, z);
   for (j = 0; j < dimension; j++)
     fprintf(outfile, pst, result.values[0][j] + gm->components[componentindex].mean[j]);
   if (gm->componentcount > 1)
     fprintf(outfile, "%d\n", classes[componentindex]);
   else 
     fprintf(outfile, "\n");
   matrix_free(z);
   matrix_free(result);
  }
 matrix_free(a);
 fclose(outfile);
}

Gaussianmixtureptr read_gaussian_mixture(char* filename, int** classes)
{ 
 /*!Last Changed 28.04.2007*/
 Gaussianmixtureptr gm;
 FILE* infile;
 int i, dimension;
 infile = fopen(filename, "r");
 if (!infile)
   return NULL;
 fscanf(infile, "%d", &dimension);
 gm = safemalloc(sizeof(Gaussianmixture), "read_gaussian_mixture", 7);
 fscanf(infile, "%d", &(gm->componentcount));
 *classes = safemalloc(gm->componentcount * sizeof(int), "read_gaussian_mixture", 9);
 gm->weights = safemalloc(gm->componentcount * sizeof(double), "read_gaussian_mixture", 10);
 gm->components = safemalloc(gm->componentcount * sizeof(Gaussian), "read_gaussian_mixture", 11);
 for (i = 0; i < gm->componentcount; i++)
   fscanf(infile, "%lf", &(gm->weights[i]));
 for (i = 0; i < gm->componentcount; i++)
  {
   fscanf(infile, "%d", &((*classes)[i]));
   gm->components[i].mean = load_array(infile, dimension);
   gm->components[i].covariance = load_matrix(infile, dimension, dimension);
  }
 fclose(infile);
 return gm;
}

double gaussian_mixture_multivariate(Instanceptr inst, Gaussianmixtureptr gm)
{
 /*!Last Changed 26.02.2007*/
 double sum = 0.0;
 int i;
 for (i = 0; i < gm->componentcount; i++)
   sum += gm->weights[i] * gaussian_multivariate(inst, gm->components[i].mean, gm->components[i].covariance);
 return sum;
}

double get_gaussian_mixture_multivariate_component(Instanceptr inst, Gaussianmixtureptr gm, int component)
{
 /*!Last Changed 13.03.2007*/
 double sum;
 int i = component;
 sum = gm->weights[i] * gaussian_multivariate(inst, gm->components[i].mean, gm->components[i].covariance);
 return sum;
}

double dirichlet_mixture_multivariate(Instanceptr inst, Dirichletmixtureptr dm)
{
 /*!Last Changed 28.02.2007*/
 double sum = 0.0;
 int i;
 for (i = 0; i < dm->componentcount; i++)
   sum += dm->weights[i] * dirichlet(inst, dm->components[i].alpha);
 return sum;
}

double get_dirichlet_mixture_multivariate_component(Instanceptr inst, Dirichletmixtureptr dm, int component)
{
 /*!Last Changed 28.02.2007*/
 double sum;
 int i = component;
 sum = dm->weights[i] * dirichlet(inst, dm->components[i].alpha);
 return sum;
}

double calc_obslik(Hmmptr hmm, Instanceptr inst, int stateindex)
{
 /*!Last Changed 26.03.2007*/
 double obslik;
 Gaussianptr g;
 Gaussianmixtureptr gm;
 Multinomialptr m;
 Dirichletptr d;
 Dirichletmixtureptr dm;
 switch (hmm->statetype)
  {
   case          STATE_DISCRETE:m = (Multinomialptr) (hmm->states[stateindex].b);
                                obslik = multinomial_multivariate(inst, m);
                                break;
   case          STATE_GAUSSIAN:g = (Gaussianptr) (hmm->states[stateindex].b);
                                obslik = gaussian_multivariate(inst, g->mean, g->covariance);
                                break;
   case  STATE_GAUSSIAN_MIXTURE:gm = (Gaussianmixtureptr) (hmm->states[stateindex].b);
                                obslik = gaussian_mixture_multivariate(inst, gm);
                                break;
   case         STATE_DIRICHLET:d = (Dirichletptr) (hmm->states[stateindex].b);
                                obslik = dirichlet(inst, d->alpha);
                                break;
   case STATE_DIRICHLET_MIXTURE:dm = (Dirichletmixtureptr) (hmm->states[stateindex].b);
                                obslik = dirichlet_mixture_multivariate(inst, dm);
                                break;
   default                     :obslik = 1.0;
                                break;
  }
 return obslik;
}

double calc_obslikMix(Hmmptr hmm, Instanceptr inst, int stateindex, int componentindex)
{
 /*!Last Changed 26.03.2007*/
 double obslik;
 Gaussianmixtureptr gm;
 Dirichletmixtureptr dm;
 switch (hmm->statetype)
  {
   case  STATE_GAUSSIAN_MIXTURE:gm = (Gaussianmixtureptr) (hmm->states[stateindex].b);
                                obslik = get_gaussian_mixture_multivariate_component(inst, gm, componentindex);
                                break;
   case STATE_DIRICHLET_MIXTURE:dm = (Dirichletmixtureptr) (hmm->states[stateindex].b);
                                obslik = get_dirichlet_mixture_multivariate_component(inst, dm, componentindex);
                                break;
   default                     :obslik = 1.0;
                                break;
  }
 return obslik;
}

int* viterbi(Hmmptr hmm, Instanceptr traindata)
{
 /*!Last Changed 25.02.2007 updated for multivariate data*/
 /*!Last Changed 22.01.2007*/
 double **gamma;
 int **phi, *qs;
 int i, j, t, maxindex, seqlen;
 double maxvalue, gamma_initial, obslik;
 double *temp1D2, *temp1D3;
 Instanceptr inst = traindata;
 seqlen = frame_count(traindata);
 gamma = (double **) safecalloc_2d(sizeof(double*), sizeof(double), seqlen, hmm->statecount, "viterbi", 7);
 phi = (int **) safemalloc_2d(sizeof(int*), sizeof(int), seqlen, hmm->statecount, "viterbi", 8);
 qs = (int *) safemalloc(seqlen * sizeof(int), "viterbi", 9);
 for (i = 0; i < hmm->statecount; i++)
  {
   obslik = calc_obslik(hmm, inst, i);
   gamma_initial = safelog(hmm->states[i].pi) + safelog(obslik);
   gamma[0][i] = gamma_initial;
   phi[0][i] = 0;
  }
 t = 1; 
 inst = inst->sublist;
 while (inst)
  {
   for (j = 0; j < hmm->statecount; j++)
    {
     temp1D2 = log_of_array(hmm->states[j].a_dot_j, hmm->statecount);
     temp1D3 = sum_of_two_arrays(gamma[t - 1], temp1D2, hmm->statecount);
     maxindex = findMaxOutput(temp1D3, hmm->statecount);
     maxvalue = temp1D3[maxindex];
     if (maxvalue == 0)
       maxindex = 0;
     obslik = calc_obslik(hmm, inst, j);
     gamma[t][j] = maxvalue + safelog(obslik);
     phi[t][j] = maxindex + 1;
     safe_free(temp1D2);
     safe_free(temp1D3);
    }
   inst = inst->next;
   t++;
  }
 qs[seqlen - 1] = findMaxOutput(gamma[seqlen - 1], hmm->statecount);
 qs[seqlen - 1]++;
 for (i = seqlen - 2; i >= 0; i--)
   qs[i] = phi[i + 1][qs[i + 1] - 1];
 free_2d((void**) gamma, seqlen);
 free_2d((void**) phi, seqlen);
 return qs;
}

double iterate_em_hmm(Hmmptr hmm, Instanceptr traindata, double *accumA, double *accumB, double **accumMix)
{
 /*!Last Changed 26.03.2007*/
 int i, j, k, t, f, m, p, f2 = 0, seqlen;
 int numrealfeatures = current_dataset->multifeaturecount - 1;
 Gaussianptr g;
 Gaussianmixtureptr gm;
 Multinomialptr mn;
 Instanceptr inst;
 double  loglikelihood;
 double  numeratorA, denominatorA, numeratorB, denominatorB, denominatorMC;
 double **alpha, **beta, **gamma, ***xi, *scale, ***gammaMix;
 matrix numeratorM, numeratorC;
 seqlen = frame_count(traindata);

 scale = (double *) safecalloc(seqlen, sizeof(double), "iterate_em_hmm", 13);
 alpha = (double **) safecalloc_2d(sizeof(double*), sizeof(double), seqlen, hmm->statecount, "iterate_em_hmm", 14);
 beta = (double **) safemalloc_2d(sizeof(double*), sizeof(double), seqlen, hmm->statecount, "iterate_em_hmm", 15);
 gamma = (double **) safemalloc_2d(sizeof(double*), sizeof(double), seqlen, hmm->statecount, "iterate_em_hmm", 16);
 xi = (double ***) safemalloc_3d(sizeof(double**), sizeof(double*), sizeof(double), seqlen, hmm->statecount, hmm->statecount, "iterate_em_hmm", 17);
 calc_fwd(hmm, traindata, alpha, &loglikelihood, scale);
 calc_bwd(hmm, traindata, seqlen, beta, scale);
 calc_gamma(hmm, seqlen, alpha, beta, gamma);
 /*initialization t=0*/
 calc_xi(hmm, traindata, seqlen, alpha, beta, xi);
 /* reestimate frequency of state i in time t=1 */
 for (i = 0; i < hmm->statecount; i++) 
   hmm->states[i].pi = WEIGHT_DELTA + (1 - WEIGHT_DELTA) * gamma[0][i];
 /* reestimate transition matrix  and symbol prob in each state */
 for (i = 0; i < hmm->statecount; i++)
  { 
   denominatorA = 0.0;
   for (t = 0; t < seqlen - 1; t++) 
     denominatorA += gamma[t][i];
   accumA[i] += denominatorA;
   for (j = 0; j < hmm->statecount; j++)
    {
     numeratorA = 0.0;
     for (t = 0; t < seqlen - 1; t++) 
       numeratorA += xi[t][i][j];
     hmm->states[j].a_dot_j[i] = WEIGHT_DELTA + (1 - WEIGHT_DELTA) * numeratorA;
     hmm->states[i].a_i_dot[j] = hmm->states[j].a_dot_j[i];
    }
   denominatorB = denominatorA + gamma[seqlen - 1][i];
   accumB[i] += denominatorB;
   switch (hmm->statetype)
    {
     case          STATE_DISCRETE:mn = (Multinomialptr) (hmm->states[i].b);
                                  p = 0;
                                  for (f = 0; f < current_dataset->featurecount; f++)
                                    if (current_dataset->features[f].type == STRING)
                                     {
                                      for (k = 0; k < current_dataset->features[f].valuecount; k++)
                                       {
                                        numeratorB = 0.0;
                                        inst = traindata;
                                        t = 0;
                                        while (inst)
                                         {
                                          if (inst->values[f].stringindex == k)
                                            numeratorB += gamma[t][i];
                                          if (t == 0)
                                            inst = inst->sublist;
                                          else
                                            inst = inst->next;
                                          t++;
                                         }
                                        mn->prob[p][k] = WEIGHT_DELTA + (1 - WEIGHT_DELTA) * numeratorB;
                                       }
                                      p++;
                                     }
                                  break;
     case          STATE_GAUSSIAN:g = (Gaussianptr) (hmm->states[i].b);
                                  numeratorM = matrix_alloc(1, numrealfeatures);
                                  numeratorC = matrix_alloc(numrealfeatures, numrealfeatures);
                                  initialize_matrix(&numeratorM, 0);
                                  initialize_matrix(&numeratorC, 0);
                                  inst = traindata;
                                  t = 0;
                                  while (inst)
                                   {
                                    for (f = 0; f < numrealfeatures; f++)
                                     {
                                      numeratorM.values[0][f] += gamma[t][i] * real_feature(inst, f);
                                      for (f2 = 0; f2 < numrealfeatures; f2++)
                                        numeratorC.values[f][f2] += gamma[t][i] * (real_feature(inst, f) - g->mean[f]) * (real_feature(inst, f2) - g->mean[f2]);
                                     }
                                    if (t == 0)
                                      inst = inst->sublist;
                                    else
                                      inst = inst->next;
                                    t++;
                                   }
                                  memcpy(g->mean, numeratorM.values[0], numeratorM.col * sizeof(double));
                                  matrix_identical(&(g->covariance), numeratorC);
                                  matrix_free(numeratorM);
                                  matrix_free(numeratorC);
                                  break;
     case  STATE_GAUSSIAN_MIXTURE:gm = (Gaussianmixtureptr) (hmm->states[i].b);
                                  gammaMix = (double ***) safemalloc_3d(sizeof(double**), sizeof(double*), sizeof(double), seqlen, hmm->statecount, gm->componentcount, "iterate_em_hmm", 22);
                                  numeratorM = matrix_alloc(1, numrealfeatures);
                                  numeratorC = matrix_alloc(numrealfeatures, numrealfeatures);
                                  calc_gammaMix(hmm, traindata, seqlen, gamma, gammaMix);
                                  for (m = 0; m < gm->componentcount; m++)
                                   {
                                    denominatorMC = 0.0;
                                    initialize_matrix(&numeratorM, 0);
                                    initialize_matrix(&numeratorC, 0);
                                    inst = traindata;
                                    t = 0;
                                    while(inst)
                                     {
                                      denominatorMC += gammaMix[t][i][m];
                                      for (f = 0; f < numrealfeatures; f++)
                                       {
                                        numeratorM.values[0][f] += gammaMix[t][i][m] * real_feature(inst, f);
                                        for (f2 = 0; f2 < numrealfeatures; f2++)
                                          numeratorC.values[f][f2] += gammaMix[t][i][m] * (real_feature(inst, f) - gm->components[m].mean[f]) * (real_feature(inst, f2) - gm->components[m].mean[f2]);
                                       }
                                      if (t == 0)
                                        inst = inst->sublist;
                                      else
                                        inst = inst->next;
                                      t++;
                                     }
                                    accumMix[i][m] = denominatorMC;
                                    memcpy(gm->components[m].mean, numeratorM.values[0], numeratorM.col * sizeof(double));
                                    matrix_identical(&(gm->components[m].covariance), numeratorC);
                                    gm->weights[m] = denominatorMC;
                                   }
                                  matrix_free(numeratorM);
                                  matrix_free(numeratorC);
                                  free_3d((void ***)gammaMix, seqlen, hmm->statecount);
                                  break;
     case         STATE_DIRICHLET:break;
     case STATE_DIRICHLET_MIXTURE:break;
    }
  }
 free_3d((void ***)xi, seqlen, hmm->statecount);
 free_2d((void **)alpha, seqlen);
 free_2d((void **)beta, seqlen);
 free_2d((void **)gamma, seqlen);
 safe_free(scale);
 return loglikelihood;
}

void printcontemission(FILE *controlfile, Gaussianptr g)
{
 int ft, ft2;
 int numrealfeatures = current_dataset->multifeaturecount - 1;
 fprintf(controlfile, "\n mean\n");
 for (ft = 0; ft < numrealfeatures; ft++)
  {
	  fprintf(controlfile, "%.14f ", g->mean[ft]);
  }
 fprintf(controlfile,"\n\ncovariance\n");
 for (ft = 0; ft < numrealfeatures; ft++)
  {
   for (ft2 = 0; ft2 < numrealfeatures; ft2++)
	    fprintf(controlfile, "%.14f ", g->covariance.values[ft][ft2]);
   fprintf(controlfile,"\n");
  }
}

double baumwelch(Hmmptr hmm, Instanceptr traindata, int numiteration)
{
 /*!Last Changed 26.03.2007*/
 int t, s, i, j, iter = 0, numsequence;
 double ll = 0.0, llseq;
 double *accumA, *accumB, **accumMix;
 int componentcount;
 Instanceptr inst;
 Hmmptr hmmiter, hmmtemp; 
 accumA = safemalloc(sizeof(double) * hmm->statecount, "baumwelch", 9);
 accumB = safemalloc(sizeof(double) * hmm->statecount, "baumwelch", 10);
 componentcount = get_component_count(hmm->states[0], hmm->statetype);
 if (componentcount > 0)
   accumMix = (double **) safemalloc_2d(sizeof(double*), sizeof(double), hmm->statecount, componentcount,"baumwelch", 15);
 hmmiter = alloc_hmm(hmm->statecount, hmm->statetype, componentcount, hmm->hmmtype);
 hmmtemp = alloc_hmm(hmm->statecount, hmm->statetype, componentcount, hmm->hmmtype);
 do{
   init_hmm(hmmtemp);
   ll = 0.0;
   t = 0;
   init_array(accumA, hmm->statecount);
   init_array(accumB, hmm->statecount);
   inst = traindata;
   while (inst)
    { 
     copy_hmm(hmmiter, hmm);
     llseq = iterate_em_hmm(hmmiter, inst, accumA, accumB, accumMix);
	    sum_hmm(hmmtemp, hmmiter);
     ll = ll + llseq;
     t++;
     inst = inst->next;
    }
   numsequence = t;
   for (s = 0; s < hmmtemp->statecount; s++)
    {
     hmmtemp->states[s].pi /= numsequence;
     divide_array(hmmtemp->states[s].a_i_dot, accumA[s], hmmtemp->statecount);
     if (componentcount > 0)
       update_state_model(hmmtemp->states[s].b, accumB[s], accumMix[s], hmmtemp->statetype);
     else
       update_state_model(hmmtemp->states[s].b, accumB[s], NULL, hmmtemp->statetype);
    }
   for (i = 0; i < hmm->statecount; i++)
     for (j = 0; j < hmm->statecount; j++)
       hmmtemp->states[j].a_dot_j[i] = hmmtemp->states[i].a_i_dot[j];
   copy_hmm(hmm, hmmtemp);
   iter++;
 }while (iter < numiteration);
 free_hmm(hmmiter);
 free_hmm(hmmtemp);
 safe_free(accumA);
 safe_free(accumB);
 if (componentcount > 0)
   free_2d((void **)accumMix, hmm->statecount);
 return ll;
}

void printgaussianhmm(FILE *controlfile, Hmmptr hmm)
{
 int i, j, st, ft, ft2, numrealfeatures;
 Gaussianptr g;
 numrealfeatures = current_dataset->multifeaturecount - 1;
 fprintf(controlfile, "a :\n");
 for (i = 0; i < hmm->statecount; i++)
 	for (j = 0; j < hmm->statecount; j++)
   {
 		 fprintf(controlfile, "%i %i %f", i, j, hmm->states[i].a_i_dot[j]);
 		 fprintf(controlfile, "\n");
 	 }
 fprintf(controlfile, "Pi :\n");
 for (i = 0; i < hmm->statecount; i++)
   fprintf(controlfile, "%f ", hmm->states[i].pi);
 fprintf(controlfile, "\n");
 fprintf(controlfile, "number of states %d\n", hmm->statecount);
 fprintf(controlfile, "b:\n");
 for (st = 0; st < hmm->statecount; st++)
  {
	  g = (Gaussianptr)(hmm->states[st].b);
 	 fprintf(controlfile, "mean state %d\n", st);
	  for (ft = 0; ft < numrealfeatures; ft++)
 		  fprintf(controlfile,"%f ", g->mean[ft]);
   fprintf(controlfile, "\n"); 
  }
 for (st = 0; st < hmm->statecount; st++)
  {
	  g = (Gaussianptr)(hmm->states[st].b);
 	 fprintf(controlfile, "cov state %d\n", st);
 	 for (ft = 0; ft < numrealfeatures; ft++)
    {
			  for (ft2 = 0; ft2 < numrealfeatures; ft2++)
 				  fprintf(controlfile, "%.25f ", g->covariance.values[ft][ft2]);
     fprintf(controlfile, "\n");
    } 
   fprintf(controlfile, "\n"); 
  }	
 fprintf(controlfile, "\n");
}

void printcontobservation(FILE *controlfile, Instanceptr inst)
{
 int i;
 int numrealfeatures = current_dataset->multifeaturecount - 1;
 Instanceptr inst1 = inst;
 fprintf(controlfile, "observation\n ");
 for (i = 0; i < numrealfeatures; i++)
  {
	  fprintf(controlfile, "inst :%f feature:%d \n ", real_feature(inst1, i), i);
  }
 fprintf(controlfile, "\n\n");
}

void printcontinstance(FILE *controlfile, Instanceptr inst)
{
 int i, instindex;
 Instanceptr inst1 = inst;
 int numrealfeatures = current_dataset->multifeaturecount - 1;
 fprintf(controlfile, "instance\n");
 instindex = 0;
 for (i = 0; i < numrealfeatures; i++)
  {
 	 fprintf(controlfile, "instance %d\n", instindex);
	  fprintf(controlfile, "inst :%f feature:%d \n ", real_feature(inst1, i), i);
  }
 instindex++;
 inst1 = inst1->sublist;
 while (inst1)
  {
 	 for (i = 0; i < numrealfeatures; i++)
 		 {
 			 fprintf(controlfile, "instance %d\n", instindex);
 			 fprintf(controlfile, "inst :%f feature:%d \n", real_feature(inst1, i), i);
 		 }
 		instindex++;
 		inst1 = inst1->next;
  }
 fprintf(controlfile, "\n\n");
}

void printgamma(FILE *controlfile, double **gamma, Hmmptr hmm, int seqlen)
{
 int i, t;
 fprintf(controlfile, "\ngamma:\n");
 for (t = 0; t < seqlen; t++)
  {
 	 fprintf(controlfile, "%d ", t);
 	 for (i = 0; i < hmm->statecount; i++)
 		  fprintf(controlfile, "%.14f ", gamma[t][i]);
 	 fprintf(controlfile, "\n");
  }
}

void printgaussianmixtureemission(FILE *controlfile, Gaussianmixtureptr gm, int numberofcomponents)
{
 int i, j, k;
 int numrealfeatures = current_dataset->multifeaturecount - 1;
 fprintf(controlfile, "\ngaussian mixture:\n mean\n");
 for (i = 0; i < numberofcomponents; i++)
  {
   fprintf(controlfile, " component %d\n ", i + 1);
	  for (j = 0; j < numrealfeatures; j++)
		   fprintf(controlfile, "%.14f ", gm->components[i].mean[j]);
   fprintf(controlfile, "\n");
  }
 fprintf(controlfile, "\ngaussian mixture:\n covariance\n");
 for (i = 0; i < numberofcomponents; i++)
  {
   fprintf(controlfile, " component %d\n", i + 1);
	  for (j = 0; j < numrealfeatures; j++)
    {
		   for (k = 0; k < numrealfeatures; k++)
 	 		  fprintf(controlfile, "%.19f ", gm->components[i].covariance.values[j][k]);
		   fprintf(controlfile, "\n");
		  }
  }
 fprintf(controlfile,"\ngaussian mixture:\n components\n");
 for (i = 0; i < numberofcomponents; i++)
  {
		 fprintf(controlfile, " %.14f ", gm->weights[i]);
  }
 fprintf(controlfile,"\n");
}

Hmmptr best_hmm_for_single(Instanceptr traindata, Instanceptr cvdata, Hmm_parameterptr p)
{
 /*!Last Changed 14.04.2007*/
 double ll, bestll;
 Hmmptr hmm, besthmm;
 hmm = random_hmm(2, p->statetype, 1, p->hmmtype, traindata); 
 baumwelch(hmm, traindata, p->iteration);
 ll = bestll = loglikelihood_of_hmm(hmm, cvdata);
 while (ll >= bestll)
  {
   bestll = ll;
   besthmm = clone_hmm(hmm);
   free_hmm(hmm);
   hmm = apply_operator(besthmm, 0, p, traindata);
   ll = loglikelihood_of_hmm(hmm, cvdata);  
  }
 free_hmm(hmm);
 return besthmm;
}

Hmmptr best_hmm_for_mixture(Instanceptr traindata, Instanceptr cvdata, Hmm_parameterptr p)
{
 /*!Last Changed 14.04.2007*/
 HMM_OPERATOR_TYPE bestone = ADD_ONE_STATE;
 int i;
 double ll[HMM_OPERATORS], bestll;
 Hmmptr hmm[HMM_OPERATORS], besthmm;
 hmm[ADD_ONE_STATE] = random_hmm(2, p->statetype, 1, p->hmmtype, traindata);
 hmm[ADD_ONE_COMPONENT] = clone_hmm(hmm[ADD_ONE_STATE]);
 hmm[ADD_ONE_BOTH] = clone_hmm(hmm[ADD_ONE_STATE]);
 baumwelch(hmm[ADD_ONE_STATE], traindata, p->iteration);
 ll[ADD_ONE_STATE] = bestll = loglikelihood_of_hmm(hmm[ADD_ONE_STATE], cvdata);
 while (bestone != NO_CHANGE)
  {
   besthmm = clone_hmm(hmm[bestone]);
   for (i = 0; i < HMM_OPERATORS; i++)
    {
     free_hmm(hmm[i]);
     hmm[i] = apply_operator(besthmm, i, p, traindata);
    }
   bestone = NO_CHANGE;
   for (i = 0; i < HMM_OPERATORS; i++)
    {     
     ll[i] = loglikelihood_of_hmm(hmm[i], cvdata);  
     if (ll[i] > bestll)
      {
       bestone = i;
       bestll = ll[i];
      }
    }
  }
 for (i = 0; i < HMM_OPERATORS; i++)
   free_hmm(hmm[i]);
 return besthmm;
}

Hmmptr apply_operator(Hmmptr hmm, int operatortype, Hmm_parameterptr p, Instanceptr traindata)
{
 /*!Last Changed 14.04.2007*/
 Hmmptr result;
 int statecount, componentcount;
 if (p->weightfreezing)
  {
   result = clone_hmm(hmm);
   if (in_list(operatortype, 2, ADD_ONE_STATE, ADD_ONE_BOTH))
     add_state(result, 1, traindata);
   if (in_list(operatortype, 2, ADD_ONE_COMPONENT, ADD_ONE_BOTH))
     add_component(result, 1, traindata);
  }
 else
  {
   statecount = hmm->statecount;
   componentcount = get_component_count(hmm->states[0], hmm->statetype);
   if (in_list(operatortype, 2, ADD_ONE_STATE, ADD_ONE_BOTH))
     statecount++;
   if (in_list(operatortype, 2, ADD_ONE_COMPONENT, ADD_ONE_BOTH))
     componentcount++;
   result = random_hmm(statecount, p->statetype, componentcount, p->hmmtype, traindata);
  }
 baumwelch(result, traindata, p->iteration);
 return result;
}

void best_hmm_for_together(Hmmptr* model, Instanceptr* traindata, Instanceptr cvdata, Hmm_parameterptr p)
{
 /*!Last Changed 14.04.2007*/
 Hmmptr candidate, besthmm, tmp;
 int i, j, bestone = 0, operatorcount;
 double besterror, error;
 if (in_list(p->statetype, 3, STATE_DISCRETE, STATE_GAUSSIAN, STATE_DIRICHLET))
   operatorcount = 1;
 else
   operatorcount = 3;
 for (i = 0; i < current_dataset->classno; i++)
  {
   model[i] = random_hmm(2, p->statetype, 1, p->hmmtype, traindata[i]);
   baumwelch(model[i], traindata[i], p->iteration);
  }
 besterror = hmm_error_rate(model, cvdata);
 printf("Start error:%.6f\n", besterror);
 while (bestone != NO_CHANGE)
  {
   bestone = NO_CHANGE;
   besthmm = NULL;
   for (i = 0; i < current_dataset->classno; i++)
     for (j = 0; j < operatorcount; j++)
      {
       candidate = apply_operator(model[i], j, p, traindata[i]);
       tmp = model[i];
       model[i] = candidate;
       error = hmm_error_rate(model, cvdata);
       printf("Candidate error:%.6f\n", error);
       if (error < besterror)
        {
         if (besthmm)
           free_hmm(besthmm);
         besthmm = candidate;
         besterror = error;
         bestone = i;
         printf("Best error:%.6f\n", besterror);
        }
       else
         free_hmm(candidate);
       model[i] = tmp;
      }
   if (bestone != NO_CHANGE)
    {
     free_hmm(model[bestone]);
     model[bestone] = besthmm;
    }
  }
}

double hmm_error_rate(Hmmptr* model, Instanceptr data)
{
 /*!Last Changed 14.04.2007*/
 int wrong = 0, count = 0, real, predicted;
 Instanceptr tmp = data;
 while (tmp)
  {
   real = give_classno(tmp);
   predicted = test_hmm_instance(tmp, model);
   if (real != predicted)
     wrong++;
   tmp = tmp->next;
   count++;
  }
 return wrong / (count + 0.0);
}

int test_hmm_instance(Instanceptr inst, Hmmptr* hmmarray)
{
 /*!Last Changed 14.04.2007*/
 int i, maxclass = -1, seqlen;
 double maxll, ll, **alpha, *scale;
 seqlen = frame_count(inst);
 for (i = 0; i < current_dataset->classno; i++)
  {
   alpha = (double **) safemalloc_2d(sizeof(double*), sizeof(double), seqlen, hmmarray[i]->statecount, "test_hmm_instance", 6);
   scale = safecalloc(seqlen, sizeof(double), "test_hmm_instance", 7);
   calc_fwd(hmmarray[i], inst, alpha, &ll, scale);
   if (maxclass == -1 || ll > maxll)
    {
     maxll = ll;
     maxclass = i;
    }
   free_2d((void **) alpha, seqlen);
   safe_free(scale);
  }
 return maxclass;
}

double loglikelihood_of_hmm(Hmmptr hmm, Instanceptr data)
{
 /*!Last Changed 04.04.2007*/
 double **alpha, *scale, ll, total_loglikelihood = 0.0;
 int seqlen, count = 0;
 Instanceptr inst;
 inst = data;
 while (inst)
  {
   seqlen = frame_count(inst);
   alpha = (double **) safemalloc_2d(sizeof(double*), sizeof(double), seqlen, hmm->statecount, "loglikelihood_of_hmm", 5);
   scale = safecalloc(seqlen, sizeof(double), "loglikelihood_of_hmm", 6);
   calc_fwd(hmm, inst, alpha, &ll, scale);   
   free_2d((void **) alpha, seqlen);
   safe_free(scale);
   total_loglikelihood += ll;
   inst = inst->next;
   count++;
  }
 return total_loglikelihood / count;
}

void calc_fwd(Hmmptr hmm, Instanceptr traindata, double **alpha, double *loglikelihood, double *scale)
{
 /*!Last Changed 13.03.2007*/
 int i, j, t;
 double sumvalue, obslik;
 double *temp1D2;
 Instanceptr inst = traindata;
 /*initialization t=0*/
 for (i = 0; i < hmm->statecount; i++)
  { 
   obslik = calc_obslik(hmm, inst, i);
   alpha[0][i] = hmm->states[i].pi * obslik;
   if (scale)
     scale[0] += alpha[0][i];
  }  
 /*recursion t=1:T-1*/
 t = 1;
 inst = inst->sublist;
 while (inst)
  {
   for (j = 0; j < hmm->statecount; j++)
    {
     temp1D2 = multiply_two_arrays_by_value(alpha[t - 1], hmm->states[j].a_dot_j, hmm->statecount);
     sumvalue = sum_of_list_double(temp1D2, hmm->statecount);
     obslik = calc_obslik(hmm, inst, j);
     alpha[t][j] = sumvalue * obslik;
     if (scale)
       scale[t] += alpha[t][j];
     safe_free(temp1D2);
    }
   inst = inst->next;
   t++;
  }
 if (scale)
  {
  	for (i=0;i<t;i++)
		   for (j = 0; j < hmm->statecount; j++)
		    	alpha[i][j] /= scale[i];
   temp1D2 = log_of_array(scale, hmm->statecount);
   *loglikelihood = sum_of_list_double(temp1D2, hmm->statecount);
   safe_free(temp1D2);
  }
 else
   *loglikelihood = safelog(sum_of_list_double(alpha[t - 1], hmm->statecount));
}

void calc_bwd(Hmmptr hmm, Instanceptr traindata, int seqlen, double **beta, double *scale)
{
 /*!Last Changed 13.03.2007*/
 int i, j, t;
 double sumvalue, obslik;
 double *temp1D2;
 Instanceptr inst;
 /*initialization t = T - 1*/
 for (i = 0; i < hmm->statecount; i++)
   beta[seqlen - 1][i] = 1;  
 /*recursion t=T-2:0*/
 reverse_instancelist(&(traindata->sublist));
 inst = traindata->sublist; /* listeyi ters cevirmek lazim*/
 inst = inst->next;
 t = seqlen - 2; 
 while (t >= 0)
  {
   for (i = 0; i < hmm->statecount; i++)
    {
     temp1D2 = multiply_two_arrays_by_value(beta[t + 1], hmm->states[i].a_i_dot, hmm->statecount);
     for (j = 0; j < hmm->statecount; j++)
      {
       obslik = calc_obslik(hmm, inst, j);
       temp1D2[j] *= obslik;
      }
     sumvalue = sum_of_list_double(temp1D2, hmm->statecount);
     beta[t][i] = sumvalue;
     safe_free(temp1D2);
    }
   t--;
   inst = inst->next;
   if (!inst)
     inst = traindata;
  }
 reverse_instancelist(&(traindata->sublist)); 
 if (scale)
  {
   for (i = 0; i < seqlen; i++)
		   for (j = 0; j < hmm->statecount; j++)
			    beta[i][j] /= scale[i];
  }
}

void calc_gamma(Hmmptr hmm, int seqlen, double **alpha, double **beta, double **gamma)
{
 /*!Last Changed 26.03.2007*/
 int i, j, t;
 double denominator;
 for (t = 0; t < seqlen; t++) 
  {
   denominator = 0.0;
   for (j = 0; j < hmm->statecount; j++) 
    {
     gamma[t][j] = alpha[t][j] * beta[t][j];
     denominator += gamma[t][j];
    }
   for (i = 0; i < hmm->statecount; i++) 
     gamma[t][i] = gamma[t][i] / denominator;
  }
}

void calc_gammaMix(Hmmptr hmm, Instanceptr traindata, int seqlen, double **gamma, double ***gammaMix)
{
 /*!Last Changed 26.03.2007*/
 int j, t, m;
 int componentcount;
 double obslik, obslikMix;
 Instanceptr inst = traindata;
 for (t = 0; t < seqlen; t++) 
  {
   for (j = 0; j < hmm->statecount; j++) 
    {
     obslik = calc_obslik(hmm, inst, j);
     componentcount = get_component_count(hmm->states[j], hmm->statetype);
     for (m = 0; m < componentcount; m++)
      {        
       obslikMix = calc_obslikMix(hmm, inst, j, m);
       gammaMix[t][j][m] = gamma[t][j] * obslikMix / obslik;
      }
    }
   if (t == 0)
     inst = inst->sublist;
   else
     inst = inst->next;
  }
}

void calc_xi(Hmmptr hmm, Instanceptr traindata, int seqlen, double **alpha, double **beta, double ***xi)
{
 /*!Last Changed 26.03.2007*/
 int i, j, t;
 double sum, obslik;
 Instanceptr inst = traindata;
 inst = inst->sublist;
 for (t = 0; t < seqlen - 1; t++) 
  {
   sum = 0.0;  
   for (i = 0; i < hmm->statecount; i++) 
     for (j = 0; j < hmm->statecount; j++) 
      {
       obslik = calc_obslik(hmm, inst, j);
       xi[t][i][j] = alpha[t][i] * beta[t + 1][j] * (hmm->states[i].a_i_dot[j]) * obslik;
       sum += xi[t][i][j];
      }
   for (i = 0; i < hmm->statecount; i++) 
     for (j = 0; j < hmm->statecount; j++)
       xi[t][i][j] /= sum;
   inst = inst->next;
  }
}
