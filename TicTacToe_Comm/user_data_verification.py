import sys
import json
from server_config_verification import printstd

def validate_user_config(filepath : str):
    valid_keys = ["username", "password"]
    try:
        with open(filepath, 'r') as file_obj:
            database_data = json.load(file_obj)
    except FileNotFoundError:
        printstd("Error: <user database path> doesn't exist.")
        sys.exit()
    except json.decoder.JSONDecodeError:
        printstd("Error: <user database path> is not in a valid JSON format.")
        sys.exit()
    if not isinstance(database_data, list):
        printstd("Error: <user database path> is not a JSON array.")
        sys.exit()
    for record in database_data:
        for key in record:
            if key not in valid_keys:
                printstd("Error: <user database path> contains invalid user record formats.")
                sys.exit()



def main():
    validate_user_config("user_config.json")

if __name__ == "__main__":
    main()