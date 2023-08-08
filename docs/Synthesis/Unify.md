# Unify

Recipe and controller/facility actions are unified by:
- Names, which must match exactly
- $Nth$ term unification of recipe to controller action
	- Where term $i$ of a recipe action is an **object** constant, must match exactly. 
	- If term $i$ of a recipe action is a **variable** it can take any value and is ignored.
		- It could be that you would prefer variables in a concurrent action to match across constituent actions but this has essentially no practical usage, would cause a slight performance hit but easy to implement anyway. 
	- If the controller action involves a term past the final term ($Nth$ term) of the recipe action, it is ignored, e.g. action's last term indexes the resource which allows a shorthand representation. Otherwise, you could continue placing variables in the recipe action and it would not be any different at all.
