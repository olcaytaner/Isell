#include <limits.h>
#include "divs.h"
#include "instance.h"
#include "instancelist.h"
#include "libarray.h"
#include "libmemory.h"

extern Datasetptr current_dataset;

int condition_cover_instance3(Instanceptr instance, Condition condition)
{
	int fno;
	fno = condition.featureindex;
 switch (current_dataset->features[fno].type)
  {
   case INTEGER:if (condition.comparison == '<' && instance->values[fno].intvalue > condition.value)
                  return NO;
                if (condition.comparison == '>' && instance->values[fno].intvalue < condition.value)
                  return NO;
                break;
   case    REAL:if (condition.comparison == '<' && instance->values[fno].floatvalue > condition.value)
                  return NO;
                if (condition.comparison == '>' && instance->values[fno].floatvalue < condition.value)
                  return NO;
                break;
   case  STRING:if (instance->values[fno].stringindex != condition.value)
                  return NO;
                break;
  }
	return YES;
}

BOOLEAN m_belongs(Instanceptr current, Condition* counter_example, int conditioncount, int M)
{
 int i;
 int NS = 0;
 Condition selector;
 for (i = 0; i < conditioncount; i++)
  {
   selector = counter_example[i];
   if (condition_cover_instance3(current, selector))
    {
     NS++;
     if (NS >= M)
       return BOOLEAN_TRUE;
    }
  }
 return BOOLEAN_FALSE;
}

BOOLEAN epsilon_neighbor(Instanceptr current, Condition** training_example, int rulecount, int* conditioncounts, int M, int epsilon)
{
 int i;
 Condition* counter_example;
 int NI = 0;
 for (i = 0; i < rulecount; i++)
  {
   counter_example = training_example[i];
   if (!m_belongs(current, counter_example, conditioncounts[i], M))
    {
     NI++;
     if (NI > epsilon)
       return BOOLEAN_FALSE;
    }
  }
 return BOOLEAN_TRUE;
}

int classify_using_divs_oracle(Instanceptr current, Model_divsptr model, int M, int epsilon)
{
 int i, *votes, maxclass;
 votes = safecalloc(current_dataset->classno, sizeof(int), "classify_using_divs_oracle", 2);
 for (i = 0; i < model->datacount; i++)
   if (epsilon_neighbor(current, model->hypotheses[i], model->rulecounts[i], model->conditioncounts[i], M, epsilon))
     votes[model->classno[i]]++;
 maxclass = find_max_in_list(votes, current_dataset->classno);
 safe_free(votes);
 return maxclass;
}

void tune_consistency_and_generality(Model_divsptr model, Instanceptr cvdata)
{
 Instanceptr tmp;
 int M, epsilon, error, besterror = +INT_MAX;
 for (epsilon = 0; epsilon <= 25; epsilon += 5)
   for (M = 1; M <= 20; M++)
    {
     error = 0;
     tmp = cvdata;
     while (tmp != NULL)
      {
       if (classify_using_divs_oracle(tmp, model, M, epsilon) != give_classno(tmp))
         error++;
       tmp = tmp->next;
      }
     if (error < besterror)
      {
       besterror = error;
       model->M = M;
       model->epsilon = epsilon;
      }
    }
}

Condition* discriminating_rule(Instanceptr training, Instanceptr counter, int* count)
{
 int i, add;
 char comparison;
 double value;
 Condition* rule = NULL;
 *count = 0;
 for (i = 0; i < current_dataset->featurecount; i++)
  {
   add = NO;
   switch (current_dataset->features[i].type)
    {
     case INTEGER:if (training->values[i].intvalue != counter->values[i].intvalue)
                   {
                    add = YES;
                    value = counter->values[i].intvalue;
                    if (training->values[i].intvalue < counter->values[i].intvalue)
                      comparison = '<';
                    else 
                      comparison = '>';
                   }
                  break;
     case    REAL:if (training->values[i].floatvalue != counter->values[i].floatvalue)
                   {
                    add = YES;
                    value = counter->values[i].floatvalue;
                    if (training->values[i].floatvalue < counter->values[i].floatvalue)
                      comparison = '<';
                    else 
                      comparison = '>';
                   }
                  break;
     case  STRING:if (training->values[i].stringindex != counter->values[i].stringindex)
                   {
                    add = YES;
                    comparison = '=';
                    value = training->values[i].stringindex;
                   }
                  break;
    }
   if (add)
    {
     rule = alloc(rule, ((*count) + 1) * sizeof(Condition), (*count) + 1);
     rule[*count].featureindex = i;
     rule[*count].comparison = comparison;
     rule[*count].value = value;
     (*count)++;
    }
  }
 return rule;
}

Model_divsptr learn_ruleset_divs(Instanceptr instances)
{
 int i, count;
 Instanceptr training_example, counter_example;
 Model_divsptr m;
 Condition* current;
 m = safemalloc(sizeof(Model_divs), "learn_ruleset_divs", 4);
 m->datacount = data_size(instances);
 m->classno = safemalloc(m->datacount * sizeof(int), "learn_ruleset_divs", 6);
 m->hypotheses = safecalloc(m->datacount, sizeof(Condition**), "learn_ruleset_divs", 7);
 m->conditioncounts = safecalloc(m->datacount, sizeof(int *), "learn_ruleset_divs", 8);
 m->rulecounts = safecalloc(m->datacount, sizeof(int), "learn_ruleset_divs", 9);
 training_example = instances;
 i = 0;
 while (training_example)
  {
   counter_example = instances;
   m->classno[i] = give_classno(training_example);
   while (counter_example)
    {
     if (m->classno[i] != give_classno(counter_example))
      {
       current = discriminating_rule(training_example, counter_example, &count);
       m->hypotheses[i] = alloc(m->hypotheses[i], (m->rulecounts[i] + 1) * sizeof(Condition*), m->rulecounts[i] + 1);
       m->hypotheses[i][m->rulecounts[i]] = current;
       m->conditioncounts[i] = alloc(m->conditioncounts[i], (m->rulecounts[i] + 1) * sizeof(int), m->rulecounts[i] + 1);
       m->conditioncounts[i][m->rulecounts[i]] = count;
       (m->rulecounts[i])++;
      }
     counter_example = counter_example->next;
    }
   i++;
   training_example = training_example->next;
  }
 return m; 
}

