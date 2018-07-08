CC = gcc
CFLAGS = -Wall -g -std=c89 -pedantic
ISELLFLAGS = -Wall -lm -g -std=c89 -pedantic
OBJECTS = algorithm.o binary.o cart.o c45rules.o classification.o clustering.o cn2.o combine.o command.o convex.o data.o dataset.o datastructure.o decisionforest.o decisiontree.o dimreduction.o distributions.o divs.o eps.o graphdata.o hmm.o image.o indtests.o instance.o instancelist.o interval.o isell.o krule.o ktree.o lang.o lerils.o libarray.o libfile.o libiteration.o libmath.o libmemory.o libmisc.o librandom.o libstring.o loadmodel.o matrix.o metaclassification.o metaregression.o mixture.o mlp.o model.o multiplemodel.o multivariaterule.o multivariatetree.o network.o nodeboundedtree.o omnivariaterule.o omnivariatetree.o operator.o ordering.o parameter.o part.o partition.o perceptron.o plot.o poly.o process.o pstricks.o rbf.o readnet.o regression.o regressionrule.o regressiontree.o ripper.o rise.o rule.o savecode.o savemodel.o savenet.o screen.o softregtree.o softtree.o srm.o statistics.o structure.o svm.o svmcache.o svmkernel.o svmprepare.o svmq.o svmregtree.o svmsimplex.o svmsolve.o svmtree.o tests.o univariaterule.o univariatetree.o vc.o xmlparser.o
isell: $(OBJECTS) 
	$(CC) $(ISELLFLAGS) $(OBJECTS) -o isell
