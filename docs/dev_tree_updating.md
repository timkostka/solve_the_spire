When a new child node is formed from a parent who is updated:
- update tree_solved immediately
- update composite_objective immediately

after all children are formed for non-player choice nodes:
- update tree_solved and composite_objective of parent
  - if either changes value, update parent above that until values don't change

for player choice nodes:
- update created nodes in the reverse order in which they were formed
