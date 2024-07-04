filename = "e:\Python Projects\L Hours Calculator\hours.txt"


def hourstomin():
    inputus = input("Enter hours minutes with space in between\n").split()
    hour = int(inputus[0])
    minutes = int(inputus[1])
    print(f"You total minutes are {hour*60 + minutes}")


def view_hours():
    with open(filename, "r") as fobj:
        hour = float(fobj.readline().strip())
        hours = hour // 60
        minutes = ((hour / 60) - (hour // 60)) * 60
        print(f"{hours:.0f} hours and {minutes:.0f} minutes")
        fobj.close()
        return


def add_hours():
    fobj = open(filename, "r")
    hour = float(fobj.readline().strip())
    print("Please add minutes!\nEnter 'q' to quit")
    fobj.close()

    fobj = open(filename, "w")
    while True:
        user_input = input()
        if user_input == "q":
            break
        hour += int(user_input)
    fobj.write(str(hour))
    hours = hour // 60
    minutes = ((hour / 60) - (hour // 60)) * 60
    print(f"{hours:.0f} hours and {minutes:.0f} minutes")
    fobj.close()


MESSAGE = (
    "Press the following Numbers\n1. View Hours\n2. Add Hours\n3. Quit\n4. Converter"
)
print(MESSAGE)
while True:
    menu_number = input("")
    if menu_number == "3":
        quit()
    if menu_number == "1":
        view_hours()
    if menu_number == "2":
        add_hours()
    if menu_number == "4":
        hourstomin()
