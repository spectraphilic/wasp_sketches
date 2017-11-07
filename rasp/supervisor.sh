cat << EOF
[program:read_from_xbee]
directory=$PWD
user=$USER
command=$PWD/venv27/bin/python read_from_xbee.py
stderr_logfile=$PWD/log/read_from_xbee.err.log
stdout_logfile=$PWD/log/read_from_xbee.out.log
autostart=true
autorestart=true

[program:send_to_server]
directory=$PWD
user=$USER
command=$PWD/venv27/bin/python send_to_server.py
stderr_logfile=$PWD/log/send_to_server.err.log
stdout_logfile=$PWD/log/send_to_server.out.log
autostart=true
autorestart=true
EOF
