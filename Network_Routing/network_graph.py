from collections import deque
from typing import Dict
from threading import Lock
import heapq
import itertools
import math


class Node:
    '''
    Node will store its own name, port.
    '''
    def __init__(self, node_id: str, port: int):
        self.node_id = node_id
        self.port = port
        self.alive = True
        self.routing_table: set[dict[Node, float], dict[Node, Node]] = None


    def __str__(self):
        return f"Node ID: {self.node_id} and Port: {self.port}"
    
    def isAlive(self) -> bool:
        return self.alive
    
    def query(self, destination_node_id):
        def getPath_Cost(source ,destination, named_link) -> str:
            path = f"{destination}"
            curr = destination
            while curr != source:
                try:
                    path = named_link[curr] + path
                    curr = named_link[curr]
                except KeyError:
                    pass
    
            return path


        named_cost = {}
        named_link = {}
        for node, cost in self.routing_table[0].items():
            named_cost[node.node_id] = cost

        for node1, node2 in self.routing_table[1].items():
            if (node2 is not None):
                named_link[node1.node_id] = node2.node_id 
        
       
        # for entry in self.routing_table[0]:
        #     if (entry == self):
        #         continue
        path = getPath_Cost(self.node_id, destination_node_id, named_link)
        
        return path, named_cost[destination_node_id]

    def print_routing_table(self) -> None:
        def getPath_Cost(source ,destination, named_link) -> str:
            path = f"{destination}"
            curr = destination
            seen = {}
            while curr != source:
                try:
                    path = named_link[curr] + path
                    curr = named_link[curr]
                   # seen[named_link] = True
                except KeyError:
                    pass
    
            return path



        named_cost = {}
        named_link = {}

        if (self.routing_table[0] is None or self.routing_table[1] is None):
            print(f"I am Node {self.node_id}", flush=True)
            return

        for node, cost in self.routing_table[0].items():
            named_cost[node.node_id] = cost

        for node1, node2 in self.routing_table[1].items():
            if (node2 is not None):
                named_link[node1.node_id] = node2.node_id 
        
        def sort_key(e):
            return e.node_id
        
        
        my_list = list(self.routing_table[0])
        my_list.sort(key=sort_key)
        print(f"I am Node {self.node_id}", flush=True)
        

        
       
        # for item in my_list:
        #     print(item)
    
        for entry in my_list:
            if (entry == self):
                continue
            if (entry.node_id in named_link):
                path = getPath_Cost(self.node_id, entry.node_id, named_link)
                print(f"Least cost path from {self.node_id} to {entry.node_id}: {path}, link cost: {named_cost[entry.node_id]}", flush=True)

        


