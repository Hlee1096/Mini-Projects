import threading
import socket
from network_graph import Node, NetworkGraph
from typing import Dict
from packets_command import CommandProcessor
from queue import Queue
from sub_thread import SocketJoinThread
import select
import sys
import time
import os

working = True
READ_SIZE = 2048

class ListeningThread(threading.Thread):
    '''
    ListeningThread will monitor STDIN and recieving socket and Update packets must be forwarded to the Routing Calculations Thread
    Also processes dynamic commands.
    '''
    def __init__(self, graph: NetworkGraph, message_queue: Queue[str]):
        threading.Thread.__init__(self)
        self.net_graph = graph
        self.message_queue = message_queue
        


    def run(self):
        poller = select.poll()
        poller.register(sys.stdin, select.POLLIN)
        cmd_processor = CommandProcessor(self.net_graph, self.message_queue)

        HOST = '127.0.0.1'
        PORT = self.net_graph.root.port
        
        # Listening Socket handles connections.
        listening_socket = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM)
        listening_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        listening_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
        listening_socket.bind((HOST, PORT))
        listening_socket.listen()

        poller.register(listening_socket.fileno(), select.POLLIN)

        socket_list = []
       
        global working
        while (working):
            polled_list = poller.poll(-1)
            for readied_fd in polled_list:
                
                # If the the stdin has input
                if readied_fd[0] == sys.stdin.fileno():
                    #text = os.read(readied_fd[0], READ_SIZE).decode("ascii").strip()
                    text = os.read(readied_fd[0], READ_SIZE).decode("ascii").strip()
                    
                    if not text:
                        continue
                    #print(f"{text.strip().rstrip()}")
                    if (cmd_processor.verify_command(text) is False):
                        working = False
                        break
                    splitted_text = text.split(" ")
                    if (splitted_text[0] == "UPDATE"):
                        self.message_queue.put(text, block=False)
                        continue

                    #print(f"read: {text}")
                    cmd_processor.execute_command(text)
                    

                    continue

                # If listening socket has new data which means someone is trying to connect
                if readied_fd[0] == listening_socket.fileno():
                    #print("called in listening")
                    (conn, address) = listening_socket.accept()
                    conn.setblocking(False)
                    socket_list.append((conn, address))
                    poller.register(conn.fileno(), select.POLLIN)
                    continue
                
                #print(f"fd is {readied_fd[0]}")
                message = (os.read(readied_fd[0], READ_SIZE)).decode('ascii').strip()

                
                #print(f"reading: {message}")
                if not message:
                    working = False
                    break
                
                # Guarding loop to check validity of packets
                for item in message.split("\n"):
                    item_trim_seq_num = item.split(" ", maxsplit=1)
                    #print(f"item trim seq: {item_trim_seq_num}")
                    if (len(item_trim_seq_num) < 1):
                        continue
                    
                    if (item_trim_seq_num[0] == ''):
                        continue

                    if (item_trim_seq_num[0] == "JOINED"):
                        continue
                    elif (item_trim_seq_num[0] == "RESPONSE"):
                        continue
                    elif (item_trim_seq_num [1] == "SPLIT"):
                        continue
                    elif (item_trim_seq_num[0] == "MERGE"):
                        continue
                    elif ("MERGE" in item_trim_seq_num[1]):
                        continue
                    elif (cmd_processor.verify_update_format(item_trim_seq_num[1]) is False):
                        working = False
                        break

                # if (cmd_processor.verify_update_format(message) is False):
                #     working = False
                #     break
                
                #print(f"receiving {message}")
                for item in message.split("\n"):
                    #print(f"stplited {item}")
                    self.message_queue.put(item, block=False)
                
        
            
            
        listening_socket.close()
        for sock in socket_list:
            sock[0].close()
        #print("closing listening.")
        return
    




