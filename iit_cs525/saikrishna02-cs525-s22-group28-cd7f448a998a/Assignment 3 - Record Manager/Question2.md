## Question 1

**a)** From the question, we know that maximum matching means that in Graph there is no edge to add to M that can form a pair.

We need to use a greedy algorithm to solve this question.

```python
Maximun-Match = [];
def find_M(Maximun-Match)
    for e in E :
        if e can not form a pair with edges in Maximum-Match:
            Masimum-Match.append(e)
    return M
```

**b)** From this question, we know that M is the maximum-match,  since a pair have two vertices, there needs to be 2|M| vertices.

As we all know, a' = the maximum size of all matchings in G. So that for each vertice in V,  it can be included maximum in one pair. Because of this 2|M|=|V|>=a' .

Then we got |M|>=a'/2 

## Question2

M is a maximum matching in G <=> M/B is a maximum matching in G/B ；

We know that the lifting path through a blossom should add even edges in the blossom. 

So,if we can't lift the matching to maximum matching, it must because that we didn't take good use of the odd edges in the blossom. 

like this:![image-20220414171634449](C:\Users\86186\AppData\Roaming\Typora\typora-user-images\image-20220414171634449.png)



## Question 3

As we all know the maximum matching in a graph

α′(𝐺)=1/2min(|𝑉|+|𝑈|−𝑜(𝐺−𝑈)).(U⊆V)

As we know, each minimizer U is called a Tutte-Berge set, we know that U can make the value of odd(G-U) - |U| maximum.

And from the question, we know that all components of G-U are odd and factor-critical

We assume that the component of G-U are V1,V2,V3,V4.... Vk ans the maximum matching in G is M. ans Mi⊆M  is the matching edges in G(Vi). and Mu is the adjacent edges from V1,V2,V3,V4,...Vk.to U

We know that |M|<=|V|/2  ,so |M| =|M1|+|M2|+......|Mk|+|Mu|<=|V1|/2+|V2|/2+....|Vk|/2+|U|![img](file:///C:/Users/86186/AppData/Local/Temp/msohtmlclip1/01/clip_image002.gif)![img](file:///C:/Users/86186/AppData/Local/Temp/msohtmlclip1/01/clip_image002.gif)

![img](file:///C:/Users/86186/AppData/Local/Temp/msohtmlclip1/01/clip_image004.gif)

![img](file:///C:/Users/86186/AppData/Local/Temp/msohtmlclip1/01/clip_image006.gif)

![img](file:///C:/Users/86186/AppData/Local/Temp/msohtmlclip1/01/clip_image008.gif)

And If we want to make |M1|+|M2|+......|Mk|+|Mu| equals |V1|/2+|V2|/2+....|Vk|/2+|U|,we must let|Mi| =|Vi|/2 and |Mu| = |U|.

For each component Vi, if vertices are even, then the component is a perfect matching. And if component Vi's vertices are odd, then the component is near-perfectly. So that we know it is factor-critical.

If there are any even components in G-U. There must be at least odd(G-U) - |U| vertices that are not matched. So that every vertice in U  should have a pair vertice in G-U. And we all know that U is an inclusion-wise maximal set, so there can't be any even component in G-U.

 In conclusion, all components G-U are odd and factor-critical.

## Question 4

We assume that V is any node in v(G), and **node** is the vertex in G/C that replaces the compression loop C. To prove that G is factor critical, we need to prove that G-v is perfectly.

If V ∈ C, it is easy to extend M to a perfect match in G-v.  

 If V ∉ C, then G/C−v has a perfectly matched M matching C, for example through **edge-u**. When we reduce c into C, let w be the vertex in C corresponding to the **edge-u**. We can add edges to the even-length path C-w to cover all nodes in c-W which can make M in G-v extended to a perfect match. 

## Question5

I think we should use the blossom algorithm to solve this problem.

Use the blossom algorithm to find the augmenting path and check if every vertice in X-M have a pair node in G-X .

```python
def find_maximum_matching(X, M) : M*
     P ← find_augmenting_path(X, M)
     if P is non-empty then
         return find_maximum_matching(X, augment M along P)
     else
         return M
    end if
end function
find_maximum_matching(X, M) ;
def main():
    for(v in X){ # use different start node to find maximum matching in X
        if every vertice in X-M have a pair node in G-X :
            return true
        else:
            find_maximum_matching(X, M)
            }
	return false
def find_augmenting_path(X,M,v):
    use blossom algorithm to find augmenting path.
```

The time complexity is O(e*v^3) 

## Question 6 

We can either use [BFS](https://www.geeksforgeeks.org/breadth-first-traversal-for-a-graph/) to find if there is a path from u to v.  

```java
public class Graph {
    int V; // No. of vertices
    ArrayList<ArrayList<Integer>> adj;
    Graph(int V){
        this.V = V;
        adj = new ArrayList<>();
        for(int i=0;i<V;i++)
            adj.add(new ArrayList<>());
    }
    // function to add an edge to graph
    void addEdge(int v, int w)
    {
        adj.get(v).add(w);
        adj.get(w).add(v);
    }
    // A BFS based function to check whether d is reachable from s.
    boolean isReachable(int s, int d)
    {
        // Base case
        if (s == d)
            return true;
        // Mark all the vertices as not visited
        boolean[] visited = new boolean[V];
        for (int i = 0; i < V; i++)
            visited[i] = false;
        // Create a queue for BFS
        Queue<Integer> queue = new LinkedList<>();
        // Mark the current node as visited and enqueue it
        visited[s] = true;
        queue.add(s);
        while (!queue.isEmpty()) {
            // Dequeue a vertex from queue and print it
            s = queue.remove();
            // Get all adjacent vertices of the dequeued vertex s
            // If a adjacent has not been visited, then mark it
            // visited  and enqueue it
            for (int i=0; i<adj.get(s).size();i++) {
                // If this adjacent node is the destination node,
                // then return true
                if (adj.get(s).get(i) == d)
                return true;
                // Else, continue to do BFS
                if (!visited[adj.get(s).get(i)]) {
                    visited[adj.get(s).get(i)] = true;
                    queue.add(adj.get(s).get(i));
                }
            }
        }
        // If BFS is complete without visiting d
        return false;
    }
    // Driver program to test methods of graph class
    public static void main(String[] args)
    {
        Graph g = new Graph(4);
        for (edge:Graph):
        	g.addEdge(edge);
        int u = 1, v = 3;
        if (g.isReachable(u, v))
            System.out.println("\n There is a path from "+u+" to "+v);
        else
            System.out.println("\n There is no path from "+u+" to "+v);
    }
}

```

**Output:*****Time Complexity:** O(V + E)* 
***Auxiliary Space:** O(V)* 