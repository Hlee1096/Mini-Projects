from network_graph import Node, NetworkGraph
from queue import Queue
import sys
import os
class CommandProcessor:
    sequence_number = 0
    def __init__(self, graph, msg_queue):
        self.network_graph: NetworkGraph = graph
        self.message_queue = msg_queue # Calculating Thread
        #self.sending_queue = send_queue # Sending Thread

    def execute_command(self, command: str) -> None:
        tokens = command.split(' ')
        match tokens[0]:
            case "CHANGE":
                self.network_graph.change_command(tokens[1], float(tokens[2]))
                CommandProcessor.sequence_number += 1
                #print(CommandProcessor.sequence_number)
                self.message_queue.put("RECALCULATE", block=False)
                #self.sending_queue.put("BROADCAST", block=False)
            
            case "FAIL":
                if tokens[1] == self.network_graph.root.node_id:
                    self.network_graph.root.alive = False
                    print(f"Node {tokens[1]} is now DOWN.")
                else:
                    self.network_graph.fail_command(tokens[1])
                    self.message_queue.put("RECALCULATE", block=False)
            case "RECOVER":
                if tokens[1] == self.network_graph.root.node_id:
                    self.network_graph.root.alive = True
                    print(f"Node {tokens[1]} is now UP.")
                else:
                    self.network_graph.recover_command(tokens[1])
                    self.message_queue.put("RECALCULATE", block=False)

            case "QUERY":
                if tokens[1] == "PATH":
                    source_node_id = tokens[2]
                    dest_node_id = tokens[3]
                    #print(source_node_id, dest_node_id)
                    source_node = self.network_graph.name_to_node[source_node_id]
                    #print(f"in query path: {source_node}")
                    self.network_graph.link_state_routing_protocol(source_node)
                    #print(self.network_graph)
                    path, cost = source_node.query(dest_node_id)
                    print(f"Least cost path from {source_node_id} to {dest_node_id}: {path}, link cost: {cost}", flush=True)
                else:
                    path, cost = self.network_graph.root.query(tokens[1])
                    print(f"Least cost path from {self.network_graph.root.node_id} to {tokens[1]}: {path}, link cost: {cost}", flush=True)

            case "RESET":
                self.network_graph.reset_graph_from_file()
                CommandProcessor.sequence_number += 1
                self.message_queue.put("RECALCULATE", block=False)
                print(f"Node {self.network_graph.root.node_id} has been reset.", flush=True)
            case "BATCH":
                file_name = tokens[2]
                with open(file_name, 'r') as f:
                    for line in f:
                        if self.verify_command(line.strip()) is True:
                            self.execute_command(line.strip())
                print("Batch update complete.", flush=True)
                self.message_queue.put("RECALCULATE", block=False)
            case "CYCLE":
                self.network_graph.cycle_detect()
            case "SPLIT":
                CommandProcessor.sequence_number += 1
                self.message_queue.put("SPLIT", block=False)
            case "MERGE":
                CommandProcessor.sequence_number += 1
                node1 = tokens[1]
                node2 = tokens[2]
                self.message_queue.put(f"MERGE {node1} {node2}", block=False)
            case _:
                print("unknown in execute")


    def verify_command(self, packet: str) -> bool:
        tokens = packet.split(' ')
        
        match tokens[0]:
            case "UPDATE":
                return self.verify_update_format(packet)
            case "CHANGE":
                if (len(tokens[1:]) != 2):
                    print("Error: Invalid command format. Expected exactly two tokens after CHANGE.", flush=True)
                    os._exit(1)
        
                try:
                    float(tokens[2])
                except ValueError:
                    print("Error: Invalid command format. Expected numeric cost value.", flush=True)
                    os._exit(1)
            
            case "FAIL":
                if (len(tokens[1:]) != 1):
                    print("Error: Invalid command format. Expected: FAIL <Node-ID>.", flush=True)
                    os._exit(1)
                if (len(tokens[1]) != 1):
                    print("Error: Invalid command format. Expected a valid Node-ID.", flush=True)
                    os._exit(1)
                if (tokens[1].isalpha() is False or tokens[1].isupper() is False):
                    print("Error: Invalid command format. Expected a valid Node-ID.", flush=True)
                    os._exit(1)
            case "RECOVER":
                if (len(tokens[1:]) != 1):
                    print("Error: Invalid command format. Expected exactly: RECOVER <Node-ID>.", flush=True)
                    os._exit(1)
                if (tokens[1].isalpha() is False or tokens[1].isupper() is False):
                    print("Error: Invalid command format. Expected a valid Node-ID.", flush=True)
                    os._exit(1)
            case "QUERY": # Remember as it is with QUERY PATH
                if tokens[1] != 'PATH':
                    if (len(tokens[1]) != 1):
                        print("Error: Invalid command format. Expected a valid Destination.", flush=True)
                        os._exit(1)
                    if (tokens[1].isalpha() is False or tokens[1].isupper() is False):
                        print("Error: Invalid command format. Expected a valid Destination.", flush=True)
                        os._exit(1)   
                else:
                    if (len(tokens[1:]) != 3):
                        print("Error: Invalid command format. Expected two valid identifiers for Source and Destination.", flush=True)
                        os._exit(1)
                    if (len(tokens[2]) != 1):
                        print("Error: Invalid command format. Expected two valid identifiers for Source and Destination.", flush=True)
                        os._exit(1)
                    if (tokens[2].isalpha() is False or tokens[2].isupper() is False):
                        print("Error: Invalid command format. Expected two valid identifiers for Source and Destination.", flush=True)
                        os._exit(1)
                    if (tokens[3].isalpha() is False or tokens[3].isupper() is False):
                        print("Error: Invalid command format. Expected two valid identifiers for Source and Destination.", flush=True)
                        os._exit(1)
            case "MERGE": # Optional
                if (tokens[1].isalpha() is False or tokens[1].isupper() is False):
                    print("Error: Invalid command format. Expected two valid identifiers for MERGE.", flush=True)
                    os._exit(1)
                if (tokens[2].isalpha() is False or tokens[2].isupper() is False):
                    print("Error: Invalid command format. Expected two valid identifiers for MERGE.", flush=True)
                    os._exit(1)
            case "SPLIT": # Optional
                if (len(tokens) != 1):
                    print("Error: Invalid command format. Expected exactly: SPLIT.", flush=True)
                    os._exit(1)
            case "RESET":
                if (len(tokens) != 1):
                    print("Error: Invalid command format. Expected exactly: RESET.", flush=True)
                    os._exit(1)
            case "CYCLE": # Optional
                if (len(tokens) != 2):
                    print("Error: Invalid command format. Expected exactly: CYCLE DETECT.", flush=True)
                    os._exit(1)
                    
            case "BATCH":
                if (len(tokens[1:]) != 2):
                    print("Error: Invalid command format. Expected: BATCH UPDATE <Filename>.", flush=True)
                    os._exit(1)
            case _:
                print("unknown in verify", flush=True)
                os._exit(1)
            
        return True

    def verify_update_format(self, packet: str) -> bool:
        tokens = packet.split(' ')
        if tokens[0] != 'UPDATE':
            print(f"in packets command: invalid {packet}")
            print("Error: Invalid update packet format.")
            os._exit(1)
        return True
    
    def make_split_packet(self) -> str:
        split_packet = str(CommandProcessor.sequence_number) + " " + "SPLIT\n"
        #print("in packet command ",split_packet)
        return split_packet
    
    def make_merge_packet(self, node1, node2) -> str:
       # 
        merge_packet = f"MERGE {node1} {node2}\n"
        return merge_packet
    
    def make_update_packet_without_seq(self) -> str:
        with self.network_graph.lock:
            update_packet = "UPDATE " + self.network_graph.root.node_id + " "
            neighbours = self.network_graph.get_neighbour_nodes_queue()
            for n in neighbours:
                update_packet = update_packet + n[0].node_id + ":" + str(n[1]) +  ":" + str(n[0].port) + ","
        
       
        #print(f"about to return {update_packet}")
        return update_packet.rstrip(",") + '\n'

    def make_update_packet(self) -> str:
        '''
        Format:
        UPDATE <Node-ID> <Neighbour1>:<Cost1>:<Port1>,<Neighbour2>:<Cost2>:<Port2>,...
        '''
        with self.network_graph.lock:
            update_packet = str(CommandProcessor.sequence_number) + " " + "UPDATE " + self.network_graph.root.node_id + " "
            neighbours = self.network_graph.get_neighbour_nodes_queue()
            for n in neighbours:
                update_packet = update_packet + n[0].node_id + ":" + str(n[1]) +  ":" + str(n[0].port) + ","
        
       
        #print(f"about to return {update_packet}")
        return update_packet.rstrip(",") + '\n'



if __name__ == '__main__':
    cmd_pro = CommandProcessor(None, None)

    # cmd_pro.verify_command("CHANGE A two")
    # cmd_pro.verify_command("FAIL AB")
    # cmd_pro.verify_command("RECOVER ab")
    # cmd_pro.verify_command("QUERY AB")
    # cmd_pro.verify_command("QUERY PATH A b")
    #cmd_pro.verify_command("MERGE A b")
    # cmd_pro.verify_command("RESET A b")
    # cmd_pro.verify_command("BATCH UPDATE")
    # cmd_pro.verify_command("CHANGE A 2.5 extra")
    # cmd_pro.verify_command("FAIL")
    print("hi")