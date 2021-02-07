# Mutable extension
This is the course project for Database systems during winter term 20-21 at Saarland University, it implements three extensions to the [mutable](https://bigdata.uni-saarland.de/projects/mutable/) system

## Milestone 1
This implements Row and Column Store database layouts with the help of mutable linearizations.
- Row Store is fully implemented
- Column Store is only missing the dynamic size decrease of allocated memory (for reference use RowStore)

## Milestone 2 
This implements a B+ Tree. Just take a look at it.  
Known Flaws:
- COMPUTE_CAPACITY is not statically working, as expected. (Still seems to be computed during runtime and not precomputed)
- Bulkloading is not as efficient as it could be. The second loop while still relatively short, could/should be included in the first one and dynamically create all levels/nodes above.

## Milestone 3
This implements the DP_ _ccp_ plan enumeration algorithm by Moerkotte et Neumann.
- Really fast implementation that could use some comments, but all is documented in the corresponding paper.
- Only flaw is a small initial bottleneck (maybe due to the two vectors in EnumerateCsgRec).

## Team __TheFlyingSalmons__
- Joshua Sonnet
- Lukas Schaller
