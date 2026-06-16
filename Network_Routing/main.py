import sys
from queue import Queue
from network_graph import Node, NetworkGraph
import threads




NODE_ID = 0
PORT_NO = 1
CONFIG_FILE = 2
ROUTING_DELAY = 3
UPDATE_INTERVAL = 4


def valid_node_id(node_id: str) -> bool:
    if (len(node_id) != 1):
        return False
    if (node_id.isalpha() is False or node_id.isupper() is False):
        return False
    return True

def valid_port_number(port_num: str) -> bool:
    try:
        int(port_num)
    except ValueError:
        return False
    return True

def config_file_found(config_file_path: str) -> bool:
    try:
        with open(config_file_path) as _:
            return True
    except FileNotFoundError:
        return False
    

def valid_tokens(tokens: list[str]) -> bool:
    try:
        float(tokens[1])
    except ValueError:
        return False
    
    return True
def config_file_validation(config_file_path: str) -> None:
    '''
    The first line contains an integer n (num of neighbours), followed by n lines, each containing 3 tokens: 
    neighbour's Node-ID, the cost (a non-negative floating- point number), and the neighbour's listening port.

    Example:
        4
        A 2.3 6000
        C 3.2 6002
        E 2.8 6004
        J 5.4 6009
    
    '''

    with open(config_file_path, "r") as config_file:
        first_line = config_file.readline().strip()
        try:
            int(first_line)
        except ValueError:
            print("Error: Invalid configuration file format. (First line must be an integer.)")
            sys.exit(1)

        for _ in range (0, int(first_line)):
            line = config_file.readline().strip()
            tokens = line.split(" ")
            if (len(tokens) != 3):
                print("Error: Invalid configuration file format.")
                sys.exit(1)
            if (valid_tokens(tokens) is False):
                print("Error: Invalid configuration file format.(Each neighbour entry must have exactly three tokens; cost must be numeric.)")
                sys.exit(1)

def arguments_validation(argv: list[str]) -> tuple[str, int, str, int, int]:
    '''
    Example of valid arguments ./Routing.sh [F 6005 Fconfig.txt 60 10]
    ./Routing.sh <Node-ID> <Port-NO> <Node-Config-File> <RoutingDelay> <UpdateInterval>
    '''

    if (len(argv) != 5):
        print("Error: Insufficient arguments provided. Usage: ./Routing.sh <Node-ID> <Port-NO> <Node-Config-File>")
        sys.exit(1)

    if (valid_node_id(argv[0]) is False):
        print("Error: Invalid Node-ID.")
        sys.exit(1)

    if (valid_port_number(argv[1]) is False):
        print("Error: Invalid Port number. Must be an integer.")
        sys.exit(1)

    if (config_file_found(argv[2]) is False):
        print("Error: Configuration file missing.txt not found.")
        sys.exit(1)

    return (argv[0], int(argv[1]), argv[2], float(argv[3]), float(argv[4]))



if __name__ == "__main__":

    #argv = ["f",'B', '6005', 'Bconfig.txt', '0.5', '0.1']
    #setup_arguments = arguments_validation(argv[1:])
    #sys.stdout.write(f"{sys.argv}\n")
    setup_arguments = arguments_validation(sys.argv[1:])
    config_file_validation(setup_arguments[CONFIG_FILE])
    

    root_node = Node(setup_arguments[NODE_ID], setup_arguments[PORT_NO])
    net_graph = NetworkGraph()
    net_graph.build_graph_from_file(root_node, setup_arguments[CONFIG_FILE])
    #print(net_graph)
    message_queue = Queue() # listening to routing
    routing_thread_queue = Queue() # routing to sending



    listening_thread = threads.ListeningThread(net_graph, message_queue)
    sending_thread = threads.SendingThread(net_graph, setup_arguments[UPDATE_INTERVAL], routing_thread_queue)
    routing_calculation_thread = threads.RoutingCalculationThread(net_graph, message_queue, setup_arguments[ROUTING_DELAY], routing_thread_queue)

    listening_thread.start()
    sending_thread.start()
    routing_calculation_thread.start()


    
    listening_thread.join()
    sending_thread.join()
    routing_calculation_thread.join()
  
 
    
