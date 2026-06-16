from typing import Iterator, Any
import threading
import socket
from network_graph import Node, NetworkGraph

from queue import Queue




class SocketJoinThread(threading.Thread):
    def __init__(self, socket_queue, node_list, graph, socket_to_node):
        threading.Thread.__init__(self)
        self.socket_queue: Queue[Any] = socket_queue
        self.node_list = node_list
        self.net_graph = graph
        self.socket_to_node = socket_to_node
        
    
    def run(self):
        HOST = '127.0.0.1'
        for node_edge in self.node_list:
            client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            while (True):
                try:
                    #print(f"trying to connect to {node_edge[0].node_id}:{node_edge[0].port}")
                    client_socket.connect((HOST, node_edge[0].port))
                    self.socket_to_node[client_socket] = node_edge[0]
                    #print("connected!")
                    joined_packet = f"JOINED {self.net_graph.root.node_id}" + '\n'
                    #print(f"sending out {joined_packet.strip()}")
                    client_socket.send(joined_packet.encode())
                    break
                except ConnectionRefusedError:
                    #print("trying again")
                    pass
            #self.socket_list.append(client_socket)
            self.socket_queue.put(client_socket, block=False)

        