class SendingThread(threading.Thread):
    '''
    SendingThread will output into STDOUT and send them via sockets.
    '''
    def __init__(self, graph: NetworkGraph, update_interval: float, routing_thread_queue: Queue[str]):
        threading.Thread.__init__(self)
        self.net_graph = graph
        self.update_interval = update_interval
        self.routing_thread_queue = routing_thread_queue
        self.socket_to_node = {}
  

    def run(self):
        HOST = '127.0.0.1'
        neighbour_dequeue = self.net_graph.get_neighbour_nodes_queue()
        socket_list = []
        socket_queue = Queue()
        socket_join_subthread = SocketJoinThread(socket_queue, neighbour_dequeue, self.net_graph, self.socket_to_node)
        socket_join_subthread.start()
        cmd_processor = CommandProcessor(self.net_graph, None)
        global working
        start_time = time.time()
        previous_update_packet = None
        first_flag = False
        printed_route_flag = False
        
        while (working):
            curr_time = time.time()
            sent = False
            if (socket_queue.empty() is False):
                sock = socket_queue.get(block=False)
                socket_list.append(sock)
                # update_packet_message = cmd_processor.make_update_packet()
                # sock.send(update_packet_message.encode())
                socket_queue.task_done()

            if (curr_time - start_time >= self.update_interval and self.net_graph.root.isAlive() is True):
                start_time = curr_time
                update_packet_message = cmd_processor.make_update_packet()
                #print(f"WHAT ARE THOSE{update_packet_message}")
                

                for s in socket_list:
                    if (self.socket_to_node[s].isAlive() is False):
                        continue

                    #print(f"about to send {update_packet_message} to {self.socket_to_node[s].node_id}")
                    try:
                        s.sendall(update_packet_message.encode())
                    except Exception:
                        passki
                
                if ((first_flag is False or update_packet_message != previous_update_packet) and printed_route_flag is True):
                    first_flag = True
                    previous_update_packet = update_packet_message
                    x = update_packet_message.split(" ", maxsplit=1)
                    print(f"{x[1].strip()}", flush=True)
                

            if (self.routing_thread_queue.empty() is False and self.net_graph.root.isAlive() is True):
                message = self.routing_thread_queue.get(block=False)
                message_split = message.split(" ")
                #print(f"in here {message}")
                if (message_split[0] == "BROADCAST"):
                    broadcast_packet = cmd_processor.make_update_packet()
                    for s in socket_list:
                        if (self.socket_to_node[s].isAlive() is False):
                            continue
                    
                        
                        s.sendall(broadcast_packet.encode())
                        
                
                    if (previous_update_packet != broadcast_packet or first_flag is False):
                            first_flag = True
                            previous_update_packet = broadcast_packet
                            printed_route_flag = True
                            x = broadcast_packet.split(" ", maxsplit=1)
                            print(f"{x[1].strip()}", flush=True)

                elif (message_split[0] == "UPDATE"):
                    #print("in here")
                    
                    CommandProcessor.sequence_number += 1
                    forward_packet = str(CommandProcessor.sequence_number) + " " + message + '\n'
                    #print(f"fowarding {forward_packet}")
                    for s in socket_list:
                        if (self.socket_to_node[s].isAlive() is False):
                            continue
                        try:    
                            s.sendall(forward_packet.encode())
                        except Exception:
                            pass
                elif (message_split[0] == "SPLIT"):
                    split_packet = cmd_processor.make_split_packet()
                    #print("split packet is", split_packet)
                    for s in socket_list:
                        if (self.socket_to_node[s].isAlive() is False):
                            continue
                        try:
                            s.sendall(split_packet.encode())
                        except Exception:
                            pass
                elif(message_split[0] == "MERGE"):
                    merge_packet = cmd_processor.make_merge_packet(message_split[1], message_split[2])
                    #print(f"sending {merge_packet}")
                    for s in socket_list:
                        if (self.socket_to_node[s].isAlive() is False):
                            continue
                        try:
                            s.sendall(merge_packet.encode())
                        except Exception:
                            pass
                elif (message_split[0] == "JOINED" or message_split[0] == "RESPONSE"):
                    #print(message)
                    for s in socket_list:
                        if (self.socket_to_node[s].isAlive() is False):
                            continue
                        try:
                            s.sendall(message.encode())
                        except Exception:
                            pass

                self.routing_thread_queue.task_done()



        #print("in here")
        for sock in socket_list:
            sock.close()

        socket_join_subthread.join()
        #print("closing sending")
        return
        



        

