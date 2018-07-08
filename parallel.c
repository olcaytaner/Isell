#include "parallel.h"

#ifdef MPI
int ldt()
{
	/*!Last Changed 04.10.2004 added writerules*/
	/*!Last Changed 02.02.2004*/
 /*!Last Changed 09.02.2003*/
	double timestart, timeend; 
	Ruleset r;
	int i, nodeindex, finished = -1, *indexlist, count = 0, fno, leftcount, rightcount, leftcounts[MAXCLASSES], rightcounts[MAXCLASSES];
	int ratio[MAXVALUES][MAXCLASSES], fnoend;
	double bestsplit, leftsum, rightsum, leftmean, rightmean;
	Decisionconditionptr rules, dummyrule;
	MPI_Datatype* myruletype;
	Instanceptr tmplist;
	MPI_Status status;
	Partition p;
	if (prunetype != PREPRUNING)
   create_cvdata();
 if (parallel && rank != 0)
 {
		switch (paralleltype)
		 {
		  case 0:MPI_Recv(&count,1,MPI_INT,0,1,MPI_COMM_WORLD,&status);
		         indexlist = (int *)safemalloc(count*sizeof(int), "ldt", 21);
           MPI_Recv(indexlist,count,MPI_INT,0,2,MPI_COMM_WORLD,&status);
		         MPI_Recv(&fno,1,MPI_INT,0,3,MPI_COMM_WORLD,&status);
		         MPI_Recv(&fnoend,1,MPI_INT,0,4,MPI_COMM_WORLD,&status);
		         while (fno != -1)
											 {
												 if (indexlist[0] != -1)
               tmplist = create_instancelist(indexlist, count, &traindata);
				         find_best_split_for_features_between_i_and_j(tmplist, fno, fnoend);
													safe_free(indexlist);
		           MPI_Recv(&count,1,MPI_INT,0,1,MPI_COMM_WORLD,&status);
		           indexlist = (int *)safemalloc(count*sizeof(int), "ldt", 32);
             MPI_Recv(indexlist,count,MPI_INT,0,2,MPI_COMM_WORLD,&status);
		           MPI_Recv(&fno,1,MPI_INT,0,3,MPI_COMM_WORLD,&status);
		           MPI_Recv(&fno,1,MPI_INT,0,4,MPI_COMM_WORLD,&status);
													if (indexlist[0] != -1)
				           traindata = restore_instancelist(traindata, tmplist);
											 }
											if (indexlist[0] == -1)
				         traindata = restore_instancelist(traindata, tmplist);
		         safe_free(indexlist);
											break;
		  case 1:MPI_Recv(&count,1,MPI_INT,0,1,MPI_COMM_WORLD,&status);
		         rules = (Decisionconditionptr)safemalloc(count*sizeof(Decisioncondition), "ldt", 44);
											myruletype = create_datatype_for_rule(rules);
           MPI_Recv(rules,count,*myruletype,0,2,MPI_COMM_WORLD,&status);
		         MPI_Recv(&fno,1,MPI_INT,0,3,MPI_COMM_WORLD,&status);
		         while (fno != -1)
											 {
             tmplist = create_instancelist_from_rules(rules, count, &traindata);
				         find_best_split_for_features_between_i_and_j(tmplist, fno, fno);
													safe_free(rules);
		           MPI_Recv(&count,1,MPI_INT,0,1,MPI_COMM_WORLD,&status);
	  	         rules = (Decisionconditionptr)safemalloc(count*sizeof(Decisioncondition), "ldt", 54);
             MPI_Recv(rules,count,*myruletype,0,2,MPI_COMM_WORLD,&status);
		           MPI_Recv(&fno,1,MPI_INT,0,3,MPI_COMM_WORLD,&status);
				         traindata = restore_instancelist(traindata, tmplist);
											 }
											safe_free(myruletype);
		         safe_free(rules);
											break;
    case 2:MPI_Recv(&count,1,MPI_INT,0,1,MPI_COMM_WORLD,&status);
		         indexlist = (int *)safemalloc(count*sizeof(int), "ldt", 63);
           MPI_Recv(indexlist,count,MPI_INT,0,2,MPI_COMM_WORLD,&status);
		         MPI_Recv(&fno,1,MPI_INT,0,3,MPI_COMM_WORLD,&status);
           MPI_Recv(&(p.count),1,MPI_INT,0,4,MPI_COMM_WORLD,&status);
					      p.classes = (int *)safemalloc(p.count * sizeof(int), "ldt", 67);
											MPI_Recv(p.classes,p.count,MPI_INT,0,5,MPI_COMM_WORLD,&status);
											p.assignments = (int *)safemalloc(p.count * sizeof(int), "ldt", 69);
											MPI_Recv(p.assignments,p.count,MPI_INT,0,6,MPI_COMM_WORLD,&status);
		         while (fno != -1)
											 {
													if (indexlist[0] != -1)
													 {
               tmplist = create_instancelist(indexlist, count, &traindata);
													 }
			          safe_free(indexlist);
             if (current_dataset->features[fno].type == STRING)
													 {
														 find_occurences_for_discrete_feature(tmplist, fno, ratio);
															MPI_Send(ratio,MAXVALUES*MAXCLASSES,MPI_INT,0,1,MPI_COMM_WORLD);
													 }
													else
													 {
               left_and_right_sum(tmplist, p, fno, &leftsum, &rightsum, &leftcount, &rightcount);
															MPI_Send(&leftsum,1,MPI_DOUBLE,0,1,MPI_COMM_WORLD);
															MPI_Send(&rightsum,1,MPI_DOUBLE,0,2,MPI_COMM_WORLD);
															MPI_Send(&leftcount,1,MPI_INT,0,3,MPI_COMM_WORLD);
															MPI_Send(&rightcount,1,MPI_INT,0,4,MPI_COMM_WORLD);
															MPI_Recv(&leftmean,1,MPI_DOUBLE,0,7,MPI_COMM_WORLD,&status);
															MPI_Recv(&rightmean,1,MPI_DOUBLE,0,8,MPI_COMM_WORLD,&status);
               left_and_right_variance(tmplist, p, fno, &leftsum, &rightsum, leftmean, rightmean);
															MPI_Send(&leftsum,1,MPI_DOUBLE,0,5,MPI_COMM_WORLD);
															MPI_Send(&rightsum,1,MPI_DOUBLE,0,6,MPI_COMM_WORLD);
               MPI_Recv(&bestsplit,1,MPI_DOUBLE,0,9,MPI_COMM_WORLD,&status);
               find_counts_for_split(tmplist,fno,bestsplit,leftcounts,rightcounts);
               MPI_Send(leftcounts,MAXCLASSES,MPI_INT,0,7,MPI_COMM_WORLD); 
               MPI_Send(rightcounts,MAXCLASSES,MPI_INT,0,8,MPI_COMM_WORLD);
													 }
													free_partition(p);
											  MPI_Recv(&count,1,MPI_INT,0,1,MPI_COMM_WORLD,&status);
		           indexlist = (int *)safemalloc(count*sizeof(int), "ldt", 102);
             MPI_Recv(indexlist,count,MPI_INT,0,2,MPI_COMM_WORLD,&status);
		           MPI_Recv(&fno,1,MPI_INT,0,3,MPI_COMM_WORLD,&status);
             MPI_Recv(&(p.count),1,MPI_INT,0,4,MPI_COMM_WORLD,&status);
					        p.classes = (int *)safemalloc(p.count * sizeof(int), "export_datafolds", 106);
											  MPI_Recv(p.classes,p.count,MPI_INT,0,5,MPI_COMM_WORLD,&status);
											  p.assignments = (int *)safemalloc(p.count * sizeof(int), "export_datafolds", 108);
											  MPI_Recv(p.assignments,p.count,MPI_INT,0,6,MPI_COMM_WORLD,&status);
													if (indexlist[0] != -1)
				           traindata = restore_instancelist(traindata, tmplist);
											 }
											if (indexlist[0] == -1)
				         traindata = restore_instancelist(traindata, tmplist);
		         safe_free(indexlist);
											free_partition(p);
											break;
		  case 3:MPI_Recv(&count,1,MPI_INT,0,1,MPI_COMM_WORLD,&status);
		         indexlist = (int *)safemalloc(count*sizeof(int), "ldt", 119);
           MPI_Recv(indexlist,count,MPI_INT,0,2,MPI_COMM_WORLD,&status);
		         MPI_Recv(&nodeindex,1,MPI_INT,0,3,MPI_COMM_WORLD,&status);
		         while (nodeindex != -1)
											 {
             tmplist = create_instancelist(indexlist, count, &traindata);
													safe_free(indexlist);
             fno = find_best_ldt_feature(tmplist, &bestsplit);
             MPI_Send(&fno,1,MPI_INT,0,1,MPI_COMM_WORLD);
													MPI_Send(&bestsplit,1,MPI_DOUBLE,0,2,MPI_COMM_WORLD);
		           MPI_Recv(&count,1,MPI_INT,0,1,MPI_COMM_WORLD,&status);
		           indexlist = (int *)safemalloc(count*sizeof(int), "ldt", 130);
             MPI_Recv(indexlist,count,MPI_INT,0,2,MPI_COMM_WORLD,&status);
		           MPI_Recv(&nodeindex,1,MPI_INT,0,3,MPI_COMM_WORLD,&status);
				         traindata = restore_instancelist(traindata, tmplist);
											 }
		         safe_free(indexlist);
											break;
		 }
  return 0;
 }
 c45node = createnewnode(NULL, 1);
 c45node->instances = traindata;
	timestart = clock();
 create_ldtchildren(c45node, NULL);
	timeend = clock();
	runtime = timeend - timestart;
	if (prunetype != PREPRUNING)
   prune_tree(c45node, c45node, cvdata);
 test_tree(c45node,testdata);
	if (writerules)
	 {
		 r = create_ruleset_from_decision_tree(c45node);
			write_ruleset(r);
   free_ruleset(r);
	 }
	if (drawmodel)
	 {
		 traindata = NULL;
	  accumulate_instances_tree(&traindata, c45node);
		 plot_data_and_tree2d(traindata, current_dataset, c45node, imagefile);
	 }
 deallocate_tree(c45node);
 safe_free(c45node);
 c45node = NULL;
 traindata = NULL;
	if (parallel && rank == 0 && proccount>1)
	 {
  	count = 1;
			if (paralleltype == 1)
				 myruletype = create_datatype_for_rule(&dummyrule);
		 for (i = 1; i < proccount; i++)
			 {
				 MPI_Send(&count,1,MPI_INT,i,1,MPI_COMM_WORLD);
					if (paralleltype != 1)
				   MPI_Send(&finished,1,MPI_INT,i,2,MPI_COMM_WORLD);
					else
				   MPI_Send(&dummyrule,1,*myruletype,i,2,MPI_COMM_WORLD);						 
					MPI_Send(&finished,1,MPI_INT,i,3,MPI_COMM_WORLD);
					if (paralleltype == 2)
					 {
				   MPI_Send(&count,1,MPI_INT,i,4,MPI_COMM_WORLD);
				   MPI_Send(&finished,1,MPI_INT,i,5,MPI_COMM_WORLD);
					  MPI_Send(&finished,1,MPI_INT,i,6,MPI_COMM_WORLD);
					 }
			 }
			if (paralleltype == 1)
				 safe_free(myruletype);
	 }
 return 0;
}
#endif
