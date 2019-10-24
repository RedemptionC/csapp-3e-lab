./sdriver.pl -t trace11.txt -s ./tshref -a "-p"
#
# trace11.txt - Forward SIGINT to every process in foreground process group
#
tsh> ./mysplit 4
Job [1] (1527) terminated by signal 2
tsh> /bin/ps a
  PID TTY      STAT   TIME COMMAND
  574 tty1     Ss     0:00 /init
  575 tty1     S      0:00 sh -c "$VSCODE_WSL_EXT_LOCATION/scripts/wslServer.sh" b37e54c98e1a74ba89e03073e5a3761284e3ffb0 stable .vscode-server 0  
  576 tty1     S      0:00 sh /mnt/c/Users/namei/.vscode/extensions/ms-vscode-remote.remote-wsl-0.39.5/scripts/wslServer.sh b37e54c98e1a74ba89e03073e5a3761284e3ffb0 stable .vscode-server 0
  580 tty1     S      0:00 sh /home/red/.vscode-server/bin/b37e54c98e1a74ba89e03073e5a3761284e3ffb0/server.sh  --port=0 --fileWatcherPolling=0
  582 tty1     Sl     0:16 /home/red/.vscode-server/bin/b37e54c98e1a74ba89e03073e5a3761284e3ffb0/node /home/red/.vscode-server/bin/b37e54c98e1a74ba89e03073e5a3761284e3ffb0/out/vs/server/main.js  --port=0 --fileWatcherPolling=0
  613 tty1     Sl     0:01 /home/red/.vscode-server/bin/b37e54c98e1a74ba89e03073e5a3761284e3ffb0/node /home/red/.vscode-server/bin/b37e54c98e1a74ba89e03073e5a3761284e3ffb0/out/bootstrap-fork --type=watcherService
  621 tty1     Sl     0:31 /home/red/.vscode-server/bin/b37e54c98e1a74ba89e03073e5a3761284e3ffb0/node /home/red/.vscode-server/bin/b37e54c98e1a74ba89e03073e5a3761284e3ffb0/out/bootstrap-fork --type=extensionHost --uriTransformerPath=/home/red/.vscode-server/bin/b37e54c98e1a74ba89e03073e5a3761284e3ffb0/out/vs/server/uriTransformer.js
  638 pts/0    Ss     0:00 /bin/bash
  649 tty1     Sl     0:05 /home/red/.vscode-server/extensions/ms-vscode.cpptools-0.25.1/bin/Microsoft.VSCode.CPP.Extension.linux
  682 tty1     Sl     0:04 /home/red/.vscode-server/extensions/ms-vscode.cpptools-0.25.1/bin/Microsoft.VSCode.CPP.IntelliSense.Msvc.linux 649 0
  741 pts/0    T      0:00 ./myspin 50
 1259 pts/0    T      0:00 ./myspin 40
 1427 pts/0    T      0:00 ./myspin 40
 1522 pts/0    S      0:00 make rtest11
 1523 pts/0    S      0:00 /bin/sh -c ./sdriver.pl -t trace11.txt -s ./tshref -a "-p"
 1524 pts/0    S      0:00 /usr/bin/perl ./sdriver.pl -t trace11.txt -s ./tshref -a -p
 1525 pts/0    S      0:00 ./tshref -p
 1530 pts/0    R      0:00 /bin/ps a
