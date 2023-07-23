# Unify

Recipe and controller actions are unified by:
- Name
- Nth term unification of recipe to controller action
	- Where term i of a recipe action is an object constant, must match exactly. 
	- If term i of a recipe is a variable it can take any value and is ignored.
	- If the controller action involves a term past the final term (nth term) of the recipe action, it is ignored, e.g. action's last term indexes the resource which allows a shorthand representation
