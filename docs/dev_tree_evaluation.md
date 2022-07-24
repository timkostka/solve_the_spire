Given some Nodes, how to best determine which Nodes to expand first?

The current (7/2022) solution puts all non-terminal, non-expanded nodes into a set and uses a metric to determine which to expand next:
```
    // nodes which need expanded after a path is formed
    std::set<Node *, PathObjectiveSort> optional_nodes;
```

with objective of:

```
// sort nodes so that we know which to expand first
struct PathObjectiveSort {
    bool operator() (Node * const first, Node * const second) const {
        return first->path_objective > second->path_objective ||
            (first->path_objective == second->path_objective &&
                first < second);
    }
};

...

    // return an objective function used to evaluate the best decision
    // this function may only use information within this node--no information
    // from children is allowed
    double GetPathObjective() {
        // DEBUG
        // this is illegal, isn't it? Since it looks at children?
        //return CalculateCompositeObjective() + 1000.0 * layer;
        double x = 5.0 * hp;
        for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
            if (monster[i].Exists()) {
                x += monster[i].max_hp - monster[i].hp;
            }
        }
        // favor evaluating later turns
        x += 1000.0 * layer;
        //x += turn * 1000;
        return x;
    }

```

To limit the memory requirement of tree evaluation, I quickly found that it's necessary to evaluate the bottom of the tree first, rather than expanding nodes at the same level. This allows one to prune the tree more effectively. Even with tens of GB available and the ability to store many hundreds of millions Nodes, this was necessary.

I've implemented this by using the `layer` variable and incorporating that heavily into the path objective.

### Alternative

Instead of using the layer directly, perhaps have a different set at each layer level and simply evaluating the deepest layer first. This eliminates the use of `layer` within the metric, and reduces the size of the set, which should aid efficiency.

Or, we could also notice that when we add a set of new nodes to the optional list, we always want to evaluate those before ones that already exist. Maybe we could use a vector or list rather than a set. But how does that work when we need to delete nodes?
