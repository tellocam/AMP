liebes Manuul
TAS seht für Test-and-Set
TATAS steht für Test-and-Test-and-Set

du kannst die sachen compilen und laufen lassen folgendermassen:


gcc -fopenmp TAS_Lock.c -o TAS_Lock
gcc -fopenmp TATAS_Lock.c -o TATAS_Lock

gefolgt von:

./TAS_Lock 
./TATAS_Lock 

Im moment hab ich alles erst sehr hardcoded, also die number of threads sind noch net gecoded z.B sodass man sie in der commandline direkt angeben kann. Sollt aber net schwer sein. Dann was uns auch noch fehlt is hald eben die überlegung der bewertung der fairness.
Also was man machen kann ist folgendes das wär mein vorschlag:
Du könntest machen was dir spass macht und weitere locks implementieren, aber dann muss man dann im nachhinein hald alle C files anpassen und unser neues einfacheres framework drann anpassen, drum wäre mein vorschlag, das man zuerst die timings und die variablen für die fairness implementiert das wir dann das überall mitnehmen können in den anderen Locks. aber man kann das auch andersrum machen denk ich das is net so schlimm. ich glaube sowie ich das verstanden hat könnts reichen wenn man einfach counters für successful/unsuccessful lock acquires macht für die bewertung der fairness.. hoffe du kannst damit iwas anfangen und kannst dich mal reinarbeiten und schauen was abgeht in dem code! 

GaLiGrü, Camilo