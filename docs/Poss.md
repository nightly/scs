# Executability

A local `Poss` schema contains formal terms and a relational formula. Compatible facility composition can use the conjunction of local preconditions, but genuinely joint operations are defined by the facility's typed joint-executability callback. A synchronized transfer or clamp-drill tuple may consequently be executable even when its components are not executable as isolated actions.

Every `JointAction` contains exactly one `ResourceStep` per resource. Validation rejects missing or duplicate resource indices. The optional conservative `may_be_possible` callback may reject an ungrounded joint schema before equality-type enumeration; returning true only means that some grounding may be executable, and the ground `possible` callback remains authoritative.

Callbacks must be deterministic and equivariant under identifier renaming. Arena construction repeats bounded samples and evaluates renamed samples of candidate filtering, joint executability, observation, and cost; violations make the model invalid. SCS cannot prove these properties for arbitrary C++ functions, so boundedness and equivariance remain documented model obligations.