class NetworkGraph:
    '''
    Adjacency List Implementation
    Utilises a hashtable (dictionary) to store vertexes and linkedlist for each vertex storing neighbouring vertex(node) and cost.
    
    Dictionary with Dequeue
    A = [B, 1]
    B = [A, 1] -> [... , ...]

    '''

    TOKEN_NAME = 0
    TOKEN_WEIGHT = 1
    TOKEN_PORT = 2

    def __init__(self):
        self.root: Node = None
        self.graph: Dict[Node, deque[list[Node, float]]] = {}
        self.name_to_node: Dict[str, Node] = {}
        self.lock = Lock()
        self.nodes: int = 0
        self.changed = False
        self.config_file = None


    def __str__(self):
        result = []
        result.append(f"Root: {self.root}")
        result.append(f"Total Nodes: {self.nodes}")
        result.append("Graph:")

        for node in sorted(self.graph.keys(), key=lambda n: n.node_id):
            edges = self.graph[node]
            edge_list = ", ".join(f"({dest.node_id}, {weight})" for dest, weight in edges)
            result.append(f"{node.node_id} -> [{edge_list}]")

        return "\n".join(result)
    
    def get_port_from_name(self, node_id):
        node = self.name_to_node[node_id]
        return node.port
    
    def is_changed(self) -> bool:
        return self.changed

    def get_neighbour_nodes_queue(self):
        return self.graph[self.root]
    
    def reset_graph_from_file(self):
        with self.lock:
            self.name_to_node = {}
            self.graph = {}
            self.name_to_node[self.root.node_id] = self.root
            self.graph[self.root] = deque()
            with open(self.config_file, "r") as config_file:
                first_line = config_file.readline().strip()
                for _ in range(0, int(first_line)):
                    line = config_file.readline().strip()
                    tokens = line.split(" ")
                    # Below adds the new node to the root node linkedlist
                    new_node = Node(tokens[self.TOKEN_NAME], int(tokens[self.TOKEN_PORT]))
                    self.graph[self.root].append([new_node, float(tokens[self.TOKEN_WEIGHT])])
                    self.name_to_node[tokens[self.TOKEN_NAME]] =  new_node


                    # Below adds the new node to the graph and adds its own linkedlist
                    self.graph[new_node] = deque()
                    self.graph[new_node].append([self.root, float(tokens[self.TOKEN_WEIGHT])])

    def build_graph_from_file(self, main_node: Node, config_file_path: str):
        with self.lock:
            self.root = main_node
            self.name_to_node = {}
            self.nodes = 0
            self.name_to_node[main_node.node_id] = main_node
            self.graph[self.root] = deque()
            self.config_file = config_file_path
            with open(config_file_path, "r") as config_file:
                first_line = config_file.readline().strip()
                for _ in range(0, int(first_line)):
                    line = config_file.readline().strip()
                    #print(line, flush=True)
                    tokens = line.split(" ")
                    # Below adds the new node to the root node linkedlist
                    new_node = Node(tokens[self.TOKEN_NAME], int(tokens[self.TOKEN_PORT]))
                    self.graph[self.root].append([new_node, float(tokens[self.TOKEN_WEIGHT])])
                    self.name_to_node[tokens[self.TOKEN_NAME]] =  new_node


                    # Below adds the new node to the graph and adds its own linkedlist
                    self.graph[new_node] = deque()
                    self.graph[new_node].append([self.root, float(tokens[self.TOKEN_WEIGHT])])

                    self.nodes += 1
            
    def update_graph(self, source_node_id: str, node_id: str, port, weight):
        '''
        Check if node exist in the graph by checking the source node neighbour
        If not make a new node for it in dictionary and update source node neighbour
        '''
        # B A:3.5:6000,C:1.0:6010
        # UPDATE B A:1.0:6000,C:1.0:6010

        with self.lock:
            try:
                source_node = self.name_to_node[source_node_id]
                source_node_edge_list = self.graph[source_node]
            except KeyError:
                new_source_node = Node(source_node_id, int(port))
                self.name_to_node[source_node_id] = new_source_node
                self.graph[new_source_node] = deque()
                source_node_edge_list = self.graph[new_source_node]
            exist = False

            # Loop changes the source_node's edge to node_id to the weight
            exist_in_source_node_list = False
            for edge_weight in source_node_edge_list:
                if (edge_weight[0].node_id == node_id):
                    if (float(edge_weight[1]) != float(weight)):
                        #print(f"changing from {float(edge_weight[1])} to {weight}")
                        edge_weight[1] = float(weight)
                        
                        self.changed = True
                    exist = True
                    exist_in_source_node_list = True
                    break
            
            # Loop changes the node_id to source_node edge to weight
            exist_in_node_list = False
            try:
                n = self.name_to_node[node_id]
                n_edge_list = self.graph[n]
                exist = True
                for edge_weight in n_edge_list:
                    if (edge_weight[0].node_id == source_node_id):
                        if (float(edge_weight[1]) != float(weight)):
                            edge_weight[1] = float(weight)
                            self.changed = True
                            exist_in_node_list = True
                        #print(self)
                        return
                        
            except KeyError:
                exist = False
            
          
            if (exist_in_source_node_list is False):
                n = self.name_to_node[source_node_id]
                try:
                    self.graph[n].append([self.name_to_node[node_id], float(weight)])
                except KeyError:
                    new_node = Node(node_id, int(port))
                    self.name_to_node[node_id] = new_node
                    self.graph[n].append([new_node, float(weight)])
                #self.changed = True
            
            if (exist_in_node_list is False):
                n = self.name_to_node[node_id]
                try:
                    neighbour_list = self.graph[n]
                    neighbour_list.append([self.name_to_node[source_node_id], float(weight)])
                except KeyError:
                    neighbour_list = deque()
                    self.graph[n] = neighbour_list
                    neighbour_list.append([self.name_to_node[source_node_id], float(weight)])
                    exist = True
                    self.changed = True
               
        
            #print(self)
                
            if (exist is False):
                new_node = Node(node_id, int(port))
                self.graph[new_node] = deque()
                self.graph[new_node].append([source_node, float(weight)])
                self.graph[source_node].append([new_node, float(weight)])
                self.name_to_node[node_id] = new_node
                self.nodes += 1
                self.changed = True
            # print(self)
          
        

    def link_state_routing_protocol(self, source_node: Node):
        '''
        Init the routing table for node s
            Uses Djikstra algorithm with a priority queue
            https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm

            Dest | Link | Cost
        
            
            init table first
            then add value for neighbours of node
        '''
        
        
        with self.lock:
            # for n in self.graph:
            #     source_node.routing_table[n.node_id] = [None, float('inf')]
            #     if (n == source_node):
            #         source_node.routing_table[n.node_id] = [source_node.node_id, 0]

            # neighbour_list = self.graph[source_node]
            # print(neighbour_list)
            # for node_edge in neighbour_list:
            #     source_node.routing_table[node_edge[0].node_id] = [node_edge[0].node_id, node_edge[1]]
            def add_task(pq ,task, priority=0):
                'Add a new task or update the priority of an existing task'
                if task in entry_finder:
                    remove_task(task)
                count = next(counter)
                entry = [priority, count, task]
                entry_finder[task] = entry
                heapq.heappush(pq, entry)
            
            def remove_task(task):
                'Mark an existing task as REMOVED.  Raise KeyError if not found.'
                entry = entry_finder.pop(task)
                entry[-1] = REMOVED

            def is_empty(pq):
                for item in pq:
                    if item[2] is not REMOVED:
                        return False
                return True

            def pop_task(pq):
                'Remove and return the lowest priority task. Raise KeyError if empty.'
                while pq:
                    priority, count, task = heapq.heappop(pq)
                    if task is not REMOVED:
                        del entry_finder[task]
                        return task
                raise KeyError('pop from an empty priority queue')

            entry_finder = {}    
            counter = itertools.count()
            REMOVED = '<removed-task>'

            dist = {} # Holds distance from source to v  node:float
            dist[source_node] = 0
            prev = {} # Predecessor of v        node:node
            prio_queue = []
            add_task(prio_queue, source_node, 0)
           # heapq.heappush(prio_queue, [0, source_node])

            neighbour_list_of_source = self.graph[source_node]
            if (len(neighbour_list_of_source) == 0):
                source_node.routing_table = (None, None)
                return
            
            for vertex in self.graph:
                if vertex != source_node and vertex.isAlive() is True:
                    prev[vertex] = None
                    dist[vertex] = float('inf')
                    add_task(prio_queue, vertex, float('inf') )
                    #print("in here",prio_queue)
                    #heapq.heappush(prio_queue, [float('inf'), vertex])

            
            while is_empty(prio_queue) is False:
                #u = heapq.heappop(prio_queue)
                u = pop_task(prio_queue)
                neighbour_list = self.graph[u]  
                # for item in neighbour_list:
                #     print(item[0].node_id, item[1])

                for edge_weight in neighbour_list:    #Graph.Distance(u,v) returns the length of the edge joining (i.e. the distance between) the two neighbor-nodes u and v
                    v = edge_weight[0]
                    if (v.isAlive() is False):
                        continue
                    if (v == source_node):
                        continue
                    
                    if (dist[u] == float('inf')):
                        alt = 0 + float(edge_weight[1])
                    else:
                        alt = dist[u] + float(edge_weight[1])
                    
                    if alt < dist[v]:
                        prev[v] = u
                        dist[v] = alt
                        add_task(prio_queue, v, alt)

      
            source_node.routing_table = (dist, prev)
            #print(self)
    
                
            # print(source_node.routing_table)
                
    def change_command(self, neighbour_id: str, new_cost: float) -> None:
        with self.lock:
            node = self.name_to_node[neighbour_id]
            root_neighbours = self.graph[self.root]

            for edge_weight in root_neighbours:
                if edge_weight[0].node_id == neighbour_id:
                    edge_weight[1] = new_cost
                    break

            neighbour_node_neighbours = self.graph[node]
            for edge_weight in neighbour_node_neighbours:
                if edge_weight[0].node_id == self.root.node_id:
                    edge_weight[1] = new_cost
                    break

            #print(self)
            return
    
    def fail_command(self, node_id: str) -> None:
        with self.lock:
        #nodes_to_update = []
            node = self.name_to_node[node_id]
            node.alive = False
        #node_neighbours = self.graph[node]

        # for node_edge in node_neighbours:
        #     nodes_to_update.append(node_edge[0])

    def recover_command(self, node_id: str) -> None:
        with self.lock:
            node = self.name_to_node[node_id]
            node.alive = True
    
    def cycle_detect(self) -> None:
        '''
        https://stackoverflow.com/questions/65527674/logic-for-method-to-detect-cycle-in-an-undirected-graph
        '''

        def dfs(node, parent, graph, visited) -> bool:
            visited[node] = True
            for neighbour in graph[node]:
                child = neighbour[0]
                try:
                    if (visited[child] is True):
                        if (child != parent):
                            return True
                except KeyError:
                    return dfs(child, node, graph, visited)
                
            return False

        with self.lock:
            visited = {}
            flag = dfs(self.root, None, self.graph, visited)
            if flag is True:
                print("Cycle detected.", flush=True)

            if flag is False:
                print("No cycle found.", flush=True)
            

            #popleft

    def merge(self, node_id_1: str, node_id_2: str):

        def check_node_exist_in_list(n: Node, edge_weight_list) -> bool:
            # Will check if a node exist in a edge_weight list
            for edge_weight in edge_weight_list:
                if (n == edge_weight[0]):
                    return True

            return False
        
        def get_edge_weight_in_list(n: Node, edge_weight_list):
            for edge_weight in edge_weight_list:
                if (n == edge_weight[0]):
                    x = edge_weight
                    edge_weight_list.remove(edge_weight)
                    return x
                
        def get_edge_weight_in_list_non_removal(n: Node, edge_weight_list):
            for edge_weight in edge_weight_list:
                if (n == edge_weight[0]):
                    return edge_weight



        with self.lock:
            node1: Node = self.name_to_node[node_id_1]
            node2: Node = self.name_to_node[node_id_2]

            if (node2 == self.root):
                return
            node1_list = self.graph[node1]
            node2_list = self.graph[node2]
            
            # finds all the edges in node2 to replace/update node1
            node2_edge_weight_list = []
            for edge_weight in node2_list:
                # No need to update the edge between node1 and node2, gonna remove it
                if (edge_weight[0].node_id == node_id_1):
                    continue
                node2_edge_weight_list.append(edge_weight)
            

            # Find all edges in node1 to update with node2
            for edge_weight in node1_list:
                node_of_edge_weight: Node = edge_weight[0]
                if (check_node_exist_in_list(node_of_edge_weight, node2_edge_weight_list) is True):
                    # If True, then check if edge_weight of node2 is cheaper, if so overwrite
                    node2_edge_weight = get_edge_weight_in_list(node_of_edge_weight, node2_edge_weight_list)
                    if (node2_edge_weight[1] < edge_weight[1]):
                        edge_weight[1] = node2_edge_weight[1]
                    continue
                else:
                    pass
            
            # Leftovers from node2_edge_weight_list.
            for edge_weight in node2_edge_weight_list:
                node1_list.append(edge_weight)

            # Leftovers used to have direct link with noed 2 but now its linked with node 1
            for edge_weight in node2_edge_weight_list:
                neighbour_node_of_node2 = edge_weight[0]
                neighbour_list = self.graph[neighbour_node_of_node2]
                if (check_node_exist_in_list(neighbour_node_of_node2, node1_list) is True):
                    edge_weight_neigh_from_node1 = get_edge_weight_in_list_non_removal(neighbour_node_of_node2, node1_list)
                    if (edge_weight[1] < edge_weight_neigh_from_node1[1]):
                        edge_weight_neigh_from_node1[1] = edge_weight[1]
                    neighbour_list.append([node1, edge_weight[1]])
                else:
                    neighbour_list.append([node1, edge_weight[1]])


            # Remove all instances of node2 edges in the graph
            for n, neighbours in self.graph.items():
                if n == node2:
                    # make node2 empty
                    self.graph[node2] = deque()
                    continue
                else:
                    # If not node2, update its edge weight list to remove all b
                    new_edge_weight_list = deque()
                    for edge_weight in neighbours:
                        if (edge_weight[0] != node2):
                            new_edge_weight_list.append(edge_weight)
                        
                    self.graph[n] = new_edge_weight_list

            # Update all nodes in graph with node1 new edges
            node1_edge_weight_list = self.graph[node1]
            for n in self.graph.keys():
                edge_weight_list = self.graph[n]
                if (n == node1):
                    continue

                for edge_weight in edge_weight_list:
                    if (edge_weight[0] == node1):
                        node1_edge_weight = get_edge_weight_in_list_non_removal(n, node1_edge_weight_list)
                        if (node1_edge_weight[1] < edge_weight[1]):
                            edge_weight[1] = node1_edge_weight[1]
                    
            
            


                

    def split(self):
        def sort_key(e):
                return e.node_id

        with self.lock:
            my_list = list(self.graph)
     
            my_list.sort(key=sort_key) # Sort the set of nodes V in alphabetical order.

            # Below splits the vertices in the graph into 2 lists
            k = math.floor(len(my_list)/2)
            v1: Node = []
            v2: Node = []

            for i in range(0, k):
                v1.append(my_list[i])
            
            for i in range(k, len(my_list)):
                v2.append(my_list[i])

            # print("v1")
            # for item in v1:
            #     print(item)

            # print("v2")
            # for item in v2:
            #     print(item)
            
            # print(k)
            # for item in my_list:
            #     print(item)

            for nodes in v1:
                node_neighbour_list = self.graph[nodes]

                new_node_neighbour_list = deque()
                for edge_weight in node_neighbour_list:
                    if (edge_weight[0] in v2):
                        continue
                    else:
                        new_node_neighbour_list.append(edge_weight)
                self.graph[nodes] = new_node_neighbour_list
            
            for nodes in v1:
                node_neighbour_list = self.graph[nodes]
                new_node_neighbour_list = deque()
                for edge_weight in node_neighbour_list:
                    if (edge_weight[0] in v2):
                        continue
                    else:
                        new_node_neighbour_list.append(edge_weight)
                self.graph[nodes] = new_node_neighbour_list

            for nodes in v2:
                node_neighbour_list = self.graph[nodes]
                new_node_neighbour_list = deque()
                for edge_weight in node_neighbour_list:
                    if (edge_weight[0] in v1):
                        continue
                    else:
                        new_node_neighbour_list.append(edge_weight)
                self.graph[nodes] = new_node_neighbour_list
            
            #print(self)
            
            