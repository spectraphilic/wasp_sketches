from datetime import datetime
import os
import sys


help_text = """
Makes it easier to read log files. Converts epoch time to human readable
format.

Example:

$ python log_readable.py LOG.TXT
2019-02-22 14:21:00.459000 DEBUG command ls
[...]
"""

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(help_text)
        sys.exit(1)

    filename = sys.argv[1]

    with os.fdopen(sys.stdout.fileno(), 'wb', closefd=False) as stdout:
        for line in open(filename, 'rb'):
            try:
                time, tail = line.split(b' ', 1)
                time = datetime.utcfromtimestamp(float(time))
            except ValueError:
                pass
            else:
                time = time.strftime('%Y-%m-%d %H:%M:%S.%f')
                time = time[:-3] # Display ms, not microseconds
                time = bytes(time, 'ascii')
                line = time + b' ' + tail

            stdout.write(line)
