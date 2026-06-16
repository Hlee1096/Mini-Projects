#!/bin/bash



# bash Routing.sh A 6000 Aconfig.txt 0.5 0.1
# bash Routing.sh B 6005 Bconfig.txt 0.5 0.1
# bash Routing.sh C 6010 Cconfig.txt 0.5 0.1
# bash Routing.sh D 6015 Dconfig.txt 0.5 0.1



# bash Routing.sh H 6000 Hconfig.txt 0.5 0.1
# bash Routing.sh I 6005 Iconfig.txt 0.5 0.1
# bash Routing.sh J 6010 Jconfig.txt 0.5 0.1
# bash Routing.sh K 6015 Kconfig.txt 0.5 0.1
# bash Routing.sh L 6020 Lconfig.txt 0.5 0.1

#!/bin/bash

# gnome-terminal -- bash -c "bash Routing.sh A 6000 Aconfig.txt 0.5 0.1; exec bash"
# gnome-terminal -- bash -c "bash Routing.sh B 6005 Bconfig.txt 0.5 0.1; exec bash"
# gnome-terminal -- bash -c "bash Routing.sh C 6010 Cconfig.txt 0.5 0.1; exec bash"
#gnome-terminal -- bash -c "bash Routing.sh D 6015 Dconfig.txt 0.5 0.1; exec bash"

gnome-terminal -- bash -c "bash Routing.sh H 6000 Hconfig.txt 0.5 0.1; exec bash"
gnome-terminal -- bash -c "bash Routing.sh I 6005 Iconfig.txt 0.5 0.1; exec bash"
gnome-terminal -- bash -c "bash Routing.sh J 6010 Jconfig.txt 0.5 0.1; exec bash"
gnome-terminal -- bash -c "bash Routing.sh K 6015 Kconfig.txt 0.5 0.1; exec bash"
gnome-terminal -- bash -c "bash Routing.sh L 6020 Lconfig.txt 0.5 0.1; exec bash"

# gnome-terminal -- bash -c "bash Routing.sh H 6000 Hconfig.txt 0.5 0.1 > H.log 2>&1; exec bash"
# gnome-terminal -- bash -c "bash Routing.sh I 6005 Iconfig.txt 0.5 0.1 > I.log 2>&1; exec bash"
# gnome-terminal -- bash -c "bash Routing.sh J 6010 Jconfig.txt 0.5 0.1 > J.log 2>&1; exec bash"
# gnome-terminal -- bash -c "bash Routing.sh K 6015 Kconfig.txt 0.5 0.1 > K.log 2>&1; exec bash"
# gnome-terminal -- bash -c "bash Routing.sh L 6020 Lconfig.txt 0.5 0.1 > L.log 2>&1; exec bash"