algorithm.o: algorithm.h c45rules.h decisiontree.h eps.h hmm.h instance.h instancelist.h lang.h libmisc.h libstring.h mlp.h regressionrule.h regressiontree.h rule.h savecode.h xmlparser.h
binary.o: binary.h libmisc.h
cart.o: cart.h
c45rules.o: c45rules.h instance.h instancelist.h libmisc.h rule.h
classification.o: c45rules.h classification.h data.h hmm.h instance.h instancelist.h decisiontree.h dataset.h clustering.h rule.h matrix.h libmisc.h mlp.h multivariaterule.h multivariatetree.h nodeboundedtree.h omnivariaterule.h omnivariatetree.h perceptron.h svm.h svmprepare.h svmtree.h univariaterule.h univariatetree.h
clustering.o: clustering.h data.h dataset.h instance.h instancelist.h libmisc.h 
cn2.o: cn2.h
combine.o: combine.h libmisc.h typedef.h 
command.o: command.h libmisc.h libstring.h screen.h
data.o: data.h dataset.h instance.h instancelist.h matrix.h messages.h libmisc.h 
convex.o: convex.h
dataset.o: dataset.h distributions.h instance.h instancelist.h interval.h lang.h messages.h libmisc.h libstring.h process.h statistics.h  xmlparser.h
datastructure.o: datastructure.h matrix.h libmisc.h
decisionforest.o: data.h decisionforest.h decisiontree.h instancelist.h univariatetree.h
decisiontree.o: classification.h data.h dataset.h decisiontree.h instance.h instancelist.h messages.h libmath.h libmisc.h mlp.h model.h options.h rule.h svm.h svmkernel.h vc.h
dimreduction.o: data.h dataset.h datastructure.h dimreduction.h instance.h instancelist.h matrix.h messages.h libmisc.h libstring.h parameter.h typedef.h
distributions.o: distributions.h instance.h libmath.h
divs.o: divs.h
eps.o: data.h decisiontree.h eps.h instance.h lang.h messages.h libmisc.h libstring.h perceptron.h poly.h rule.h statistics.h
graphdata.o: graphdata.h messages.h libmath.h libmisc.h libstring.h typedef.h
hmm.o: distributions.h hmm.h instance.h instancelist.h loadmodel.h libarray.h libmath.h libmemory.h libmisc.h librandom.h 
image.o: image.h
indtests.o: distributions.h graphdata.h indtests.h libmath.h messages.h libmisc.h statistics.h
instance.o: data.h dataset.h decisiontree.h instance.h messages.h libmisc.h parameter.h regressiontree.h
instancelist.o: data.h dataset.h decisiontree.h instance.h instancelist.h messages.h libmisc.h parameter.h regressiontree.h
interval.o: interval.h libmemory.h libmisc.h libstring.h
isell.o: isell.h libmemory.h process.h
krule.o: krule.h
ktree.o: ktree.h
lang.o: lang.h messages.h libmisc.h libstring.h
lerils.o: lerils.h
libarray.o: libarray.h libmath.h libmemory.h libmisc.h messages.h
libfile.o: libfile.h libmemory.h libmisc.h librandom.h libstring.h
libiteration.o: libiteration.h libmemory.h libmisc.h
libmath.o: libmath.h
libmemory.o: libmemory.h messages.h
libmisc.o: messages.h libmisc.h options.h typedef.h
librandom.o: interval.h libarray.h libmemory.h librandom.h
libstring.o: libmemory.h libstring.h
loadmodel.o: dataset.h decisiontree.h loadmodel.h matrix.h messages.h libmisc.h mlp.h typedef.h
matrix.o: matrix.h messages.h libmisc.h vc.h
metaclassification.o: metaclassification.h
metaregression.o: metaregression.h
mixture.o: libmisc.h mixture.h statistics.h
mlp.o: classification.h instance.h instancelist.h matrix.h libmisc.h mlp.h perceptron.h statistics.h
model.o: classification.h dataset.h decisiontree.h instance.h instancelist.h lang.h messages.h libmisc.h libstring.h model.h statistics.h
multiplemodel.o: instance.h instancelist.h messages.h libmisc.h libstring.h model.h multiplemodel.h regressiontree.h statistics.h
multivariaterule.o: data.h decisiontree.h instance.h instancelist.h libmisc.h mlp.h multivariaterule.h multivariatetree.h partition.h rule.h svmtree.h univariaterule.h
multivariatetree.o: data.h dataset.h decisiontree.h instance.h instancelist.h matrix.h libmisc.h mlp.h multivariatetree.h rule.h univariatetree.h
network.o: distributions.h graphdata.h messages.h libmath.h libmisc.h libstring.h network.h parameter.h readnet.h statistics.h structure.h
nodeboundedtree.o: data.h decisiontree.h instancelist.h libmisc.h model.h multivariatetree.h nodeboundedtree.h omnivariatetree.h srm.h univariatetree.h vc.h
omnivariaterule.o: data.h decisiontree.h instance.h instancelist.h libmath.h libmisc.h libstring.h model.h multivariaterule.h omnivariaterule.h rule.h statistics.h univariaterule.h
omnivariatetree.o: decisiontree.h instancelist.h libmisc.h libstring.h model.h multivariatetree.h omnivariatetree.h rule.h statistics.h univariatetree.h vc.h
operator.o: graphdata.h libmisc.h operator.h
ordering.o: libmisc.h ordering.h
parameter.o: libmisc.h libstring.h parameter.h xmlparser.h
part.o: part.h
partition.o: libmisc.h partition.h
perceptron.o: libmisc.h perceptron.h
plot.o: plot.h
poly.o: lang.h matrix.h libmisc.h mixture.h model.h perceptron.h poly.h statistics.h
process.o: binary.h classification.h clustering.h combine.h data.h dataset.h decisiontree.h distributions.h eps.h graphdata.h hmm.h indtests.h instance.h instancelist.h lang.h loadmodel.h matrix.h libmisc.h libstring.h mixture.h mlp.h model.h multiplemodel.h multivariatetree.h network.h nodeboundedtree.h omnivariaterule.h omnivariatetree.h operator.h options.h ordering.h partition.h perceptron.h poly.h process.h rbf.h readnet.h regression.h regressionrule.h regressiontree.h rule.h savecode.h savemodel.h savenet.h screen.h srm.h statistics.h structure.h svm.h svmcache.h svmkernel.h svmprepare.h svmq.h svmsolve.h tests.h univariatetree.h vc.h xmlparser.h
pstricks.o: pstricks.h
rbf.o: clustering.h instance.h instancelist.h libmisc.h perceptron.h rbf.h
readnet.o: messages.h libmisc.h libstring.h readnet.h typedef.h
regression.o: data.h dataset.h decisiontree.h instance.h instancelist.h libmisc.h mlp.h options.h parameter.h regression.h regressionrule.h regressiontree.h svm.h svmkernel.h svmprepare.h typedef.h
regressionrule.o: clustering.h dataset.h decisiontree.h instance.h instancelist.h messages.h libmisc.h libstring.h model.h regressionrule.h regressiontree.h statistics.h
regressiontree.o: clustering.h dataset.h decisiontree.h instance.h instancelist.h messages.h libmisc.h libstring.h model.h regressiontree.h statistics.h univariatetree.h
ripper.o: ripper.h
rise.o: rise.h
rule.o: classification.h data.h decisiontree.h instance.h matrix.h messages.h libmisc.h parameter.h partition.h rule.h statistics.h univariatetree.h
savecode.o: classification.h instance.h instancelist.h regression.h savecode.h 
savemodel.o: decisiontree.h instancelist.h matrix.h messages.h libmisc.h savemodel.h
savenet.o: libmisc.h savenet.h
screen.o: messages.h libmisc.h screen.h typedef.h
softregtree.o: softregtree.h
softtree.o: softtree.h
srm.o: instance.h instancelist.h libmisc.h srm.h typedef.h
statistics.o: classification.h distributions.h messages.h libmisc.h libstring.h statistics.h tests.h
structure.o: graphdata.h indtests.h messages.h libmisc.h operator.h ordering.h structure.h
svm.o: instance.h messages.h libmisc.h svm.h svmkernel.h svmsolve.h
svmcache.o: libmisc.h svmcache.h
svmkernel.o: messages.h libmisc.h svmkernel.h
svmprepare.o: instance.h instancelist.h libmisc.h partition.h
svmq.o: libmisc.h svm.h
svmregtree.o: svmregtree.h
svmsimplex.o: svmsimplex.h
svmsolve.o: libmisc.h
svmtree.o: decisiontree.h instance.h instancelist.h libarray.h libmisc.h svmkernel.h svmprepare.h univariatetree.h
tests.o: dimreduction.h distributions.h libarray.h libfile.h libmath.h libmemory.h libmisc.h libstring.h parameter.h tests.h
univariaterule.o: data.h decisiontree.h instance.h instancelist.h libmisc.h partition.h rule.h univariaterule.h univariatetree.h
univariatetree.o: decisiontree.h instance.h instancelist.h libmisc.h statistics.h univariatetree.h
vc.o: dataset.h decisiontree.h libmath.h libmisc.h multivariatetree.h statistics.h univariatetree.h vc.h
xmlparser.o: libmemory.h libmisc.h libstring.h messages.h xmlparser.h
clean:
	rm -rf *.o
