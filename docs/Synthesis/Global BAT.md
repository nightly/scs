# Global BAT

The global BAT simply merges everything into one from individual resources.
- Preconditions
- Successor state axioms
- Object domain is also stored in BAT

CoopMatrix and RoutesMatrix must be provided separately and is not merged from resource BATs. These matrixes can be initialised with any number passed to constructor of each as long as it exceeds the number of resources used, (e.g. 100 is valid). The numbers involved in `cm.Add()` or `rm.Add()` are extracted from actions or other formula variables/constants, not based on any particular resource vector order.