class RoutingCalculationThread(threading.Thread):
    '''
    Thread is responsible for computing the least-cost paths
    At startup (after a delay specified by the RoutingDelay command-line argument).
    
    '''
    def __init__(self, graph: NetworkGraph, message_queue: Queue[str], routing_delay: float, sending_queue: Queue[str]):
        threading.Thread.__init__(self)
        self.net_graph = graph
        self.message_queue = message_queue # Between listening and routing
        self.sending_queue = sending_queue
        self.routing_delay = routing_delay
    
    def run(self):
        time.sleep(self.routing_delay)
        first_time = False
        expected_sequence_number = 0 # For commands like SPLIT, MERGE
        global working
        prev = None
        sequence_number_node_expect: Dict[str, int] = {}
        prev_dict = None
        seen_packets = set()
        cmd_processor = CommandProcessor(self.net_graph, None)
        while (working):
            if (len(seen_packets) == 50):
                # Clears the seen packets to prevent memory overload
                seen_packets = set()


            #print(self.net_graph)
            # if (f"now expecting {expected_sequence_number} and current send seq is {CommandProcessor.sequence_number}" != prev):
            #     prev = f"now expecting {expected_sequence_number} and current send seq is {CommandProcessor.sequence_number}"
                
            #     print(prev)

            # if (str(sequence_number_node_expect) != prev_dict):
            #     prev_dict = str(sequence_number_node_expect)
            #     print(prev_dict)
            #     print(self.net_graph)

            #print("non cloking")
            if (self.message_queue.empty() is False):
                # This is for command received by STDIN and commands like JOINED AND RESPONSE


                packet_msg = self.message_queue.get(block=False)
                if (packet_msg == ""):
                    self.message_queue.task_done()
                    continue
                #print(f"packet recieved from route thread {packet_msg}")
                packet_msg = packet_msg.split(' ')

                
                
                # BELOW IS FROM STDIN OR is jOINED PACKET
                if (packet_msg[0] == "RECALCULATE"):
                    #print('after changed')
                    self.net_graph.link_state_routing_protocol(self.net_graph.root)
                    # print(self.net_graph)
                    self.net_graph.root.print_routing_table()
                    self.sending_queue.put("BROADCAST", block=False)

                elif (packet_msg[0] == "SPLIT"):
                    expected_sequence_number += 1
                    self.net_graph.split()
                   # print(self.net_graph)
                    self.net_graph.link_state_routing_protocol(self.net_graph.root)
                    self.net_graph.root.print_routing_table()
                    self.sending_queue.put("SPLIT", block=False)
                
                elif (packet_msg[0] == "MERGE"):
                    packet_msg_joined = " ".join(packet_msg)
                    if (packet_msg_joined in seen_packets):
                        self.message_queue.task_done()
                        continue

                    seen_packets.add(packet_msg_joined)
                    node_id1 = packet_msg[1]
                    node_id2 = packet_msg[2]
                    expected_sequence_number += 1
                 #   print(f"before merge{self.net_graph}")
                    #self.sending_queue.put(f"MERGE {node_id1} {node_id2}", block=False)
                    self.net_graph.merge(node_id1, node_id2)
                   # print(f"after merge{self.net_graph}")
                    self.net_graph.link_state_routing_protocol(self.net_graph.root)
                    self.net_graph.root.print_routing_table()
                    self.sending_queue.put(f"MERGE {node_id1} {node_id2}", block=False)

                elif (packet_msg[0] == "JOINED"):
                    # joined packet format : JOINED NODE_ID
                    # Response packet format : RESPONSE requesting_node_id UPDATE <Node-ID> <Neighbour1>:<Cost1>:<Port1>,<Neighbour2>:<Cost2>:<Port2>,
                    packet_msg_joined = " ".join(packet_msg)
                    packet_msg_joined = packet_msg_joined + '\n'
                    if (packet_msg_joined in seen_packets):
                        # Don't forward it
                        self.message_queue.task_done()
                        continue
                    seen_packets.add(packet_msg_joined)

                    # Make RESPONSE PACKET for the new node who joine 
                    # Also need to forward the joined packet to other nodes
                    node_just_joined = packet_msg[1]
                    if (node_just_joined == self.net_graph.root.node_id):
                        self.message_queue.task_done()
                        continue

                    #print(f"node just joined {packet_msg}")
                    update_packet = cmd_processor.make_update_packet_without_seq()
                    response_packet = f"RESPONSE {node_just_joined} {update_packet}\n"
                    seen_packets.add(response_packet)
                    #print(f"sending response packet: {response_packet}and forwarding joined packet: {packet_msg_joined}")
                    self.sending_queue.put(packet_msg_joined)
                    self.sending_queue.put(response_packet)

                elif (packet_msg[0] == "RESPONSE"):
                    packet_msg_joined = " ".join(packet_msg)
                    #print(f"got response: {packet_msg_joined}\n")
                    if (packet_msg_joined in seen_packets):
                        # Don't forward it
                        self.message_queue.task_done()
                        continue
                    seen_packets.add(packet_msg_joined)
                    
                    # If we are the requesting node who just joined we process it else we forward it
                    if (packet_msg[1] == self.net_graph.root.node_id):
                        #print("in here")
                        source_node_id = packet_msg[3]
                        update_string = packet_msg[4]
                        update_nodes = update_string.split(',')
                        for update in update_nodes:
                            tokens = update.split(':')
                            # print(tokens)
                            self.net_graph.update_graph(source_node_id, tokens[0], tokens[2], tokens[1])
                        #  print(self.net_graph)

                        if (self.net_graph.changed is True):
                            self.net_graph.changed = False
                            self.net_graph.link_state_routing_protocol(self.net_graph.root)
                            self.net_graph.root.print_routing_table()
                           # print(self.net_graph)
                    else:
                        self.sending_queue.put(packet_msg_joined + '\n')

                else:
                    sequence_number = int(packet_msg[0])
                    #print(f"sequence number of packet is {sequence_number}")
                    #print(f"sequence number of packet is {sequence_number} and {packet_msg}")
                    if (packet_msg[1] == "SPLIT"):
                        if (sequence_number < expected_sequence_number):
                            self.message_queue.task_done()
                            continue

                        expected_sequence_number = int(packet_msg[0]) + 1
                        self.net_graph.split()
                        #print(self.net_graph)
                        self.net_graph.link_state_routing_protocol(self.net_graph.root)
                        self.net_graph.root.print_routing_table()
                        CommandProcessor.sequence_number = expected_sequence_number - 1
                        self.sending_queue.put("SPLIT", block=False)
                        
                    elif (packet_msg[1] == "MERGE"):
                        print(f"packet message merge got is: {packet_msg} and expected seq is {expected_sequence_number}")
                        if (sequence_number < expected_sequence_number):
                            self.message_queue.task_done()
                            continue

                        packet_msg_joined = " ".join(packet_msg)
                        if (packet_msg_joined in seen_packets):
                            self.message_queue.task_done()
                            continue
                        
                        seen_packets.add(packet_msg_joined)
                        #print(f"before merge{self.net_graph}")
                        expected_sequence_number = int(packet_msg[0]) + 1
                        #self.sending_queue.put(f"MERGE {packet_msg[2]} {packet_msg[3]}", block=False)
                        self.net_graph.merge(packet_msg[2], packet_msg[3])
                        #print(f"after merge\n {self.net_graph}")
                        self.net_graph.link_state_routing_protocol(self.net_graph.root)
                        self.net_graph.root.print_routing_table()
                        CommandProcessor.sequence_number = expected_sequence_number -1
                        self.sending_queue.put(f"MERGE {packet_msg[2]} {packet_msg[3]}", block=False)
                        #print("finished merge")
                        #
                    elif (packet_msg[1] == "UPDATE"):
                        source_node_string = packet_msg[2]
                        if (source_node_string not in sequence_number_node_expect):
                            sequence_number_node_expect[source_node_string] = sequence_number
                            #self.sending_queue.put(f"{" ".join(packet_msg[1:])}", block=False)
                        
                        if (sequence_number < sequence_number_node_expect[source_node_string]):
                            self.message_queue.task_done()
                            continue

                        if (len(packet_msg[3:]) < 1):
                            self.message_queue.task_done()
                            continue
                        sequence_number_node_expect[source_node_string] = sequence_number + 1
                        update_nodes = packet_msg[3].split(',')

                        for update in update_nodes:
                            tokens = update.split(':')
                            #print(tokens)
                            if (len(tokens) != 3):
                                continue
                            self.net_graph.update_graph(source_node_string, tokens[0], tokens[2], tokens[1])
                        #  print(self.net_graph)

                        if (self.net_graph.changed is True or first_time is False):
                            #print("in here 2")
                            first_time = True
                            self.net_graph.changed = False
                            self.net_graph.link_state_routing_protocol(self.net_graph.root)
                            self.net_graph.root.print_routing_table()
                            CommandProcessor.sequence_number = expected_sequence_number + 1
                            #print(f"about to forward this: {" ".join(packet_msg[1:])}")
                            #print(self.net_graph)
                            self.sending_queue.put(f"{" ".join(packet_msg[1:])}", block=False)

                    
                self.message_queue.task_done()
            
            if (first_time is False):
                first_time = True
                self.net_graph.link_state_routing_protocol(self.net_graph.root)
                self.net_graph.root.print_routing_table()
                self.sending_queue.put("BROADCAST", block=False)


        #print("closing routing")

