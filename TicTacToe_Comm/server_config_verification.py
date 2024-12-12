import sys
import json
import os.path

def printstd(string):
    sys.stderr.write(string + "\n")


def valid_port(port ):
    if isinstance(port, str):
        printstd("Error: port number out of range")
        sys.exit()
    if not (port >= 1024 and port <= 65535):
        printstd("Error: port number out of range")
        sys.exit()

def validate_server_config(filepath : str) -> tuple[str, str]:
    default_keys = ["port", "userDatabase"]
    try:
        with open(filepath, 'r') as file_obj:
            json_data = json.load(file_obj)
    except FileNotFoundError:
        printstd("Error:<server config path> doesn't exist.")
        sys.exit()
    except json.decoder.JSONDecodeError:
        printstd("Error: <server config path> is not in a valid JSON format.")
        sys.exit()
        
    for key in json_data:
        if len(default_keys) == 0:
            break
        if key in default_keys:
            default_keys.remove(key)
        
        
    if len(default_keys) > 0:
        missing_key_message = ""
        index = 0
        while index < len(default_keys):
            if index == len(default_keys)-1:
                missing_key_message += default_keys[index]
            else:
                missing_key_message += default_keys[index] + ", "
            index += 1
        printstd(f"Error: <server config path> missing key(s): {missing_key_message}")
        sys.exit()

    valid_port(json_data["port"])

    port = json_data["port"]
    userDatabase = os.path.expanduser(json_data["userDatabase"])
    
    return (port, userDatabase)
    


def main():
    validate_server_config("server_config.json")

if __name__ == "__main__":
    main()