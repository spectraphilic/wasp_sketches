cat << EOF
[program:read_from_xbee]
directory=$PWD
user=$USER
command=$PWD/venv/bin/python read_from_xbee.py
stderr_logfile=$PWD/log/read_from_xbee.err.log
stdout_logfile=$PWD/log/read_from_xbee.out.log
autostart=true
autorestart=true

[program:archive]
directory=$PWD
user=$USER
command=$PWD/venv/bin/python archive.py
stderr_logfile=$PWD/log/archive.err.log
stdout_logfile=$PWD/log/archive.out.log
autostart=true
autorestart=true

; [program:send_to_server]
; directory=$PWD
; user=$USER
; command=$PWD/venv/bin/python send_to_server.py
; stderr_logfile=$PWD/log/send_to_server.err.log
; stdout_logfile=$PWD/log/send_to_server.out.log
; autostart=true
; autorestart=true
EOF