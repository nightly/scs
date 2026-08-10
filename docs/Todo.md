# Supported fragment and future work

The exact implementation currently supports relational, deterministic, complete-information, Markovian BATs. Functional fluents, sensing, nondeterministic effects, exogenous events, and temporal objectives are natural future extensions.

Boundedness and callback equivariance are model obligations. Runtime active-domain checks and independent replay detect concrete violations, but these properties cannot be decided automatically for arbitrary theories or C++ callbacks.
